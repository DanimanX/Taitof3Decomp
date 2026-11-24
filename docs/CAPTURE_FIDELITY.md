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
