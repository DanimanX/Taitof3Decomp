/* ROM 0x0F54F8: expanding/deploying enemy initializer (A0 = object).
 * Seeds HP/hitboxes from 0x0F8214 (0x0CEDE4), installs script 0x0F56E4
 * (0x0CF1D8), loads sprite/hitbox 0x0F570E (0x0CFA86), sets +0x1D = 0xDE,
 * +0x1E = 0x8080, doubles +0x4E, and arms 0x0F5532. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CFA86: load {pal.w, spr.l, hit.l} from A1 into +0x1C/+0x10/+0x68. */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t *a1_io, uint32_t *d0_io)
{
    uint32_t a1 = *a1_io;
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

int Fn_0F54F8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1;

    Obj_0CEDE4(m, obj, 0x000F8214u);                    /* 0x0F54F8..0x0F54FC */
    Obj_StartScript(m, obj, 0x000F56E4u);               /* 0x0F5502..0x0F5506 */
    a1 = 0x000F570Eu;                                   /* 0x0F550C */
    sub_0CFA86(m, obj, &a1, &d0);                      /* 0x0F5510 */

    wr8(m, obj + 0x1Du, 0xDEu);                         /* 0x0F5516 */
    wr16(m, obj + 0x1Eu, 0x8080u);                      /* 0x0F551C */

    uint8_t v = rd8(m, obj + 0x4Eu);                    /* 0x0F5522 */
    d0 = (d0 & 0xFFFFFF00u) | v;
    wr8(m, obj + 0x4Eu, (uint8_t)(v + v));              /* 0x0F5526 */
    wr32(m, obj, 0x000F5532u);                          /* 0x0F552A */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
