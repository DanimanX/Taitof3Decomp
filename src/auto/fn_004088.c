#include "fn_auto.h"
/* ROM 0x004088: initialize the persistent TAITO record directory and copy its defaults. */
#include "fn_call_frames.h"

int Fn_004088(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned i = 7; i > 0;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 8; i > 0;)
        push_long(m, r, r->d[--i]);

    wr32(m, r->a[5] - 0x1448u, 0x407418u);
    if (rd32(m, 0x407410u) != 0x54414954u ||
        rd32(m, 0x407414u) != 0x4F202020u) {
        wr32(m, 0x407410u, 0x54414954u);
        wr32(m, 0x407414u, 0x4F202020u);

        /* 0x0040D4..0x00413C: D7 is a signed initial DBF count.
         * The five offsets at 0x0040F2 land at 0x0040FC/108/114/120/134. */
        int count = (int16_t)rd16(m, 0x6750u) + 1;
        uint32_t directory = 0x407418u;
        uint32_t destination = 0x407818u;
        for (int i = 0; i < count; ++i) {
            wr32(m, directory, destination);
            directory += 4u;
            uint32_t source = rd32(m, 0x64E8u + 4u * (unsigned)i);
            unsigned length;
            switch (rd8(m, source)) {
            case 0: length = 3; break;
            case 1: length = 4; break;
            case 2: length = 8; break;
            case 3: length = 2u + rd8(m, source + 1u); break;
            case 4: length = 2; break;
            default: return -1;
            }
            while (length-- != 0u)
                wr8(m, destination++, rd8(m, source++));
        }
    }

    for (unsigned i = 0; i < 8; ++i, r->a[7] += 4u)
        r->d[i] = rd32(m, r->a[7]);
    for (unsigned i = 0; i < 7; ++i, r->a[7] += 4u)
        r->a[i] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return 0;
}
