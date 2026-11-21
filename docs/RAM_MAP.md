# Work RAM map — dariusg

Canonical machine-readable source: [`symbols/dariusg.tsv`](../symbols/dariusg.tsv) (applied to Ghidra by `scripts/ghidra/apply_symbols.py`). This page summarises layout.

Global base register **A5 = 0x408000** (set once in Reset at 0x010004); most globals are `(-off,A5)`.

| Range | Content |
|---|---|
| 0x40221C–0x402235 | input channels `io_ch[6]` (active-low cur/pressed/prev/released) + coin output shadows |
| 0x402242–0x4022A9 | scroll shadows (`pf_scroll_xy[4]`, ext scroll, flip, base table) uploaded each frame to 0x660000 |
| 0x4022AC–0x4022B3 | frame/IRQ5 counters, IRQ2↔IRQ3 handshake flags (volatile) |
| 0x4022B4–0x4062B3 | 16 × 1 KiB task stacks |
| 0x4064B4 | kernel temporary SP top (overrun path) |
| 0x4066B4–0x406BAE | kernel tables: saved SPs, task status/wakeup/mailboxes, RNG seed |
| 0x41FFFC | initial SSP (vector 0) |

| 0x408000 (A5) .. | game globals: game flags, sound mute, EEPROM image 0x406C54, collision list cursors 0x4088FE.., terrain bitmap 0x40891C (0x1400 B) |
| 0x410000–0x4194DF | object pool: alloc bitmap + 11 groups × 32 × 0x6C `EntityObj` |
| 0x41A980–0x41ADxx | collision lists: bomb dmg[2], shootables 0x41A988, player shots 0x41ABE0, hazards 0x41ACE0 |
| 0x41B190 / 0x41B21A | `players[2]` (0x8A each): pos_x/y 16.16 at +0x14/+0x18, angle +0x20, hp +0x24, score +0x48 — see docs/PLAYER.md |

## How these were found (reproducible)
- `scripts/lua/session_ramdiff.lua` → savestates `states/dariusg/0{1,2,3}_*.sta` + checkpoint dumps.
- `scripts/lua/session_corr.lua` + `scripts/ramcorr.py`: per-frame RAM vs input schedule correlation.
- `scripts/lua/write_taps.lua`: writer PC + A-registers for watched addresses.
- Score located by matching on-screen values (snapshots) against RAM encodings.
Caveat: words inside task stacks (0x4022B4–0x4062B3) correlate with inputs because tasks keep live values in registers across yields; resolve them via `task_saved_sp` before trusting them.
