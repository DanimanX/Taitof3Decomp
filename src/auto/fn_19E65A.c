/* ROM 0x19E65A: boss/enemy scripted-path tick (A0 = object).
 * If +0x54 > 0x12C or A5+0x4B38 == -1, transitions to 0x17AADA (sets +0x23
 * bit 2 and +0x06 = 1). Otherwise steps 10-byte movement script (0x178982),
 * copies +0x50/+0x54 to +0x14/+0x18, ticks 0x179A5C anim (+0x40), subtracts
 * (+0x10[0..2] << 3) hotspot offset, draws sprite (0x001282), optionally
 * spawns 0x179F26 (0x01057C) when A5+0x1D58 bit 0 and A5+0x0879 bits 0..1
 * are clear, and re-arms 0x19E65A. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../video/sprite.h"

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = (*d1_io & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x1795CA (0x179A5C index 13 init): 18-byte record {dur.w, spr.l, pal.w, 2c.l, 30.l, dx.b, dy.b}. */
static void sub_1795CA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    int16_t dx = (int16_t)(int8_t)rd8(m, a1);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 1u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
}

/* ROM 0x17980E (0x179A5C index 13 tick). */
static int sub_17980E(const f3mem_t *m, uint32_t obj)
{
    int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);
    if (timer == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + 0x12u;
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            wr32(m, obj + 0x3Au, a1);
            sub_1795CA(m, obj);
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        }
        if (d == 0) {
            a1 = rd32(m, a1 + 2u);
            uint16_t d0w = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, d0w);
            wr32(m, obj + 0x3Au, a1);
            if (d0w == 13u)
                sub_1795CA(m, obj);
            else if (Obj_AnimInit(m, obj, d0w) < 0)
                return -1;
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        }
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    } else if (timer > 0) {
        wr16(m, obj + 0x3Eu, (uint16_t)(timer - 1));
    }
    uint32_t a1 = rd32(m, obj + 0x3Au);
    int16_t dx = (int16_t)(int8_t)rd8(m, a1 + 0x10u);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 0x11u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
    return 0;
}

int Fn_19E65A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int16_t)rd16(m, obj + 0x54u) > 0x012C ||             /* 0x19E5CC */
        (int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {          /* 0x19E5D8 */
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x04u)); /* 0x17AAC4 */
        wr16(m, obj + 0x06u, 1u);                           /* 0x17AACA (beq never taken) */
        wr32(m, obj + 0x00u, 0x0017AADAu);                  /* 0x17AAD2 */
        return 0;
    }

    if (Obj_Script10(m, obj) < 0)                           /* 0x19E5E4 -> 0x178982 */
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));             /* 0x19E5EA */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));             /* 0x19E5F0 */

    uint16_t anim_idx = rd16(m, obj + 0x40u);               /* 0x19E5F6 */
    if (anim_idx == 13u) {                                  /* 0x19E5FA..0x19E604 */
        if (sub_17980E(m, obj) < 0)
            return -1;
    } else if (Obj_AnimTick(m, obj, anim_idx) < 0) {
        return -1;
    }

    a1 = rd32(m, obj + 0x10u);                              /* 0x19E606 */
    int16_t dy = (int16_t)(rd16(m, a1) << 3);               /* 0x19E60A, 0x19E610 */
    a1 += 2u;
    int16_t dx = (int16_t)(rd16(m, a1) << 3);               /* 0x19E60C, 0x19E60E */
    a1 += 2u;
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - dx)); /* 0x19E612 */
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - dy)); /* 0x19E616 */

    draw_sprite_regs(m, obj, &d0, &d1, &a1);                /* 0x19E61A -> 0x001282 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x01u) == 0u &&          /* 0x19E620 */
        (rd16(m, F3_A5 + 0x0878u) & 0x0003u) == 0u) {       /* 0x19E62A */
        d0 = Obj_Spawn(m, obj, 0x00179F26u, 5u, 2u, 0u);    /* 0x19E634..0x19E648 */
    }

    wr32(m, obj + 0x00u, 0x0019E65Au);                      /* 0x19E652 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
