#include "obj_17F48E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 -> 0x0039D8: sound command, +0x53 in the alternate sound mode */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x184888: load the 8-word / 2-long stat record at A1 into the object */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1 + 0x00u)); a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x00u)); a1 += 2u;
    wr16(m, obj + 0x24u, rd16(m, a1 + 0x00u)); a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1 + 0x00u)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 0x00u)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x00u)); a1 += 4u;
    wr16(m, obj + 0x62u, rd16(m, a1 + 0x00u)); a1 += 2u;
    wr32(m, obj + 0x66u, rd32(m, a1 + 0x00u));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);   /* divul.l #10 */
}

/* ROM 0x17F48E: the two-phase attack. +0x06 holds the pre-attack wait; until
 * it runs out only 0x17F486 (re-install this tick) is reached (LAB_0017f486),
 * then the same body as 0x17F43C runs. */
int Obj_17F48E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17F48E: subq.w #1,(0x6,A0); bgt 0x17F486 */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        wr32(m, obj + 0x00u, 0x0017F48Eu);          /* LAB_0017f486 */
        return 0;
    }

    /* 0x17F494: movea.l (0x8,A0),A4; +0x50/+0x54 = parent + offsets */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        uint16_t d0 = (uint16_t)(uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t d1 = (uint16_t)(uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, d0);
        wr16(m, obj + 0x54u, d1);
        wr16(m, obj + 0x14u, d0);
        wr16(m, obj + 0x18u, d1);
    }

    play_sound(m, 0x2A);                            /* 0x17F4B8 */

    sub_184888(m, obj, 0x0017F562u);                /* 0x17F4CC */

    {
        uint16_t idx = rd16(m, 0x0017F578u);        /* 0x17F4D8 */
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, 0x0017F57Au);
        if (Obj_AnimInit(m, obj, idx) < 0)          /* jsr (A1) */
            return -1;
    }

    /* 0x17F4F4: movement script 0x17F580 and its 6-byte state */
    wr32(m, obj + 0x44u, 0x0017F580u);
    wr16(m, obj + 0x48u, rd16(m, 0x0017F584u));
    wr16(m, obj + 0x4Au, 0u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 0x10u)); /* 0x17F508 */

    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C4D4u, 0x0003u, 0x0002u, 0u); /* 0x17F50E */
        wr32(m, obj + 0x62u, child);                /* 0x17F52C */
        if ((int32_t)child <= 0)
            return -1;                              /* 0x17C78E: Obj_Free */
    }

    /* 0x17F534: two words from +0x10, each <<3, into +0x16/+0x1A */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = rd16(m, a1);
        uint16_t d0 = rd16(m, a1 + 2u);
        wr16(m, obj + 0x16u, (uint16_t)(d0 << 3));
        wr16(m, obj + 0x1Au, (uint16_t)(d1 << 3));
    }
    wr16(m, obj + 0x58u, 0u);                       /* 0x17F548 */
    wr16(m, obj + 0x5Au, 0u);
    wr8(m, obj + 0x34u, 0x02u);                     /* 0x17F550 */
    wr32(m, obj + 0x0Cu, 0x0017D4C0u);              /* 0x17F556 */

    wr32(m, obj + 0x00u, 0x0017C240u);              /* 0x17F55E -> 0x17C238 */
    return 0;
}
