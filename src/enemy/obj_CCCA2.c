#include "obj_CCCA2.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CBEB0: speed scaled by the difficulty level (A5+0x87E) */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0)
{
    uint16_t d1 = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x87Eu) << 3) + 0x0C00u);
    uint32_t p = (uint32_t)((int32_t)(int16_t)d0 * (int32_t)(int16_t)d1);
    p <<= 4;
    return (uint16_t)(p >> 16);
}

/* ROM 0x0CCC08: apply the spawn argument (+0x0C) offsets and angle,
 * place at the parent's position */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)(int16_t)(int8_t)(d0 >> 24)));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)(int16_t)(int8_t)(d0 >> 16)));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 8) & 0xFFu));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
}

/* ROM 0x0CCBC0: read the bullet record at A1, then 0x0CCC08 */
static void sub_0CCBC0(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x56u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x58u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x5Cu, rd8(m, a1++));
    wr16(m, obj + 0x54u, (uint16_t)(int16_t)(int8_t)rd8(m, a1));
    sub_0CCC08(m, obj);
}

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

/* ROM 0x0CD12C: become a 0x0CD256 bullet (record 0x0CD15E) */
static void become_CD12C(const f3mem_t *m, uint32_t obj)
{
    sub_0CCBC0(m, obj, 0x000CD15Eu);
    wr16(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x1Du, 0x2Eu);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr16(m, obj + 0x42u, sub_0CBEB0(m, 0x16u));
    wr8(m, obj + 0x4Fu, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x00u, 0x000CD256u);
}

/* ROM 0x0CD1B6: become a 0x0CD256 bullet (record by +0x0E bit 6) */
static void become_CD1B6(const f3mem_t *m, uint32_t obj)
{
    play_sound(m, 0x002Au);
    uint32_t a1 = (rd32(m, obj + 0x0Cu) & 0x4000u) ? 0x000CD234u : 0x000CD220u;
    sub_0CCBC0(m, obj, a1);
    wr16(m, obj + 0x42u, 0x0018u);
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr16(m, obj + 0x42u, sub_0CBEB0(m, 0x1Au));
    wr8(m, obj + 0x4Fu, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x00u, 0x000CD256u);
}

/* ROM 0x0CCCA2: charge-up orbit around the parent, then jump to +0x3A */
int Obj_CCCA2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t next = rd32(m, obj + 0x3Au);
    int expire = (int16_t)(rd16(m, obj + 0x06u) - 1u) <= 0;
    if (expire && next != 0x000CD12Cu && next != 0x000CD1B6u)
        return -1;   /* continuation not translated */

    wr16(m, obj + 0x1Eu, (uint16_t)(rd16(m, obj + 0x1Eu) - rd16(m, obj + 0x42u)));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)(d0 >> 24)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)(d0 >> 16)));
    uint16_t d1 = (uint16_t)((uint16_t)(0u - (uint16_t)(rd8(m, obj + 0x1Fu) - 0x101u)) >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d1));
    d1 = (uint16_t)((uint16_t)(0u - (uint16_t)(rd8(m, obj + 0x1Eu) - 0x101u)) >> 5);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    Obj_DrawSprite(m, obj);
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000CCCA2u);
        return 0;
    }
    if (next == 0x000CD12Cu) become_CD12C(m, obj);
    else become_CD1B6(m, obj);
    return 0;
}
