#include "fn_auto.h"
/* ROM 0x131144: initialize a linked segment and extend the chain to ten segments. */
#include "fn_call_frames.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"

static void offset_segment(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t entry = r->a[1] + 2 * (int16_t)r->d[0];
    for (unsigned axis = 0; axis < 2u; ++axis) {
        uint32_t product = (uint32_t)((int16_t)rd16(m, entry + (axis ? 0u : 0x80u)) * 32);
        r->d[1] = (product << 16) | (product >> 16);
        uint16_t delta = (uint16_t)r->d[1];
        uint32_t origin = obj + 0x50u + 2u * axis;
        uint32_t relative = obj + 0x54u + 2u * axis;
        wr16(m, origin, (uint16_t)(rd16(m, origin) - delta));
        wr16(m, relative, (uint16_t)(rd16(m, relative) + delta));
    }
}

int Fn_131144(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_long(m, r, 0x13114Au);
    Obj_ResetFields(m, obj);
    r->a[7] += 4u;
    wr8(m, obj + 0x1Du, 0x78u);
    wr16(m, obj + 0x28u, 1u);
    wr16(m, obj + 0x2Cu, 2u);
    wr16(m, obj + 0x30u, 2u);
    wr16(m, obj + 0x2Eu, 0xDu);
    wr16(m, obj + 0x32u, 0xDu);
    uint32_t parent = rd32(m, obj + 8u);
    wr32(m, obj + 0x44u, parent);
    wr32(m, parent + 0x48u, obj);
    wr32(m, obj + 0x48u, 0u);
    wr16(m, obj + 0x50u, rd16(m, parent + 0x50u));
    wr16(m, obj + 0x52u, rd16(m, parent + 0x52u));
    wr16(m, obj + 0x54u, 0u);
    wr16(m, obj + 0x56u, 0u);
    r->d[0] = set_word(r->d[0], rd16(m, parent + 0x20u));
    r->a[1] = 0x027E40u;
    offset_segment(m, r); /* 0x13119C–0x1311C0 */

    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x50u));
    r->d[1] = set_word(r->d[1], rd16(m, obj + 0x52u));
    push_long(m, r, 0x1311D2u);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    uint16_t angle = Obj_Atan2(m, (uint16_t)r->d[0], (uint16_t)r->d[1]);
    r->a[7] += 20u;
    wr16(m, obj + 0x20u, angle);
    push_long(m, r, 0x1311DCu);
    r->d[0] = Rng_NextMem(m);
    r->a[7] += 4u;
    uint8_t spread = (uint8_t)((r->d[0] & 0x3Fu) - 0x20u);
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + spread));
    r->a[1] = 0x027E40u;
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x20u));
    offset_segment(m, r); /* 0x1311F2–0x131216 */

    uint16_t index = (uint8_t)(r->d[0] + 4u) >> 3;
    r->d[0] = set_word(r->d[0], index);
    /* Full extension 0x0520 supplies a word displacement and a word index
     * scaled by four. The PC bases are 0x131220 and 0x13122A. */
    wr32(m, obj + 0x58u, rd32(m, 0x131308u + 4u * index));
    wr32(m, obj + 0x5Cu, rd32(m, 0x131388u + 4u * index));
    wr8(m, obj + 6u, 0u);
    r->a[1] = rd32(m, obj + 8u);
    wr16(m, obj + 0x22u, rd16(m, r->a[1] + 0x22u));
    uint16_t count = (uint16_t)(rd16(m, obj + 0x22u) + 1u);
    wr16(m, obj + 0x22u, count);
    if ((int16_t)count < 10)
        call_spawn(m, r, 0x131266u, 0x131144u, 5u, 2u, 0u);
    wr32(m, obj, 0x131272u);
    r->a[7] += 4u;
    return 0;
}
