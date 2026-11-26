/* ROM 0x180F74: delayed boss sub-part activation tick (A0 = object).
 * Counts down +0x06 and re-arms 0x180F74 while > 0. Once <= 0, plays hooked
 * sound 0x2A, initializes stats/hitboxes from 0x181026 (0x184888), initializes
 * 0x179A5C animation from 0x18103C, installs script10 0x181050 (+0x44/+0x48/
 * +0x4A=0), sets +0x34=3, +0x0C=0x180FDC, and transitions to 0x17BEC8. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_180F74(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);          /* 0x180F74 */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                               /* 0x180F78 */
        wr32(m, obj + 0x00u, 0x00180F74u);                     /* 0x180F6C */
        return 0;
    }

    Sound_PlayHooked(m, 0x002Au);                              /* 0x180F7A..0x180F8A */
    Obj_184888(m, obj, 0x00181026u);                           /* 0x180F8E..0x180F94 */
    uint32_t d0 = rd32(m, obj + 0x24u);                        /* 0x1848C4..0x1848D0 leaves quotient in D0 */

    {
        uint16_t idx = rd16(m, 0x0018103Cu);                   /* 0x180F9A..0x180FA0 */
        d0 = (d0 & 0xFFFF0000u) | idx;
        wr16(m, obj + 0x40u, idx);                             /* 0x180FA2 */
        wr32(m, obj + 0x3Au, 0x0018103Eu);                     /* 0x180FA6 */
        if (Obj_AnimInit(m, obj, idx) < 0)                     /* 0x180FAA..0x180FB4 */
            return -1;
    }

    uint32_t a1 = 0x00181050u;                                 /* 0x180FB6 */
    r->d[0] = d0;
    r->a[1] = a1;
    wr32(m, obj + 0x44u, a1);                                  /* 0x180FBC */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));                    /* 0x180FC0 */
    wr16(m, obj + 0x4Au, 0u);                                  /* 0x180FC6 */
    wr8(m, obj + 0x34u, 3u);                                   /* 0x180FCA */
    wr32(m, obj + 0x0Cu, 0x00180FDCu);                         /* 0x180FD0 */
    wr32(m, obj + 0x00u, 0x0017BEC8u);                         /* 0x180FD8 -> 0x17BEC0 */
    return 0;
}
