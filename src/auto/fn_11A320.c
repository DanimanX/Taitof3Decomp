#include "fn_auto.h"
/* ROM 0x11A320: verified: the capture discriminates (an empty body fails it): Pull a pattern from the parent queue and initialize its sprite list. */
#include "batch_obj_regs.h"

/* ROM 0x11A444..0x11A64C: the records start with a divisor followed by
 * long sprite pointers. The second table is used by handlers 0x11A52C
 * and 0x11A588 after their first DBF loop. */
static void choose_sprites(const f3mem_t *m, f3regs_t *r, uint32_t table,
                           uint32_t rng_return, uint16_t count, int dbf)
{
    uint16_t bank = rd16(m, r->a[0] + 0x22u);
    r->d[0] = batch_set_word(r->d[0], bank);
    r->a[2] = rd32(m, table + (int32_t)(int16_t)bank * 4);
    uint16_t divisor = rd16(m, r->a[2]);
    r->a[2] += 2u;
    r->d[1] = batch_set_word(r->d[1], divisor);
    if (dbf)
        r->d[2] = batch_set_word(r->d[2], (uint16_t)(count - 1u));
    do {
        wr32(m, r->a[7] - 4u, rng_return);
        uint32_t sample = Rng_NextMem(m) & 0xFFu;
        uint32_t remainder = sample % divisor;
        r->d[0] = (sample / divisor << 16) | remainder;
        wr32(m, r->a[1], rd32(m, r->a[2] + remainder * 4u));
        r->a[1] += 4u;
        if (dbf)
            r->d[2] = batch_set_word(r->d[2], (uint16_t)(r->d[2] - 1u));
    } while (--count != 0);
}

int Fn_11A320(const f3mem_t *m, f3regs_t *r)
{
    static const struct {
        uint32_t handler, layout, rng_return, extra_return;
        uint16_t count;
    } patterns[] = {
        { 0x11A444u, 0x11A6C2u, 0x11A468u, 0, 5 },
        { 0x11A47Eu, 0x11A6DAu, 0x11A4A2u, 0, 5 },
        { 0x11A4B8u, 0x11A6F2u, 0x11A4DCu, 0, 3 },
        { 0x11A4F2u, 0x11A702u, 0x11A516u, 0, 3 },
        { 0x11A52Cu, 0x11A712u, 0x11A550u, 0x11A576u, 5 },
        { 0x11A588u, 0x11A72Eu, 0x11A5ACu, 0x11A5D2u, 4 },
        { 0x11A5E4u, 0x11A746u, 0x11A604u, 0, 1 },
        { 0x11A616u, 0x11A74Eu, 0x11A63Au, 0, 5 },
    };

    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 8u);
    if (rd8(m, parent + 0x34u) != 0) {
        r->a[1] = parent;
        batch_free_object(m, r, 0x11A390u);
        return 0;
    }
    int16_t index = (int16_t)rd16(m, parent + 0x22u);
    if (index < 0) {
        r->a[1] = parent;
        r->d[0] = batch_set_word(r->d[0], (uint16_t)index);
        wr32(m, obj, 0x11A320u); /* 0x11A334 -> 0x11A318 */
        return 0;
    }

    uint32_t record = rd32(m, parent + 0x44u + (uint32_t)index * 4u);
    unsigned type = (record >> 8) & 0xFFu;
    /* 0x11A37E + 0x03E8. The nine handler pointers end at 0x11A78A. */
    uint32_t handler = rd32(m, 0x11A766u + type * 4u);
    unsigned pattern = 0;
    if (handler != 0x11A396u) {
        while (pattern < sizeof patterns / sizeof patterns[0] &&
               patterns[pattern].handler != handler)
            ++pattern;
        if (pattern == sizeof patterns / sizeof patterns[0])
            return -1;
    }

    wr32(m, obj + 0x0Cu, record);
    wr16(m, parent + 0x22u, (uint16_t)(index - 1));
    wr32(m, obj + 0x14u, 0x01400000u);
    wr32(m, obj + 0x18u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x18u, rd8(m, obj + 0x0Cu));
    uint8_t bank = rd8(m, obj + 0x0Du);
    wr16(m, obj + 0x22u, bank);
    wr16(m, obj + 0x42u, rd16(m, 0x11A78Au + bank * 2u));
    r->a[1] = obj + 0x0Fu;
    r->a[2] = handler;
    r->d[0] = type;
    if (handler == 0x11A396u) {
        wr32(m, obj, 0x11A39Eu);
        return 0;
    }

    wr32(m, obj + 0x3Au, patterns[pattern].layout);
    r->a[1] = obj + 0x44u;
    choose_sprites(m, r, 0x11A794u, patterns[pattern].rng_return,
                   patterns[pattern].count, patterns[pattern].count != 1);
    if (patterns[pattern].extra_return != 0)
        choose_sprites(m, r, 0x11A7A8u, patterns[pattern].extra_return, 1, 0);
    wr32(m, obj, 0x11A658u); /* 0x11A650 */
    return 0;
}
