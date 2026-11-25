#include "obj_17BE1C.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../audio/sound_cmd.h"
#include "../video/sprite.h"

/* ROM 0x17BE1C: the scripted mover's live tick. Bit 4 of A5-0x144C zeroes the
 * hit counter (+0x24); while nothing is left of it the object jumps to its own
 * death handler in +0x66. Otherwise it steps the 10-byte movement script at
 * +0x44, gives up (0x17C78E) once +0x50/+0x54 leave the 0x1849F8 region,
 * mirrors the offset into the screen position, ticks the animation and centres
 * the sprite, appends itself to the hazard list at A5+0x906, draws, and (in the
 * second-player mode, A5+0x1D58 bit 1) explodes when it is on screen. Every
 * completed path re-latches 0x17BE1C through 0x17BE14. */

/* ROM 0x17BE14: re-latch this routine */
static void lab_17BE14(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0017BE1Cu);
}

/* ROM 0x17C704: the destruction tail of these bodies -- award the score in
 * +0x62 to the receiver +0x35 names, clear the hit-colour override, play sound
 * 0x35 and hand over to 0x17C614, which runs the movement script, animates,
 * centres, and either frees (+0x3E < 0) or draws and latches 0x17C660. */
static int lab_17C704(const f3mem_t *m, uint32_t obj)
{
    /* 0x17C704: reward(0x62) through 0x17B02E: 1 -> (+0x36)->+0x8, 2 -> +0x36 */
    {
        uint32_t d1 = rd16(m, obj + 0x62u);
        uint32_t rcv = 0xFFFFFFFFu;
        uint8_t k = rd8(m, obj + 0x35u);
        if (k == 1u)
            rcv = rd32(m, rd32(m, obj + 0x36u) + 8u);
        else if (k == 2u)
            rcv = rd32(m, obj + 0x36u);
        if ((int32_t)rcv >= 0)
            wr32(m, rcv + 0x48u, rd32(m, rcv + 0x48u) + d1);
    }
    wr8(m, obj + 0x1Du, 0u);                           /* 0x17C722 */
    {
        int16_t cmd = (int16_t)0x0035;
        if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
            cmd = (int16_t)(cmd + 0x53);
        Sound_PlayCmd(m, cmd);                         /* 0x17C726 jsr 0x13F238 */
    }

    /* 0x17C73A bra.w 0x17C614 */
    if (Obj_Script10(m, obj) < 0)
        return -1;
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0017C660u);                 /* 0x17C61A */
    return 0;
}

/* ROM 0x17C59A: destroyed -- award the score, then start the explosion. The
 * shrink-away animation at 0x17C662 is where this capture's object goes. */
static int lab_17C59A(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x34u) & 0x02u) != 0u)
        return -1;                                     /* 0x17C5A0 -> 0x17C662 */
    return -1;                                         /* 0x17C5AA jsr 0x17B02E */
}

/* ROM 0x17C78E: off-screen, free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x17E0B4: death handler -- three 0x17E6B6 fragments, sound 0x2A */
static void lab_17E0B4(const f3mem_t *m, uint32_t obj)
{
    /* 0x17E0B4: bfextu (0x20,A0){0:8},D1, then three children with arg
     * b-0x20, b, b+0x20 (group 9, two groups) */
    uint8_t d1 = rd8(m, obj + 0x20u);
    for (int i = 0; i < 3; i++) {
        Obj_Spawn(m, obj, 0x0017E6B6u, 9u, 2u, (uint32_t)d1);
        d1 = (uint8_t)(d1 + 0x20u);
    }
    /* 0x17E0C6: pea (0x2A).w ; jsr 0x13F238 (+0x53 in the alternate mode) */
    {
        int16_t cmd = (int16_t)0x002Au;
        if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
            cmd = (int16_t)(cmd + 0x53);
        Sound_PlayCmd(m, cmd);
    }
    wr16(m, obj + 0x06u, 1u);
    wr32(m, obj + 0x00u, 0x0017E108u);
}

int Obj_17BE1C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17BE1C: btst.b #4,-0x144C(A5) ; beq.w 0x17BE2A */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)
        wr32(m, obj + 0x24u, 0u);                      /* 0x17BE26 clr.l */

    /* 0x17BE2A: tst.l (0x24,A0) ; bgt.w 0x17BE38 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x17BE32: movea.l (0x66,A0),A1 ; jmp (A1). 0x17E0B4 is the burst
         * handler; 0x17C59A needs 0x17B02E, which has no translation. */
        switch (rd32(m, obj + 0x66u)) {
        case 0x0017E0B4u:
            lab_17E0B4(m, obj);
            return 0;
        case 0x0017C704u:
            return lab_17C704(m, obj);
        case 0x0017C59Au:
            return lab_17C59A(m, obj);
        default:
            return -1;
        }
    }

    /* 0x17BE38: jsr 0x178982 -- one pass of the movement script */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* 0x17BE3E..0x17BE5A: cmp2.w (0x1849F8)/(0x1849FC) ; bcs 0x17C78E */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        int okx = x > (int16_t)rd16(m, 0x001849F8u) && x < (int16_t)rd16(m, 0x001849FAu);
        int oky = y > (int16_t)rd16(m, 0x001849FCu) && y < (int16_t)rd16(m, 0x001849FEu);
        if (!okx || !oky) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x17BE5E: mirror the parent-relative offset into the screen position */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17BE6A: move.w (0x40,A0),D0w ; movea.l (0x4,0x179A5C,D0w),A1 ; jsr */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17BE7A: centre by the frame's own (h, w), each << 3 */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    /* 0x17BE8E: append to the hazard list at A5+0x906 */
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x906u, a1 + 4u);
    }

    Obj_DrawSprite(m, obj);                            /* 0x17BE98 */

    /* 0x17BE9E: btst.b #1,(0x1D58,A5) ; beq.b 0x17BEBC */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        /* 0x17BEA6 / 0x17BEAE: cmpi.w #-10 / #0x11C on +0x14 */
        if (x >= -10 && x <= 0x11C) {
            /* 0x17BEB6: jmp 0x0CBB2C -- explosion tick, not translated yet */
            return -1;
        }
    }

    lab_17BE14(m, obj);                                /* 0x17BEBC bra.w */
    return 0;
}
