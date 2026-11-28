# Decompilation coverage — Elevator Action Returns (maincpu)

**1.5% decompiled · 98.5% remaining**  
`█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░`

| | functions | code bytes | % of code in functions |
|---|---:|---:|---:|
| Decompiled to C | 12 / 1600 | 2,530 / 172,914 | **1.5%** |
| …of which verified bit-exact vs MAME | 12 | 2,530 | 1.5% |
| **Remaining** | 1588 | 170,384 | **98.5%** |

## Map by 64 KiB ROM bank

`█` decompiled · `░` remaining (by code bytes)

| ROM bank | functions (done/total) | code bytes | decompiled | map |
|---|---:|---:|---:|---|
| `0x000000` | 7/301 | 21,582 |   0.7% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x010000` | 0/22 | 1,458 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x0A0000` | 1/166 | 14,656 |   3.1% | `█░░░░░░░░░░░░░░░░░░░` |
| `0x0B0000` | 2/170 | 13,750 |   6.7% | `█░░░░░░░░░░░░░░░░░░░` |
| `0x0C0000` | 1/291 | 36,014 |   1.5% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x0D0000` | 0/16 | 2,290 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x0E0000` | 0/3 | 138 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x120000` | 0/14 | 8,940 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x130000` | 1/147 | 23,800 |   1.9% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x140000` | 0/126 | 14,634 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x150000` | 0/233 | 18,432 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x160000` | 0/71 | 4,800 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x170000` | 0/40 | 12,420 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |

Code bytes = instruction bytes of the 1600 Ghidra functions in maincpu (172,914 of 173,776 disassembled bytes). The function list comes from Ghidra seeded with every PC MAME executed in traced play (`elvactr/work/functions.tsv`). A function counts as decompiled when its entry is annotated `ROM 0x…` in `elvactr/src/`. Regenerate with `make coverage-elvactr`.
