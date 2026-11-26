/* ROM 0x193EE0: boss tentacle segment initializer (A0 = object).
 * Calls 0x17A870 to seed fields from (+0x0C)->+0x04 descriptor, dispatch
 * 0x179A5C animation init, and scale HP via 0x18495E. Stores segment index
 * (A2)+ into +0x22, sets +0x44 = -1, spawns ring-buffer child 0x193D82 in
 * groups 3..4 into +0x58, sets +0x06 = 1, and arms 0x193F22. */
#include "fn_auto.h"
#include "../enemy/obj_179588.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* Run 0x179A5C[idx] init half and reproduce its exact A1/D1 register outputs. */
static int anim_init_17A870(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint16_t idx)
{
    uint32_t base = rd32(m, obj + 0x3Au);
    uint8_t ang = rd8(m, obj + 0x20u);

    switch (idx) {
    case 0u:
        Obj_AnimInit(m, obj, 0u);
        r->a[1] = base + 6u;
        return 0;
    case 1u:
        Obj_AnimInit(m, obj, 1u);
        r->a[1] = base + (uint32_t)((uint8_t)(ang + 8u) >> 4) * 4u;
        return 0;
    case 2u:
        Obj_AnimInit(m, obj, 2u);
        r->a[1] = base + (uint32_t)((uint8_t)(ang + 4u) >> 3) * 4u;
        return 0;
    case 3u:
        Obj_AnimInit(m, obj, 3u);
        r->a[1] = base + 14u;
        return 0;
    case 4u:
        Obj_AnimInit(m, obj, 4u);
        r->a[1] = base + (uint32_t)((uint8_t)(ang + 8u) >> 4) * 12u + 12u;
        return 0;
    case 5u:
        Obj_AnimInit(m, obj, 5u);
        r->a[1] = base + 14u;
        r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, base + 15u);
        return 0;
    case 6u: {
        uint32_t a1 = base + (uint32_t)((uint8_t)(ang + 8u) >> 4) * 14u + 12u;
        Obj_AnimInit(m, obj, 6u);
        r->a[1] = a1;
        r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        return 0;
    }
    case 7u: {
        uint32_t a1 = base + (uint32_t)((uint8_t)(ang + 4u) >> 3) * 14u + 12u;
        Obj_AnimInit(m, obj, 7u);
        r->a[1] = a1;
        r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        return 0;
    }
    case 8u:
        Obj_AnimInit(m, obj, 8u);
        r->a[1] = base + 8u;
        return 0;
    case 9u:
        Obj_AnimInit(m, obj, 9u);
        r->a[1] = base + (uint32_t)((uint8_t)(ang + 8u) >> 4) * 6u + 6u;
        return 0;
    case 10u:
        Obj_AnimInit(m, obj, 10u);
        r->a[1] = base + (uint32_t)((uint8_t)(ang + 4u) >> 3) * 6u + 6u;
        return 0;
    case 11u:
        Obj_179588_Tick(m, obj);
        r->a[1] = base + 16u;
        return 0;
    case 12u:
        Obj_AnimInit(m, obj, 12u);
        r->a[1] = base + (uint32_t)((uint8_t)(ang + 8u) >> 4) * 14u + 12u;
        return 0;
    case 13u: {
        /* ROM 0x1795CA: 18-byte record {timer.w, spr.l, pal.w, box1.l, box2.l, dx.b, dy.b} */
        uint32_t a1 = base;
        wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
        wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
        int16_t dx = (int16_t)(int8_t)rd8(m, a1);
        int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 1u);
        wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
        wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
        r->a[1] = a1;
        r->d[1] = (uint32_t)(int32_t)(int8_t)dy;
        return 0;
    }
    case 14u: {
        uint32_t a1 = base + (uint32_t)((uint8_t)(ang + 8u) >> 4) * 16u + 14u;
        Obj_AnimInit(m, obj, 14u);
        r->a[1] = a1;
        r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        return 0;
    }
    case 15u: {
        uint32_t a1 = base + (uint32_t)((uint8_t)(ang + 4u) >> 3) * 16u + 14u;
        Obj_AnimInit(m, obj, 15u);
        r->a[1] = a1;
        r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        return 0;
    }
    case 16u: {
        int8_t b = (int8_t)(uint8_t)(ang - 0x20u);
        if (b < 0) b = 0;
        if (b > 0x40) b = 0x40;
        Obj_AnimInit(m, obj, 16u);
        r->a[1] = base + (uint32_t)((uint8_t)(b + 2) >> 2) * 6u + 6u;
        return 0;
    }
    default:
        return -1;
    }
}

int Fn_193EE0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a3 = rd32(m, obj + 0x0Cu);                    /* 0x17A870 */
    uint32_t a2 = rd32(m, a3 + 0x04u);                    /* 0x17A874 */
    uint32_t anim_ptr = rd32(m, a2 + 0x12u);
    uint16_t anim_idx = rd16(m, anim_ptr);
    if (anim_idx > 16u)
        return -1;

    r->a[3] = a3;
    wr32(m, obj + 0x24u, 0u);                             /* 0x17A878 */
    wr32(m, obj + 0x28u, 0u);                             /* 0x17A87C */
    wr16(m, obj + 0x1Cu, 0u);                             /* 0x17A880 */
    wr16(m, obj + 0x1Eu, 0u);                             /* 0x17A884 */
    wr16(m, obj + 0x22u, 0u);                             /* 0x17A888 */
    wr32(m, obj + 0x4Cu, 0xFFFFFFFFu);                    /* 0x17A88C */

    wr16(m, obj + 0x24u, rd16(m, a2)); a2 += 2u;          /* 0x17A894 */
    wr16(m, obj + 0x28u, rd16(m, a2)); a2 += 2u;          /* 0x17A898 */
    wr32(m, obj + 0x2Cu, rd32(m, a2)); a2 += 4u;          /* 0x17A89C */
    wr32(m, obj + 0x30u, rd32(m, a2)); a2 += 4u;          /* 0x17A8A0 */
    wr16(m, obj + 0x34u, rd16(m, a2)); a2 += 2u;          /* 0x17A8A4 */
    wr16(m, obj + 0x16u, rd16(m, a2)); a2 += 2u;          /* 0x17A8A8 */
    wr16(m, obj + 0x1Au, rd16(m, a2)); a2 += 2u;          /* 0x17A8AC */
    a2 += 4u;                                             /* 0x17A8B0: anim_ptr already read */

    wr16(m, obj + 0x40u, anim_idx);                       /* 0x17A8B4 */
    wr32(m, obj + 0x3Au, anim_ptr + 2u);                  /* 0x17A8B8 */
    anim_init_17A870(m, r, obj, anim_idx);                /* 0x17A8BC..0x17A8C6 */
    Obj_18495E(m, obj);                                   /* 0x17A8C8 */

    wr16(m, obj + 0x22u, rd16(m, a2)); a2 += 2u;          /* 0x193EE6 */
    r->a[2] = a2;
    wr32(m, obj + 0x44u, 0xFFFFFFFFu);                    /* 0x193EEA */

    uint32_t child = Obj_Spawn(m, obj, 0x00193D82u, 3u, 2u, 0u); /* 0x193EF2..0x193F0A */
    r->d[0] = child;
    wr32(m, obj + 0x58u, child);                          /* 0x193F0E */
    wr16(m, obj + 0x06u, 0x0001u);                        /* 0x193F12 */
    /* 0x193F18: beq.b 0x193F28 never taken (1 != 0) */
    wr32(m, obj, 0x00193F22u);                            /* 0x193F1A */
    return 0;
}
