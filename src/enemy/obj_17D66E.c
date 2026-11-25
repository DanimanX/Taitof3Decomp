#include "obj_17D66E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"
#include "obj_17D6C8.h"

/* ROM 0x17D66E: spawn-wait start. Arms +0x4C from the global at A5+0x1D28,
 * places the object from its +0x0C record (0x17BDA2), gives up (0x17C78E,
 * free) outside the 0x1849F8 region, otherwise spawns the 0x17C908 child. A
 * live child parks the object on 0x17D6C8 for one frame; a full pool plays
 * sound 0x11, seeds the stats from 0x17D856, starts animation 9 (record
 * 0x17D86C), installs the 0x17D8CE behaviour script and returns through
 * 0x17D72C. */

/* ROM 0x13F238 -> 0x0039D8: sound command word on the stack, +0x53 in the
 * alternate sound mode. */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    Sound_PlayCmd(m, cmd);
}

/* ROM 0x184888: seed the stats from the record at a1; the first word pair is
 * divided by ten (divul.l). */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x62u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x66u, rd32(m, a1));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);
}

/* cmp2.w (A_base)/(A_base+2) against D0w: true when lo < D0w < hi (bcs to the
 * out-of-range handler). */
static int in_box(const f3mem_t *m, uint32_t base, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, base), hi = (int16_t)rd16(m, base + 2u);
    int16_t x = (int16_t)v;
    return x > lo && x < hi;
}

/* ROM 0x17D66E */
int Obj_17D66E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D66E: move.l (0x1D28,A5),(0x4C,A0); beq.w 0x17C78E */
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
    if ((int32_t)rd32(m, obj + 0x4Cu) == 0) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x17D678: bsr 0x17BDA2 */
    Obj_PolarPlace(m, obj);

    /* 0x17D67C..0x17D698: cmp2.w (0x1849F8)/(0x1849FC); bcs 0x17C78E */
    if (!in_box(m, 0x001849F8u, rd16(m, obj + 0x50u)) ||
        !in_box(m, 0x001849FCu, rd16(m, obj + 0x54u))) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x17D69C: push 0, 1, 0xB, 0x17C908 -> jsr 0x1057C */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C908u, 11u, 1u, 0u);

        /* 0x17D6B8: move.w #1,(0x6,A0); beq 0x17D6CE -- Z comes from the
         * spawn's D0 (a live slot is a 24-bit address, the full pool
         * 0xFFFFFFFF clears it). */
        wr16(m, obj + 0x06u, 1u);
        if (child != 0xFFFFFFFFu) {
            /* 0x17D6C0: move.l #0x17D6C8,(A0) */
            wr32(m, obj + 0x00u, 0x0017D6C8u);
            return 0;
        }
    }

    /* 0x17D6CE: sound 0x11 */
    play_sound(m, 0x0011u);

    /* 0x17D6E2: stats record 0x17D856 */
    sub_184888(m, obj, 0x0017D856u);

    /* 0x17D6EE: animation record 0x17D86C -- index 9 -> 0x179600 */
    {
        uint32_t a1 = 0x0017D86Cu;
        uint16_t d0w = rd16(m, a1); a1 += 2u;
        wr16(m, obj + 0x40u, d0w);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, d0w) < 0)
            return -1;
    }

    /* 0x17D70A: behaviour record 0x17D8CE */
    wr32(m, obj + 0x44u, 0x0017D8CEu);
    wr16(m, obj + 0x48u, rd16(m, 0x0017D8D2u));
    wr16(m, obj + 0x4Au, 0u);

    /* 0x17D71E: +0x0C = +0x20 */
    wr16(m, obj + 0x0Cu, rd16(m, obj + 0x20u));

    /* 0x17D724: move.l #0x17D72C,(A0) */
    wr32(m, obj + 0x00u, 0x0017D72Cu);
    return 0;
}
