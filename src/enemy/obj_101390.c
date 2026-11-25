#include "obj_101390.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"
#include "obj_E8B72.h"

/* ROM 0x101390: a falling/descending object. While the bomb flag (A5+0x1D58
 * bit 1) is set and +0x14 is inside [-0xA, 0x11C] it copies +0x4F into the
 * blink byte +0x1D and jumps to the 0x101412 body, which spawns the visual
 * halves; otherwise, with the time counter +0x24 still positive and the pause
 * flag (A5-0x144B bit 5) clear, it advances +0x3A/+0x3E (the latter by the
 * scroll speed A5+0x4228), flags itself off-screen and runs the script,
 * animation, hazard-list, blink and sprite steps. If flag7 bit 3 is set it
 * hands over to 0x0CEE74 (start dying) instead of the draw tail.
 *
 * 0x101412: 0x13F238 + 0x0039D8 play sound 0x1B, then in groups 5..6 spawn two
 * 0x100CF8 halves at +0x100000/+0x200000 from the object's +0x14/+0x18, each
 * inheriting +0x60, +0x4E and a +0x58 split 0xF either side of 0x168, and
 * always start dying. */

/* ROM 0x101412: body reached both from the bomb/on-screen shortcut and from
 * the negative time counter. Returns 0 or -1. D1/D2/D3w/D4b/D5w live in
 * registers here: D5w is updated between the two spawns and reused. */
static int lab_101412(const f3mem_t *m, uint32_t obj)
{
    Sound_PlayHooked(m, 0x001Bu);               /* 0x101412 (0x13F238 + 0x0039D8) */

    /* 0x101426: move.l (0x14,A0),D1 ; addi.l #0x100000,D1 */
    uint32_t d1 = rd32(m, obj + 0x14u) + 0x00100000u;
    /* 0x101430: move.l (0x18,A0),D2 ; addi.l #0x200000,D2 */
    uint32_t d2 = rd32(m, obj + 0x18u) + 0x00200000u;
    /* 0x10143A: move.w (0x60,A0),D3w */
    uint16_t d3w = rd16(m, obj + 0x60u);
    /* 0x10143E: move.b (0x4E,A0),D4b */
    uint8_t d4b = rd8(m, obj + 0x4Eu);
    /* 0x101442: move.w (0x58,A0),D5w */
    uint16_t d5w = rd16(m, obj + 0x58u);

    /* 0x101446: pea #0 ; move.w #2 ; move.w #5 ; move.l #0x100CF8 ; jsr 0x1057C */
    uint32_t child = Obj_Spawn(m, obj, 0x00100CF8u, 0x0005u, 0x0002u, 0u);
    /* 0x101462: tst.l D0 ; bmi -> 0x0CEE74 (no half-spawned fields written) */
    if ((int32_t)child < 0)
        goto dying;
    /* 0x10146A: movea.l D0,A1 */
    wr32(m, child + 0x14u, d1);
    wr32(m, child + 0x18u, d2);
    wr16(m, child + 0x60u, d3w);
    wr8(m, child + 0x4Eu, d4b);
    /* 0x10147C: move.w D5w,D0w ; subi.w #0xF,D0w ; bpl 0x10148A ;
     *         addi.w #0x168,D0w */
    {
        uint16_t d0w = (uint16_t)(d5w - 0x000Fu);
        if ((int16_t)d0w < 0)
            d0w = (uint16_t)(d0w + 0x0168u);
        /* 0x10148A: move.w D0w,(0x58,A1) */
        wr16(m, child + 0x58u, d0w);
    }

    /* 0x10148E: pea #0 ; move.w #2 ; move.w #5 ; move.l #0x100CF8 ; jsr 0x1057C */
    child = Obj_Spawn(m, obj, 0x00100CF8u, 0x0005u, 0x0002u, 0u);
    /* 0x1014AA: tst.l D0 ; bmi -> 0x0CEE74 */
    if ((int32_t)child < 0)
        goto dying;
    wr32(m, child + 0x14u, d1);
    wr32(m, child + 0x18u, d2);
    wr16(m, child + 0x60u, d3w);
    wr8(m, child + 0x4Eu, d4b);
    /* 0x1014C4: addi.w #0xF,D5w ; cmpi.w #0x168,D5w ; bcs 0x1014D4 ;
     *         subi.w #0x168,D5w */
    d5w = (uint16_t)(d5w + 0x000Fu);
    if (d5w >= 0x0168u)
        d5w = (uint16_t)(d5w - 0x0168u);
    /* 0x1014D4: move.w D5w,(0x58,A1) */
    wr16(m, child + 0x58u, d5w);

    /* 0x1014D8: bra.l 0x0CEE74 */
    Obj_StartDying(m, obj);
    return 0;

dying:
    /* 0x1013E6 / 0x101464 / 0x1014AC: jump to 0x0CEE74 */
    Obj_StartDying(m, obj);
    return 0;
}

int Obj_101390_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x101390: btst.b #1,(0x1D58,A5) ; beq 0x1013B4 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        /* 0x101398: cmpi.w #-0xA,(0x14,A0) ; blt 0x1013B4 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        /* 0x1013A0: cmpi.w #0x11C,(0x14,A0) ; bgt 0x1013B4 */
        if (x < -0x000A || x > 0x011C)
            goto lab_1013b4;
        /* 0x1013A8: move.b (0x4F,A0),(0x1D,A0) ; 0x1013AE jmp 0x101412 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        return lab_101412(m, obj);
    }

lab_1013b4:
    /* 0x1013B4: tst.l (0x24,A0) ; ble.w 0x101412 */
    if (!((int32_t)rd32(m, obj + 0x24u) > 0))
        return lab_101412(m, obj);
    /* 0x1013BC: btst.b #5,(-0x144B,A5) ; bne.w 0x101412 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        return lab_101412(m, obj);

    /* 0x1013C6: move.l (0x50,A0),D0 ; add.l D0,(0x3A,A0) */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
    /* 0x1013CE: move.l (0x54,A0),D0 ; sub.l (0x4228,A5),D0 ;
     *         add.l D0,(0x3E,A0) */
    wr32(m, obj + 0x3Eu,
         rd32(m, obj + 0x3Eu) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));

    /* 0x1013DA: jsr 0x0CF85A */
    Obj_FlagOffscreen(m, obj);
    /* 0x1013E0: btst.b #3,(0x7,A0) ; bne.l 0x0CEE74 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)
        return lab_101412(m, obj);

    /* 0x1013EC: move.l (0x3A,A0),(0x14,A0) */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    /* 0x1013F2: move.l (0x3E,A0),(0x18,A0) */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    /* 0x1013F8: jsr 0x0D0690 */
    Obj_StepScript(m, obj);
    /* 0x1013FE: jsr 0x0D0DC2 */
    Obj_0D0DC2(m, obj);
    /* 0x101404: jsr 0x0CBE34 */
    Obj_BlinkColour(m, obj);
    /* 0x10140A: jsr 0x001282 ; 0x101410 rts */
    Obj_DrawSprite(m, obj);
    return 0;
}
