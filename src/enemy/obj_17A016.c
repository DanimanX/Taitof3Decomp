#include "obj_17A016.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../audio/sound_cmd.h"

/* ROM 0x17A016: entrance tick of the small sprite burst. Unlike the shrapnel
 * family it *increments* A5+0x4B24 first and bails when the result exceeds 8,
 * latches the frame offsets into +0x16/+0x1A, seeds the 0x17A0D8 frame list
 * and plays sound command 0x1C before installing 0x17A070. */

/* ROM 0x17A0C4: give the slot back and free the object */
static void lab_17A0C4(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) - 1u));
    Obj_Free(m, obj); /* 0x0105EC */
}

int Obj_17A016_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A016: addq.w #1,(0x4B24,A5) ; cmpi.w #8,(0x4B24,A5) ; bgt -> 0x17A0C4 */
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u));
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x08) {
        lab_17A0C4(m, obj);
        return 0;
    }

    /* 0x17A024: move.w (0xC,A0),(0x16,A0) ; move.w (0xE,A0),(0x1A,A0) */
    wr16(m, obj + 0x16u, rd16(m, obj + 0x0Cu));
    wr16(m, obj + 0x1Au, rd16(m, obj + 0x0Eu));

    /* 0x17A030: movea.l #0x17A0D6,A1 ; move.w (A1)+,D0w ;
     * move.w D0w,(0x40,A0) ; move.l A1,(0x3A,A0). The record's first field is
     * the animation index (0), then the 0x17A0D8 frame list
     * ({1, 0x000622E4}, ..., 12 frames, 0xFFFF terminator). */
    {
        uint32_t a1 = 0x0017A0D6u;
        uint16_t d0 = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, d0);
        wr32(m, obj + 0x3Au, a1);

        /* 0x17A040: movea.l (0x0,A1,D0w*8),A1 ; jsr (A1) -- INIT slot */
        if (Obj_AnimInit(m, obj, d0) < 0)
            return -1;
    }

    wr16(m, obj + 0x1Cu, 0u); /* 0x17A04C */
    wr16(m, obj + 0x1Eu, 0u); /* 0x17A050 */

    /* 0x17A054: move.w #0x1C,-(SP) ; jsr 0x13F238 (cmd += 0x53 in alt mode) ;
     * jsr 0x0039D8 (Sound_PlayCmd) */
    {
        int16_t cmd = 0x1C;
        if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
            cmd = (int16_t)(cmd + 0x53);
        Sound_PlayCmd(m, cmd);
    }

    wr32(m, obj + 0x00u, 0x0017A070u); /* 0x17A068 move.l #0x17A070,(A0) */
    return 0;
}
