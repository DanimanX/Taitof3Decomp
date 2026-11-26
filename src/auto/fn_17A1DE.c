/* ROM 0x17A1DE: boss spark/particle init (A0 = object).
 * Frees self via 0x0105EC if particle count A5+0x4B24 > 0x1E; else increments
 * A5+0x4B24, places relative to parent via 0x17BDA2, installs script 0x17A26A
 * and anim 0 (0x17A242), clears +0x1E/+0x1C, sets speed +0x42 = 0x400 +
 * Obj_RandRange(0x80), and transitions to 0x179C6E. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

int Fn_17A1DE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x001E) {     /* 0x17A1DE..0x17A1E4 */
        Obj_Free(m, obj);                                 /* 0x179D18 */
        return 0;
    }

    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u)); /* 0x17A1E8 */
    Obj_PolarPlace(m, obj);                               /* 0x17A1EC */

    /* Track D1/D2/D3/A4 outputs left by 0x17BDA2. */
    uint16_t d2 = (uint16_t)(rd8(m, obj + 0x0Eu) << 2);
    uint16_t d3 = rd8(m, obj + 0x0Fu);
    int32_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)d3 * 2u);
    uint32_t p1 = (uint32_t)(s * (int32_t)(int16_t)d2);
    uint32_t d1 = ((p1 & 0xFFFFu) << 16) | (uint32_t)rd16(m, obj + 0x18u);
    r->d[2] = (uint32_t)d2;
    r->d[3] = (uint32_t)d3;
    r->a[4] = rd32(m, obj + 0x08u);

    uint32_t scr = 0x0017A26Au;                           /* 0x17A1F2 */
    wr32(m, obj + 0x44u, scr);                            /* 0x17A1F8 */
    wr16(m, obj + 0x48u, rd16(m, scr + 4u));              /* 0x17A1FC */
    wr16(m, obj + 0x4Au, 0u);                             /* 0x17A202 */

    uint32_t anim_rec = 0x0017A242u;                      /* 0x17A206 */
    uint16_t idx = rd16(m, anim_rec);                     /* 0x17A20C (0) */
    wr16(m, obj + 0x40u, idx);                            /* 0x17A20E */
    wr32(m, obj + 0x3Au, anim_rec + 2u);                  /* 0x17A212 */
    if (Obj_AnimInit(m, obj, idx) < 0)                    /* 0x17A216..0x17A220 */
        return -1;
    uint32_t a1 = 0x0017A24Au;                            /* 0x179550 leaves A1 = +0x3A + 6 */

    wr16(m, obj + 0x1Eu, 0u);                             /* 0x17A222 */
    wr16(m, obj + 0x1Cu, 0u);                             /* 0x17A226 */

    uint16_t rnd = Obj_RandRange(m, 0x0080u);             /* 0x17A22A..0x17A230 */
    uint16_t spd = (uint16_t)(rnd + 0x0400u);             /* 0x17A236 */
    wr16(m, obj + 0x42u, spd);                            /* 0x17A23A */
    wr32(m, obj, 0x00179C6Eu);                            /* 0x17A23E -> 0x179C66 */

    r->d[0] = (uint32_t)spd;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
