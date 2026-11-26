/* ROM 0x1957AE: boss encounter init after +0x06 countdown reaches 0 (A0 = object).
 * Decrements +0x06 and re-arms 0x1957AE while > 0. Once <= 0, resets boss
 * globals (0x1847E8), sets +0x16/+0x1A/+0x50/+0x54/+0x64/+0x62 and bounds,
 * clears bit 4 of A5+0x4B26 and A5+0x4BA4/4B9E, sets HP = 0x03E80000 scaled
 * by 0x18495E and sub-bars A5+0x4B0C/4B10/4B14/4B18, spawns 16 parts in group
 * 5 (0x17A8D0 / 0x17AAEE) and 0x197310 in group 6, queues 6 palette uploads
 * (0x5C80/0x6400/0x6380/0x6D00/0x6D80/0x6E00), spawns 6 group-3 parts
 * (0x118D12, 0x118CF6, 0x17B19E, 3x 0x17A664), and arms 0x195BAA. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

int Fn_1957AE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x1957AE */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {                                        /* 0x1957B2 */
            wr32(m, obj + 0x00u, 0x001957AEu);              /* 0x1957A6 */
            return 0;
        }
    }

    Boss_ResetGlobals(m, obj);                              /* 0x1957B4 (0x1847E8) */
    wr16(m, obj + 0x16u, 0x0040u);                          /* 0x1957BA */
    wr16(m, obj + 0x1Au, 0x0010u);                          /* 0x1957C0 */
    wr16(m, obj + 0x50u, 0x0080u);                          /* 0x1957C6 */
    wr16(m, obj + 0x54u, 0x006Eu);                          /* 0x1957CC */
    wr32(m, obj + 0x64u, 0x00195D5Cu);                      /* 0x1957D2 */
    wr16(m, F3_A5 + 0x4B38u, 1u);                           /* 0x1957DA */
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u)); /* 0x1957E0 */
    wr16(m, obj + 0x62u, 0u);                               /* 0x1957E6 */
    wr16(m, F3_A5 + 0x4B2Au, 0x005Au);                      /* 0x1957EA */
    wr16(m, F3_A5 + 0x4B2Cu, 0x0064u);                      /* 0x1957F0 */
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);                      /* 0x1957F6 */
    wr16(m, F3_A5 + 0x4B32u, 0x001Cu);                      /* 0x1957FC */
    wr16(m, F3_A5 + 0x4BA4u, 0u);                           /* 0x195802 */
    wr16(m, F3_A5 + 0x4B9Eu, 0u);                           /* 0x195806 */

    wr32(m, obj + 0x24u, 0x03E80000u);                      /* 0x19580A */
    Obj_18495E(m, obj);                                     /* 0x195812 */

    {
        uint32_t hp = rd32(m, obj + 0x24u);                 /* 0x195818 */
        wr32(m, F3_A5 + 0x4B0Cu, hp);
        wr32(m, F3_A5 + 0x4B10u, hp);
        wr32(m, F3_A5 + 0x4B14u, hp);
        uint32_t tenth = hp / 0x0Au;                        /* 0x195828 */
        wr32(m, F3_A5 + 0x4B18u, tenth);
        wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - tenth * 3u);
        wr32(m, F3_A5 + 0x4B14u, rd32(m, F3_A5 + 0x4B14u) - tenth);
    }

    static const struct { uint32_t fn, arg; } parts16[16] = {
        { 0x0017A8D0u, 0x00197632u },
        { 0x0017A8D0u, 0x00197642u },
        { 0x0017A8D0u, 0x00197652u },
        { 0x0017A8D0u, 0x00197662u },
        { 0x0017A8D0u, 0x00197672u },
        { 0x0017A8D0u, 0x00197622u },
        { 0x0017A8D0u, 0x00197682u },
        { 0x0017A8D0u, 0x00197692u },
        { 0x0017A8D0u, 0x001976A2u },
        { 0x0017A8D0u, 0x001976B2u },
        { 0x0017A8D0u, 0x001976C2u },
        { 0x0017A8D0u, 0x001976D2u },
        { 0x0017A8D0u, 0x001976E2u },
        { 0x0017A8D0u, 0x001976F2u },
        { 0x0017AAEEu, 0x001975F2u },
        { 0x0017A8D0u, 0x00197612u },
    };
    for (int i = 0; i < 16; i++)
        (void)Obj_Spawn(m, obj, parts16[i].fn, 5u, 2u, parts16[i].arg);

    (void)Obj_Spawn(m, obj, 0x00197310u, 6u, 1u, 0u);       /* 0x195A2C */

    static const struct { uint32_t src; uint16_t dst; } pals[6] = {
        { 0x0001A200u, 0x5C80u },
        { 0x00019500u, 0x6400u },
        { 0x0001D880u, 0x6380u },
        { 0x0001E740u, 0x6D00u },
        { 0x0001B400u, 0x6D80u },
        { 0x0001B400u, 0x6E00u },
    };
    for (int i = 0; i < 6; i++)
        Pal_QueueUpload(m, pals[i].src, pals[i].dst, 0x0020u);

    static const struct { uint32_t fn, arg; } parts6[6] = {
        { 0x00118D12u, 0x00198040u },
        { 0x00118CF6u, 0x0019804Eu },
        { 0x0017B19Eu, 0x00000000u },
        { 0x0017A664u, 0x0019805Cu },
        { 0x0017A664u, 0x00198070u },
        { 0x0017A664u, 0x00198084u },
    };
    uint32_t d0 = 0u;
    for (int i = 0; i < 6; i++)
        d0 = Obj_Spawn(m, obj, parts6[i].fn, 3u, 2u, parts6[i].arg);
    r->d[0] = d0;

    wr32(m, obj + 0x00u, 0x00195BAAu);                      /* 0x195BA2 */
    return 0;
}
