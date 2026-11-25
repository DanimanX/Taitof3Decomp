#include "obj_101B20.h"
#include "obj_E8B72.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables scaled by D1w (0..50) */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    int32_t idx = (int32_t)(int16_t)d0w * 4;
    uint32_t d0 = rd32(m, (uint32_t)(0x000D2D64 + idx));
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2ECC + idx));
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
    return 0;
}

/* ROM 0x101B20 */
int Obj_101B20_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t speed = rd16(m, obj + 0x60u);
    if (speed > 50u)
        return -1;

    Obj_SeedFromParams(m, obj, 0x00101D40u);

    uint16_t r0 = (uint16_t)(Obj_Next4Way(m) & 0x000Fu);
    uint32_t frame_ptr = rd32(m, 0x00101D78u + (uint32_t)r0 * 4u);
    Obj_SetFrame(m, obj, frame_ptr);

    uint16_t r1 = (uint16_t)(Obj_Next4Way(m) & 0x0003u);
    wr8(m, obj + 0x1Cu, (uint8_t)r1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & (uint8_t)~0x02u) | 0x04u));

    uint32_t d0, d1;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), speed, &d0, &d1) < 0)
        return -1;
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);

    uint16_t r2 = (uint16_t)(Obj_Next4Way(m) & 0x0003u);
    uint32_t tbl = rd32(m, obj + 0x68u);
    uint32_t rec = rd32(m, tbl + (uint32_t)r2 * 4u);
    wr16(m, obj + 0x1Eu, rd16(m, rec + 0x00u));
    wr16(m, obj + 0x2Cu, rd16(m, rec + 0x02u));
    wr16(m, obj + 0x2Eu, rd16(m, rec + 0x04u));
    wr16(m, obj + 0x30u, rd16(m, rec + 0x06u));
    wr16(m, obj + 0x32u, rd16(m, rec + 0x08u));
    wr32(m, obj + 0x0Cu, rd32(m, rec + 0x0Au));

    uint16_t yoff = rd16(m, obj + 0x0Eu);
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - yoff));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - yoff));
    wr32(m, obj + 0x00u, 0x00101BB6u);
    return 0;
}
