# Referencia de Hardware Taito F3 (Derivada de MAME `taito_f3.cpp`)

Referencia técnica rápida para configurar Ghidra (`68000:BE:32:MC68020`) y ejecutar trazas con `astrobleem/mame-mcp`.

---

## 1. Especificaciones Base del Sistema

- **CPU Principal (`:maincpu`):** Motorola **MC68EC020** @ 16 MHz (`F3_MAIN_CLK`), bus de datos de 32 bits (Big-Endian), bus de direcciones de **24 bits** (`0x000000`–`0xFFFFFF`).
- **Procesador Ghidra:** `68000:BE:32:MC68020` (confirmado en Ghidra 12.1.4).
- **Video:** 320×232 visible, ~58.94 Hz (`26.686 MHz / 4 / 432 / 262`), paleta de 24 bits (`0x2000` = 8192 entradas).
- **Almacenamiento persistente:** EEPROM 93C46 (16-bit).
- **Watchdog:** Reset vía escritura en `0x4A0000`.
- **Subsistema de Sonido (`:taito_en`):** Motorola 68000 (`:taito_en:audiocpu`, ROM `0x140000`) + Ensoniq ES5505 / ES5510 (`:taito_en:ensoniq`), acoplado mediante DPRAM MB8421 de 2 KB.

---

## 2. Mapa de Memoria Principal (`taito_f3_state::f3_map`)

| Rango Hex (`24-bit`) | Rango Decimal (para JSON `mame-mcp`) | Tamaño | Acceso | Handler MAME / Función |
|---|---|---|---|---|
| `0x000000` – `0x1FFFFF` | `0` – `2097151` | 2 MiB | R | **Main Program ROM** (`:maincpu`). Vectores M68K en `0x000000..0x0000FF`. |
| `0x300000` – `0x30007F` | `3145728` – `3145855` | 128 B | W | `sound_bankswitch_w` (usado por juegos específicos como `kirameki`). |
| `0x400000` – `0x41FFFF` | `4194304` – `4325375` | 128 KiB | R/W | **Work RAM** principal (con mirror en `0x420000`–`0x43FFFF`). |
| `0x440000` – `0x447FFF` | `4456448` – `4489215` | 32 KiB | R/W | **Palette RAM** (`palette_24bit_w`, 8192 colores × 32-bit). |
| `0x4A0000` – `0x4A001F` | `4849664` – `4849695` | 32 B | R/W | **F3 Control / I/O** (`f3_control_r` / `f3_control_w`): inputs, watchdog, coins, EEPROM. |
| `0x4C0000` – `0x4C0003` | `4980736` – `4980739` | 4 B | W | **Timer Control** (`f3_timer_control_w`): configuración de pseudo-HBlank IRQ5. |
| `0x600000` – `0x60FFFF` | `6291456` – `6356991` | 64 KiB | R/W | **Sprite RAM** (`spriteram_r/w`). |
| `0x610000` – `0x61BFFF` | `6356992` – `6406143` | 48 KiB | R/W | **Playfield RAM** (`pf_ram_r/w`, capas de tilemap). |
| `0x61C000` – `0x61DFFF` | `6406144` – `6414335` | 8 KiB | R/W | **Text RAM** (`textram_r/w`, capa de texto/HUD). |
| `0x61E000` – `0x61FFFF` | `6414336` – `6422527` | 8 KiB | R/W | **Char RAM** (`charram_r/w`, caracteres dinámicos en RAM). |
| `0x620000` – `0x62FFFF` | `6422528` – `6488063` | 64 KiB | R/W | **Line RAM** (`lineram_r/w`, efectos raster por línea, scroll/zoom/alpha). |
| `0x630000` – `0x63FFFF` | `6488064` – `6553599` | 64 KiB | R/W | **Pivot RAM** (`pivot_r/w`, capa pivot / rotación). |
| `0x660000` – `0x66000F` | `6684672` – `6684687` | 16 B | W | **Video Control 0** (`control_0_w`, scroll/control de capas). |
| `0x660010` – `0x66001F` | `6684688` – `6684703` | 16 B | W | **Video Control 1** (`control_1_w`, prioridades/blend/clipping). |
| `0xC00000` – `0xC007FF` | `12582912` – `12584959` | 2 KiB | R/W | **Sound DPRAM** (`taito_en:dpram`, MB8421 puerto izquierdo). |
| `0xC80000` – `0xC80003` | `13107200` – `13107203` | 4 B | W | `sound_reset_0_w` (`CLEAR_LINE` del reset de la CPU de sonido). |
| `0xC80100` – `0xC80103` | `13107456` – `13107459` | 4 B | W | `sound_reset_1_w` (`ASSERT_LINE` del reset de la CPU de sonido). |

### Detalle de `F3_CTRL_IO` (`0x4A0000` – `0x4A001F`, accesos de 32 bits)

- **Lectura (`f3_control_r`, offsets 0..5):**
  - `0x4A0000` (`offset 0`): `IN.0` (MSW: Test, Coins, EEPROM read; LSW: P1/P2 buttons, Start, Tilt, Service).
  - `0x4A0004`..`0x4A0017` (`offsets 1..5`): `IN.1`..`IN.5` (Joysticks, jugadores 3/4, DIPs/controles según juego).
- **Escritura (`f3_control_w`):**
  - `0x4A0000` (`offset 0`): **Watchdog reset**.
  - `0x4A0004` (`offset 1`, bits 24–31): **Coin lockouts (P1/P2)** y **Coin counters (P1/P2)**.
  - `0x4A0010` (`offset 4`, bits 0–7): **EEPROM 93C46 output** (CS/CLK/DI).
  - `0x4A0014` (`offset 5`, bits 24–31): **Coin lockouts / counters (P3/P4)**.

---

## 3. Vectores de Interrupción Clave (MC68EC020)

| Dirección | Vector # | Nombre | Comportamiento en Taito F3 |
|---|---|---|---|
| `0x000000` | 0 | Initial SSP | Puntero de pila supervisor inicial (apunta a `0x400000..0x41FFFF`). |
| `0x000004` | 1 | Initial PC | Punto de entrada `Reset` / POST del juego. |
| `0x000068` | 26 | Autovector Level 2 | **VBlank IRQ (`interrupt2`)** — disparada cada frame (~58.94 Hz). Programa además el timer para IRQ3 (+10,000 ciclos). |
| `0x00006C` | 27 | Autovector Level 3 | **Video Sync IRQ (`trigger_int3`)** — señal de fin aproximado de VBlank esperada por el handler de VBlank. |
| `0x000074` | 29 | Autovector Level 5 | **Timer / Pseudo-HBlank IRQ** — usada por títulos que configuran `0x4C0000`. |
| `0x000080`–`0x0000BC` | 32–47 | `TRAP #0`..`#15` | Revisar siempre si el juego los emplea como llamadas de servicio internas. |

---

## 4. Perfiles Listos para `trace_memory_access` (`mame-mcp`)

### Perfil A — I/O, Watchdog, EEPROM, Timer y Audio DPRAM
```json
{
  "system": "<SHORT_NAME>",
  "rompath": "/PROCESOS/Taitof3Decomp/roms",
  "frames": 600,
  "ranges": [
    { "name": "f3_ctrl_io",   "start": 4849664,  "end": 4849695 },
    { "name": "f3_timer",     "start": 4980736,  "end": 4980739 },
    { "name": "sound_dpram",  "start": 12582912, "end": 12584959 },
    { "name": "sound_reset",  "start": 13107200, "end": 13107459 }
  ],
  "injectPreset": "none"
}
```

### Perfil B — Registros de Video, Paleta y Capas Gráficas
```json
{
  "system": "<SHORT_NAME>",
  "rompath": "/PROCESOS/Taitof3Decomp/roms",
  "frames": 300,
  "ranges": [
    { "name": "video_ctrl",  "start": 6684672, "end": 6684703 },
    { "name": "palette_ram", "start": 4456448, "end": 4489215 },
    { "name": "sprite_ram",  "start": 6291456, "end": 6356991 },
    { "name": "pf_ram",      "start": 6356992, "end": 6406143 },
    { "name": "text_ram",    "start": 6406144, "end": 6414335 },
    { "name": "line_ram",    "start": 6422528, "end": 6488063 },
    { "name": "pivot_ram",   "start": 6488064, "end": 6553599 }
  ],
  "injectPreset": "none"
}
```

### Snippet Lua para volcar `:maincpu` desentrelazada vía `run_lua_inline`
```lua
local region = manager.machine.memory.regions[":maincpu"]
assert(region, "Region :maincpu no encontrada")
local f = assert(io.open("/PROCESOS/Taitof3Decomp/work/maincpu.bin", "wb"))
local size = region.size
local chunk = 65536
for off = 0, size - 1, chunk do
    local len = math.min(chunk, size - off)
    f:write(region:read_range(off, off + len - 1, 8))
end
f:close()
print(string.format("OK: dumped :maincpu (%d bytes)", size))
```
*(Nota: validar la firma exacta de `region:read_range` o iteración por `read_u8`/`read_u32` según la versión instalada de MAME al ejecutar la Fase 1).*
