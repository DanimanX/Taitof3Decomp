# Progress — dariusg (Darius Gaiden, Taito F3)

Status legend: `TODO` / `MAPPED` (named, purpose known) / `DECOMPILED` (C in `src/`) / `VERIFIED` (C checked against an oracle).

## Phase gates

| Phase | Gate | Status | Evidence |
|---|---|---|---|
| 0 | mame-mcp + pyghidra-mcp wired in Hermes | DONE | `~/.hermes/config.yaml` |
| 1 | `work/maincpu.bin` reproducible | DONE | `scripts/reconstruct_rom.py`: MAME dump == offline rebuild (2 MiB, md5 27be2aa1…) |
| 1 | Vector table coherent | DONE | SSP=0x41FFFC, Reset=0x010000, IRQ2/3/5=0x1A34/0x1A54/0x1A6C, TRAP1..12 used, rest → 0x400 |
| 2 | Ghidra bootstrap with HW map | DONE (scratch project) | `scripts/ghidra/bootstrap.sh`: 561 fn, XREFs to CTRL_IO/SPRITE/PF/DPRAM; A5=0x408000 context |
| 3 | MMIO sweep A (I/O + audio) | DONE | `traces/mmio/sweepA_io_audio.log` (590 unique, 1200 frames attract) |
| 3 | MMIO sweep B (video) | TODO | |
| 3 | Savestates `states/0x_*.sta` | TODO | |
| 3 | `docs/RAM_MAP.md` | STARTED | `symbols/dariusg.tsv` (kernel/io vars) |

## Functions

| ROM | Name | Subsystem | Status | Notes |
|---|---|---|---|---|
| 0x010000 | Reset | boot | MAPPED | HW init, `lea $408000,A5`, → 0x402 POST |
| 0x000402 | (POST) | boot | TODO | LINE_RAM/RAM test loop, error screen |
| 0x001DB8 | Kernel_ColdBoot | kernel | MAPPED | spawns Task0_Main @0x10172 |
| 0x001A34 | IRQ2_VBlank | kernel | MAPPED | handshake with IRQ3, then Kernel_Preempt |
| 0x001A54 | IRQ3_VideoSync | kernel | MAPPED | |
| 0x001A6C | IRQ5_Timer | kernel | MAPPED | irq5_counter++ |
| 0x00222E | Kernel_Preempt | kernel | MAPPED | see docs/KERNEL.md |
| 0x002274 | Kernel_Schedule | kernel | MAPPED | 16 tasks, status bits 31/30/29 |
| 0x001E60.. | Sys_* (TRAP 1..12) | kernel | MAPPED | spawn/kill/exit/sleep/yield/recv/wait/block/send/signal/wake/flush |
| 0x001A0E | Kernel_FrameTick | kernel | MAPPED | calls 0x1C90, 0x1A78, 0xDFA, 0x161E, 0xBF6 |
| 0x001D8A | Rng_Next | kernel | VERIFIED (MAME) | `src/kernel/rng.c`; 600 frame pairs of rng_seed from MAME all reachable (avg 2385 idle-loop calls/frame) |
| 0x001A78 | IO_PollInputs | io | MAPPED | |
| 0x001B8E | IO_EdgeDetect | io | VERIFIED (truth table) | `src/io/input.c` |
| 0x001C90 | Video_UploadScroll | video | MAPPED | |
| 0x0047E8.. / 0x00092A.. | EEPROM 93C46 bit-bang | io | TODO | from sweep A |
| 0x00396C..0x003CDA | Sound DPRAM interface | audio | TODO | clear 0x397A, cmd slots C00000-34 (0x3B58), handshake C00480/C007F8 |

## Task entry points (`scripts/find_spawns.py`)

010172 (task0), 010488, 010502, 0910CC, 0AC8E4, 0B935A, 0C63E8, 0C6A64, 13F2B6, 13F2C6, 13F436, 1439BC, 144B60, 1783A8, 1A1A26 — TODO name each (scene/state machine, Phase 4 priority 6).

## Known caveats
- Auto-analysis made scalar constants into refs to SOUND_BANK/PIVOT_RAM/WORK_RAM_MIRROR (1 ref per function) — likely false XREFs, review before trusting.
- The repo's `ghidra_proj/taito_f3` is locked by Hermes' pyghidra-mcp while the gateway runs; bootstrap there once it is stopped.
