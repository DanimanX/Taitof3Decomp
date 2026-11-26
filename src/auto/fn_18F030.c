/* ROM 0x18F030: boss intro / section setup object (A0 = object).
 * Counts down +0x06 while re-arming 0x18F030. On completion jumps into
 * 0x18F05C with A1 = 0x18EFDE: resets boss globals (0x1847E8), loads the
 * 0x18EFDE parameter block (+0x64, A5+0x4B38, three palette uploads to
 * 0x5E00/0x5E80/0x5F00), sets boss HP 0x7D00000 scaled by 0x18495E and split
 * thresholds (A5+0x4B0C..0x4B18), configures boss flags, spawns 0x19090C,
 * 0x118D12 (0x1927AA) and 0x17A664 (0x1928B8), queues palette 0x6400 from
 * 0x019500, and advances to 0x18F1C2. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

int Fn_18F030(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x18F030 subq.w #1 */
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {                                /* 0x18F034 -> 0x18F028 */
        wr32(m, obj, 0x0018F030u);
        return 0;
    }

    uint32_t blk = 0x0018EFDEu;                          /* 0x18F036 lea (-0x5a,PC),A1 */
    Boss_ResetGlobals(m, obj);                           /* 0x18F05C */

    wr32(m, obj + 0x64u, rd32(m, blk));                  /* 0x18F062 */
    blk += 4u;
    wr16(m, F3_A5 + 0x4B38u, rd16(m, blk));              /* 0x18F066 */
    blk += 2u;
    wr16(m, F3_A5 + 0x4B26u, rd16(m, F3_A5 + 0x4B26u) & (uint16_t)~0x0800u); /* 0x18F06A bfclr {4:1} */

    Pal_QueueUpload(m, rd32(m, blk), 0x5E00u, 0x0020u);  /* 0x18F070 */
    blk += 4u;
    Pal_QueueUpload(m, rd32(m, blk), 0x5E80u, 0x0020u);  /* 0x18F088 */
    blk += 4u;
    Pal_QueueUpload(m, rd32(m, blk), 0x5F00u, 0x0020u);  /* 0x18F0A0 */
    blk += 4u;

    wr16(m, obj + 0x62u, 0xFFFFu);                       /* 0x18F0B8 */
    wr16(m, F3_A5 + 0x4B2Au, 0x0050u);                   /* 0x18F0BE */
    wr16(m, F3_A5 + 0x4B2Cu, 0x0032u);                   /* 0x18F0C4 */
    wr16(m, F3_A5 + 0x4B2Eu, 0x0028u);                   /* 0x18F0CA */
    wr16(m, F3_A5 + 0x4B30u, 0x2020u);                   /* 0x18F0D0 */
    wr16(m, F3_A5 + 0x4B32u, 0x0010u);                   /* 0x18F0D6 */
    wr16(m, F3_A5 + 0x4B90u, 0u);                        /* 0x18F0DC */

    wr32(m, obj + 0x24u, 0x07D00000u);                   /* 0x18F0E0 */
    Obj_18495E(m, obj);                                  /* 0x18F0E8 */
    uint32_t hp = rd32(m, obj + 0x24u);                  /* 0x18F0EE */
    wr32(m, F3_A5 + 0x4B0Cu, hp);                        /* 0x18F0F2 */
    wr32(m, F3_A5 + 0x4B10u, hp);                        /* 0x18F0F6 */
    wr32(m, F3_A5 + 0x4B14u, hp);                        /* 0x18F0FA */
    uint32_t tenth = hp / 10u;                           /* 0x18F0FE divul.l #0xa */
    wr32(m, F3_A5 + 0x4B18u, tenth);                     /* 0x18F106 */
    wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - tenth * 5u); /* 0x18F10E..0x18F116 */
    wr32(m, F3_A5 + 0x4B14u, rd32(m, F3_A5 + 0x4B14u) - tenth);      /* 0x18F11A..0x18F11E */

    wr16(m, F3_A5 + 0x4B80u, 0u);                        /* 0x18F122 */
    wr16(m, F3_A5 + 0x4B82u, 0u);                        /* 0x18F126 */
    wr16(m, F3_A5 + 0x4B46u, 0u);                        /* 0x18F12A */
    wr16(m, F3_A5 + 0x4B44u, 0xFFFFu);                   /* 0x18F130 */
    wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) | 0x0008u); /* 0x18F136 bfset {12:1} */
    wr8(m, F3_A5 + 0x4B8Eu, 0x07u);                      /* 0x18F13C */
    wr16(m, F3_A5 + 0x4B92u, 0u);                        /* 0x18F142 */

    (void)Obj_Spawn(m, obj, 0x0019090Cu, 6u, 1u, 0u);            /* 0x18F146 */
    (void)Obj_Spawn(m, obj, 0x00118D12u, 5u, 2u, 0x001927AAu);   /* 0x18F162 */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A664u, 3u, 2u, 0x001928B8u); /* 0x18F180 */
    Pal_QueueUpload(m, 0x00019500u, 0x6400u, 0x0020u);   /* 0x18F19E */

    wr32(m, obj, 0x0018F1C2u);                           /* 0x18F1BA */
    r->a[1] = blk;                                       /* 0x0018EFF0 */
    return 0;
}
