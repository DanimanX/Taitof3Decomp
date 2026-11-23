# PC-indexed table bases (`movea.l (bd,PC,Xn),An`)

Three workers hit "unresolvable `A16`" in `0x1078F0` / `0x107CD0` / `0x107F08`
and stopped. There is no mystery register: `A16` is the 68020 **program
counter**, and the base rule is unambiguous once it is checked against a
routine that already passes.

## The rule

For a brief-format `movea.l (bd,PC,Xn),An` (`22 7B` + one extension word) at
address `P`:

    base = (P + 2) + bd

`P + 2` is the address of the extension word, i.e. the PC value the 68020
uses. `bd` is the signed displacement: a 16-bit word (or 8-bit byte when the
brief extension says so).

**Verified** on `obj_107112` (already bit-exact): `4E BB 05 21 005A` at
`0x1071B6` → `0x1071B6 + 2 + 0x5A = 0x107212`, whose four entries are
`0x107222 / 0x10725E / 0x107378 / 0x1073A6` — exactly the handlers that
routine dispatches to.

## Worked example: 0x1078F0

    P = 0x1078F0, opcode 22 7B, ext 0520, bd = 0x0120
    base = 0x1078F2 + 0x120 = 0x107A12

Table at `0x107A12` (4-byte pointers):

| d0 | value | meaning |
|---:|---|---|
| 0 | `0x00000000` | **NULL** — mode 0 has no handler |
| 1 | `0x001061DE` | mode 1 |
| 2 | `0x001062A6` | mode 2 |
| 3 | `0x00106390` | mode 3 |
| 4 | `0x001063D0` | mode 4 |
| 5 | `0x001064C6` | mode 5 |
| 6 | `0x00106566` | mode 6 |
| 7 | `0x00000000` | **NULL** |
| 8..13 | `0x1061E6`, `0x1062BE`, ... | modes 1..6 with the `+7` variant |

`d0` is `(A4+0x6)` (the parent's mode, 0..6) plus 7 when bit 7 of
`(A4+0x4E)` is set. Each handler then indexes an 8-byte `{w, w, l}` record by
the byte at `(A4+0x22)`.

The NULLs are real, not a decode error: mode 0 genuinely has no table entry.
A translation must therefore treat mode 0 (and mode 0 in the `+7` form) as a
path with no observable effect rather than as untranslated code.

## Signed displacements (a second trap)

`bd` is **signed**. `0x107CD0` uses `22 7B 05 20 FD40`: read as unsigned that
is `+0xFD40` and lands in garbage at `0x117A12`; read as signed it is `-0x2C0`
and the base is `0x107A12`, the same table as `0x1078F0`. Three call sites in
this family (`0x1078F0`, `0x107CD0`, `0x107F08`) reach one of two sibling
tables (`0x107A12`, `0x107FAC`) that share the shape NULL / 6 modes / NULL /
6 modes-with-+7.

## Why this mattered

Three routines were abandoned on the belief that a runtime base register
needed discovering. They only needed the correct PC-relative arithmetic. Use
`work/audit/a16_table.py <instr>` to resolve any such instruction; it prints
the base and the pointed-to records, treats a NULL entry as legitimate, and
applies the sign to the displacement.
