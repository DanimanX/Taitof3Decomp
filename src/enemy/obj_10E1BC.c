#include "obj_10E1BC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_common.h"
#include "../audio/sound_cmd.h"
#include "obj_DC008.h"

/* ROM 0x13F238: sound command remap hook */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x0D0E04: D0 score goes to the parent record */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t st = rd8(m, obj + 0x35u);
    if (st == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (st == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0DC2: append obj to the hazard list at A5+0x906 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x08FEu, a1);
    Coll_AddHazard(m, obj);
}

/* ROM 0x10E1BC */
int Obj_10E1BC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);          /* movea.l (0x8,A0),A4 */
    wr32(m, obj + 0x14u, rd32(m, a4 + 0x3Au));              /* move.l (0x3A,A4),(0x14,A0) */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x3Au))); /* add.w D0w,(0x14,A0) */
    wr32(m, obj + 0x18u, rd32(m, a4 + 0x3Eu));              /* move.l (0x3E,A4),(0x18,A0) */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0006u)); /* addq.w #6,(0x18,A0) */
    wr16(m, obj + 0x32u, rd16(m, a4 + 0x42u));

    /* 0x10E1DE: move.b (0x6,A4),D0; beq.l -> 0x0CEE74 (flags from the MOVE) */
    uint8_t d0b = rd8(m, a4 + 0x06u);
    if (d0b == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x10E1E8 */
    if (d0b == 5u)
        goto L_10E222;
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        if (rd8(m, a4 + 0x07u) != 0u)
            goto L_10E222;
        if (d0b != 1u)
            sub_0D0DC2(m, obj);
        Obj_BlinkColour(m, obj);
        return 0;
    }

    /* 0x10E216 */
    sub_0D0E04(m, obj, 1000u);
    /* falls through to 0x10E222 */

L_10E222:
    /* 0x10E222 */
    {
        uint8_t d0 = rd8(m, obj + 0x07u);
        wr8(m, a4 + 0x07u, (uint8_t)(rd8(m, a4 + 0x07u) | (uint8_t)(1u << (d0 & 7u))));
    }
    {
        uint16_t d1w = rd16(m, obj + 0x14u);                       /* X */
        uint16_t d2w = (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, a4 + 0x42u));
        d2w = (uint16_t)(d2w - 0x0008u);                           /* Y */
        uint16_t d3w = (uint16_t)(rd16(m, a4 + 0x3Eu) - 0x0008u);
        uint16_t d4w = 0x0010u;
        for (;;) {
            /* 0x10E240: pea (0x0).w; move.w #2,-(SP); move.w #3,-(SP);
             * move.l #0xDC008,-(SP); jsr 0x1057C */
            uint32_t spawned = Obj_Spawn(m, obj, 0x000DC008u, 3u, 2u, 0u);
            if ((int32_t)spawned < 0)
                break;
            wr16(m, spawned + 0x14u, d1w);
            wr16(m, spawned + 0x18u, d2w);
            d2w = (uint16_t)(d2w - d4w);
            if ((int16_t)d3w < (int16_t)d2w)        /* cmp.w D2w,D3w; blt */
                continue;
            break;
        }
    }

    /* 0x10E272: move.w #0x34,-(SP); jsr 0x13F238; jsr 0x39D8 */
    {
        int16_t cmd = sub_13F238(m, 0x0034);
        if (m->dpram) {
            Sound_PlayCmd(m, cmd);
        } else {
            uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
            f3mem_t tmp_m = *m;
            tmp_m.dpram = tmp_dpram;
            Sound_PlayCmd(&tmp_m, cmd);
        }
    }

    /* 0x10E286: bra.l 0x0CEE74 */
    Obj_StartDying(m, obj);
    return 0;
}
