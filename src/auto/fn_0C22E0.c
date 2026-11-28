#include "fn_auto.h"
/* ROM 0x0C22E0: spawn-style setup of an effect object (A0 = object). Plays
 * hooked sound 0x17 around a push of A0, clears the velocity/animation fields
 * through 0x0CBD86, copies +0x0C (word) into +0x14, sign-extends +0x0E into
 * +0x18, puts the zero-extended byte +0x0F into +0x44, installs the script
 * 0x0C2362 at +0x48, +0x60 = 0x26 and arms 0x0C2332. D0 ends as that byte. */
#include "../enemy/obj_E8B72.h"   /* Sound_PlayHooked (0x13F238 + 0x0039D8) */

int Fn_0C22E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7] - 4u;              /* 0x0C22E0 movem.l {A0},-(SP) */
    wr32(m, sp, r->a[0]);
    Sound_PlayHooked(m, 0x0017u);            /* 0x0C22E4..0x0C22F4 */
    r->a[0] = rd32(m, sp);                   /* 0x0C22F8 movem.l (SP)+,{A0} */
    r->a[7] = sp + 4u;

    /* 0x0C22FC jsr 0x0CBD86 */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);

    wr16(m, obj + 0x14u, rd16(m, obj + 0x0Cu));               /* 0x0C2302 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Eu); /* 0x0C2308 ext.w */
    wr16(m, obj + 0x18u, (uint16_t)r->d[0]);                  /* 0x0C230E */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd8(m, obj + 0x0Fu);  /* 0x0C2312 clr.w ; 0x0C2314 */
    wr16(m, obj + 0x44u, (uint16_t)r->d[0]);                  /* 0x0C2318 */
    wr32(m, obj + 0x48u, 0x000C2362u);                        /* 0x0C231C */
    wr16(m, obj + 0x60u, 0x0026u);                            /* 0x0C2324 */
    wr32(m, obj, 0x000C2332u);                                /* 0x0C232A */
    return 0;                                                 /* 0x0C2330 rts */
}
