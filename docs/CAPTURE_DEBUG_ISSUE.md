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
