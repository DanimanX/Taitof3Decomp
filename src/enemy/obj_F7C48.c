#include "obj_F7C48.h"
#include "obj_E8B72.h"

/* ROM 0x0D06E0: pick the nearer live player (0x41B190 / 0x41B21A) -> +0x42 = its 360-angle.
 * +0x42 stays 0xB4 when neither is alive. */
static void pick_player_angle(const f3mem_t *m, uint32_t obj)
{
    static const uint32_t pl[2] = { 0x41B190u, 0x41B21Au };
    uint16_t ang[2] = { 0xFFFFu, 0xFFFFu };
    uint16_t dx[2] = { 0u, 0u }, dy[2] = { 0u, 0u };
    uint32_t a2 = rd32(m, obj + 0x68u);
    wr16(m, obj + 0x42u, 0xB4u);
    for (int i = 0; i < 2; i++) {
        uint32_t p = pl[i];
        if ((int16_t)rd16(m, p + 0x4Cu) < 0) continue;
        uint16_t x = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u) + rd16(m, a2) - 0x17u);
        uint16_t y = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, p + 0x18u) + rd16(m, a2 + 2u) - 0xFu);
        ang[i] = Angle360(m, x, y);
        dx[i] = (int16_t)x < 0 ? (uint16_t)(0u - x) : x;
        dy[i] = (int16_t)y < 0 ? (uint16_t)(0u - y) : y;
    }
    int a_ok = (int16_t)ang[0] >= 0, b_ok = (int16_t)ang[1] >= 0;
    if (!a_ok) {
        if (b_ok) wr16(m, obj + 0x42u, ang[1]);
        return;
    }
    if (!b_ok) { wr16(m, obj + 0x42u, ang[0]); return; }
    uint16_t d2 = dy[0] > dx[0] ? dy[0] : dx[0];
    uint16_t d5 = dy[1] > dx[1] ? dy[1] : dx[1];
    wr16(m, obj + 0x42u, d5 < d2 ? ang[1] : ang[0]);
}

/* ROM 0x0F7C48: init */
int Obj_F7C48_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0xF8040u);
    Obj_InitPalette(m, obj);
    wr32(m, obj + 0x14u, 0x1400000u);
    wr32(m, obj + 0x18u, 0x740000u);
    wr32(m, obj + 0x68u, 0xF8150u);
    pick_player_angle(m, obj);
    if (rd16(m, obj + 0x42u) < 0xB4u) {
        wr32(m, obj + 0x3Eu, 0xE80000u);
        Obj_SetScript(m, obj, 0xF8078u);
    } else {
        wr32(m, obj + 0x3Eu, 0xFFE00000u);
        Obj_SetScript(m, obj, 0xF80B4u);
    }
    Obj_SetAnim(m, obj, 0xF80F0u);
    uint16_t d0 = (uint16_t)((Rng_Mix1D60(m) & 0x7Fu) + 0xB0u);
    wr32(m, obj + 0x3Au, (uint32_t)d0 << 16);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u));
    wr32(m, obj, 0xF7CDCu);
    return 0;
}
