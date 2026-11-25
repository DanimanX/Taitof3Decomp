#include "obj_11B17C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x12579E: once the part has no HP left, spawn the 0x11B1BE debris
 * (group 0, count 1) and the 0x11BF5C explosion (group 5, count 2), both
 * with the packed offset D1. */
static void sub_12579E(const f3mem_t *m, uint32_t obj, uint32_t d1)
{
    if ((int32_t)rd32(m, obj + 0x24u) > 0)
        return;
    (void)Obj_Spawn(m, obj, 0x0011B1BEu, 0u, 1u, d1);
    (void)Obj_Spawn(m, obj, 0x0011BF5Cu, 5u, 2u, d1);
}

/* The per-part callbacks at (+0x0C)->+0: `lea rec(PC),A1 ; jsr 0x0CBD52 ;
 * move.l #d1,D1 ; bsr 0x12579E ; rts`. Returns -1 for a callback that has
 * no translation yet. */
static int part_callback(const f3mem_t *m, uint32_t obj, uint32_t cb)
{
    uint32_t rec, d1;
    switch (cb) {
    case 0x0012573Eu: rec = 0x0012580Cu; d1 = 0x00180010u; break;
    case 0x00125756u: rec = 0x00125846u; d1 = 0x00180018u; break;
    case 0x0012576Eu: rec = 0x00125874u; d1 = 0x00080034u; break;
    case 0x00125786u: rec = 0x001258AEu; d1 = 0x0010001Cu; break;
    default: return -1;
    }
    Obj_Animate(m, obj, rec);
    sub_12579E(m, obj, d1);
    return 0;
}

/* ROM 0x11B17C: boss/multi-part sub-object tick */
int Obj_11B17C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11B17C: the parent's +0x59 retires the part */
    uint32_t parent = rd32(m, obj + 0x08u);
    if (rd8(m, parent + 0x59u) != 0u) {
        Obj_Free(m, obj);                       /* 0x11B1B0 */
        return 0;
    }

    /* 0x11B188: movea.l (0xC,A0),A1 ; movea.l (A1),A1 ; jsr (A1) */
    uint32_t tbl = rd32(m, obj + 0x0Cu);
    if (part_callback(m, obj, rd32(m, tbl)) < 0)
        return -1;

    /* 0x11B190: still alive -> register, colour, draw and re-arm (0x11B174) */
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        Obj_11B06E(m, obj);
        Obj_11B09A(m, obj);
        wr32(m, obj + 0x00u, 0x0011B17Cu);
        return 0;
    }

    /* 0x11B1A2: dead -> frame from the table's second long, then 0x11B102:
     * credit +0x4C to the score receiver, bump the global counter, restore the
     * colour and become the 0x11B12C fade */
    wr32(m, obj + 0x10u, rd32(m, tbl + 4u));
    uint32_t a1 = Obj_0CBDDE(m, obj);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr8(m, obj + 0x35u, 0u);
    wr32(m, obj + 0x00u, 0x0011B12Cu);
    return 0;
}
