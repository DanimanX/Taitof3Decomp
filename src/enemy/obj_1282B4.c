#include "obj_1282B4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* Frame list at 0x128462: two 4-frame frames of 0x043BB8 / 0x043BCC. */
#define ANIM_128462 0x00128462u

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

/* ROM 0x0CBDBA: unless the object is in the "dead" colour (0x35 == 2) quarter
 * the referenced object's momentum (+0x28), then drop the +0x36 back-reference. */
static void sub_0CBDBA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0x02u) {
        uint32_t d0 = rd32(m, obj + 0x36u);
        if (d0 != 0u)
            wr32(m, d0 + 0x28u, (uint32_t)((int32_t)rd32(m, d0 + 0x28u) >> 2));
    }
    wr32(m, obj + 0x36u, 0u);
}

/* ROM 0x0CBDDE: A1 = owner to credit; one (0x8,A1) hop while +0x35 == 1. */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x128416 (the 0x1282B4 tail falls into it): credit the owner (+0x48) and
 * the A5+0x886 counter, play sound 0x34, spawn one 0x13E4EE child (group 0,
 * one group) and park on 0x128454, which frees the object next tick. */
static void sub_128416(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = sub_0CBDDE(m, obj);
    uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);

    Sound_PlayCmd(m, 0x0034);

    uint32_t arg = sub_13E4D0(m, obj);
    (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg);
    wr32(m, obj + 0x00u, 0x00128454u);
}

/* ROM 0x1282B4: cruise tick of the 0x128226 bullet. Frozen (A5-0x144B bit 4)
 * or out of HP it goes to 0x128416. Otherwise it animates from 0x128462; while
 * the scroll lock A5+0x1D58 bit 1 is set and it is on screen it takes the blink
 * + relay path; then it flies along its heading (+0x20 via 0x0CBE12), trims the
 * parent momentum, lists/draws itself, and once it leaves the left edge
 * ((0x14,A0) < -0x30) goes to 0x128454 (free); otherwise it stays on itself. */
int Obj_1282B4_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto tail_128416;                                   /* 0x1282BA bne.w */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto tail_128416;                                   /* 0x1282C2 ble.w */

    Obj_Animate(m, obj, ANIM_128462);                       /* 0x1282CC jsr 0x0CBD52 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {          /* 0x1282D2 btst #1 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= (int16_t)-0x0A && x <= (int16_t)0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x1282EA */
            Obj_RelayTimer(m, rd16(m, obj + 0x4Cu));        /* 0x1282F0 jmp 0x0CBD12 */
            return 0;
        }
    }

    /* 0x1282F6: move.w (0x20,A0),D0w ; jsr 0x0CBE12 */
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    sub_0CBDBA(m, obj);                                     /* 0x128300 jsr 0x0CBDBA */
    sub_11B06E(m, obj);                                     /* 0x128306 jsr 0x11B06E */
    Obj_DrawSprite(m, obj);                                 /* 0x12830C jsr 0x001282 */

    /* 0x128312: cmpi.w #-0x30,(0x14,A0) ; blt 0x128454 */
    if ((int16_t)rd16(m, obj + 0x14u) < (int16_t)-0x30) {
        Obj_Free(m, obj);                                   /* 0x128454 */
        return 0;
    }

    wr32(m, obj + 0x00u, 0x001282B4u);                      /* 0x1282AC */
    return 0;

tail_128416:
    sub_128416(m, obj);
    return 0;
}
