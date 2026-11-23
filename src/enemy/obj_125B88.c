#include "obj_125B88.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../kernel/rng.h"

/* ROM 0x11C32C: blink the palette index at +0x1D between 6 and 8 while +0x35
 * is set (the first toggle clears +0x35); otherwise copy +0x4F. */
static void sub_11C32C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        goto restore_11c356;                           /* 0x11C330 */
    if (rd8(m, obj + 0x1Du) != 0x06u) {
        wr8(m, obj + 0x1Du, 0x06u);                    /* 0x11C34C */
        return;
    }
    wr8(m, obj + 0x35u, 0u);                           /* sf (0x35,A0) */
    wr8(m, obj + 0x1Du, 0x08u);                        /* 0x11C342 */
    return;

restore_11c356:
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));          /* 0x11C356 */
}

/* ROM 0x0CC33A: angle from (-D0,-D1) to the scroll block's box centre. With no
 * block it is the fixed angle 0x80. */
static uint16_t sub_0CC33A(const f3mem_t *m, uint32_t obj, uint16_t d0w, uint16_t d1w)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
    if (a1 == 0u)
        return 0x0080u;                                /* 0x0CC342 */

    d0w = (uint16_t)(0u - d0w);                        /* neg.w D0w */
    uint16_t t = (uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu));
    t = (uint16_t)((int16_t)t >> 1);                   /* asr.w #1 */
    d0w = (uint16_t)(d0w + t);
    d0w = (uint16_t)(d0w + rd16(m, a1 + 0x14u));
    d0w = (uint16_t)(d0w - rd16(m, obj + 0x14u));      /* 0x0CC362 */

    d1w = (uint16_t)(0u - d1w);                        /* neg.w D1w */
    t = (uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u));
    t = (uint16_t)((int16_t)t >> 1);
    d1w = (uint16_t)(d1w + t);
    d1w = (uint16_t)(d1w + rd16(m, a1 + 0x18u));
    d1w = (uint16_t)(d1w - rd16(m, obj + 0x18u));      /* 0x0CC378 */

    return Obj_Atan2(m, d0w, d1w);                     /* 0x0CC37C */
}

/* ROM 0x125B88: the 0x125CA8 routed enemy. While its HP (+0x24) lasts it
 * animates from the 0x125D08 table; once the scroll flag A5+0x1D58 bit 1 is
 * set and it is on screen it parks the palette, adds its timer +0x4C to the
 * clock of the object at A5+0x1D50 and moves along the byte at +0x20. Off
 * screen (or off the flag) it keeps counting +0x06 down and then drops four
 * 0x0CCD5C children before switching to 0x125CA8. */
int Obj_125B88_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x125B88: btst.b #4,(-0x144B,A5) ; bne 0x125CBC */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto stop_125cbc;
    /* 0x125B92: tst.l (0x24,A0) ; ble 0x125CAC */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto drop_125cac;

    Obj_Animate(m, obj, 0x00125D08u);                  /* 0x125BA0: lea 0x16C(PC) */

    /* 0x125BA6: on-screen gate */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u ||
        (int16_t)rd16(m, obj + 0x14u) < -0x0A ||
        (int16_t)rd16(m, obj + 0x14u) > 0x011C)
        goto move_125bca;

    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));          /* 0x125BBE */
    /* 0x125BC4: jmp 0x0CBD12 = add the timer to A5+0x1D50's clock, then the
     * 0x0CBB2C explosion tick */
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }
    Obj_ExplodeTick(m, obj);

move_125bca:
    /* 0x125BCA: move.w (0x20,A0),D0w ; lsr.w #8 ; jsr 0x0CBE12 */
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));

    /* 0x125BD6: jsr 0x0CBDA4 (append to A5+0x8FE and A5+0x906) */
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
    }

    sub_11C32C(m, obj);                                /* 0x125BDC */

    Obj_DrawSprite(m, obj);                            /* 0x125BE2: jsr 0x1282 */

    /* 0x125BE8: cmp2.w (0x0CC112),D0w on +0x14 (bounds 0xFFFFFFE0..
     * 0x00000140) and +0x18 (0xFFFFFFE0..0x000000E8); bcs 0x125C10 when the
     * value is outside */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        if ((int16_t)(x - (int16_t)0xFFE0u) < 0 || (int16_t)(0x0140u - x) < 0)
            goto spawn_125c10;
        if ((int16_t)(y - (int16_t)0xFFE0u) < 0 || (int16_t)(0x00E8u - y) < 0)
            goto spawn_125c10;
        uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, n);
        if ((int16_t)n > 0) {
            wr32(m, obj + 0x00u, 0x00125B88u);         /* 0x125B80 */
            return 0;
        }
    }

spawn_125c10:
    /* 0x125C10: D0w = D1w = 0x10 -> sub_0CC33A, angle << 8 + 0x1380 packed
     * into the argument long of four 0x0CCD5C children (group 9, 2 slots),
     * each 0xD00 below the previous */
    {
        uint16_t ang = sub_0CC33A(m, obj, 0x0010u, 0x0010u);
        uint16_t arg = (uint16_t)((uint16_t)(ang << 8) + 0x1380u); /* 0x125C1E */
        for (int i = 0; i < 4; i++) {
            uint32_t d1 = 0x10100000u;                 /* move.l #0x10100000,D1 */
            d1 = (d1 & 0xFFFF0000u) | (uint32_t)arg;   /* 0x125C2A: move.w D0w,D1w */
            Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, d1);
            arg = (uint16_t)(arg - 0x0D00u);           /* 0x125C46 */
        }
    }
    wr32(m, obj + 0x00u, 0x00125CA8u);                 /* 0x125CA0 */
    return 0;

drop_125cac:
    /* 0x125CAC: jsr 0x0CBDDE (A1 = +0x36, or its +0x8 when +0x35 is 1/2),
     * then add the +0x4C timer to the clock of the object in A1 */
    {
        uint32_t a1 = rd32(m, obj + 0x36u);
        uint8_t b = rd8(m, obj + 0x35u);
        if (b == 0x02u || b == 0x01u)
            a1 = rd32(m, a1 + 0x08u);
        /* 0x125CB4: move.w (0x4C,A0),D0w ; add.l D0,(0x48,A1) */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }
    /* falls through to 0x125CBC */

stop_125cbc:
    /* 0x125CBC: sound 0x35, then spawn a 0x13E4EE with the packed zoom */
    Sound_PlayHooked(m, 0x0035u);
    {
        uint32_t sprdef = rd32(m, obj + 0x10u);
        uint16_t d1w = (uint16_t)((uint16_t)rd16(m, sprdef) << 3);
        uint32_t d0 = (uint32_t)((d1w & 0xFFu) << 8);         /* bfins {8:8} */
        d1w = (uint16_t)((uint16_t)rd16(m, sprdef + 2u) << 3);
        d0 = (d0 & 0xFFFFFF00u) | (uint32_t)(d1w & 0xFFu);    /* bfins {0:8} */
        /* 0x125CDE: move.w #0,-(SP) is the group, matching the 0-relative pool
         * base at 0x410060 that the routine writes; the next word is the slot count */
        Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0);           /* 0x125CE8 */
    }
    wr32(m, obj + 0x00u, 0x00125CFAu);                 /* 0x125CF2 */
    return 0;
}
