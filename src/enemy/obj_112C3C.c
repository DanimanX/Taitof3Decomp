#include "obj_112C3C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBDA4 (+0x0CBDAE): append obj to lists A5+0x8FE and A5+0x906 */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0CBDDE: A1 = owner to credit */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x113400: position = parent + offset (+0x52,+0x54), mirrored when parent flag bit3 */
void Obj_113400_FollowParent(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t a2 = rd32(m, obj + 0x10u);
    if (!(rd8(m, a1 + 0x50u) & 0x08u)) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + rd16(m, obj + 0x52u)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + rd16(m, obj + 0x54u)));
        return;
    }
    /* 0x11342E */
    wr8(m, obj + 0x1Cu, 0x01u);
    uint16_t d0 = (uint16_t)(rd16(m, a1 + 0x14u) - rd16(m, obj + 0x52u));
    d0 = (uint16_t)(d0 - (uint16_t)(rd16(m, a2 + 0x02u) << 4));
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + rd16(m, obj + 0x54u)));
}

/* ROM 0x112C3C */
int Obj_112C3C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t f = rd8(m, a1 + 0x50u);
    if (f & 0x20u) {
        Obj_Free(m, obj);                   /* 0x112D0C */
        return 0;
    }
    if (f & 0x04u) {
        wr32(m, obj + 0x00u, 0x00112C3Cu);  /* 0x112C34 */
        return 0;
    }

    /* the script handler called at 0x112CF2 (jsr (A1)) */
    uint32_t handler = rd32(m, rd32(m, obj + 0x0Cu));
    if (handler != 0x001140A6u && handler != 0x001140B4u)
        return -1;

    if (!(f & 0x1Au)) {
        /* 0x112C70 */
        wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | rd8(m, obj + 0x35u)));
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        if (rd32(m, obj + 0x2Cu) != 0u && (int32_t)rd32(m, a1 + 0x24u) > 0) {
            int32_t d0 = (int32_t)rd32(m, obj + 0x24u);
            if (d0 != 0) {
                wr32(m, obj + 0x24u, 0u);
                uint8_t k = rd8(m, rd32(m, obj + 0x36u) + 0x22u);
                if (k != 0u && k != 4u && k != 5u)
                    d0 >>= 1;               /* asr.l #1 */
            }
            /* 0x112CBE */
            int32_t hp = (int32_t)(rd32(m, a1 + 0x24u) + (uint32_t)d0);
            wr32(m, a1 + 0x24u, (uint32_t)hp);
            if (hp > 0) {
                sub_0CBDA4(m, obj);
            } else {
                /* 0x112CD0 */
                uint32_t own = sub_0CBDDE(m, obj);
                wr32(m, own + 0x48u, rd32(m, own + 0x48u) + 0x4E20u);
                a1 = rd32(m, obj + 0x08u);
                wr8(m, a1 + 0x50u, (uint8_t)(rd8(m, a1 + 0x50u) | 0x40u));
            }
        }
    }

    /* 0x112CE8 */
    wr8(m, obj + 0x35u, 0u);
    if (handler == 0x001140A6u)
        Obj_Animate(m, obj, 0x001144BCu);   /* ROM 0x1140A6 */
    else
        wr32(m, obj + 0x10u, 0x0004DD60u);  /* ROM 0x1140B4 */
    Obj_113400_FollowParent(m, obj);
    a1 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x00112C3Cu);
    return 0;
}
