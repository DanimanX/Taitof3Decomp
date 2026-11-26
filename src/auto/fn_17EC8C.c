/* ROM 0x17EC8C: delayed boss turret/part activation (A0 = object).
 * Counts down +0x06 and re-arms 0x17EC8C while > 0; on expiry plays sound
 * 0x11, initializes stats via 0x184888 (0x17ECEE), starts anim 9 (0x182956),
 * installs script 0x17ED04 and data 0x1828E6, and transitions to 0x17C14C. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_17EC8C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t timer = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x17EC8C */
    wr16(m, obj + 0x06u, (uint16_t)timer);
    if (timer > 0) {                                      /* 0x17EC90 -> 0x17EC84 */
        wr32(m, obj, 0x0017EC8Cu);
        return 0;
    }

    Sound_PlayHooked(m, 0x11u);                           /* 0x17EC92..0x17ECA2 */
    Obj_184888(m, obj, 0x0017ECEEu);                      /* 0x17ECA6..0x17ECAC */

    uint32_t anim_rec = 0x00182956u;                      /* 0x17ECB2 */
    uint16_t idx = rd16(m, anim_rec);                     /* 0x17ECB8 (9) */
    uint32_t anim_data = anim_rec + 2u;
    wr16(m, obj + 0x40u, idx);                            /* 0x17ECBA */
    wr32(m, obj + 0x3Au, anim_data);                      /* 0x17ECBE */
    if (Obj_AnimInit(m, obj, idx) < 0)                    /* 0x17ECC2..0x17ECCC */
        return -1;

    uint16_t dir = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint16_t pal = rd16(m, anim_data + (uint32_t)dir * 6u + 4u);

    uint32_t a1 = 0x0017ED04u;                            /* 0x17ECCE */
    wr32(m, obj + 0x44u, a1);                             /* 0x17ECD4 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));               /* 0x17ECD8 */
    wr16(m, obj + 0x4Au, 0u);                             /* 0x17ECDE */
    wr32(m, obj + 0x0Cu, 0x001828E6u);                    /* 0x17ECE2 */
    wr32(m, obj, 0x0017C14Cu);                            /* 0x17ECEA -> 0x17C144 */

    r->d[0] = (uint32_t)pal;
    r->a[1] = a1;
    return 0;
}
