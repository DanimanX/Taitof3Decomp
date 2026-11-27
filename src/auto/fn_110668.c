/* ROM 0x110668: stage-intro setup for a mid-boss formation. Blits three
 * compressed tile blocks into playfield RAM (0x9062C -> 0x612400,
 * 0x90658 -> 0x612020, 0x90674 -> 0x612420, all with flags 0), spawns one
 * 0x110C5E child in group 3 and three 0x110DCC children in group 5, tagging
 * the latter +0x4E = 0, 1, 2, then re-arms the object with 0x110756.
 *
 * At rts D0 = the last 0x1057C result (the child object, or 0xFFFFFFFF when
 * group 5 was full), A1 = the child it tagged last, A0 = the entry object. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x01057C: spawn one object; returns the new object or 0xFFFFFFFF */
static uint32_t fn110668_spawn(const f3mem_t *m, uint32_t obj, uint32_t fn,
                               uint16_t group, uint16_t ngroups)
{
    return Obj_Spawn(m, obj, fn, group, ngroups, 0u);
}

int Fn_110668(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x110668..0x110698: jsr 0x53B2(0x9062C, 0x612400, 0) and two more */
    Gfx_DrawCompressedTilemap(m, 0x0009062Cu, 0x00612400u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x00090658u, 0x00612020u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x00090674u, 0x00612420u, 0u);

    /* 0x1106B6..0x1106CE: jsr 0x1057C(obj, 0x110C5E, group 3, count 2, arg 0).
     * Its result is dropped: only its children are wanted. */
    (void)fn110668_spawn(m, obj, 0x00110C5Eu, 3u, 2u);

    /* 0x1106D2 / 0x1106FA / 0x110724: three group-5 children, +0x4E = 0..2.
     * A failed spawn (negative D0) jumps straight to the re-arm. */
    for (uint16_t t = 0u; t < 3u; t++) {
        uint32_t d0 = fn110668_spawn(m, obj, 0x00110DCCu, 5u, 2u);
        r->d[0] = d0;                                       /* tst.l D0 ; bmi */
        if ((int32_t)d0 < 0)
            break;
        r->a[1] = d0;                                       /* movea.l D0,A1 */
        wr8(m, d0 + 0x4Eu, (uint8_t)t);                     /* move.b #t,(0x4E,A1) */
    }

    wr32(m, obj, 0x00110756u);                              /* 0x11074E re-arm */
    return 0;                                               /* 0x110754 rts */
}
