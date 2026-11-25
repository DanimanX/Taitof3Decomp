#include "obj_E78EC.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "obj_E8B72.h"

/* ROM 0x0E78EC: falling/descending enemy. While its script link (+0x8, byte
 * +0x6) is alive it copies the link's +0x14/+0x18 and descends +0x11D per tick
 * (0x10A when hit-flagged, byte +0x22). Every fourth tick it steps the link's
 * byte +0x6 phase and runs the per-phase frame list from the long table at
 * 0xE7A1A (3 phases, indexed by the phase byte read as unsigned). It is then
 * added to the hazard list and 0x10E alternate sprite draws are issued at
 * +0x50/+0x54, each preceded by pulling +0x54 up by 0x10. Verified against a
 * MAME tick capture from work/maincpu.bin. */

/* ROM 0xE7914: pull +0x54 up by 0x10 and draw; the tail at 0xE7944 issues
 * 0x10E such pairs. The extra jsr at 0xE7986 runs with no subi before it
 * (0x10E jsr, 0x10D subi.w). */
#define E78EC_DRAW_PAIRS 14u  /* 15 jsr 0x1282 (0xE794A..0xE79F2), 14 subi.w #0x10 */

int Obj_E78EC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0xE78EC: movea.l (0x8,A0),A1 ; tst.b (0x6,A1) ; beq 0xCEE74 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);                 /* 0xCEE74 */
        return 0;
    }

    /* 0xE78FA: copy the link's path position and speed */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    /* 0xE7906: move.w #0x11D,D0w ; tst.b (0x22,A0) ; beq 0xE7914 ;
     * moveq -0xD,D0 (D0w = 0xFFF3) ; 0xE7914: add.w D0w,(0x18,A0) */
    {
        uint16_t d0w = 0x011Du;
        if (rd8(m, obj + 0x22u) != 0u)
            d0w = 0xFFF3u;              /* moveq sign-extends */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
    }

    /* 0xE7918: subq.b #1,(0x23,A0) ; bne 0xE7944 */
    {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u) {
            /* 0xE7920: subq.b #1,(0x6,A0) ; bmi 0xCEE74 */
            int8_t ph = (int8_t)(rd8(m, obj + 0x06u) - 1u);
            wr8(m, obj + 0x06u, (uint8_t)ph);
            if (ph < 0) {
                Obj_StartDying(m, obj);         /* 0xCEE74 */
                return 0;
            }
            /* 0xE792A: clr.w D0w ; move.b (0x6,A0),D0b ;
             * movea.l (0xE8,PC,D0w*4),A1 (extension word 0xE7932);
             * jsr 0xCFA86 */
            uint8_t idx = rd8(m, obj + 0x06u);
            uint32_t frame = rd32(m, 0x0E7A1Au + (uint32_t)idx * 4u);
            Obj_SetFrame(m, obj, frame);        /* 0xCFA86 */
            /* 0xE793E: move.b #4,(0x23,A0) */
            wr8(m, obj + 0x23u, 0x04u);
        }
    }

    /* 0xE7944: hazard list, then the draw tail */
    Coll_AddHazard(m, obj);                     /* 0xD0DCC */

    /* 0xE794A/0xE794C: jsr 0x1282 */
    Obj_DrawSprite(m, obj);
    for (unsigned i = 0; i < E78EC_DRAW_PAIRS; i++) {
        /* 0xE7950: subi.w #0x10,(0x18,A0) ; jsr 0x1282 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u));
        Obj_DrawSprite(m, obj);
    }

    /* 0xE79F8: rts */
    return 0;
}
