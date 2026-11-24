#include "obj_118F4E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0CCC08: launch from the arg bytes (+0xC): signed byte 0 -> +0x50,
 * signed byte 1 -> +0x52, unsigned byte 2 -> +0x20; then restamp position
 * from the parent plus those offsets and reset anim state. */
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

/* ROM 0x118F4E: reset fields, speed offsets from the word pair table at
 * 0x118FC0 indexed by the low word of the spawn arg, then 0x0CCC08. */
int Obj_118F4E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x118F4E jsr 0x0CBD86 */
    Obj_ResetFields(m, obj);

    /* 0x118F54: move.b #0x26,(0x1D,A0) */
    wr8(m, obj + 0x1Du, 0x26u);

    /* 0x118F5A: lea (0x64,PC),A1 = 0x118FC0; move.w (A1,D0.w*4)+0/+2 */
    uint32_t idx = rd32(m, obj + 0x0Cu) & 0xFFFFu;
    uint32_t a1 = 0x00118FC0u + idx * 4u;
    wr16(m, obj + 0x50u, rd16(m, a1));
    wr16(m, obj + 0x52u, rd16(m, a1 + 2u));

    /* 0x118F70 jsr 0x0CCC08 */
    sub_0CCC08(m, obj);

    /* 0x118F76 */
    wr16(m, obj + 0x42u, 3u);

    /* 0x118F7C */
    wr32(m, obj + 0x00u, 0x00118F84u);
    return 0;
}
