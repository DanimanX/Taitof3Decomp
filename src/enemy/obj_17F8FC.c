#include "obj_17F8FC.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 + 0x3B66: forced sound command from a word on the stack; the
 * pre-cmd hook shifts 1..0x40 by +0x53 when A5+0x3306 is set. */
static void play_sound_forced(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmdForced(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmdForced(&tmp_m, cmd);
    }
}

/* ROM 0x17F732: move.w #0x22,-(SP); jsr 0x13F238; jsr 0x3B66; lea (0x2,SP),SP;
 * 0x17F746: move.l #0x17F74E,(A0) */
static void stop_17f732(const f3mem_t *m, uint32_t obj)
{
    play_sound_forced(m, 0x22);
    wr32(m, obj, 0x0017F74Eu);
}

/* ROM 0x17F8FC: the 0x17F6C8 twin that also appends itself to the hazard list
 * A5+0x906 on every drawn frame. Same frame-script loop, same countdown at
 * +0x64 (re-arm 0x17F8F4, stop 0x17F732: forced sound 0x22 then 0x17F74E). */
int Obj_17F8FC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17F8FC: btst.b #0,(0x409D58).l ; bne 0x17F732 */
    if (rd8(m, 0x409D58u) & 0x01u) {
        stop_17f732(m, obj);
        return 0;
    }
    /* 0x17F908: btst.b #4,(-0x144C,A5) ; bne 0x17F732 */
    if (rd8(m, F3_A5 - 0x144Cu) & 0x10u) {
        stop_17f732(m, obj);
        return 0;
    }

    /* 0x17F912: movea.l (0x8,A0),A4; velocity integration on the parent
     * position. A4 keeps the ORIGINAL A0 for the rest of the routine (the
     * hazard append below stores A4, the object: `move.l A0,(A1)+` tests every
     * captured pair this way). */
    uint32_t a4 = obj;
    uint32_t parent = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1 = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);

    /* 0x17F936: D0w = (0x40,A0); movea.l (0x4,0x179A5C,D0w*8),A1; jsr (A1) --
     * the tick half. The caller has just reloaded the animation base from the
     * script (script dispatch is the init half, 0x179550, which puts
     * 0x17F9CC -> {1, 0x0005A4F8} in place), so it is re-applied here before
     * the tick moves +0x3A onto the first frame record (0x17F9CE). Either
     * dispatch returning an unknown index leaves the object untouched. */
    {
        uint16_t idx = rd16(m, obj + 0x40u);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
        if (Obj_AnimTick(m, obj, idx) < 0)
            return -1;
    }

    /* 0x17F946: centre the frame on its {w.w, h.w} anchor:
     *   move.w (A1)+,D1w; asl.w #3,D1w; sub.w D1w,(0x18,A0)
     *   move.w (A1)+,D0w; asl.w #3,D0w; sub.w D0w,(0x14,A0) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1w = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0w = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w));
    }

    Obj_DrawSprite(m, obj);                            /* 0x17F95A jsr 0x1282 */

    /* 0x17F960: movea.l (0x906,A5),A1; move.l A0,(A1)+; move.l A1,(0x906,A5)
     * -- A0 here is the value left in the register pair A4/A0 by A0 = A4 */
    {
        uint32_t p = rd32(m, F3_A5 + 0x906u);
        wr32(m, p, a4);
        wr32(m, F3_A5 + 0x906u, p + 4u);
    }

    /* 0x17F96A: subq.w #1,(0x64,A0); bgt 0x17F8F4 -- the branch tests the NEW
     * value (0x17F8F4: move.l #0x17F8FC,(A0)) */
    {
        uint16_t n = (uint16_t)(rd16(m, obj + 0x64u) - 1u);
        wr16(m, obj + 0x64u, n);
        if ((int16_t)n > 0) {
            wr32(m, obj, 0x0017F8FCu);
            return 0;
        }
    }

    /* 0x17F970: bra.w 0x17F732 */
    stop_17f732(m, obj);
    return 0;
}
