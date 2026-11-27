#include "fn_auto.h"
/* ROM 0x12C2EA: UNVERIFIED (weak capture): Wait for the sound trigger or retire the object when disabled. */
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"

/* ROM 0x0039D8: preserve the save area and dispatch frames around the
 * sound helper, which implements the command's Work RAM and DPRAM writes. */
static void save_sound_frames(const f3mem_t *m, const f3regs_t *r, uint32_t sp,
                              uint16_t cmd)
{
    for (unsigned i = 3u; i > 0u;) {
        --i;
        wr32(m, sp - 12u + 4u * i, r->a[i]);
    }
    for (unsigned i = 3u; i > 0u;) {
        --i;
        wr32(m, sp - 24u + 4u * i, r->d[i]);
    }

    uint32_t globals = r->a[5];
    if (rd8(m, globals - 0x1444u) != 0u ||
        (int16_t)cmd > (int16_t)rd16(m, 0x006750u))
        return;

    /* 0x003A12 calls the command handler via the table at 0x003A22. */
    uint32_t table = rd32(m, globals - 0x1448u);
    uint32_t data = rd32(m, table + 4u * cmd);
    uint8_t type = rd8(m, data++);
    wr32(m, sp - 28u, 0x003A16u);
    switch (type) {
    case 0:
        if ((int8_t)rd8(m, data) >= 0) {
            wr32(m, sp - 32u, 0x003A48u);
            wr32(m, sp - 32u, 0x003A5Au);
        } else {
            wr32(m, sp - 32u, 0x003A78u);
            wr32(m, sp - 32u, 0x003A8Au);
            wr32(m, sp - 32u, 0x003A9Cu);
        }
        break;
    case 1:
        wr32(m, sp - 32u, 0x003ABCu);
        wr32(m, sp - 32u, 0x003ACEu);
        wr32(m, sp - 32u, 0x003AE0u);
        break;
    case 2: {
        /* 0x003AE6..0x003AF0 computes and pushes the channel slot. */
        uint16_t off = (uint16_t)((rd8(m, data) - 1u) * 8u);
        off = (uint16_t)((off & 0xFF00u) | (uint8_t)(off + rd8(m, data + 2u)));
        uint32_t slot = globals + 0x817u + (uint32_t)(int32_t)(int16_t)off;
        wr32(m, sp - 32u, slot);
        if (rd8(m, slot) != 0u)
            wr32(m, sp - 36u, 0x003B12u);
        wr32(m, sp - 36u, 0x003B2Cu);
        wr32(m, sp - 36u, 0x003B46u);
        break;
    }
    default: /* 0x003B4E and 0x003B64 make no nested calls. */
        break;
    }
}

int Fn_12C2EA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7];
    uint32_t globals = r->a[5];
    if ((rd8(m, globals - 0x144Bu) & 0x10u) != 0u) {
        /* 0x12C326 pushes A0, calls Obj_Free, and discards the argument. */
        wr32(m, sp - 4u, obj);
        wr32(m, sp - 8u, 0x12C32Eu);
        wr32(m, sp - 12u, r->a[1]); /* 0x0105EC MOVEM */
        wr32(m, sp - 16u, obj);
        wr32(m, sp - 20u, r->d[0]);
        Obj_Free(m, rd32(m, sp - 4u));
        r->d[0] = rd32(m, sp - 20u); /* 0x010608 */
        r->a[0] = rd32(m, sp - 16u);
        r->a[1] = rd32(m, sp - 12u);
        return 0;
    }
    if (rd8(m, globals + 0x329Bu) == 0u) {
        wr32(m, obj, 0x12C2EAu); /* 0x12C2F8 branches to 0x12C2E2. */
        return 0;
    }

    /* 0x12C2FA pushes the sound word. Both callees preserve the registers. */
    wr16(m, sp - 2u, 0x2Bu);
    wr32(m, sp - 6u, 0x12C304u);
    if (rd8(m, globals + 0x3306u) != 0u)
        wr16(m, sp - 2u, 0x7Eu); /* 0x13F254 remaps the word in place. */
    wr32(m, sp - 6u, 0x12C30Au);
    save_sound_frames(m, r, sp - 6u, rd16(m, sp - 2u));
    Sound_PlayHooked(m, 0x2Bu);

    wr16(m, obj + 6u, 0x14u); /* 0x12C30E clears Z. */
    wr32(m, obj, 0x12C31Eu);   /* 0x12C316 */
    return 0;
}
