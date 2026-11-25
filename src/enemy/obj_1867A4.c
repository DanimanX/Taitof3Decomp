/* ROM 0x1867A4: player-bound orbiting sprite. 0x17B92C drains the object's
 * pending velocity into its host (+0x8) when +0x35 is armed, then the tick
 * either hands the object to 0x18679C on the retire signal (A5+0x4B38 < 0) or
 * runs the frame script at (0xC,A0)->+0x14, offsets the object by the
 * position the host carries, emits the sprite, and appends itself to the two
 * visible lists (A5+0x8FE / A5+0x906). */
#include "obj_1867A4.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x179434: map the object's absolute position (0x14/0x18 hold
 * 0x9000/0x8200 as the high half) back to a screen-relative one through the
 * remaining velocity in +0x58. */
static void sub_179434(const f3mem_t *m, uint32_t obj)
{
    uint32_t d1 = rd32(m, obj + 0x58u) & 0x00FFFF00u;
    if (d1 == 0u)
        return;
    d1 += 0x00010000u;
    d1 *= 1u;
    {
        int32_t d0 = (int32_t)rd32(m, obj + 0x18u) - 0x00820000;
        d0 /= (int32_t)d1;
        wr16(m, obj + 0x18u, (uint16_t)(d0 + 0x82));
    }
    {
        int32_t d0 = (int32_t)rd32(m, obj + 0x14u) - 0x00900000;
        d0 /= (int32_t)d1;
        wr16(m, obj + 0x14u, (uint16_t)(d0 + 0x90));
    }
}

/* ROM 0x17B92C: if the object carries a pending delta (+0x35 != 0) hand it to
 * the host at +0x8, scaled by the difficulty byte for attacker classes 1/2. */
static void sub_17B92C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    {
        uint32_t d0 = rd32(m, obj + 0x24u);
        if (rd8(m, obj + 0x35u) != 2u) {
            uint32_t a1 = rd32(m, obj + 0x36u);
            uint8_t k = rd8(m, a1 + 0x22u);
            if (k == 1u) {
                d0 = (uint32_t)((int32_t)d0 / 0x64);
                d0 *= (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 + 0x4B2Au);
            } else if (k == 2u) {
                d0 = (uint32_t)((int32_t)d0 / 0x64);
                d0 *= (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 + 0x4B2Cu);
            }
        } else {
            d0 = (uint32_t)((int32_t)d0 / 0x64);
            d0 *= (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 + 0x4B2Eu);
        }
        {
            uint32_t a4 = rd32(m, obj + 0x08u);
            wr32(m, a4 + 0x24u, rd32(m, a4 + 0x24u) + d0);
            wr32(m, obj + 0x24u, 0u);
            wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
            wr16(m, a4 + 0x34u, rd16(m, obj + 0x34u));
        }
    }
}

/* ROM 0x17BA02: spawn a hit spark (0x17A016, group 5 of 2) at the contact
 * point when the hit test finds one. */
static int sub_17BA02(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return 0;
    {
        uint16_t d0, d1;
        /* loc_17BA76: contact point between obj and its host (+0x36) */
        uint32_t a1 = rd32(m, obj + 0x36u);
        uint16_t d2, d3;
        if (rd8(m, obj + 0x35u) == 2u) {
            if ((rd16(m, F3_A5 + 0x0878u) % 3u) != 0u)
                return 0;                       /* 0x17BB78: moveq #-1,D0 */
            d2 = (uint16_t)((uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)) >> 1);
            d3 = (uint16_t)((uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u)) >> 1);
            {
                uint16_t r = Obj_RandRange(m, d3);
                d1 = (uint16_t)(r + rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u) + d3);
                r = Obj_RandRange(m, d2);
                d0 = (uint16_t)(r + rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu) + d2);
            }
        } else {
            uint8_t k = rd8(m, a1 + 0x22u);
            if (k != 0u && k != 4u && (rd16(m, F3_A5 + 0x0878u) & 3u) != 0u)
                return 0;
            {
                int16_t x0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu));
                int16_t y0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u));
                int16_t x1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Cu));
                int16_t y1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x30u));
                int16_t lx = x1 > x0 ? x1 : x0, ly = y1 > y0 ? y1 : y0;
                int16_t hx, hy;
                x0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Eu));
                y0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x32u));
                x1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Eu));
                y1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x32u));
                hx = x1 < x0 ? x1 : x0;
                hy = y1 < y0 ? y1 : y0;
                d0 = (uint16_t)(lx + ((uint16_t)(hx - lx) >> 1));
                d1 = (uint16_t)(ly + ((uint16_t)(hy - ly) >> 1));
            }
        }
        if ((int16_t)d0 < 0)
            return 0;
        d0 = (uint16_t)(d0 - rd16(m, obj + 0x50u));
        d1 = (uint16_t)(d1 - rd16(m, obj + 0x54u));
        Obj_Spawn(m, obj, 0x0017A016u, 5u, 2u, ((uint32_t)d0 << 16) | d1);
    }
    return 0;
}

/* ROM 0x1867A4 */
int Obj_1867A4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1867A4: movea.l (0xC,A0),A3 ; jsr 0x17B92C */
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    sub_17B92C(m, obj);

    /* 0x1867AE: tst.b (0x35,A0) ; beq 0x1867C6 -- hand the hit info to the
     * host one more time */
    if (rd8(m, obj + 0x35u) != 0u) {
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
        wr16(m, a4 + 0x34u, rd16(m, obj + 0x34u));
    }

    /* 0x1867C6: cmpi.w #-1,(0x4B38,A5) ; beq 0x186868 -- retired */
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {
        /* 0x186868: move.l A0,-(SP) ; jsr 0x105EC (Obj_Free) */
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x1867D0: movea.l (0x14,A3),A1 ; jsr (A1) -- per-record behaviour of
     * the state block; the only one this family installs is the frame-script
     * tick the object already runs through 0x179A5C, so it is dispatched by
     * the object's script index (see src/objects/obj_anim.h). */
    (void)a3;
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x1867D6: follow the host position plus the local offsets */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, d0);
        wr16(m, obj + 0x54u, d1);
        wr16(m, obj + 0x14u, d0);
        wr16(m, obj + 0x18u, d1);
        wr16(m, obj + 0x1Eu, rd16(m, a4 + 0x1Eu));
        wr32(m, obj + 0x58u, rd32(m, a4 + 0x58u));
    }

    /* 0x186806: frame-list tick through 0x179A5C, then 0x179434 */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x186816: subtract the sprite anchor (0x8 steps) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = rd16(m, a1), d0 = rd16(m, a1 + 2u);
        d0 = (uint16_t)(d0 << 3);
        d1 = (uint16_t)(d1 << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }
    sub_179434(m, obj);

    /* 0x186830: jsr 0x17BA02 */
    sub_17BA02(m, obj);

    /* 0x186836: colour + clear the armed flag */
    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    wr8(m, obj + 0x35u, 0u);

    /* 0x186840: jsr 0x1282 (Obj_DrawSprite) */
    Obj_DrawSprite(m, obj);

    /* 0x186846: bftst (0x4B3A,A5){0xf:1} ; bne 0x186864 */
    if ((rd8(m, F3_A5 + 0x4B3Au + 1u) & 0x01u) != 0u)
        return 0;                            /* 0x186864: bra 0x18679C */

    /* 0x186850: append to A5+0x8FE and A5+0x906 */
    {
        uint32_t p = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x08FEu, p + 4u);
        p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0906u, p + 4u);
    }

    /* 0x186864: bra 0x18679C -> move.l #0x1867A4,(A0) ; rts */
    wr32(m, obj, 0x001867A4u);
    return 0;
}
