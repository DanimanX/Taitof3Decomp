#include "obj_17EFC8.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../audio/sound_cmd.h"

/* ROM 0x17EFC8: the second-stage fuse. Copies the parent's arm count (+0x4C),
 * frees itself when it is already spent, installs behaviour 0x17F3C4, places
 * the object from its +0x0C record (0x17F210) and -- after 0x10DA gives back a
 * child -- plays sound 0x2A and continues in 0x17F266 (0x17F260's expiry body:
 * stats, animation 0, the 0x17C4D4 child, then 0x17C360). */

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

/* ROM 0x17F266: the 0x17F260 expiry body, entered from 0x17F210 after the
 * successful spawn. A and Z are re-taken from 0x10DA's return: a non-zero
 * child clears D0's bits 23..0, and the engine's spawn reports a full pool as
 * 0xFFFFFFFF (D0 = 0), which is the only value that frees here. */
static int lab_17F266(const f3mem_t *m, uint32_t obj)
{
    play_sound(m, 0x002Au);                         /* 0x17F26A */

    /* 0x17F27A: moveq #8,D0; bfins D0,(0xC,A0){0x10:0x8} */
    {
        uint32_t f = rd32(m, obj + 0x0Cu);
        wr32(m, obj + 0x0Cu, (f & 0xFF00FFFFu) | 0x00080000u);
    }

    Obj_PolarPlace(m, obj);                         /* 0x17F282 bsr 0x17BDA2 */

    sub_184888(m, obj, 0x0017F2FAu);                /* 0x17F28C */

    /* 0x17F292: animation record 0x17F310 ({1, anim 2}) -> frames 0x17F312 */
    {
        uint32_t a1 = 0x0017F310u;
        uint16_t d0w = rd16(m, a1); a1 += 2u;
        wr16(m, obj + 0x40u, d0w);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, d0w) < 0)
            return -1;
    }

    /* 0x17F2AE: push 7, 2, 3, 0x17C4D4 -> jsr 0x1057C; move.l D0,(0x62,A0) */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C4D4u, 3u, 2u, 7u);
        wr32(m, obj + 0x62u, child);
        /* 0x17F2D0: ble 0x17C78E, i.e. the full-pool sentinel frees */
        if (child == 0xFFFFFFFFu) {
            Obj_Free(m, obj);
            return 0;
        }
    }

    /* 0x17F2D4: +0x16 = frame width << 3, +0x1A = frame height << 3 */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = rd16(m, a1); a1 += 2u;
        uint16_t d0 = rd16(m, a1);
        wr16(m, obj + 0x16u, (uint16_t)(d0 << 3));
        wr16(m, obj + 0x1Au, (uint16_t)(d1 << 3));
    }

    wr16(m, obj + 0x58u, 0u);                       /* 0x17F2E8 */
    wr16(m, obj + 0x5Au, 0u);
    wr8(m, obj + 0x34u, 0x02u);                     /* 0x17F2F0 */

    /* 0x17F2F6: bra.w 0x17C358 -> move.l #0x17C360,(A0) */
    wr32(m, obj + 0x00u, 0x0017C360u);
    return 0;
}

/* ROM 0x17EFC8 */
int Obj_17EFC8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17EFC8: movea.l (0x8,A0),A4; move.l (0x4C,A4),(0x4C,A0); ble 0x17C78E */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr32(m, obj + 0x4Cu, rd32(m, a4 + 0x4Cu));
    }
    if ((int32_t)rd32(m, obj + 0x4Cu) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x17EFD6: behaviour 0x17F3C4 {0, 0x17F3CE, dur = 2} */
    {
        uint32_t a1 = 0x0017F3C4u;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));
        wr16(m, obj + 0x4Au, 0u);
    }

    /* 0x17F210: bsr 0x17BDA2, then cmp2.w (0x1849F8)/(0x1849FC); out -> free */
    Obj_PolarPlace(m, obj);
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        if (!(x > (int16_t)rd16(m, 0x001849F8u) && x < (int16_t)rd16(m, 0x001849FAu)) ||
            !(y > (int16_t)rd16(m, 0x001849FCu) && y < (int16_t)rd16(m, 0x001849FEu))) {
            Obj_Free(m, obj);
            return 0;
        }
    }

    /* 0x17F234: push 0, 1, 0xB, 0x17C856 -> jsr 0x1057C */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C856u, 11u, 1u, 0u);

        /* 0x17F250: move.w #1,(0x6,A0); beq 0x17F266 -- Z from the spawn's D0
         * (a 24-bit address is non-zero, the full-pool 0xFFFFFFFF clears it). */
        wr16(m, obj + 0x06u, 1u);
        if (child != 0xFFFFFFFFu) {
            /* 0x17F258: move.l #0x17F260,(A0) */
            wr32(m, obj + 0x00u, 0x0017F260u);
            return 0;
        }
    }

    /* 0x17F266: bra.w, body shared with 0x17F260's expiry pass */
    return lab_17F266(m, obj);
}
