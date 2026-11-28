/* ROM 0x1319FE: UNVERIFIED (weak capture): boss projectile / beam tick
 * (A0 = object). The only captured pair spans frames 0x06DD->0x0992 and its
 * entry state is degenerate (+0x0C points at the unmapped 0x118F0000, +0x10 is
 * 0), so the sprite draw walks ROM 0x000000 as a 0x82D-column frame and the
 * captured sprite RAM / sprite cursor cannot be reproduced; the object's own
 * 0x6C bytes and the gfx RAM do match this translation exactly.
 * Frees the slot when
 * A5+0x30F2 bit 0 is set. Otherwise steps its {dur.w, def.l} animation table
 * (+0x0C) at +0x3A/+0x3E (0x0CBD24) and: on a negative duration frees the slot;
 * on a positive one moves it by the parent's position plus +0x2C/+0x30 and
 * draws it; on a zero duration it first plays sound 0x1B when +0x3E is zero,
 * spawns the three 0x0CD124 side objects (arg = +0x36, +0x800, -0x1000) when
 * +0x3E >= 0x21 and is a multiple of 12, advances +0x3E (wrapping with +0x3A
 * += 2 at 0xDC), then moves and draws it. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x0CBD24: advance the 6-byte {dur.w, def.l} entry at table+offset
 * (+0x3A), counting +0x3E up to dur and stepping the offset by 6 past it.
 * Leaves D0 = offset and D1 = counter. */
static void anim_step(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                      uint32_t *d0, uint32_t *d1)
{
    for (;;) {                                          /* 0x0CBD24 */
        uint16_t off = rd16(m, obj + 0x3Au);
        *d0 = (*d0 & 0xFFFF0000u) | off;
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)                                   /* 0x0CBD28 */
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0x0CBD30 */
        wr16(m, obj + 0x3Eu, cnt);
        *d1 = (*d1 & 0xFFFF0000u) | cnt;
        if ((int16_t)cnt <= dur) {                      /* 0x0CBD38 */
            wr32(m, obj + 0x10u,
                 rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u)); /* 0x0CBD4A */
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);                       /* 0x0CBD40 */
        off = (uint16_t)(off + 6u);                     /* 0x0CBD44 */
        wr16(m, obj + 0x3Au, off);
    }
}

/* ROM 0x12FF36: copy the +0x1D palette byte of A5+0x32AA, or the object's own
 * +0x4F when the two differ. D0.b gets the byte read and A1 that pointer. */
static void sub_12FF36(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);                             /* 0x12FF36 */
    r->a[1] = a1;
    uint8_t c = rd8(m, a1 + 0x1Du);                                     /* 0x12FF3A */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | c;
    if (c != rd8(m, a1 + 0x4Fu))                                        /* 0x12FF3E */
        wr8(m, obj + 0x1Du, c);                                         /* 0x12FF50 */
    else
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));                       /* 0x12FF46 */
}

int Fn_1319FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u) {                      /* 0x1319FE btst #0 */
        batch_free_object(m, r, 0x000131AF2u);                          /* 0x131AEA */
        return 0;
    }

    uint32_t tbl = rd32(m, obj + 0x0Cu);                                /* 0x131A08 */
    r->a[1] = tbl;
    anim_step(m, obj, tbl, &r->d[0], &r->d[1]);                         /* 0x131A0C */
    uint16_t entry = rd16(m, tbl + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]);
    if ((int16_t)entry < 0) {                                           /* 0x131A12 blt */
        batch_free_object(m, r, 0x000131AF2u);                          /* 0x131AEA */
        return 0;
    }

    if ((int16_t)entry == 0) {                                          /* 0x131A16 bgt folds */
        r->d[0] = 0;                                                    /* 0x131A1E moveq #0 */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, obj + 0x3Eu);       /* 0x131A20 */
        if ((uint16_t)r->d[0] == 0u)                                    /* 0x131A24 */
            Sound_PlayHooked(m, 0x001Bu);                               /* 0x131A28..0x131A38 */

        if ((int16_t)(uint16_t)r->d[0] >= 0x0021) {                     /* 0x131A3C blt */
            uint32_t d0 = r->d[0];
            uint32_t quot = d0 / 12u;
            uint32_t res = (quot > 0xFFFFu) ? d0 : (((d0 % 12u) << 16) | quot);
            res = (res << 16) | (res >> 16);                            /* 0x131A48 swap */
            r->d[0] = res;
            if ((uint16_t)res == 0u) {                                  /* 0x131A4A */
                uint32_t arg = rd32(m, obj + 0x36u);                    /* 0x131A50 */
                r->d[1] = arg;
                r->d[0] = Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, r->d[1]); /* 0x131A54..0x131A6A */
                r->d[1] = (r->d[1] & 0xFFFF0000u)
                        | (uint16_t)((uint16_t)r->d[1] + 0x0800u);      /* 0x131A6E addi.w */
                r->d[0] = Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, r->d[1]); /* 0x131A72..0x131A88 */
                r->d[1] = (r->d[1] & 0xFFFF0000u)
                        | (uint16_t)((uint16_t)r->d[1] - 0x1000u);      /* 0x131A8C subi.w */
                r->d[0] = Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, r->d[1]); /* 0x131A90..0x131AA6 */
            }
        }

        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);           /* 0x131AAA */
        wr16(m, obj + 0x3Eu, cnt);
        if (cnt == 0x00DCu) {                                           /* 0x131AAE */
            wr16(m, obj + 0x3Eu, 0u);                                   /* 0x131AB8 */
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u)); /* 0x131ABC */
        }
    }

    /* 0x131AC0: position = parent + offset. */
    uint32_t parent = rd32(m, obj + 0x08u);
    r->a[1] = parent;
    uint16_t x = (uint16_t)(rd16(m, parent + 0x14u) + rd16(m, obj + 0x2Cu));
    wr16(m, obj + 0x14u, x);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | x;
    uint16_t y = (uint16_t)(rd16(m, parent + 0x18u) + rd16(m, obj + 0x30u));
    wr16(m, obj + 0x18u, y);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | y;

    sub_12FF36(m, r, obj);                                              /* 0x131ADC bsr */
    batch_draw_sprite(m, r, 0x000131AE6u);                              /* 0x131AE0 */
    wr32(m, obj, 0x001319FEu);                                          /* 0x1319F6 */
    return 0;
}
