#include "obj_1199CC.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 -> 0x0039D8 */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
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

/* D2 = {x:10, y:10, angle:8, keep:4}: bfins of the three random fields */
static uint32_t debris_arg(const f3mem_t *m, uint32_t obj, uint32_t d2, uint8_t ang_base)
{
    uint16_t d0 = (uint16_t)((uint16_t)((Rng_NextMem(m) & 0x1Fu) + 0x30u) + rd16(m, obj + 0x14u));
    d2 = (d2 & 0x003FFFFFu) | ((uint32_t)(d0 & 0x3FFu) << 22);
    d0 = (uint16_t)((uint16_t)((Rng_NextMem(m) & 0x3Fu) + 0x82u) + rd16(m, obj + 0x18u));
    d2 = (d2 & 0xFFC00FFFu) | ((uint32_t)(d0 & 0x3FFu) << 12);
    uint8_t b = (uint8_t)((Rng_NextMem(m) & 0x1Fu) + ang_base);   /* addi.b */
    d2 = (d2 & 0xFFFFF00Fu) | ((uint32_t)b << 4);
    return d2;
}

/* ROM 0x1199CC: exploding wreck, 40 x 2 debris pieces */
int Obj_1199CC_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd32(m, F3_A5 + 0x422Cu) != 0u)
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00010000u);
    Obj_DrawSprite(m, obj);
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x001199CCu);   /* 0x1199C4 */
        return 0;
    }
    play_sound(m, 0x0015u);
    wr32(m, obj + 0x10u, 0x0004F730u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x18u));

    /* D2 on entry is the group's snapshot free mask from the dispatcher
     * (0x104E0: move.l (A1)+,D2); its low 4 bits survive the bfins. */
    uint32_t d2 = rd32(m, OBJ_SNAP_MASKS + 4u * rd8(m, obj + 0x04u));
    for (int i = 0; i <= 0x27; i++) {
        d2 = debris_arg(m, obj, d2, 0x80u);
        Obj_Spawn(m, obj, 0x00119B40u, 5u, 2u, d2);
        d2 = debris_arg(m, obj, d2, 0x70u);
        Obj_Spawn(m, obj, 0x00119B40u, 9u, 2u, d2);
    }
    wr32(m, obj + 0x00u, 0x00119ACAu);       /* 0x119AC2 */
    return 0;
}
