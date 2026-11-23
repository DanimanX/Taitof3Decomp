#include "obj_128A84.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* Sprite list at 0x43BF4 {1,1,tile,attr} shared by the 0x128xxx family. */
#define SPR_128A84 0x00043BF4u

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

/* ROM 0x13E4D0: spawn argument from the two sprite-def words at (0x10,A0),
 * with D0 arriving as 3 (`moveq #3,D0`). */
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

/* ROM 0x11C32C: colour override (+0x1D) alternating 6/8 while the blink flag
 * (+0x35) is set, else the object's own colour (+0x4F). */
static void sub_11C32C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        return;
    }
    if (rd8(m, obj + 0x1Du) == 0x06u) {
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, 0x08u);
        return;
    }
    wr8(m, obj + 0x1Du, 0x06u);
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

/* ROM 0x128D4A: free the object. */
static void sub_128D4A(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x128A84: hostile projectile, first tick. Frozen (A5-0x144B bit 4) or
 * out of HP it goes to the 0x128CF8 kill path. It re-points +0x10 at the
 * shared 0x43BF4 sprite; while the scroll lock A5+0x1D58 bit 1 is set it only
 * refreshes its colour and relays (0x0CBD12); otherwise it steers along +0x20,
 * trims the parent momentum, lists itself (0x0CBDA4) and draws, and once +0x42
 * has reached 0x18 it hands its target/timer state to the 0x128B44 child at
 * (0x48,A0) and frees itself (0x128B40 bra.w 0x128D4A). */
int Obj_128A84_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto kill_128CF8;                                   /* 0x128A8A bne.w */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto kill_128CE8;                                   /* 0x128A92 ble.w */

    wr32(m, obj + 0x10u, SPR_128A84);                       /* 0x128A96 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {          /* 0x128A9E btst #1 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));           /* 0x128AB6 */
        Obj_RelayTimer(m, rd16(m, obj + 0x4Cu));            /* 0x128ABC jmp 0x0CBD12 */
        return 0;
    }

    /* 0x128AC2: steer along the heading, trim the parent momentum, list the
     * object for drawing (0x0CBDA4: A5+0x8FE then A5+0x906), draw it */
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));          /* jsr 0x0CBE12 */
    sub_0CBDBA(m, obj);                                     /* jsr 0x0CBDBA   */
    sub_0CBDA4(m, obj);                                     /* jsr 0x0CBDA4   */
    sub_11C32C(m, obj);                                     /* jsr 0x11C32C   */
    Obj_DrawSprite(m, obj);                                 /* jsr 0x001282   */

    /* 0x128AE4: cmpi.w #0x18,(0x42,A0) ; bge 0x128AF4 ; addq.w #1,(0x42,A0) */
    if ((int16_t)rd16(m, obj + 0x42u) < 0x0018) {
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));
        wr32(m, obj + 0x00u, 0x00128A84u);                  /* 0x128A7C */
        return 0;
    }

    /* 0x128AF4: hand the target/timer state over to (0x48,A0). The 0x11B06E
     * call that would list the object again is *not* made on this path: the
     * capture only ever shows one 4-byte append into each of A5+0x8FE and
     * A5+0x906 per tick (two appends would advance them by 8). */
    {
        uint32_t a1 = rd32(m, obj + 0x48u);
        wr16(m, a1 + 0x4Cu, rd16(m, obj + 0x4Cu));
        wr8(m, a1 + 0x4Fu, rd8(m, obj + 0x4Fu));
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr32(m, a1 + 0x2Cu, rd32(m, obj + 0x2Cu));
        wr32(m, a1 + 0x30u, rd32(m, obj + 0x30u));
        wr32(m, a1 + 0x28u, rd32(m, obj + 0x28u));
        wr32(m, a1 + 0x24u, rd32(m, obj + 0x24u));
        wr16(m, a1 + 0x20u, rd16(m, obj + 0x20u));
        wr16(m, a1 + 0x42u, rd16(m, obj + 0x42u));
        wr16(m, a1 + 0x3Au, rd16(m, obj + 0x3Au));
        wr16(m, a1 + 0x3Eu, rd16(m, obj + 0x3Eu));
    }
    /* 0x128B40 bra.w 0x128D4A: the object is freed here */
    sub_128D4A(m, obj);
    return 0;

kill_128CE8:
    {
        /* 0x128CE8: jsr 0xCBDDE; add.l (0x4C,A0),(0x48,A1) */
        uint32_t a1 = sub_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }
    /* fall through into 0x128CF8 */

kill_128CF8:
    {
        /* 0x128CF8: free the (0x48,A0) child if any */
        uint32_t d0 = rd32(m, obj + 0x48u);
        if (d0 != 0u)
            Obj_Free(m, d0);
        /* 0x128D0C: spawn 0x13E4EE (group 5, two groups) with the +0x7 offset
         * argument, then sound 0x34 */
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x00088000u + sub_13E4D0(m, obj));
        sub_13F238(m, 0x0034);
        wr32(m, obj + 0x00u, 0x00128D4Au);                  /* 0x128D42 */
    }
    return 0;
}
