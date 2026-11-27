/* ROM 0x13CACE: stage-start init. Clears the scroll counters at A5-0xCA4,
 * loads the stage terrain (0x0C6738), resets the timeline/script state
 * (0x13F25C), resets the A5+0x1D5E counters (0x0CEC40), draws the stage banner
 * (0x13D11E), starts the timed effect (0x13CB3A), queues the 1-word palette
 * upload of 0x1FB40, stamps the stage status char (0x19F29C) and spawns the
 * 0x13EF62 object in group 0x0B with A0 = the ROM table base 0x168C0.
 * 0x5A86 leaves A1 = 0x61FD20 and 0x1057C returns the new object in D0. */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"
#include "../video/sprite.h"

/* ROM 0x13F25C: reset the stage timeline (script pointer from 0x13F9D4[stage],
 * scroll 0, speed 1.0, last pos -1, clear the two event flags) and free
 * object groups 3..4. Restores D0/A0. */
static void cace_timeline_13F25C(const f3mem_t *m, f3regs_t *r)
{
    int16_t idx = (int16_t)rd16(m, F3_A5 + 0x32DCu);            /* 0x13F266 */
    uint32_t d0 = r->d[0], a0 = r->a[0];

    r->a[0] = 0x0013F9D4u;                                      /* 0x13F260 */
    wr32(m, F3_A5 + 0x3308u,
         rd32(m, 0x0013F9D4u + (uint32_t)(int32_t)idx * 4u));    /* 0x13F26A */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)idx;
    wr8(m, F3_A5 + 0x331Au, 0u);                                /* 0x13F270 sf */
    wr32(m, F3_A5 + 0x330Cu, 0u);                               /* 0x13F274 */
    wr32(m, F3_A5 + 0x3310u, 0x00010000u);                      /* 0x13F278 */
    wr16(m, F3_A5 + 0x3314u, 0xFFFFu);                          /* 0x13F280 */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xEFu)); /* 0x13F286 */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xF7u)); /* 0x13F28C */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & 0xFBu)); /* 0x13F292 */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & 0x7Fu)); /* 0x13F298 */

    r->a[7] -= 2; wr16(m, r->a[7], 2u);                         /* 0x13F29E */
    r->a[7] -= 2; wr16(m, r->a[7], 3u);                         /* 0x13F2A2 */
    Obj_ResetGroups(m, 3u, 2u);                                 /* 0x13F2A6 -> 0x1060E */
    r->a[7] += 4;                                               /* 0x13F2AC */

    r->d[0] = d0; r->a[0] = a0;                                 /* 0x13F2B0 */
}

/* ROM 0x005A86: copy the character tiles of the {count.w, idx.w...} record at
 * A0 into char RAM 0x61E000 + D0.w*0x20, 8 swapped longs per tile, source
 * table 0x1120A. Leaves A1/A2/A3/D0/D1/D2 live (no save frame). */
static void cace_text_tiles_5A86(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1 = 0x0061E000u + (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)r->d[0] << 5); /* 0x005A86..0x005A8E */
    uint32_t a2 = 0x0001120Au;                                  /* 0x005A90 */
    uint32_t a0 = r->a[0];
    int16_t d0 = (int16_t)rd16(m, a0);                          /* 0x005A96 */
    uint32_t a3 = 0;
    uint32_t d2 = 0;
    int16_t d1 = 0;
    a0 += 2u;

    do {
        d1 = (int16_t)(uint16_t)((uint16_t)rd16(m, a0) << 5);    /* 0x005A98..0x005A9A */
        a0 += 2u;
        a3 = a2 + (uint32_t)(int32_t)d1;                        /* 0x005A9C */
        d1 = 7;                                                 /* 0x005AA0 */
        do {
            d2 = rd32(m, a3);                                   /* 0x005AA4 */
            a3 += 4u;
            d2 = (d2 >> 16) | (d2 << 16);                        /* 0x005AA6 swap */
            wr32(m, a1, d2);                                    /* 0x005AA8 */
            a1 += 4u;
            d1--;                                               /* 0x005AAA dbf */
        } while (d1 != -1);
        d0--;                                                   /* 0x005AAE dbf */
    } while (d0 != -1);

    r->a[0] = a0; r->a[1] = a1; r->a[2] = a2; r->a[3] = a3;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)d0;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)d1;
    r->d[2] = d2;
}

/* ROM 0x13D11E: draw the stage banner: upload the tiles of 0x13D558[stage] at
 * slot 0x10C05 and blit tilemap 0x10C00 to 0x61C024 (0x0055C8 keeps every
 * register, so only 0x005A86's leftovers survive). */
static void cace_banner_13D11E(const f3mem_t *m, f3regs_t *r)
{
    int16_t idx = (int16_t)rd16(m, F3_A5 + 0x32DCu);            /* 0x13D124 */
    r->a[0] = rd32(m, 0x0013D558u + (uint32_t)(int32_t)idx * 4u); /* 0x13D11E..0x13D128 */
    r->d[0] = rd8(m, 0x00010C05u);                               /* 0x13D12C moveq #0,D0 / move.b */
    cace_text_tiles_5A86(m, r);                                  /* 0x13D134 */
    Gfx_DrawTilemapBlock(m, 0x00010C00u, 0x0061C024u, 0u);       /* 0x13D13A..0x13D14A */
}

int Fn_13CACE(const f3mem_t *m, f3regs_t *r)
{
    wr16(m, F3_A5 - 0x0CA4u,
         (uint16_t)(rd16(m, F3_A5 - 0x0CA4u) & 0xFFEDu));        /* 0x13CACE andi.w #-0x13 */
    wr32(m, F3_A5 - 0x0CACu, 0u);                               /* 0x13CAD4 */
    wr32(m, F3_A5 - 0x0CA8u, 0u);                               /* 0x13CAD8 */

    (void)Fn_0C6738(m, r);                                      /* 0x13CADC jsr 0x0c6738 */
    wr8(m, F3_A5 + 0x30F4u, 0u);                                /* 0x13CAE2 sf */
    cace_timeline_13F25C(m, r);                                 /* 0x13CAE6 jsr 0x13f25c */
    Leaf_0CEC40(m, r);                                          /* 0x13CAEC jsr 0x0cec40 */
    cace_banner_13D11E(m, r);                                   /* 0x13CAF2 bsr 0x13d11e */
    (void)Fn_13CB3A(m, r);                                      /* 0x13CAF6 bsr 0x13cb3a */

    r->a[7] -= 4; wr32(m, r->a[7], 1u);                         /* 0x13CAFA push 1 */
    r->a[7] -= 4; wr32(m, r->a[7], 0u);                         /* 0x13CB00 push 0 */
    r->a[7] -= 4; wr32(m, r->a[7], 0x0001FB40u);                /* 0x13CB06 push 0x1fb40 */
    Pal_QueueUpload(m, 0x0001FB40u, 0u, 1u);                    /* 0x13CB0C -> 0x001660 */
    r->a[7] += 0xCu;                                            /* 0x13CB12 */

    (void)Fn_19F29C(m, r);                                      /* 0x13CB16 jsr 0x19f29c */

    r->a[7] -= 4; wr32(m, r->a[7], 0u);                         /* 0x13CB1C pea (0).w */
    r->a[7] -= 2; wr16(m, r->a[7], 1u);                         /* 0x13CB20 move.w #1 */
    r->a[7] -= 2; wr16(m, r->a[7], 0x000Bu);                    /* 0x13CB24 move.w #0xb */
    r->a[7] -= 4; wr32(m, r->a[7], 0x0013EF62u);                /* 0x13CB28 move.l #0x13ef62 */
    r->d[0] = Obj_Spawn(m, r->a[0], 0x0013EF62u, 0x0Bu, 1u, 0u); /* 0x13CB2E -> 0x01057C */
    r->a[7] += 0xCu;                                            /* 0x13CB34 */
    return 0;                                                   /* 0x13CB38 rts */
}
