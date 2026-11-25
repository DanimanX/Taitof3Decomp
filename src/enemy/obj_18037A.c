#include "obj_18037A.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"

/* ROM 0x18037A: script-driven enemy tick. Walks the 0x178982 movement script,
 * frees itself once +0x50/+0x54 leaves the 0x1849F8..0x1849FE play area, runs
 * the per-state handler from the two-entry table 0x179A5C (+0x40 selects the
 * pair, +0x03 selects the entry), offsets the drawn position by the sprite
 * def's half size, picks a colour override (+0x1D) from +0x35/+0x22, publishes
 * itself to the two active lists and draws the sprite. With the A5+0x1D58
 * explosion flag bit 1 set it adds its 32-bit score (+0x62) to the relay
 * timer and switches to the explosion tick 0x0CBB2C. */

#define LAB_180372 0x0018037Au  /* 0x180372 writes #0x18037A */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x180418..0x180440: draw, publish to A5+0x906 / A5+0x8FE, then either
 * explode (timer += (0x62,A0).l, +0x1D := +0x22, jmp 0x0CBB2C) or re-arm
 * 0x180372. */
static void lab_180418(const f3mem_t *m, uint32_t obj)
{
    /* 0x180418: jsr 0x1282 (Obj_DrawSprite) */
    Obj_DrawSprite(m, obj);

    /* 0x18041E: btst.b #1,(0x1D58,A5) ; beq.b 0x180440 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        /* 0x180426: movea.l (0x1D50,A5),A1 ;
         * bfextu (0x62,A0){0:16},D0 ; add.l D0,(0x48,A1)
         * (the extend word is the full 32-bit score; count 16 takes the high
         * word, i.e. the whole long since +0x62 and +0x60 are one field) */
        uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
        uint32_t d0 = rd32(m, obj + 0x62u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);

        /* 0x180434: move.b (0x22,A0),(0x1D,A0) ; jmp 0x0CBB2C */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));
        Obj_ExplodeTick(m, obj);
        return;
    }

    /* 0x180440: bra.w 0x180372 */
    wr32(m, obj + 0x00u, LAB_180372);
}

int Obj_18037A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18037A: jsr 0x178982 -- one movement-script step; a step Obj_Script10
     * cannot resolve is reported as -1, before any side effect below */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* 0x180380: cmp2.w (0x1849F8),-0x10 <= +0x50 <= 0x14F ; bcs -> 0x17C78E */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu)) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x180390: cmp2.w (0x1849FC),-0x10 <= +0x54 <= 0xF7 ; bcs -> 0x17C78E */
    {
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x1803A0: move.w (0x50,A0),(0x14,A0) ; move.w (0x54,A0),(0x18,A0) */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x1803AC: move.w (0x40,A0),D0w ; lea 0x179A5C,A1 ;
     * movea.l (0x4,A1,D0w*8),A1 -- +0x40 counts PAIRS of 8 bytes, the entry
     * sits at +4 of the pair (dispatch slot); the +0 slot is the armed
     * address. jsr (A1) */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x1803BC: A1 = (0x10,A0) ; D1w = (A1)+ ; D0w = (A1)+ ;
     * asl.w #3,D0w ; asl.w #3,D1w ;
     * sub.w D0w,(0x14,A0) ; sub.w D1w,(0x18,A0) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = rd16(m, a1);
        uint16_t d0 = rd16(m, a1 + 2u);
        d0 = (uint16_t)(d0 << 3);
        d1 = (uint16_t)(d1 << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    /* 0x1803D0: cmpi.b #6,(0x1D,A0) ; beq.w 0x1803F6 */
    if (rd8(m, obj + 0x1Du) == 0x06u) {
        /* 0x1803F6: move.b #8,(0x1D,A0) ; bra.w 0x180400 */
        wr8(m, obj + 0x1Du, 0x08u);
    } else if (rd8(m, obj + 0x35u) != 0u) {
        /* 0x1803DA: tst.b (0x35,A0) ; bne.w 0x1803EC
         * 0x1803EC: move.b #6,(0x1D,A0) ; bra.w 0x180400 */
        wr8(m, obj + 0x1Du, 0x06u);
    } else {
        /* 0x1803E2: move.b (0x22,A0),(0x1D,A0) ; bra.w 0x180400 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));
    }

    /* 0x180400: clr.b (0x35,A0) */
    wr8(m, obj + 0x35u, 0x00u);

    /* 0x180404: A1 = (0x906,A5) ; move.l A0,(A1)+ ; move.l A1,(0x906,A5) */
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
    }

    /* 0x18040E: A1 = (0x8FE,A5) ; move.l A0,(A1)+ ; move.l A1,(0x8FE,A5) */
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    }

    lab_180418(m, obj);
    return 0;
}
