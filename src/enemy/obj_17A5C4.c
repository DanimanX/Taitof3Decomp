#include "obj_17A5C4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17A5C4 */
int Obj_17A5C4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A5C4: btst.b #3,(-0x144C,A5) ; bne.w 0x17A634 */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x08u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x17A5CE: moveq #3,D0 ; jsr 0x1794A4 ; tst.w D0w ; bne.b 0x17A5BC */
    if (Obj_RandRange(m, 3u) != 0u) {
        wr32(m, obj, 0x0017A5C4u);
        return 0;
    }

    /* 0x17A5DA: copy parent (+0x08) +0x50/+0x54 + own +0x16/+0x1A -> +0x50/+0x54 */
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u)));
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au)));

    /* 0x17A5F6..0x17A618: pack two Obj_RandRange draws from +0x0E/+0x0F into D1 */
    uint16_t r1 = Obj_RandRange(m, rd8(m, obj + 0x0Eu));
    uint16_t r2 = Obj_RandRange(m, rd8(m, obj + 0x0Fu));
    uint32_t d1 = ((uint32_t)r1 << 16) | (uint32_t)r2;

    /* 0x17A618..0x17A632: Obj_Spawn(obj, 0x179DCC, group=5, n=2, arg=d1) */
    Obj_Spawn(m, obj, 0x00179DCCu, 5u, 2u, d1);
    wr32(m, obj, 0x0017A5C4u);
    return 0;
}
