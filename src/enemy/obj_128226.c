#include "obj_128226.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"
#include "../video/sprite.h"

/* Frame list at 0x128462: two 4-frame frames of 0x043BB8 / 0x043BCC. */
#define ANIM_128462 0x00128462u

/* ROM 0x13F238 -> 0x0039D8 */
static void sub_13F238(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x0CBEB0: D0w scaled by the difficulty (A5+0x87E). */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    int32_t p = (int32_t)(int16_t)d1w * (int32_t)(int16_t)d0w;
    uint32_t r = (uint32_t)p << 4;
    return (uint16_t)(r >> 16);
}

/* ROM 0x13E4D0: spawn argument from the two sprite-def words at (0x10,A0).
 *   movea.l (0x10,A0),A1 ; move.w (A1)+,D1 ; lsl.w #3,D1 ; bfins D1,D0{8:8}
 *                         move.w (A1),D1  ; lsl.w #3,D1 ; bfins D1,D0{0:8}
 * D0 arrives holding 3 (`moveq #3,D0`). */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = 3u;
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
    d0 = (d0 & ~0x0000FF00u) | ((uint32_t)d1 << 8);
    d1 = (uint16_t)(rd16(m, a1 + 2u) << 3);
    d0 = (d0 & ~0x000000FFu) | (uint32_t)(d1 & 0xFFu);
    return d0;
}

/* ROM 0x0CBDA4: append A0 to the deferred-draw list A5+0x8FE and fall through
 * into 0x0CBDAE (the hazard list A5+0x906). */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x11B06E: publish A0 on the lists while A5+0x30F3 is clear and
 * (0x2C,A0) is non-zero, then refresh the colour (0x11C32C). */
static void sub_11B06E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u)
        sub_0CBDA4(m, obj);
}

/* ROM 0x11B09A: position = origin + offset + scroll; draw once while the box
 * (sprite top-left << 4) is on screen; flip the offsets back off. */
static void sub_11B09A(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u)
                                 + rd16(m, F3_A5 + 0x30D2u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u)
                                 + rd16(m, F3_A5 + 0x30D6u)));

    if (!((int16_t)rd16(m, obj + 0x18u) >= 0x00E8 ||
          (int16_t)rd16(m, obj + 0x14u) >= 0x0140)) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d0 = (uint16_t)(rd16(m, a1) << 4);
        d0 = (uint16_t)(d0 + rd16(m, obj + 0x18u));
        if ((int16_t)d0 > 0) {
            d0 = (uint16_t)(rd16(m, a1 + 2u) << 4);
            d0 = (uint16_t)(d0 + rd16(m, obj + 0x14u));
            if ((int16_t)d0 > 0)
                Obj_DrawSprite(m, obj);
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, obj + 0x56u)));
}

/* ROM 0x0CBDBA: unless the object is in the "dead" colour (0x35 == 2) quarter
 * the referenced object's momentum (+0x28), then drop the +0x36 back-reference.
 * Returns A1 = (0x36,A0) as read afterwards. */
static uint32_t sub_0CBDBA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0x02u) {
        uint32_t d0 = rd32(m, obj + 0x36u);
        if (d0 != 0u)
            wr32(m, d0 + 0x28u, (uint32_t)((int32_t)rd32(m, d0 + 0x28u) >> 2));
    }
    wr32(m, obj + 0x36u, 0u);
    return rd32(m, obj + 0x36u);
}

/* ROM 0x0CBDDE: A1 = owner to credit; one (0x8,A1) hop while +0x35 == 1. */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x128416: credit the owner (+0x48) and the A5+0x886 counter, play sound
 * 0x34, spawn one 0x13E4EE child (group 0, one group) and park on 0x128454. */
static void sub_128416(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = sub_0CBDDE(m, obj);
    uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);

    sub_13F238(m, 0x0034);

    uint32_t arg = sub_13E4D0(m, obj);
    (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg);
    wr32(m, obj + 0x00u, 0x00128454u);
}

/* ROM 0x128226: launch tick of the 0x1281C4 bullet. Frozen (A5-0x144B bit 4)
 * or out of HP (signed (0x24,A0) <= 0) it goes straight to 0x128416. Otherwise
 * it animates from 0x128462; while the scroll lock A5+0x1D58 bit 1 is set and
 * it is on screen it takes the blink + relay path; it bleeds speed (+0x22) by
 * 6 per frame while the +0x6 timer is above 0x1E, mirrors speed >> 4 into
 * +0x50, trims the parent momentum, lists/draws itself, counts +0x6 down, and
 * at the end turns to 0x80, re-arms +0x42 with 0x0CBEB0(0xC) and hands over to
 * 0x1282B4. */
int Obj_128226_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto tail_128416;                                   /* 0x12822C bne.w */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto tail_128416;                                   /* 0x128234 ble.w */

    Obj_Animate(m, obj, ANIM_128462);                       /* 0x12823E jsr 0x0CBD52 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {          /* 0x128244 btst #1 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= (int16_t)-0x0A && x <= (int16_t)0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x12825C */
            Obj_RelayTimer(m, rd16(m, obj + 0x4Cu));        /* 0x128262 jmp 0x0CBD12 */
            return 0;
        }
    }

    /* 0x128268: cmpi.w #0x1e,(0x6,A0) ; ble -> 0x128276 */
    if ((int16_t)rd16(m, obj + 0x06u) > 0x001E)
        wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) - 6u));

    /* 0x128276: move.w (0x22,A0),D0w ; asr.w #4,D0w ; move.w D0w,(0x50,A0) */
    wr16(m, obj + 0x50u, (uint16_t)((int16_t)rd16(m, obj + 0x22u) >> 4));

    (void)sub_0CBDBA(m, obj);                               /* 0x128280 jsr 0x0CBDBA */
    sub_11B06E(m, obj);                                     /* 0x128286 jsr 0x11B06E */
    sub_11B09A(m, obj);                                     /* 0x12828C jsr 0x11B09A */

    /* 0x128292: subq.w #1,(0x6,A0) ; bgt 0x12821E */
    {
        int16_t d6 = (int16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)d6);
        if (d6 <= 0)
            goto rearm_128298;
    }
    wr32(m, obj + 0x00u, 0x00128226u);                      /* 0x12821E */
    return 0;

rearm_128298:
    /* 0x128298: heading 0x80, +0x42 = 0x0CBEB0(0xC), next tick 0x1282B4 */
    wr16(m, obj + 0x20u, 0x0080u);
    wr16(m, obj + 0x42u, sub_0CBEB0(m, 0x000C));
    wr32(m, obj + 0x00u, 0x001282B4u);
    return 0;

tail_128416:
    sub_128416(m, obj);
    return 0;
}
