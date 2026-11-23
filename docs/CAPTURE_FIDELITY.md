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
