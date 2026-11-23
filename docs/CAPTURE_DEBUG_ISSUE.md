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
