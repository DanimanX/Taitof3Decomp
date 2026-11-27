/* ROM 0x1985F6: boss-part spawn burst (A0 = object). Decrements the delay
 * word at +0x06; while it stays positive it only re-arms 0x1985F6. Otherwise
 * it spawns sixteen objects in group 3, two per call, and re-arms the parent
 * with 0x198904. The first two spawns are 0x17A8D0 (args 0x19BA6A, 0x19BA5A)
 * and are also saved at A5+0x4BBC and A5+0x4BB8; then come nine more 0x17A8D0
 * (args 0x19BA2A, 0x19B9CA, 0x19B9AA, 0x19BAAA, 0x19BABA, 0x19B9DA,
 * 0x19BA7A, 0x19B9FA, 0x19B9EA), three 0x19A1A0 and one 0x19A1C4 with arg 0,
 * and finally 0x17AAEE with arg 0x19BA8A. D0 ends as the last spawned object;
 * A0 is untouched. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"  /* Obj_Spawn (0x01057C) */

int Fn_1985F6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0;

    /* 0x1985F6 subq.w #1,(0x6,A0) ; 0x1985FA bgt 0x1985EE */
    uint16_t t = (uint16_t)(rd16(m, a0 + 0x06u) - 1u);
    wr16(m, a0 + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, a0, 0x001985F6u);              /* 0x1985EE move.l #0x1985f6,(A0) */
        return 0;                              /* 0x1985F4 rts */
    }

    /* 0x1985FC..0x198616 */
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019BA6Au);
    wr32(m, F3_A5 + 0x4BBCu, d0);              /* 0x19861A move.l D0,(0x4bbc,A5) */

    /* 0x19861E..0x198638 */
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019BA5Au);
    wr32(m, F3_A5 + 0x4BB8u, d0);              /* 0x19863C move.l D0,(0x4bb8,A5) */

    /* 0x198640..0x19865A */
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019BA2Au);

    /* 0x19865E..0x198678 */
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019B9CAu);

    /* 0x19867C..0x1986CC: three 0x19A1A0 with arg 0 (pea (0x0).w) */
    d0 = Obj_Spawn(m, a0, 0x0019A1A0u, 3u, 2u, 0u);
    d0 = Obj_Spawn(m, a0, 0x0019A1A0u, 3u, 2u, 0u);
    d0 = Obj_Spawn(m, a0, 0x0019A1A0u, 3u, 2u, 0u);

    /* 0x1986D0..0x1986E8 */
    d0 = Obj_Spawn(m, a0, 0x0019A1C4u, 3u, 2u, 0u);

    /* 0x1986EC..0x1987BA: nine 0x17A8D0 spawns */
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019B9AAu);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019BAAAu);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019BABAu);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019B9DAu);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019BA7Au);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019B9FAu);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x0019B9EAu);

    /* 0x1987BE..0x1987D8 */
    d0 = Obj_Spawn(m, a0, 0x0017AAEEu, 3u, 2u, 0x0019BA8Au);

    wr32(m, a0, 0x00198904u);                  /* 0x1988FC move.l #0x198904,(A0) */
    r->d[0] = d0;                              /* last Obj_Spawn result */
    return 0;                                  /* 0x198902 rts */
}
