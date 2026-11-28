# Decompilation coverage — Elevator Action Returns (maincpu)

**7.4% decompiled · 92.6% remaining**  
`███░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░`

| | functions | code bytes | % of code in functions |
|---|---:|---:|---:|
| Decompiled to C | 33 / 1600 | 12,738 / 172,914 | **7.4%** |
| …of which verified bit-exact vs MAME | 30 | 12,530 | 7.2% |
| **Remaining** | 1567 | 160,176 | **92.6%** |

## Map by 64 KiB ROM bank

`█` decompiled · `░` remaining (by code bytes)

| ROM bank | functions (done/total) | code bytes | decompiled | map |
|---|---:|---:|---:|---|
| `0x000000` | 9/301 | 21,582 |   1.4% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x010000` | 0/22 | 1,458 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x0A0000` | 3/166 | 14,656 |   8.0% | `██░░░░░░░░░░░░░░░░░░` |
| `0x0B0000` | 5/170 | 13,750 |  16.8% | `███░░░░░░░░░░░░░░░░░` |
| `0x0C0000` | 9/291 | 36,014 |  18.6% | `████░░░░░░░░░░░░░░░░` |
| `0x0D0000` | 1/16 | 2,290 |  17.1% | `███░░░░░░░░░░░░░░░░░` |
| `0x0E0000` | 0/3 | 138 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x120000` | 1/14 | 8,940 |   4.1% | `█░░░░░░░░░░░░░░░░░░░` |
| `0x130000` | 4/147 | 23,800 |   4.8% | `█░░░░░░░░░░░░░░░░░░░` |
| `0x140000` | 0/126 | 14,634 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x150000` | 1/233 | 18,432 |   1.9% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x160000` | 0/71 | 4,800 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |
| `0x170000` | 0/40 | 12,420 |   0.0% | `░░░░░░░░░░░░░░░░░░░░` |

Code bytes = instruction bytes of the 1600 Ghidra functions in maincpu (172,914 of 173,776 disassembled bytes). The function list comes from Ghidra seeded with every PC MAME executed in traced play (`elvactr/work/functions.tsv`). A function counts as decompiled when its entry is annotated `ROM 0x…` in `elvactr/src/`. Regenerate with `make coverage-elvactr`.
