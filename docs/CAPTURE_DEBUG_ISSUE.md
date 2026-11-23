# Captures bajo `-debug`: lo que hay que saber (2026-11-22)

## Síntoma

`scripts/capture_obj.sh`, `capture_batch.sh` y `capture_fn.sh` devuelven
`CAPTURE PARTIAL pairs=0`. Las capturas nuevas salen vacías.

## Lo que sí funciona

- El savestate carga (`-state 03_active_combat`, etc.).
- El juego ejecuta código y **avanza**: el contador de frame de juego
  (`0x4022AC`, en wram `0x22AC`) sube exactamente 1 por frame de pantalla, como
  al arrancar en frío. ~982 bytes de Work RAM cambian entre dos muestras
  separadas 1400 callbacks.
- `cpu.debug:bpset()` funciona: los puntos de ruptura disparan en las
  direcciones que se ejecutan (`0x1A34`, `0x1D96..0x1D9E`, `0x2320`,
  `0x104EE`, `0x104F0`, `0x104F4`, `0x1050E`, `0x105EC`, `0x1A76`).
- Con un punto de ruptura puesto, el bucle principal corre a **1 iteración por
  frame** (medido: 60 iteraciones / 60 frames).

## Lo que no funciona

Bajo `-debug -debugger none` con un savestate:

- El PC se queda en el stub de vblank (`0x1A34` / `0x1D96..0x1D9E`) y en el
  bucle de espera `0x1A76`. Las rutinas de objetos **nunca se ejecutan**:
  `0x128E44`, `0x191700`, los 6 puntos de la pool que miramos, todos a 0.
- Los `jsr`/`jsr (A1)` que sí disparan (`0x104EE`) se ejecutan, pero `A1`
  contiene texto de ROM (`0x00119CBA`, `0x0C6D12C0`...), no una dirección de
  función: el BLOQUE de la lista de objetos se está interpretando como datos.
- Progreso real: ~7 frames por minuto (medido con `-nothrottle`), así que
  incluso con un punto de ruptura limpio una captura de 6000 frames "tarda"
  días.

## Diagnóstico

No es MAME 0.289 (el API `manager.machine.debugger` existe y responde), ni los
savestates (cargan y el juego corre). Es la combinación `-debug` + estos
savestates: el flujo de control se corrompe en el punto donde el juego debería
entrar en la etapa de objetos.

Las capturas doradas existentes (`work/ram/obj_*_pairs.bin`, 1178 juegos) son
de antes de esta regresión y siguen siendo válidas para verificar traducciones.
Lo que no se puede hoy es **capturar rutinas nuevas**.

## Para desbloquear

1. Comprobar si hay un savestate en el que el juego entre en la etapa de
   objetos bajo `-debug` (probado: `01_attract`, `02_stage1_start`,
   `03_active_combat`, `run1_040000` — ninguno).
2. Si no, regenerar los savestates desde arranque en frío con la versión actual
   de MAME (`scripts/lua/survey_obj.lua` los produce).
3. Mientras tanto, el planner solo puede aceptar rutinas cuyo capture ya exista
   y que ejecuten los caminos observados; las que necesiten capturas nuevas
   quedan bloqueadas con la razón en `work/batches/state.tsv`.

---

## Actualización 2026-11-23 (sesión de continuación)

### Lo que sí funciona ahora

- **Los savestates `run1_*` sí cargan** y sí ejecutan objetos (`run1_120000` tiene
  11B472 vivo en 10+ slots del grupo 5). La lista de frames del `run1.tsv` es la
  del *run original*, no la del savestate: para saber qué hay vivo en un
  savestate hay que leer el pool directamente (`0x410000`/`0x410060`).
- `capture_fn.sh ... <entry> ret` con un savestate vivo funciona, pero **el
  "post" se toma un frame después del tick**: el retorno va por `trap`, no por
  `rts`, así que `F3_EXIT=ret` nunca dispara y el harness compara el estado del
  frame siguiente. Eso mezcla el RNG del sistema y hay que restar 1 al
  contador manualmente. Para capturas limpias de rutinas llamadas por
  `trap` hace falta un hook de retorno por SP (script propio), no `ret`.
- `03_active_combat.sta` está **muerto** (carga como arranque en frío: 2
  funciones por frame). Guardado fuera del árbol.
- La carrera desde `02_stage1_start` con invulnerabilidad
  (`F3_POKE32=41B1E0+0x24=0`) avanza entre stages en ~5 000 frames cada uno
  y cubre las zonas 0–5 repetidamente. 200 000 frames no llegan a zonas 6+.

### Lo que sigue sin poder capturarse

- `17F74E`: no aparece en ninguna savestate disponible ni en 200 000 frames
  de carrera invulnerable. Pertenece a un jefe de zona avanzada.
- `11B472`: capturado en `run1_120000` (5 pares, rama de muerte). El tick
  normal necesita un frame distinto del grupo 5.

---

## Obj_DrawSprite (0x001282): dos acusaciones, ambas falsas (2026-11-23)

Un worker de oleada F sostuvo que `0x001282` tenía dos defectos. Ambas
acusaciones se probaron contra la ROM y el harness y **no se sostienen**:

1. **"Escribe `(d2w>>4)` donde la ROM hace `bfextu D2{16:12}`"** — cierto en el
   byte, pero `d2w>>4` es el equivalente correcto: D2 es un long, el
   `move.w (0x18,A0),D2` sólo pisa la palabra baja y el `lsl.w #4` desplaza
   esa palabra. `bfextu {16:12}` extrae los bits 16..27, que con la palabra
   alta intacta son exactamente `y<<4 >> 8 = y>>4`. Aplicar `>>8` **rompe 147
   rutinas** (1228 → 1082 bit-exact) en un worktree aislado: la versión actual
   es la correcta.

2. **"No emite w*h entradas para defs multi-glifo"** — el bucle anidado de
   `sprite.c` recorre `cols` × `rows` exactamente como `dbf` en ROM
   (`0x13C4 51C9` interior, `0x13C0`/`0x13C6` exterior). El caso de 11939E
   tiene `obj+0x10 = 0x62E54`, y `0x620000..0x62FFFF` **es GFX RAM**, no una
   def de sprite: el worker pasó un juego de glyphs de texto a la ruta de
   sprites. La rutina falla por su propio camino de texto (0x59304), no por el
   dibujante.

Lección para futuros workers: antes de acusar a un helper compartido, medir el
delta con el harness completo en un worktree aislado. Un cambio que baja el
recuento global es evidencia en contra, no una mejora pendiente de arreglar
más rutinas.

## Obj_Free (0x00105EC): acusación falsa (2026-11-23)

Un worker de la oleada G afirmó que `Obj_Free` calcula un offset de bit
~1600 para el grupo 5 y toca la máscara equivocada. Falso: la ROM es
`move.b (4,A0),D0; lsl.w #5,D0; add.b (5,A0),D0; bfset`, y `src/objects/
obj_pool.c` lo replica exactamente (el `add.b` sólo afecta al byte bajo).
Grupo 5, índice 3 → bit **163** = `5*32+3`, que es el correcto. Para obtener
~1600 haría falta un segundo `<<5` que no existe ni en la ROM ni en la C.

Tercera acusación de esta sesión contra un helper compartido que resulta ser
falsa (las otras dos: `Obj_DrawSprite`, `Obj_MoveByAngle`). Patrón: cuando una
traducción no cuadra, el worker culpa al helper. Antes de aceptarlo: leer la
ROM cruda y, si hay un cambio candidato, medirlo con el harness completo en un
worktree aislado. Un delta negativo en el recuento global zanja la discusión.

## Investigación del "post desfasado un frame": falso (2026-11-23)

Hipótesis: `F3_EXIT=ret` toma el post en la dirección guardada en `[SP]`, que
para un tick despachado por el pool es `0x000104F0` (el despachador), así que
el post llegaría un frame tarde y contaminaría RNG y estado.

**Medido y refutado.** Instrumenté el post de `capture_fn.lua` para imprimir
`PC`/`SP` en el punto de captura, con 11B472 en `run1_120000`:

```
POST pc=0104F0 sp=405294 ret_sp=405290
```

`SP` es ya exactamente `entry_SP + 4`: el tick **sí** ha retornado; el `PC`
del despachador es sólo dónde se detiene el emulador. La captura que ya
teníamos y una captura nueva tomada con esta instrumentación dan **los mismos
25 bytes de diferencia y el mismo RNG intacto**.

Sí existía un cambio real en el pipeline (se descartó, ver abajo), pero no
explicaba nada: la mejor forma de comprobarlo fue comprobar que la captura
alternativa produce el mismo post.

### Lo que sí se aprendió sobre 11B472

Con la captura correcta, la parte de movimiento de la traducción es **exacta**:
`+0x42 = 7` (velocidad), ángulo `0x1A`, y `7 * tabla[0x1A]` da `dx=92113`,
`dy=68313`, que son exactamente los deltas del post real (diferencia 0,0).
Quedan dos divergencias ajenas al movimiento: el byte bajo de `+0x20` y una
escritura en `+0x00` (`0x11B472 → 0x11B472` con un byte intermedio distinto),
más el RNG de sistema. Ninguna se explica por el pipeline de captura.

Conclusión: **no hay nada que arreglar en la captura**; 11B472 necesita
trabajo de traducción sobre su rama de re-arme.

## Capturas "imposibles" que no lo eran: 115B3E (2026-11-23)

Dos oleadas seguidas degradaron `obj_115B3E` a stub por "captura incoherente":
alegaban que las posiciones del par exigían un `|P| ≈ 4.8` que ningún ángulo
producía, y que la secuencia de ángulo `0x92→0x94→…` no era lo que devuelve
`Obj_Atan2`.

**La captura era correcta.** Tres errores de lectura, ninguno de la captura:

1. **El speed no es `(0x52,A0)`.** Es la **word `(0x42,A0)`**, que vale
   `0x0014 = 20`. Con speed 20 y ángulo `0x92`, `20*tab[0x92] = -296200` y
   `20*tab[0x80+0x92] = -140100`: exactamente los deltas del post.
2. **El ángulo es un BYTE**, el byte **bajo** de la word `+0x20`, no el alto.
   Pasar `rd16(obj+0x20) >> 8` da `0x90`; el correcto es `& 0xFF` → `0x92`.
   (El offset es un byte, no un ángulo 16.16.)
3. **El signo del giro** se decide con `sub.w (0x20,A0),D0w` — resta **word**,
   y sólo se prueba el byte bajo. Comparar contra `rd8(obj+0x20)` (el byte
   alto) invierte el signo y resta 2 donde toca sumar.

Regla para futuros intentos: cuando una captura parezca imposible, antes de
declararla basura, deduce (speed, ángulo) del propio post con la tabla de senos
y compáralo con los campos candidatos. Nueve de cada diez veces el campo real
es otro del que se leyó, o es el byte bajo en vez del alto.

## capture.h: acusación falsa (2026-11-23)

Un worker de la oleada W sostuvo que `tests/capture.h` lee el bloque de
registros con un offset equivocado (A0 en `0x22`/A5 en `0x35`) y que por eso
las rutinas de su lote daban 0/60. Falso, y comprobado con el caso que pasa:

- `capture.h` salta **3** campos (`pair`, `which`, `frame`) y lee **16**
  registros (`CR_N = 16`: D0–D7, A0–A6, SP) desde el campo 4 del TSV.
- En `obj_115B3E_regs.tsv` el par 0 tiene 19 campos; `regs[8]` (que es
  `CR_A0 = 8`) vale `0041444C`, exactamente el A0 del objeto.
- Con ese mapeo, `Obj 0x115B3E` da **60/60**. Si el offset estuviera mal, no
  podría pasar ninguna rutina.

Séptima acusación de la sesión contra infraestructura compartida que resulta
falsa. El patrón se mantiene: cuando una traducción no cuadra, se culpa al
harness/helper. Verificación mínima antes de aceptarlo: ejecutar una rutina
que YA pasa con el mismo camino de código. Si pasa, el código compartido está
bien y el problema es la traducción o la captura concreta.

## El pipeline de captura está correcto (verificado en la ROM, 2026-11-23)

Varios workers de las oleadas P–X culparon al harness del "post un frame tarde".
Verificado contra `work/maincpu.bin`:

```
0x104E8  ...         ; bucle del pool
0x104EE  4E 91       ; jsr (A1)   <- llama al tick del objeto
0x104F0  4C DF 03 07 ; movem.l (SP)+,{D0,D1,D2,A0}   <- retorno
```

`[SP]` al entrar al tick es **`0x0104F0`**, y `F3_EXIT=ret` pone el post
exactamente ahí: la primera instrucción **después** del tick. Es la elección
correcta. No hay `trap` en el camino: el tick vuelve con `rts`.

Además, medido con instrumentación sobre `11B472` en `run1_120000`:

```
POST pc=0104F0 sp=405294 ret_sp=405290
```

`SP` ya vale `entry_SP + 4`: el tick ha retornado de verdad.

### Conclusión práctica

Las capturas que fallan lo hacen por su **contenido** (estado del pool no
coherente, objeto que no es el del tick, contador entre expiraciones), no por
el timing del post. Sesenta parejas de una rutina que da 0/60, con todos los
helpers de la sesión verificados contra ROM, es señal de captura mala — y la
única solución es recapturar, que hoy no es posible con los savestates
disponibles (el `-debug` de MAME no entra en la fase de objetos salvo con
`run1_*`, y esos solo cubren parte del juego).

Esto se comprobó siete veces esta sesión con acusaciones contra
`Obj_DrawSprite` (×2), `Obj_MoveByAngle`, `Obj_Free`, `Obj_Spawn` (×2) y
`capture.h`: **todas falsas**.

## El corpus de capturas está SANO — auditoría completa (2026-11-23)

Duda abierta toda la sesión: ¿están rotas las capturas? **No.**

Auditoría de las **1169** capturas `work/ram/obj_*_pairs.bin`: para cada par,
comprobar que el objeto en `A0` tiene en `+0x00` el puntero a la propia rutina
(lo que el despachador del pool garantiza al entrar):

```
capturas con pares: 1169   COHERENTES: 1169   con pares malos: 0
```

Todas correctas, incluidas las que las oleadas P–X declararon imposibles
(1270BE, 118B74, 10B93C: `fn` coincide en el par 0).

### La lección: casi todos los «captura rota» eran errores de lectura

`118B74` había sido descartada dos veces. Su captura es correcta y su
traducción quedó 60/60 en cuanto se corrigió **un solo cálculo**:

```
0x118B74  43FA 0030   lea (0x30,PC),A1   ->  0x118B74 + 2 + 0x30 = 0x118BA6
```

Usé `0x118BAC` (PC + 0x30, olvidando el +2 del PC relativo del 68k). La tabla
real empieza en `0x118BA6` y su entrada 0 es `{dur=3, sprdef=0x0005AD34}`,
exactamente el `+0x10` del post capturado. Con la base mala, el primer paso
leía una entrada distinta y todo lo demás divergía.

**Regla que sale de aquí:** antes de declarar una captura rota, verificar que
el `fn` en `A0` coincide con la rutina (arriba, 1 línea de Python). Si coincide,
la captura está bien y el error está en la traducción; el sospechoso número uno
es `(d16,PC)`, donde la base es **PC + 2**, no PC.
