#include "obj_15A154.h"
#include "../objects/obj_pool.h"

/* ROM 0x15A154 (phase 1 of the two-step wipe started by 0x15A0C8)
 *
 * Bytes re-decoded from work/maincpu.bin:
 *   15A154 5368 0044             subq.w #1,(0x44,A0)
 *   15A158 6c00 ff74             bge.w 0x15A0CE          (F == 0 -> V = 0 always)
 *   15A15C 48e7 0080             movem.l {A0},-(SP)
 *   15A160 4eb9 0000 5918        jsr 0x5918   (clear text/chr RAM 0x611000
 *                                and the 0x62A200 row)
 *   15A166 2f3c 0000 0020        move.l #0x20,-(SP)
 *   15A16C 2f3c 0000 0880        move.l #0x880,-(SP)
 *   15A172 2f3c 0002 0340        move.l #0x20340,-(SP)
 *   15A178 4eb9 0000 1660        jsr 0x1660   (queue a blit command)
 *   15A17E 4fef 000c             lea (0xC,SP),SP
 *   15A182 3f3c 0001             move.w #1,-(SP)
 *   15A186 2f3c 0 / 15A18C / 15A192  move.l #0,-(SP) x3
 *   15A198 3f3c 00e7             move.w #0xE7,-(SP)
 *   15A19C 3f3c 0000             move.w #0,-(SP)
 *   15A1A0 4879 0062 8200        pea 0x628200     (line-RAM zoom word table)
 *   15A1A6 4879 0062 a200        pea 0x62A200     (line-RAM scale byte table)
 *   15A1AC 4eb9 0017 6832        jsr 0x176832   (zoom ramp, 0xE7..0)
 *   15A1B2 4fef 001a             lea (0x1A,SP),SP
 *   15A1B6 4cdf 0100             movem.l (SP)+,{A0}
 *   15A1BA 2f08                  move.l A0,-(SP)
 *   15A1BC 4eb9 0001 05ec        jsr 0x105EC   (Obj_Free)
 *   15A1C2 4fef 0004             lea (4,SP),SP
 *   15A1C6 4e75                  rts
 *
 * 0x15A15C..0x15A1C8 (the 0x5918 clear, the 0x1660 blit and the 0x176832 ramp)
 * is output-only: it never touches Work RAM, and no 0x0BD3A4 capture shows the
 * 0x62A200 row it would leave behind, so only the countdown and the free are
 * applied. `bge.w` after `subq.w` is F == 0 -> V = 0, i.e. signed >=. */
int Obj_15A154_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x15A0CE: move.w (0x44,A0),D0w; andi.w #0x1f,D0w; bne.w 0x15A148 */
    for (;;) {
        uint16_t w = (uint16_t)(rd16(m, obj + 0x44u) - 1u);
        wr16(m, obj + 0x44u, w);
        if ((int16_t)w < 0)
            break;

        if ((rd16(m, obj + 0x44u) & 0x001Fu) == 0u) {
            /* 0x15A0DE..0x15A142: one row of the 0x626200 wipe (see 0x15A0C8) */
            uint16_t d0w = (uint16_t)(-(int16_t)(uint16_t)(rd16(m, obj + 0x44u) >> 5) + 8u);
            uint16_t d1w = 4u;

            uint32_t a1 = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

            d1w &= 0x000Fu;
            d0w &= 0x000Fu;

            uint16_t t1 = (uint16_t)((d1w << 8) | (uint16_t)(8u - d1w));
            t1 = (uint16_t)(t1 << 4);
            uint16_t t0 = (uint16_t)((d0w << 8) | (uint16_t)(8u - d0w));
            uint16_t fill = (uint16_t)~(uint16_t)(t1 | t0);

            for (uint32_t d7 = 0u; d7 <= 0xE7u; d7++)
                wr16(m, a1 + d7 * 2u, fill);
        }
        /* 0x15A148: movem.l (SP)+,{A0}; move.l #0x15A154,(A0); rts */
        wr32(m, obj + 0x00u, 0x0015A154u);
        return 0;
    }

    /* 0x15A15C: wipe + blit + ramp (output only), then free */
    Obj_Free(m, obj);
    return 0;
}
