#include "obj_128BB2.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"
#include "../video/sprite.h"

/* Frame list walked by 0x128CAA: `lea (0x8e,PC),A1` at 0x128CC8, PC base
 * 0x128CCA -> 0x128D58. */
#define ANIM_128D58 0x00128D58u

/* ROM 0x0CBDA4: append A0 to the deferred-draw list at A5+0x8FE, then to the
 * hazard list at A5+0x906 (it falls through into 0x0CBDAE). */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0CBDBA: drop the parent back-reference (+0x36); unless the object is in
 * the "dead" colour (0x35 == 2) the referenced object's momentum (+0x28) is
 * divided by four first. */
static void sub_0CBDBA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0x02u) {
        uint32_t d0 = rd32(m, obj + 0x36u);
        if (d0 != 0u) {
            uint32_t a1 = d0;
            wr32(m, a1 + 0x28u, (uint32_t)((int32_t)rd32(m, a1 + 0x28u) >> 2));
        }
    }
    wr32(m, obj + 0x36u, 0u);
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

/* ROM 0x0CC2BC: 16-bit random value below (d0 * (0x180 - A5+0x878)) >> 8; a
 * zero divisor becomes 1. */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0)
{
    uint16_t d1 = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x878u));
    uint32_t prod = (uint32_t)d1 * (uint32_t)d0;
    prod <<= 8;
    d1 = (uint16_t)(prod >> 16);                    /* swap */
    if (d1 == 0u)
        d1 = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x878u) % d1); /* divu remainder */
}

/* ROM 0x0CBDDE: A1 = (0x36,A0); when the object is blinking (0x35 == 1) or
 * dead (0x35 == 2) A1 follows the first parent hop (0x8,A1). Callers use it as
 * the object the +0x4C frame count is charged to. */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t c = rd8(m, obj + 0x35u);
    if (c != 0x02u && c == 0x01u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x13E4D0: spawn argument built from the two sprite-def words at
 * (0x10,A0), each scaled by 8 into one byte of D0 = 3. */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint32_t d0 = 3u;
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
    d0 = (d0 & ~0x0000FF00u) | ((uint32_t)d1 << 8);   /* bfins D1,D0{8:8} */
    d1 = (uint16_t)(rd16(m, a1 + 2u) << 3);
    d0 = (d0 & ~0x000000FFu) | (uint32_t)(d1 & 0xFFu); /* bfins D1,D0{0:8} */
    return d0;
}

/* ROM 0x13F238: sound command remap hook */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x128CAA: draw the object once, 0x20 right and 7 up of its position, with
 * the position/sprite-def fields saved and restored around the draw. */
static void sub_128CAA(const f3mem_t *m, uint32_t obj)
{
    uint16_t sx = rd16(m, obj + 0x14u);
    uint16_t sy = rd16(m, obj + 0x18u);
    uint16_t sz = rd16(m, obj + 0x1Cu);
    wr8(m, obj + 0x1Cu, 0x01u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0020u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0007u));
    Obj_Animate(m, obj, ANIM_128D58);                /* 0x128CCE jsr 0x0CBD52 */
    Obj_DrawSprite(m, obj);                          /* 0x128CD4 jsr 0x1282 */
    wr16(m, obj + 0x1Cu, sz);
    wr16(m, obj + 0x18u, sy);
    wr16(m, obj + 0x14u, sx);
}

/* ROM 0x128BAA: stay on 0x128BB2. */
static void sub_128BAA(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00128BB2u);
}

/* ROM 0x128CE8 (falls into 0x128CF8): free the old parent if any, spawn a
 * 0x13E4EE child whose argument comes from 0x13E4D0, send sound command 0x34
 * and switch to 0x128D4A. */
static void kill_128CE8(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x48u);
    if (d0 != 0u)
        Obj_Free(m, d0);                             /* 0x128D02 jsr 0x105EC */

    uint32_t arg = sub_13E4D0(m, obj);               /* 0x128D0E jsr 0x13E4D0 */
    (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, arg); /* 0x128D24 jsr 0x1057C */
    /* 0x128D32: move.w #0x34,-(SP); jsr 0x13F238; jsr 0x39D8 */
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
    wr32(m, obj + 0x00u, 0x00128D4Au);               /* 0x128D42 */
}

/* ROM 0x128BB2: bouncing bullet. Frozen (A5-0x144B bit 4) or out of HP
 * (signed (0x24,A0) <= 0) it dies through 0x128CE8; while the scroll lock
 * A5+0x1D58 bit 1 is set and it is on screen it takes the blink/explode path;
 * otherwise it steers its angle (+0x20) at its target (+0x50/+0x52) by at most
 * 3 per frame, moves along +0x20, lists itself for drawing, and fires a
 * 0xCD1AE child every 0x1E frames. */
int Obj_128BB2_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto free_128CF8;                            /* 0x128BB8 bne */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto free_128CE8;                            /* 0x128BC0 ble */

    wr32(m, obj + 0x10u, 0x00043BF4u);               /* 0x128BC4 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= (int16_t)-0x0A && x <= (int16_t)0x011C) {
            /* 0x128BE4: own colour, add +0x4C frames to the A5+0x1D50 timer,
             * then the blink/explode animation 0x0CBB2C. */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    /* 0x128BF0: angle towards the target, corrected by at most 3 per frame */
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, obj + 0x18u));
    uint16_t ang = Obj_Atan2(m, dx, dy);             /* jsr 0x6422 */
    uint8_t diff = (uint8_t)(ang - rd16(m, obj + 0x20u));   /* sub.w (0x20,A0) */
    if ((int8_t)diff > 0) {
        if ((int8_t)diff > 3)
            diff = 3;
    } else if ((int8_t)diff < -3) {
        diff = (uint8_t)-3;
    }
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + diff)); /* add.b D0b */

    /* 0x128C30: speed (+0x42) creeps down to 2 */
    if ((int16_t)rd16(m, obj + 0x42u) > 2)
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 1u));

    /* 0x128C46: move along the angle, list for drawing, refresh the colour,
     * draw, then the offset sprite pass */
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));   /* jsr 0x0CBE12 */
    sub_0CBDBA(m, obj);                              /* jsr 0x0CBDBA */
    sub_0CBDA4(m, obj);                              /* jsr 0x0CBDA4 */
    sub_11C32C(m, obj);                              /* jsr 0x11C32C */
    Obj_DrawSprite(m, obj);                          /* jsr 0x1282 */
    sub_128CAA(m, obj);                              /* bsr 0x128CAA */

    /* 0x128C6C: every 0x1E frames fire a 0xCD1AE bullet in group 9 (2 groups) */
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t <= 0) {
        wr16(m, obj + 0x06u, 0u);
        if (sub_0CC2BC(m, 0x001Eu) == 0u)
            (void)Obj_Spawn(m, obj, 0x000CD1AEu, 9u, 2u, 0x00088000u);
    }

    sub_128BAA(m, obj);                              /* 0x128CA6 */
    return 0;

free_128CE8:
    {
        /* 0x128CE8: jsr 0xCBDDE; A1 = sub_0CBDDE; D0 = (0x4C,A0);
         * add.l D0,(0x48,A1) — the frame count is charged to the referenced
         * object's script timer before the kill runs at 0x128CF8. */
        uint32_t a1 = sub_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }
    kill_128CE8(m, obj);
    return 0;

free_128CF8:
    kill_128CE8(m, obj);                             /* 0x128CF8 */
    return 0;
}
