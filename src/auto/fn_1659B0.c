#include "fn_auto.h"
/* ROM 0x1659B0: UNVERIFIED (weak capture): Retire two objects and restore background graphics after the scroll passes 0x1C0. */
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0105EC: push the argument and JSR return address at the call site,
 * then reproduce the callee's MOVEM around the object pool helper. */
void FnAuto_FreeObject(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint32_t ret)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, obj);
    wr32(m, sp - 8u, ret);
    wr32(m, sp - 12u, r->a[1]);
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 20u, r->d[0]);
    Obj_Free(m, rd32(m, sp - 4u));
    r->d[0] = rd32(m, sp - 20u); /* 0x010608 */
    r->a[0] = rd32(m, sp - 16u);
    r->a[1] = rd32(m, sp - 12u);
}

/* ROM 0x165D72: eight calls to 0x0053B2. Its MOVEM restores D0-D7/A0-A2. */
static void restore_tilemaps(const f3mem_t *m, f3regs_t *r)
{
    static const uint32_t sources[] = { 0x902ECu, 0x903BAu, 0x90484u, 0x90558u };
    static const uint32_t destinations[] = {
        0x613000u, 0x613400u, 0x613020u, 0x613420u,
        0x613040u, 0x613440u, 0x613060u, 0x613460u
    };
    uint32_t sp = r->a[7];
    for (unsigned i = 0; i < 8u; ++i) {
        uint32_t src = sources[i % 4u];
        uint32_t dst = destinations[i];

        wr16(m, sp - 2u, 0u);
        wr32(m, sp - 6u, dst);
        wr32(m, sp - 10u, src);
        wr32(m, sp - 14u, 0x165D88u + 0x1Au * i);
        /* 0x0053B2 MOVEM saves A2-A0, followed by D7-D0. */
        uint32_t save = sp - 14u;
        for (unsigned j = 3u; j > 0u;)
            wr32(m, save -= 4u, r->a[--j]);
        for (unsigned j = 8u; j > 0u;)
            wr32(m, save -= 4u, r->d[--j]);
        /* The flags long starts at the pushed word. Its top bits are zero,
         * so 0x0053DC calls the unflipped mode at 0x00535A. */
        wr32(m, save - 4u, 0x0053DEu);
        Gfx_DrawCompressedTilemap(m, src, dst, rd32(m, sp - 2u));
    }
}

int Fn_1659B0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if ((int16_t)rd16(m, r->a[5] - 0x5D98u) <= 0x1C0) {
        wr32(m, obj, 0x1659B0u); /* 0x1659B6 branches to 0x1659A8. */
        return 0;
    }

    uint32_t sp = r->a[7];
    FnAuto_FreeObject(m, r, rd32(m, r->a[5] + 0x3BBAu), 0x1659C2u); /* 0x1659B8 */

    wr32(m, sp - 4u, r->a[0]);   /* 0x1659C6 MOVEM saves A0. */
    wr32(m, sp - 8u, 0x62A600u); /* 0x1659CA PEA */
    wr32(m, sp - 12u, 0x1659D6u);
    wr32(m, sp - 16u, r->a[6]);  /* 0x176130 LINK A6,#0 */
    for (unsigned i = 8u; i > 0u;) {
        --i;
        wr32(m, sp - 48u + 4u * i, r->d[i]); /* 0x176134 MOVEM */
    }
    r->a[7] = sp - 12u;
    Fn_176130(m, r);            /* 0x1659D0 clears line RAM. */

    wr32(m, sp - 8u, 0x1659DEu); /* 0x1659DA BSR after argument cleanup */
    r->a[7] = sp - 8u;
    restore_tilemaps(m, r);
    r->a[0] = rd32(m, sp - 4u); /* 0x1659DE MOVEM restores A0. */

    r->a[7] = sp;
    FnAuto_FreeObject(m, r, r->a[0], 0x1659EAu); /* 0x1659E2 */
    return 0;
}
