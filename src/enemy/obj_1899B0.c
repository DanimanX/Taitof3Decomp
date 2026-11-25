#include "obj_1899B0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"

#define A5_(off) (F3_A5 + (off))

static void a5_bits_set(const f3mem_t *m, uint32_t off, uint8_t mask)
{ wr8(m, A5_(off), (uint8_t)(rd8(m, A5_(off)) | mask)); }
static void a5_bits_clr(const f3mem_t *m, uint32_t off, uint8_t mask)
{ wr8(m, A5_(off), (uint8_t)(rd8(m, A5_(off)) & (uint8_t)~mask)); }

/* ROM 0x18A632: state handler -- walk the 0x18A684 script, then go to state 2 */
static int state_18A632(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, A5_(0x4B26u)) & 0x08u) == 0u) {
        a5_bits_set(m, 0x4B26u, 0x08u);
        wr32(m, obj + 0x44u, 0x18A684u);
        wr16(m, obj + 0x48u, rd16(m, 0x18A684u + 4u));
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

/* ROM 0x189AF6: drain the 4B10 counter while it exceeds the threshold 4B0C */
static void sub_189AF6(const f3mem_t *m)
{
    a5_bits_clr(m, 0x4B26u, 0x10u);
    if ((int32_t)rd32(m, A5_(0x4B10u)) > (int32_t)rd32(m, A5_(0x4B0Cu))) {
        wr32(m, A5_(0x4B10u), rd32(m, A5_(0x4B10u)) - rd32(m, A5_(0x4B18u)));
        a5_bits_set(m, 0x4B26u, 0x10u);
    }
}

/* ROM 0x1899B0: boss state dispatcher (state in A5+0x4B38, handlers via +0x64) */
int Obj_1899B0_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, A5_(0x4B0Cu), rd32(m, obj + 0x24u));
    if ((int16_t)rd16(m, A5_(0x4B38u)) == -1) {
        Obj_Spawn(m, obj, 0x17AE0Au, 0, 1, 0);
        wr16(m, obj + 0x06u, 1u);
        wr32(m, obj, 0x1899ECu);
        return 0;
    }
    if ((rd8(m, A5_(0x4B26u)) & 0x20u) != 0u) {
        a5_bits_clr(m, 0x4B26u, 0x20u);
        Obj_Spawn(m, obj, 0x17AD18u, 0, 1, 0x17AD9Cu);
    }
    if ((rd8(m, A5_(0x1D58u)) & 0x02u) != 0u && (rd8(m, obj + 0x23u) & 0x80u) == 0u) {
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x80u));
        Obj_Spawn(m, obj, 0x17ABDCu, 3, 2, 0x80u);
    }
    wr16(m, A5_(0x4B04u), rd16(m, obj + 0x50u));
    int16_t st = (int16_t)rd16(m, A5_(0x4B38u));
    uint32_t h = rd32(m, rd32(m, obj + 0x64u) + (uint32_t)((int32_t)st * 4));
    if (h != 0x18A632u)
        return -1;
    if (state_18A632(m, obj) < 0)
        return -1;
    a5_bits_clr(m, 0x4B7Cu, 0x30u);
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t px = (int16_t)rd16(m, A5_(0x4B04u));
    if (x != px)
        a5_bits_set(m, 0x4B7Cu, x > px ? 0x20u : 0x10u);
    if ((rd8(m, A5_(0x4B3Bu)) & 0x02u) != 0u) {
        a5_bits_clr(m, 0x4B3Bu, 0x02u);
        if ((int16_t)rd16(m, A5_(0x4B72u)) != 8)
            return -1; /* jump table 0x189B70: not translated */
    }
    wr8(m, A5_(0x4B36u), 0u);
    if (rd8(m, A5_(0x4B28u)) == rd8(m, obj + 0x1Du))
        wr8(m, obj + 0x1Du, rd8(m, A5_(0x4B29u)));
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, rd8(m, A5_(0x4B28u)));
    else
        wr8(m, obj + 0x1Du, rd8(m, A5_(0x4B36u)));
    wr8(m, obj + 0x35u, 0u);
    wr8(m, A5_(0x4B36u), rd8(m, obj + 0x1Du));
    sub_189AF6(m);
    wr32(m, obj, 0x1899B0u);
    return 0;
}
