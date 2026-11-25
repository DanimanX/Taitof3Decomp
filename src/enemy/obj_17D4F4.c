#include "obj_17D4F4.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../audio/sound_cmd.h"

/* ROM 0x17D4F4: the spawn-in half of the 0x17D586 boss minion. It inherits the
 * parent's packed offset and speed (0x17BD68), drops the speed's low byte,
 * leaves for good (0x17C78E) while it is still outside the 0x1849F8 box,
 * seeds its stats from 0x17DD50, starts the 0x17DD66 animation list through the
 * 0x179A5C engine, installs the 0x17D64A behaviour record, plays sound 0x10
 * (after the 0x13F238 remap) and parks on the 0x17D586 flight step. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* cmp2.w (base)/(base+2) against D0w: bcs (out of range) unless
 * lo < D0w < hi. */
static int in_box(const f3mem_t *m, uint32_t base, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, base), hi = (int16_t)rd16(m, base + 2u);
    int16_t x = (int16_t)v;
    return x > lo && x < hi;
}

/* ROM 0x17D4F4 */
int Obj_17D4F4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D4F4: move.l (0x1D28,A5),(0x4C,A0) ; beq.w 0x17C78E */
    uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, obj + 0x4Cu, tgt);
    if (tgt == 0u) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17D4FE: bsr.w 0x17BD68 -- packed +0xC offset from the parent */
    Obj_InheritOffsetPos(m, obj);

    /* 0x17D502: move.l (0x58,A4),(0x58,A0) ; 0x17D508: bfclr (0x58,A0){24:8} */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        uint32_t spd = rd32(m, a4 + 0x58u);
        wr32(m, obj + 0x58u, spd & 0x00FFFFFFu);
    }

    /* 0x17D50E..0x17D52A: cmp2.w (0x1849F8)/(0x1849FC); bcs.w 0x17C78E */
    if (!in_box(m, 0x001849F8u, rd16(m, obj + 0x50u)) ||
        !in_box(m, 0x001849FCu, rd16(m, obj + 0x54u))) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17D52E: lea 0x17DD50 ; jsr 0x184888 */
    Obj_184888(m, obj, 0x0017DD50u);

    /* 0x17D53A: animation list 0x17DD66 -- first word is the 0x179A5C index */
    {
        uint32_t a1 = 0x0017DD66u;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
    }

    /* 0x17D556: behaviour record 0x17D64A */
    wr32(m, obj + 0x44u, 0x0017D64Au);
    wr16(m, obj + 0x48u, rd16(m, 0x0017D64Eu));
    wr16(m, obj + 0x4Au, 0u);

    /* 0x17D56A: pea 0x10 ; jsr 0x13F238 ; jsr 0x39D8 */
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0010));

    wr32(m, obj + 0x00u, 0x0017D586u);           /* 0x17D57E */
    return 0;
}
