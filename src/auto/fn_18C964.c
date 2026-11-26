/* ROM 0x18C964: boss-intro/section-setup object (A0 = object).
 * Counts down +0x06 while re-arming 0x18C964. On the final tick resets boss
 * globals (0x1847E8), queues 8 bank palettes (0x17B3AE), loads the 0x18C924
 * parameter block (+0x64, A5+0x4B38, three palette uploads to 0x5700/0x5580/
 * 0x5600), sets boss HP/timer flags, spawns four 0x118D12 phase scripts and
 * one 0x17B056 object, queues palette 0x5780 from 0x01A880, and advances to
 * 0x18CAB4. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

int Fn_18C964(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x18C964 subq.w #1 */
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {                                /* 0x18C968 bgt 0x18C95C */
        wr32(m, obj, 0x0018C964u);
        return 0;
    }

    uint32_t blk = 0x0018C924u;                          /* 0x18C96A lea (-0x48,PC),A1 */
    Boss_ResetGlobals(m, obj);                           /* 0x18C972 */
    Boss_UploadPalettes(m);                              /* 0x18C978 */

    wr32(m, obj + 0x64u, rd32(m, blk));                  /* 0x18C97E */
    blk += 4u;
    wr16(m, F3_A5 + 0x4B38u, rd16(m, blk));              /* 0x18C982 */
    blk += 2u;
    wr16(m, F3_A5 + 0x4B26u, rd16(m, F3_A5 + 0x4B26u) & (uint16_t)~0x0800u); /* 0x18C986 bfclr {4:1} */

    Pal_QueueUpload(m, rd32(m, blk), 0x5700u, 0x0020u);  /* 0x18C98C..0x18C99A */
    blk += 4u;
    Pal_QueueUpload(m, rd32(m, blk), 0x5580u, 0x0020u);  /* 0x18C9A4..0x18C9B2 */
    blk += 4u;
    Pal_QueueUpload(m, rd32(m, blk), 0x5600u, 0x0010u);  /* 0x18C9BC..0x18C9CA */
    blk += 4u;

    wr16(m, obj + 0x62u, 0u);                            /* 0x18C9D4 */
    wr16(m, F3_A5 + 0x4B7Eu, 0xFFFFu);                   /* 0x18C9D8 */
    wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) | 0x0002u); /* 0x18C9DE bfset {14:1} */
    wr16(m, F3_A5 + 0x4B2Au, 0x0032u);                   /* 0x18C9E4 */
    wr16(m, F3_A5 + 0x4B2Cu, 0x0032u);                   /* 0x18C9EA */
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);                   /* 0x18C9F0 */
    wr16(m, F3_A5 + 0x4B32u, 0x0030u);                   /* 0x18C9F6 */

    (void)Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018EDC0u); /* 0x18C9FC */
    (void)Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018EDCEu); /* 0x18CA1A */
    (void)Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018EDDCu); /* 0x18CA38 */
    (void)Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018EDEAu); /* 0x18CA56 */
    r->d[0] = Obj_Spawn(m, obj, 0x0017B056u, 3u, 2u, 0u);      /* 0x18CA74 */
    Pal_QueueUpload(m, 0x0001A880u, 0x5780u, 0x0020u);   /* 0x18CA90 */

    wr32(m, obj, 0x0018CAB4u);                           /* 0x18CAAC */
    r->a[1] = blk;                                       /* 0x0018C936 */
    return 0;
}
