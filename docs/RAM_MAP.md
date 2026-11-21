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

TODO (Phase 3): game state, entity pool, player, score — via RAM diffs across savestates.
