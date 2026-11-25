#include "obj_18F1C2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"

#define A5_(off) (F3_A5 + (off))

static void a5_bits_set(const f3mem_t *m, uint32_t off, uint8_t mask)
{ wr8(m, A5_(off), (uint8_t)(rd8(m, A5_(off)) | mask)); }
static void a5_bits_clr(const f3mem_t *m, uint32_t off, uint8_t mask)
{ wr8(m, A5_(off), (uint8_t)(rd8(m, A5_(off)) & (uint8_t)~mask)); }

/* ROM 0x17B02E: score receiver (1 -> (+0x36)->+8, 2 -> +0x36, else -1) */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u)
        return rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (k == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x17A642: arm the damage-flash bit while the shared counter 4B14 is
 * above the threshold 4B0C, draining it by 4B18 */
static void sub_17A642(const f3mem_t *m)
{
    a5_bits_clr(m, 0x4B26u, 0x10u);
    if ((int32_t)rd32(m, A5_(0x4B14u)) > (int32_t)rd32(m, A5_(0x4B0Cu))) {
        a5_bits_set(m, 0x4B26u, 0x10u);
        wr32(m, A5_(0x4B14u), rd32(m, A5_(0x4B14u)) - rd32(m, A5_(0x4B18u)));
    }
}

/* ROM 0x18F4E8: state-2 handler -- walk the 0x18F53A script, then go to
 * state 2 when it ends */
static int state_18F4E8(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, A5_(0x4B26u)) & 0x08u) == 0u) {
        a5_bits_set(m, 0x4B26u, 0x08u);
        wr32(m, obj + 0x44u, 0x18F53Au);
        wr16(m, obj + 0x48u, rd16(m, 0x18F53Au + 4u));
        wr16(m, obj + 0x4Au, 0u);
        a5_bits_set(m, 0x4B3Bu, 0x01u);
    }
    if (Obj_Script10(m, obj) < 0)
        return -1;
    if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
        wr16(m, A5_(0x4B38u), 2u);
        a5_bits_clr(m, 0x4B26u, 0x08u);
        a5_bits_clr(m, 0x4B3Bu, 0x01u);
    }
    return 0;
}

/* ROM 0x18F1C2: boss state dispatcher (state in A5+0x4B38, handlers via +0x64) */
int Obj_18F1C2_Tick(const f3mem_t *m, uint32_t obj)
{
    a5_bits_clr(m, 0x4B27u, 0x80u);
    if ((int16_t)rd16(m, A5_(0x3318u)) >= 0x3840)
        wr32(m, obj + 0x24u, 0u);
    wr32(m, A5_(0x4B0Cu), rd32(m, obj + 0x24u));
    if ((int16_t)rd16(m, A5_(0x4B38u)) == -1) {
        Obj_Spawn(m, obj, 0x17AE0Au, 0, 1, 0);
        wr16(m, obj + 0x06u, 1u);
        wr32(m, obj, 0x18F216u);
        return 0;
    }
    int16_t st = (int16_t)rd16(m, A5_(0x4B38u));
    if (st > 0 && (int32_t)rd32(m, A5_(0x4B0Cu)) <= 0 && st != 3) {
        wr16(m, A5_(0x4B38u), 3u);
        a5_bits_clr(m, 0x4B26u, 0x08u);
        uint32_t d0 = sub_17B02E(m, obj);
        if ((int32_t)d0 >= 0)
            wr32(m, d0 + 0x48u, rd32(m, d0 + 0x48u) + 0x186A0u);
        a5_bits_clr(m, 0x4B3Au, 0x04u);
    }
    wr16(m, A5_(0x4B04u), rd16(m, obj + 0x50u));
    wr16(m, A5_(0x4B08u), rd16(m, obj + 0x54u));
    a5_bits_clr(m, 0x4B26u, 0x10u);
    if ((rd8(m, A5_(0x1D58u)) & 0x02u) != 0u && (rd8(m, obj + 0x23u) & 0x80u) == 0u) {
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x80u));
        Obj_Spawn(m, obj, 0x18F3A0u, 3, 2, 0x100u);
    }
    int16_t d0s = (int16_t)rd16(m, A5_(0x4B38u));
    uint32_t h = rd32(m, rd32(m, obj + 0x64u) + (uint32_t)((int32_t)d0s * 4));
    if (h != 0x18F4E8u)
        return -1;
    if (state_18F4E8(m, obj) < 0)
        return -1;
    wr8(m, A5_(0x4B36u), 0u);
    if (rd8(m, A5_(0x4B28u)) == rd8(m, obj + 0x1Du))
        wr8(m, obj + 0x1Du, rd8(m, A5_(0x4B29u)));
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, rd8(m, A5_(0x4B28u)));
    else
        wr8(m, obj + 0x1Du, rd8(m, A5_(0x4B36u)));
    wr8(m, obj + 0x35u, 0u);
    wr8(m, A5_(0x4B36u), rd8(m, obj + 0x1Du));
    sub_17A642(m);
    if ((rd8(m, A5_(0x4B3Au)) & 0x80u) == 0u) {
        int16_t d0 = (int16_t)rd16(m, A5_(0x4B82u));
        if (d0 < 0)
            d0 = (int16_t)(0 - d0);
        if (d0 > 0x100) {
            int16_t step = 0x10;
            if ((int16_t)rd16(m, A5_(0x4B82u)) >= 0)
                step = -step;
            wr16(m, A5_(0x4B82u), (uint16_t)(rd16(m, A5_(0x4B82u)) + (uint16_t)step));
        } else {
            uint8_t b = (rd8(m, A5_(0x4B3Au)) & 0x20u) ? 0x80u : 0x00u;
            if (b == rd8(m, A5_(0x4B80u)))
                wr16(m, A5_(0x4B82u), 0u);
        }
    } else {
        int16_t step = 0x10;
        if ((rd8(m, A5_(0x4B3Au)) & 0x40u) == 0u)
            step = -step;
        wr16(m, A5_(0x4B82u), (uint16_t)(rd16(m, A5_(0x4B82u)) + (uint16_t)step));
        if ((int16_t)rd16(m, A5_(0x4B82u)) < -0x800)
            wr16(m, A5_(0x4B82u), 0xF800u);
        if ((int16_t)rd16(m, A5_(0x4B82u)) > 0x800)
            wr16(m, A5_(0x4B82u), 0x0800u);
    }
    wr16(m, A5_(0x4B80u), (uint16_t)(rd16(m, A5_(0x4B80u)) + rd16(m, A5_(0x4B82u))));
    wr32(m, obj, 0x18F1C2u);
    return 0;
}
