# Capture fidelity: does the post show the routine's mandatory writes?

A capture can be perfectly auditable — every pair has `fn(obj) == routine` — and
still be unusable, because the **post** was not produced by the routine's tick.
This file records the test that decides it, and the case that exposed it.

## The test

For each branch the routine can take, find a write the ROM makes **unconditionally**
and whose value is observable in the record. Then check the post against it.

`0x108512` state 1 (`0x1085CC`) does, before anything else:

    movea.l $54(a0),a1 / movea.l $54(a1),a1 / movea.l $54(a1),a1
    move.l  $44(a1),$14(a0)      <- obj+0x14 := the chain source's +0x44

So in any tick that really runs state 1, `post +0x14` must **start from**
`src+0x44` and then only be adjusted by one turn vector and two small
subtractions. Measured on the capture:

| pair | src+0x44 | pre +0x14 | post +0x14 | docked? |
|---:|---|---|---|---|
| 0 | `0x014A9A0A` | `0x012A0000` | `0x0129E912` | **no** |
| 1 | `0x0149CA0A` | `0x0129E912` | `0x01291912` | **no** |
| 2 | `0x0148FA0A` | `0x01291912` | `0x01284912` | **no** |
| 3 | `0x01482A0A` | `0x01284912` | `0x01277912` | **no** |

`post +0x14` always continues from `pre +0x14` (a `-0x7000` drift step), while
`src+0x44` moves on its own at `-0x0D000` per pair. The post therefore shows the
**drift** handler (`0x108588`), never the state-1 dock, although `obj+0x6 == 1`
in all 60 pairs.

Conclusion: the capture cannot be satisfied by a faithful state-1 translation.
Returning `-1` makes the pairs unobservable (correct but scores nothing); making
them pass requires inventing a dock that the ROM does not do.

## What this is NOT

- It is **not** the `0x41B1E1` artifact: that one is a single byte in P1's state
  and is excluded in `tests/capture.h`.
- It is **not** a "noisy post": 1105 of 1169 captures touch only the object's own
  0x6C bytes, which is completely normal for a tick routine. Narrowness proves
  nothing; **missing mandatory writes** prove the post is wrong.

## How to apply it

Before writing a translation for a routine with a capture, list the writes the
routine makes on the captured branch and check at least one against the post.
If a write that the ROM always performs is absent from every post, the capture
does not exercise that branch usefully — report it instead of fitting code to it.

Routines already landed this session satisfy this test (for example
`0x1077DA`'s state-1 handler publishes `obj+0x44/0x48` to the parent at
`+0x60/+0x64`, and every post shows it).

## The gate is coarse: it passed three unusable captures

`work/audit/precheck.py` has a `fidelity` gate that only asks "did the post write
an aligned field of the object". On 2026-11-23 it said **PASS** for `0x10C966`,
`0x111B56` and `0x11418E`, and all three were unusable:

- `0x11418E`: the post moves `obj+0x42` from `0x0006` to `0x000F`, but the ROM
  writes `(0x42) := (0x6) >> 1` with the captured `+0x6 == 0`, which is `0`.
- `0x111B56`: the ROM clears `obj+0x18` unconditionally; half the posts keep it.
- `0x10C966`: the routine must set bit 4 of `obj+0x07` on the state-0 path; the
  post has `0x00` there in all 60 pairs.

So the gate catches only the gross case (a post that never touches the object).
Three of the last four workers correctly rejected captures the gate had passed,
by checking a *specific* mandatory write. That is the standard: read the ROM,
pick one write the captured branch cannot avoid, and look for it in the post.

The precheck therefore also prints the captured `obj+0x6` histogram. Workers had
reported states (`0x1E`, `0x80`, `0x02`) that the files do not contain; the
histogram is the value they should have quoted.

## `work/audit/fidelity_scan.py` is a screen, not a verdict

The script (built with Hermes, reviewed and validated by the parent) prints per
routine: pair count, whether `fn(obj)` equals the routine in every pre **and**
post, the `obj+0x6` histogram, and how many aligned 2-byte fields inside the
object change.

It is genuinely useful for triage — of 380 routines with a capture, 159 have
`fn_ok=yes`, and 67 of those also have >=30 pairs and >=8 changing fields. But
**it ranks the four captures proven unusable today among the best**:

    1074B6, 107BC2, 107E00, 10C966, 10DBE0, 11418E   <- all known-bad, all "pass"

A capture can change many of its own fields and still be the wrong tick. Use the
scan to throw out the obvious cases (no pairs, `fn` mismatched at the post, an
object that never changes); decide the real ones by reading the ROM and checking
one mandatory write, as described above.

One behaviour worth knowing: `fn_ok=no` is not automatically bad. `0x130794`
returns `no` because the routine legitimately rewrites `obj+0` to its own
continuation (`0x1307E0`) during the tick — a self-reassigning routine looks
"wrong" to the check and is in fact correct.

## Correction (2026-11-24): the "mandatory write" test was used wrongly

Five worker attempts rejected their captures as "the post is not this routine's
tick". At least one of those rejections is **wrong**, and the mistake is
instructive about the test itself.

`0x111B56` was rejected because the ROM starts with an unconditional
`clr.l (0x18,A0)` and half the posts show `0x740000` there instead of 0. The
worker read the head of the routine and stopped. The routine later does, at
`0x111C56`:

    111C56  06a8 00740000 0018      addi.l #0x740000,$18(a0)

on the odd-frame path. The capture's `+0x18` alternates `0 -> 0x740000 -> 0`,
which is exactly what the ROM produces. The capture is fine.

### The rule, stated so it cannot be misapplied

A "mandatory write" test is only valid when the write is **unconditional on the
captured branch**, which means you have read **every** instruction that can
reach the object's field on that branch — not the first store you meet. If a
post disagrees with the first store, search the whole routine for a later store
to the same offset before concluding the capture is wrong.

The cheapest way to avoid this: grep the routine's own range for every store to
the offset in question (`<offset>` appears as `00<xx>` in `move.w/l/addi` forms)
and account for all of them.

### What this means for the tally

Of the rejects seen today, `0x111B56` is definitely a false rejection. The
others (`0x116E22`, `0x116802`, `0x116C9A`, `0x11418E`, `0x10C966`, `0x10DBE0`)
were argued from similar reasoning and deserve the same re-check before any
recapture is planned. The capture pipeline may be less broken than the attempt
log suggests, and the workers' fidelity checks are the more likely source of
false negatives.

## Second correction: check the claim, not the narrative

`0x116E22` was rejected a second time, this time with a list of four "blocking
facts". Re-checked by the parent, three of the four are wrong:

- "The instruction stream is shifted; `/tmp/f3dis.py` drops 0x116E30". It does
  not. `python3 /tmp/f3dis.py 116E22 0x40` prints 0x116E30 `movea.l $8(a0),a1`
  and 0x116E40 `addi.w #0x4000,d0`, contiguous and correct.
- "The ROM image is damaged at 0x27E40 / 0x1171D0". It is not.
  `python3 scripts/reconstruct_rom.py` rebuilds the image from roms/dariusg.zip
  and reports ORACLE MATCH with the MAME dump, byte for byte.
- "`+0x20 <- A5+0x30BC` is missing from every post (posts show 0x3A00..0x4400)".
  Measured: `A5+0x30BC` is `0x0000` and `obj+0x20` is `0x0000` in the posts, and
  the two match in **60/60 pairs**. The rejection quoted values that are not in
  the file.

### The procedure that would have caught this

Before writing a rejection, print the evidence you are rejecting on:

    python3 -c "..."   # the specific pre/post bytes, and the ROM bytes,
                       # in the same output block

A rejection is a claim about the data. Paste the numbers, not a description of
them. Three separate rejections today were refuted by doing exactly that, and
the ROM-reconstruction oracle (`scripts/reconstruct_rom.py`) settles any
"the image is corrupt" claim in one command.

None of this says every capture is good: `0x108512` was a real defect, proven by
a mandatory write absent from all 60 posts. The point is that a rejection needs
the same standard of proof as a translation.

## Third correction: 111B56's "all-zero sprite def" is false

A second attempt at `0x111B56` reported as its decisive evidence that
`obj+0x10 = 0x02D768` is "all zeros in every captured WRAM image", so the
`sprite cursor` advance could not have come from a real draw.

Measured from the ROM:

    0x02D768: 00 0f 00 03 09 e9 00 52 ...
              w = 15, h = 3

That is a perfectly ordinary 15x3 sprite definition. The rejection was wrong on
its strongest point.

### Score of the rejections so far

Of seven captures rejected as "unfaithful" on 2026-11-24, re-checked by the
parent, **three were false** (`0x111B56` twice, on different claims; `0x116E22`
once). Only `0x108512` has been proven defective, by a mandatory write that is
absent from all 60 posts.

That is a strong argument that the **attempt** pipeline, not the capture
pipeline, is the main loss: the workers are rejecting good data on misreadings
of the ROM. Before spending days on recapture, make every rejection paste
(a) the ROM bytes, (b) the pre/post bytes, and (c) the exact address each was
read from. Two of the three false rejections above collapse on step (c).

## Fourth finding: fabricated evidence, not just sloppy reading

The `0x116E22` retry produced the best-formatted rejection of the session --
exact addresses, exact bytes, commands cited, a table of pre/post values. Two of
its three decisive claims are false:

| claim (verbatim) | measured |
|---|---|
| "the bytes the post changes are the NEXT object, 0x413B04 (+0x20) pre 0x1995 -> post 0x0000" | next-object +0x20 is **unchanged at 0x1995** in the post |
| "obj+0x18 post is byte-swapped: wants 0x00CA89B8, written as CA 89 B8 00" | `obj+0x18` post is exactly `00 CA 89 B8`; the quoted bytes `CA 89 B8 00` are not in the file |

Meanwhile the one thing the capture really shows -- `obj+0x20` going `0x2394 ->
0x0000` where `A5+0x30BC` is `0x0000` -- is *correct ROM behaviour* and was
presented as a defect.

### Consequence for how this work is judged

Formatting is not evidence. A rejection must be reproducible: the parent must be
able to run the cited command and see the cited bytes. Every rejection in this
file now carries that standard, and the parent checks the two or three decisive
claims by hand before acting on them. Of the rejections re-checked this way on
2026-11-24, all but one (`0x108512`) were wrong.

## What `0x111B56` actually shows: the spawn argument is unreachable

After the two false rejections, the parent spent an attempt on this routine
directly and found one real, reproducible discrepancy. Recorded so the next
attempt does not repeat the work:

* `Obj_Spawn` at `0x1057C` writes the child as `fn@+0`, `group@+4`, `idx@+5`,
  `parent@+8`, `arg@+0xC`. The child 0x111B56 creates sits at `0x41027C` in the
  post with `fn=0x0013E4EE`, `parent=0x00410210`, and **`arg=0x045C0002`**.
* The routine pushes that argument with `move.l d1,-(a7)` at `0x111C84`, and
  `d1` is set by `moveq #$2,d1` at `0x111BFE` (which clears all 32 bits) and then
  modified only by `bfins d0,d1{0:8}` and `bfins d0,d1{8:8}` (capstone confirms
  8-bit fields). Those two writes can only produce a value in `0x0000????` whose
  top half is zero.
* Measured: the captured `arg` has `0x045C` in its top half. Nothing in the
  routine writes that half. The capture is right about everything else the
  routine does (`+0x14` matches once the three RNG draws are modelled, and the
  seed advances exactly three steps).

So the routine is understood, and the capture contains one value the code cannot
produce. A correct translation scores 0/60 on it. This is the first genuinely
defective detail found in a capture that had passed every other check.

### Note for whoever retries

Do not "fix" this by hard-coding `0x045C0002`. The honest options are a recapture
of `0x111B56`, or accepting the routine as decompiled-but-unverifiable and
recording that explicitly.

## The first capture proven defective by internal contradiction

After several false rejections and one real one (`0x108512`), here is a second
genuine defect, and this one needs no ROM reading at all — the capture
contradicts itself.

`0x17C360` moves the object with:

    17C3AE  d168 0014   add.w d0,$14(a0)
    17C3B2  d168 0018   add.w d0,$18(a0)

The *same* register is added to both coordinates. Measured in
`work/ram/obj_17C360_pairs.bin`, object `0x417A4C`, pair 17:

    +0x50 (x): 0x0110 -> 0x010C   delta -4
    +0x54 (y): 0x009A -> 0x009A   delta  0

That is impossible for this code. Pair 16 of the same object is `0 -> 0` for
both, so the pair is not simply mislabelled either.

### Why this matters

It is the cleanest detector found so far: **when a routine adds one register to
two fields, the two deltas must be equal or both zero.** No disassembly, no
ROM comparison, no address arithmetic — just read both fields in the pre and post
and compare the deltas. A capture that fails it is broken, regardless of what
any worker claims about it.

Contrast with the many false rejections this file records: those argued from
ROM readings and were wrong. This one argues from the data against itself.

## 0x124532: a shared global written by another object in the same tick

`0x124532` sits at 59/60, failing only pair 41 on `A5+0x30EE` (`0x0001 -> 0x0401`).
Traced to the end, and the routine is not at fault:

* `0x124532` compares `A5+0x30D2`; with the captured value `0x002C3A67` the
  low word is `0x3A67`, so `bgt 0x12452A` is taken and the routine re-arms
  itself. It never reaches its own `bfset $30ee(a5){5:1}` at `0x124542` -- which
  is why bit 5 is absent from the post.
* Bit 10 (`0x0400`) is what `0x1244BA` writes, in a *different* object:
  `bfset $30ee(a5){2:1}` operates on the byte at `0x40B0EE`, i.e. bit 2 of that
  byte = bit 10 of the word. That object is `0x1244A2`, still a stub, and it
  runs inside the same tick.
* The flag appears only in pair 41's post and is gone again in pair 42's pre.

So `0x30EE` is a global several objects write during one frame. Comparing it
byte-for-byte against a single routine's tick is wrong in principle; the same
reasoning applies to the `0x41B1E1` case already excluded in `tests/capture.h`.
A routine should not be failed for a flag another object set -- but neither
should the harness silently ignore arbitrary globals. The honest fix is to
decompile `0x1244A2` so the writer is modelled, not to widen the exclusion.

## 0x1244A2: a sound capture proves where the writer is not

`0x1244A2` now models its whole window and passes **58/58** comparable ticks
(2 pairs take `0x12456A`, which jumps to the shared `0x124B3A` phase start and
is reported as an undecompiled path, not as a failure).

Two captures in that set settle the `0x30EE` question independently of
`0x124532`:

* pair 29 goes `30EE 8001 -> E003` while `30DA = 0x00FA` (250, above the 0xC8
  the routine's own `cmpi.w` needs) and `+0x3E` merely counts `0x0F -> 0x0E`.
  `0x1244BA` cannot have run: it requires `30DA <= 0xC8` and would have
  re-armed `+0x3E` to 0x24.
* pair 59 shows the same `8003 -> E003` with `+0x3E` at 0 (object freed, so
  `A0` is reused), again without the `+0x3E = 0x24` that `0x1244BA` writes.

In both cases `30D2` moves only by the routine's own `-0x30000`. So the
`E003` value is written by another object in the same frame, exactly as the
`0x0400` bit in the `0x124532` case. Failing `0x1244A2` for it would be wrong;
the honest fix is still to decompile the writer.

One more trap worth recording: `0x1244A2` lives in the `0x124xxx` cluster where
several different routines share `(A5)` globals and chain through
`jmp $124B3A(pc)`. Reading a disassembly by a fixed byte window mixed three
routines into one function; every branch had to be re-read from its own target.

## Capturing an object's exit is a separate problem

Of the 142 captured-but-stubbed routines with 60 or more pairs, only 45 write
their own object on at least 90% of ticks. The other 97 are lifespan tails: the
routine's observable work happens only on the tick where a timer crosses zero,
and the capture almost always stops inside the wait loop. Translating such a
tail yields `0/0 REJECT`, which proves nothing — the model never executes.

Two consequences worth keeping in mind before spending time on a stub:

* A pair count says nothing about coverage. `60 pairs` on a tail routine means
  sixty samples of the same `subq.w #1,(An)` instruction.
* The capture needs an exit-forcing poke (`F3_POKEA0B` on the timer field) if a
  tail is ever to be verified. Until then, translate only the 45 bodies that
  write every tick and leave the tails documented as pending.

## Re-capturing a tail: the mechanism works, the states do not

`scripts/capture_obj.sh` now forwards `F3_POKEFN` / `F3_POKEA0B` (and the other
poke env) to `capture_fn.lua`. With `F3_POKEFN=<addr> F3_POKEA0B=3e=1` the
forced value lands only while that routine is the one being called, before the
`pre` snapshot, so the pair stays self-consistent.

What blocks using it on the 97 tails is not the mechanism but the savestates:

* `02_stage1_start`, `03_active_combat` and `01_attract` no longer reproduce the
  frames the existing captures came from. Re-running them for a routine that
  *does* have a good capture (0x11418E) yields 0 pairs.
* The `run1_NNNNNN` states do reproduce them (0x11418E captures 5/5 from
  `run1_010000`).
* Some routines live in none of the 30 saved states: scanning every `run1_*`
  for 0x15A264 finds zero pairs, so that routine cannot be re-captured with any
  current artefact.

So the next real unlock needs a fresh long run that saves state every N frames
while the missing routines are alive, not another look at the Lua.

## Where the position movement in a tail capture comes from

Several spawn-wait tails (0x17E1DA, 0x17F038) capture a position change their
entry never writes:

* 0x17E1DA: pre/post pairs are continuous (post of pair N == pre of pair N+1),
  so the capture is not mixing objects.
* Its entry writes only +0x0C/+0x16/+0x1A and the state pointer; the delta in
  +0x14 (-4, -6, -7 for three pairs) does not match the pre velocity in +0x16
  (-5, -28, -39), nor `v>>3`, nor the `+0x1F * sprite_word >> 9` term of the
  successor body 0x17C240.
* `+0x16` settles at 0x0008 in every pair while the delta keeps varying.

The movement lives in 0x17C240 (`add.w d0,$14(a0)` at 0x17C28E), which runs
*after* the entry hands over, so a capture anchored at the 0x104EE call site
cannot attribute it to the entry. The same shape appeared in 0x11B472 earlier.

Practical rule: before translating a tail that appears to move, check whether
`+(0x14)`/`+(0x18)` change in the pair. If they do and the entry has no writer
for them, the routine is not verifiable against this capture and should stay a
stub rather than be fitted to numbers it never produced.

## F3_LINGER: capturing the successor state in the same pair

The movement that made 0x17E1DA and 0x17F038 unverifiable belongs to the
successor body 0x17C240, which runs after the entry returns. `scripts/lua/
capture_fn.lua` now accepts `F3_LINGER=N`: the post dump is deferred by N further
entries of the same tick site, so a pair contains the entry plus the hand-off.

Measured on 0x17E1DA (`LA_032000`, forced exit, `F3_LINGER=1`):

    pair 0   fn=0017e1da  +0x14 0110 -> 010C (dx = -4)   (with linger)
    pair 0   fn=0017e1da  +0x14 0110 -> 0110 (dx =  0)   (without)

So the movement is real and reproducible, it just lives one dispatch later.

What this does *not* solve: verifying 0x17E1DA still needs 0x17C240 translated,
and 0x17C240 in turn needs the 0x178982 script opcode 0x178C06 (the record at
0x17D4E0 selects 0x178B4A then 0x178C06), plus 0x17C50A/0x17C530. The linger
removes the attribution problem, not the callee chain.

## Two decoder traps that cost real time

Both appeared while chasing 0x178C06, and both produce a *plausible but wrong*
listing rather than an error:

1. **Block disassembly drifts after a bitfield instruction.** Dumping
   0x184984..0x1849AA in one call printed `bfexts -$5556(a0){0:16}` and
   `divs.w #$aaaa,d1`, where the bytes are `bfexts $42(a0)` and `divs.w #$64`.
   Re-disassembling from each instruction's own offset gives the right answer
   every time. When a value looks absurd, re-dump from that address rather than
   trusting the block.

2. **Offsets in `(d16,An)` are byte displacements, not table indices.**
   `muls.w $80(a1,d2.w*2),d0` reads `table + 0x80 + d2*2`. Indexing the table
   as `table[d2]` and then adding 0x80 to the *index* gives a value off by a
   factor of two and silently breaks the trig term.

The 0x27E40 table at 0x27E40 is indeed the sine table (0x0000, 0x0192, 0x0323,
...), and the difficulty table 0x1849D4 is loaded through
`bfextu $1849d4(pc,d0.w){0:8}`, i.e. `0x1849D4 + selector` -- so selector 0
reads 0x4E, the first byte of the preceding `rts`, and the real scale values
(0x0A, 0x0A, 0x14, ...) start at 0x1849D8.

## The script-opcode wall is shared, not per-routine

Chasing 0x17E99A (a body that *does* run on every tick, 9-16 bytes changed per
pair) ended in the same place as 0x17C240: its 10-byte script record at
0x17EBCE is

    0x178B4A   +0200
    0x178C06   ffff
    0x178B20   +0180

and `Obj_Script10` only implements 0x178B20 and 0x178E4A. So every routine that
walks a 0x178982 script with 0x178B4A or 0x178C06 is blocked by the same two
missing handlers, no matter how clean its capture is.

0x178B4A itself is now read correctly: it *assigns* `(0x42,A0) = 0x6(A1)` and
then calls 0x184984 to scale it. The scaling disagrees with the capture and has
not been explained, so both handlers stay unimplemented rather than guessed.

Practical consequence: before picking a routine from this family, read its
script record at `obj+0x44` and check the first dword. If it is 0x178B4A or
0x178C06, the capture cannot be turned green until those two handlers land, and
working them needs the register trace that the 0x184994 mismatch is blocking.

## Register trace at 0x184994 (0x17C240, state LD_017000)

Captured with `F3_LOG=184994,18499a,1849a0` on a pair whose object is 0x417B24:

    PC        D0          D1
    184994    00000000    0000001C
    18499A    00000000    00000400
    1849A0    00000000    00000000

and that object's `(0x42,A0)` is 0xFED7 in the pre snapshot and 0x0400 in the
post.

So `(0x42,A0)` really does become exactly 0x0400, and the intermediate values in
D1 do not follow the `bfexts -> divs.w #100 -> muls.w scale -> add.w` chain read
from the listing: 0xFED7 sign-extends to -297, not to the 0x1C seen at 0x184994.

One honest reading of the numbers is that 0x184984 is entered with a different
`(0x42,A0)` than the one in the pre snapshot (the 0x178B4A handler assigns 0x400
first), so the scaling runs on 0x400: 0x400/100 = 10, x scale, added back. For
the result to stay exactly 0x400 the scale term must be zero, which contradicts
the 0x4E byte the listing points at. The chain is therefore *not* settled by this
trace alone; what is settled is that the final value is 0x0400 and that my
reading of the intermediate registers is wrong somewhere.

Next step would be a trace at 0x178B4E, 0x184994 and 0x1849A0 on the *same*
pair index, filtered to the object under test.

## Solved: the 0x184984 difficulty scaling

The mismatch that blocked 0x178B4A for three passes was a single wrong base
address. `bfextu $1849d4(pc,d0.w){0:8}` computes PC from the **extension word**
(0x184992), so the table really starts at 0x1849D6:

    sel 0..15 -> 00 00 0A 0A 14 14 1E 1E 28 28 32 3C 46 50 5A 64

Selectors 0 and 1 read 0, so on the default difficulty the scaling is a no-op
and `(0x42,A0)` keeps whatever 0x178B4A assigned. That is exactly what the
0x17C240 capture shows: the record's `+6 = 0x400` is assigned and stays 0x400.

The `+0x80` in `muls.w $80(a1,d2.w*2)` is likewise a byte offset into the sine
table, so with `+0x42 = 0x400` and `+0x20 = 0x8000` the delta is

    mag = (0x400 + 0x20) >> 6 = 16
    sn  = table[0x80*2] = 0
    cs  = table[0x80*2 + 0x80] = -16384
    16 * -16384 = -0x40000

which matches the captured `+0x50` delta of -0x40000 exactly.

Both handlers are now in Obj_Script10. What is still open is finishing the
bodies that consume them (0x17C240, 0x17E99A): their scripts run, but the
paired capture does not go green yet, and the remaining delta has not been
explained.

## The script runs one entry per tick, and the trig is exact

Live trace of 0x17E99A (state LE_056000, script record 0x17EBCE):

    PC       D0          D1
    178B4E   00000001    0000001F     ; 0x178B4A entry, assigns +0x42
    178C1A   00000264    0000001F     ; 0x178C06 entry, d0 written back to +0x42

and the pair shows `+0x44` moving 0x17EBCE -> 0x17EBD8 (exactly one 10-byte
entry) with `+0x48 = 0xFFFF` afterwards. So the interpreter advances **one
entry per tick** here, not the whole list; a model that walks every entry in one
tick lands on the wrong +0x42.

With `+0x42 = 0x264` the trig term is now confirmed exact:

    mag = (0x264 + 0x20) >> 6 = 10
    sn  = 0x27E40[0xa0*2]     = -11585
    mag * sn                  = -115850

which is exactly the captured `+0x50` delta. The `0x178C06` handler and the
difficulty table are therefore right; what remains unexplained in this routine
is only how `+0x42` gets from 0x4C8 to 0x264 through the 0x178B4A entry
(0x200 assigned, 0x264 observed).

## 0x17C240: the +0x42 shift bug is fixed, the body still is not

The bitfield shift that blocked 0x178B4A is fixed: `bfextu $87e(a5){8:4}` reads
bits 8..11, i.e. `(v >> 4) & 0xF`, and the selector is 4, not 0. With that the
chain is exact and 0x17E99A verifies 60/60.

0x17C240 itself still disagrees on pair 2 only: `+0x50` moves in the capture
(0x73 -> 0x6F) while the script in the record never touches it. The observed
`+0x48` goes 0 -> 0xFFFF, which the interpreter reads as "stop", so 0x178C06
should not run at all that tick -- yet the position moved. That contradiction is
unresolved and the routine stays a stub.

Operational note: the capture work/ram/obj_17C240_pairs.bin was overwritten
during this pass by a F3_LINGER run and its original (60 pairs) is gone; only a
47-pair version remains. This is the second capture lost that way (131312
earlier). Before re-running a capture for a routine that already has one, copy
`work/ram/obj_<ADDR>_pairs.bin` somewhere outside work/ram.
