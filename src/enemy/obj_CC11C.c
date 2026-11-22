#include "obj_CC11C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0CEC4C: movement init + per-slot path record (table 0x0CECE4) */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t d0w = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        d0w = (uint16_t)(d0w + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)((int32_t)(int16_t)d0w * 4));
    wr32(m, obj + 0x0Cu, a2);
    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));
}

/* ROM 0x0CEE2C: palette from table 0x0CEE40[+0x4E] */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint8_t c = rd8(m, 0x000CEE40u + rd8(m, obj + 0x4Eu));
    wr8(m, obj + 0x1Du, c);
    wr8(m, obj + 0x4Fu, c);
}

/* ROM 0x0CF1D8: set script, request restart */
static void sub_0CF1D8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x44u, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
}

/* ROM 0x0CFA7C: frame record A1 + table 0x0CFA96[d0] -> sprite + hit box */
static void sub_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w)
{
    uint16_t off = rd16(m, 0x000CFA96u + (uint32_t)((int32_t)(int16_t)d0w * 2));
    a1 += (uint32_t)(int32_t)(int16_t)off;
    /* 0x0CFA86 */
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0CFA86: frame record at A1 -> sprite + hit box */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0CBD86: clear position/animation state */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x0CBE66: hit/collision record at A1 */
static void sub_0CBE66(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr8(m, obj + 0x34u, rd8(m, a1)); a1++;
    wr8(m, obj + 0x65u, rd8(m, a1));
    wr16(m, obj + 0x22u, rd16(m, obj + 0x0Eu));
    wr8(m, obj + 0x4Eu, rd8(m, obj + 0x0Cu));
}

/* ROM 0x0D0C7A: difficulty table 0x0D0CC2[(A5+0x87F) >> 3] */
static uint16_t sub_0D0C7A(const f3mem_t *m)
{
    uint8_t i = (uint8_t)(rd8(m, F3_A5 + 0x87Fu) >> 3);
    return rd16(m, 0x000D0CC2u + (uint32_t)i * 2u);
}

/* ROM 0x0D0E3A: 4-way interleaved global counter in A5+0x1D60 */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    d0 = (uint16_t)(d0 + (uint16_t)(d0 << 2) + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

/* ROM 0x0CF046 (chain = 1: each child is the parent of the next, group 5)
 * and 0x0CF138 (chain = 0, group 3): spawn D1 objects of fn A1, listed at
 * A5+0x2862; on failure free the spawned ones. Returns D0. */
static uint32_t spawn_list(const f3mem_t *m, uint32_t obj, uint32_t fn, uint16_t d1w,
                           uint16_t group, int chain)
{
    uint32_t parent = obj, last = obj;
    uint32_t a2 = F3_A5 + 0x2862u;
    uint32_t d0;
    d1w = (uint16_t)(d1w - 1u);
    for (;;) {
        d0 = Obj_Spawn(m, parent, fn, group, 2u, 0u);
        wr32(m, a2, d0); a2 += 4u;
        if ((int32_t)d0 < 0) {
            a2 = F3_A5 + 0x2862u;
            for (;;) {
                d0 = rd32(m, a2); a2 += 4u;
                if ((int32_t)d0 < 0)
                    return d0;
                Obj_Free(m, d0);
            }
        }
        last = d0;
        if (chain)
            parent = d0;
        wr8(m, last + 0x4Eu, 0u);
        if (d1w-- == 0u)
            break;
    }
    wr8(m, last + 0x4Eu, rd8(m, obj + 0x4Eu));
    return d0 & 0xFFFF0000u;   /* clr.w D0w */
}
static uint32_t sub_0CF046(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d1w, uint32_t d0)
{ (void)d0; return spawn_list(m, obj, a1, d1w, 5u, 1); }
static uint32_t sub_0CF138(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d1w, uint32_t d0)
{ (void)d0; return spawn_list(m, obj, a1, d1w, 3u, 0); }

/* ---- case bodies (generated from the listing; flow follows the ROM) ---- */

/* ROM 0x0D5EB2: type 0x6E */
static void case_0D5EB2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr8(m, obj + 0x64u, 0x00u);
    wr16(m, obj + 0x20u, 0x00004000u);
    sub_0CBD86(m, obj);
    a1 = 0x000D6108u;
    sub_0CBE66(m, obj, a1);
    wr8(m, obj + 0x4Fu, 0x00u);
    wr16(m, obj + 0x42u, 0x00000004u);
    wr8(m, obj + 0x22u, 0x00u);
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x00u, 0x000D5EE8u);
    return;
}

/* ROM 0x0D616A: type 0x6F */
static void case_0D616A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr8(m, obj + 0x64u, 0x00u);
    sub_0CBD86(m, obj);
    a1 = 0x000D641Au;
    sub_0CBE66(m, obj, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr8(m, obj + 0x4Fu, 0x00u);
    wr32(m, obj + 0x00u, 0x000D6192u);
    return;
}

/* ROM 0x0D8ABA: type 0xAC */
static void case_0D8ABA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    sub_0CBD86(m, obj);
    wr8(m, obj + 0x4Fu, 0x00u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u));
    a1 = 0x000D8BBAu;
    sub_0CBE66(m, obj, a1);
    wr16(m, obj + 0x3Au, 0x00000036u);
    wr32(m, obj + 0x10u, 0x0005ED2Cu);
    wr32(m, obj + 0x00u, 0x000D8AF2u);
    return;
}

/* ROM 0x0D8BFE: type 0xC1 */
static void case_0D8BFE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000002u);
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000001u);
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000003u);
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    wr32(m, obj + 0x00u, 0x000D8C7Eu);
    return;
}

/* ROM 0x0D8C8C: type 0xC2 */
static void case_0D8C8C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000004u);
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000005u);
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000006u);
    wr32(m, obj + 0x00u, 0x000D8CEEu);
    return;
}

/* ROM 0x0D8CFC: type 0xC3 */
static void case_0D8CFC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000D8D30u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000007u);
    wr32(m, obj + 0x00u, 0x000D8D22u);
    return;
}

/* ROM 0x0DC886: type 0x60 */
static void case_0DC886(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000DCAEEu;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    d0 = (d0 & 0xFFFF0000u) | 0x0071u;
    wr16(m, obj + 0x42u, (uint16_t)d0);
    wr32(m, obj + 0x08u, 0x00710000u);
    a1 = 0x000DCB26u;
    sub_0CFA7C(m, obj, a1, (uint16_t)d0);
    wr32(m, obj + 0x00u, 0x000DC8BEu);
    return;
}

/* ROM 0x0DCDBA: type 0x78 */
static void case_0DCDBA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000DD020u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    d0 = (d0 & 0xFFFF0000u) | 0x00F7u;
    wr16(m, obj + 0x42u, (uint16_t)d0);
    wr32(m, obj + 0x08u, 0x00F70000u);
    a1 = 0x000DD058u;
    sub_0CFA7C(m, obj, a1, (uint16_t)d0);
    wr32(m, obj + 0x00u, 0x000DCDF2u);
    return;
}

/* ROM 0x0DD2EC: type 0x61 */
static void case_0DD2EC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000DD5D2u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    a1 = 0x000DD642u;
    sub_0CFA86(m, obj, a1);
    wr32(m, obj + 0x00u, 0x000DD314u);
    return;
}

/* ROM 0x0DD6D4: type 0x79 */
static void case_0DD6D4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000DD9B6u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    a1 = 0x000DDA26u;
    sub_0CFA86(m, obj, a1);
    wr32(m, obj + 0x00u, 0x000DD6FCu);
    return;
}

/* ROM 0x0DDAB8: type 0x62 */
static void case_0DDAB8(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000DDC86u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    a1 = 0x000DDCBEu;
    sub_0CFA86(m, obj, a1);
    wr32(m, obj + 0x00u, 0x000DDAE0u);
    return;
}

/* ROM 0x0DE1EC: type 0x7B */
static void case_0DE1EC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000DE3B6u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    a1 = 0x000DE3EEu;
    sub_0CFA86(m, obj, a1);
    wr32(m, obj + 0x00u, 0x000DE214u);
    return;
}

/* ROM 0x0DF8BC: type 0x80 */
static void case_0DF8BC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0050u));
    d0 = Obj_Spawn(m, obj, 0x000DF972u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000DF972u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000DF972u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0E036E: type 0x82 */
static void case_0E036E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0050u));
    d0 = Obj_Spawn(m, obj, 0x000E0424u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E0424u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E0424u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0E0466: type 0xA1 */
static void case_0E0466(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000E0266u;
    d1 = 0x00000002u;
    d0 = sub_0CF046(m, obj, a1, (uint16_t)d1, d0);
    if ((int16_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = rd32(m, F3_A5 + 0x00002862u);
    wr32(m, a1, 0x000DEC9Eu);
    d0 = Obj_Spawn(m, obj, 0x000E029Eu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    a1 = 0x000E022Eu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000E06CCu;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000DEC56u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000E04CCu);
    return;
}

/* ROM 0x0E11F4: type 0x65 */
static void case_0E11F4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0050u));
    d0 = Obj_Spawn(m, obj, 0x000E12AAu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E12AAu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E12AAu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0E1B5E: type 0x66 */
static void case_0E1B5E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000E2318u;
    d1 = 0x00000002u;
    d0 = sub_0CF046(m, obj, a1, (uint16_t)d1, d0);
    if ((int16_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = rd32(m, F3_A5 + 0x00002862u);
    wr32(m, a1, 0x000E226Cu);
    a1 = 0x000E1DF6u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000E1E2Eu;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000E1EA6u;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000E1BAEu);
    return;
}

/* ROM 0x0E25D8: type 0x67 */
static void case_0E25D8(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000E27C0u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000E2774u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    a1 = 0x000E27ACu;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000E262Au);
    return;
}

/* ROM 0x0E3FA6: type 0x83 */
static void case_0E3FA6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0050u));
    d0 = Obj_Spawn(m, obj, 0x000E405Cu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E405Cu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E405Cu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0E4802: type 0x85 */
static void case_0E4802(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0050u));
    d0 = Obj_Spawn(m, obj, 0x000E48B8u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E48B8u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0028u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000E48B8u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0E4D42: type 0x69 */
static void case_0E4D42(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000E4EBCu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000E4E56u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    d0 = (d0 & 0xFFFF0000u) | sub_0D0E3A(m);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000E4D8Eu);
    return;
}

/* ROM 0x0E5326: type 0x7C */
static void case_0E5326(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000E54A0u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000E543Au;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    d0 = (d0 & 0xFFFF0000u) | sub_0D0E3A(m);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000E5372u);
    return;
}

/* ROM 0x0E8476: type 0x7A */
static void case_0E8476(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000EABAAu;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, 0x00000005u);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000E8494u);
    return;
}

/* ROM 0x0E84EE: type 0x6A */
static void case_0E84EE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000E88C4u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    d0 = (d0 & 0xFFFF0000u) | sub_0D0E3A(m);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr32(m, obj + 0x00u, 0x000E8516u);
    return;
}

/* ROM 0x0E8DAE: type 0x6B */
static void case_0E8DAE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000E949Eu;
    d1 = 0x00000002u;
    d0 = sub_0CF046(m, obj, a1, (uint16_t)d1, d0);
    if ((int16_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = rd32(m, F3_A5 + 0x00002862u);
    wr32(m, a1, 0x000E940Au);
    a1 = 0x000E9250u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000E9288u;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000E930Cu;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000E8DFEu);
    return;
}

/* ROM 0x0E9D9A: type 0x6D */
static void case_0E9D9A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000EA28Cu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000EA03Cu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000EA074u;
    sub_0CF1D8(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000E9DE0u);
    return;
}

/* ROM 0x0EA39E: type 0x7F */
static void case_0EA39E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0050u));
    d0 = Obj_Spawn(m, obj, 0x000EA418u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0050u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0005u));
    d0 = Obj_Spawn(m, obj, 0x000EA418u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0EBA6E: type 0x03 */
static void case_0EBA6E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0008u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000EBA96u);
    return;
}

/* ROM 0x0EC636: type 0x2B */
static void case_0EC636(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0008u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000EC65Eu);
    return;
}

/* ROM 0x0EC9DA: type 0x05 */
static void case_0EC9DA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0008u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000ECA04u);
    return;
}

/* ROM 0x0ED1EA: type 0x29 */
static void case_0ED1EA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0006u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000ED214u);
    return;
}

/* ROM 0x0EF33E: type 0x2A */
static void case_0EF33E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0005u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000EF366u);
    return;
}

/* ROM 0x0F075E: type 0x18 */
static void case_0F075E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0007u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F0788u);
    return;
}

/* ROM 0x0F18AA: type 0x23 */
static void case_0F18AA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000F18E8u, (uint16_t)0x00000003u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0F1D24: type 0x2D */
static void case_0F1D24(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000F3AF0u;
    sub_0CEC4C(m, obj, a1);
    sub_0CEE2C(m, obj);
    a1 = 0x000F3B28u;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000F3B40u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000F1D5Cu);
    return;
}

/* ROM 0x0F1E5C: type 0x1B */
static void case_0F1E5C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0004u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F1E84u);
    return;
}

/* ROM 0x0F28C0: type 0x30 */
static void case_0F28C0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0003u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x000EABE2u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F28EAu);
    return;
}

/* ROM 0x0F300A: type 0x0A */
static void case_0F300A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000F8214u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, 0x00000004u);
    wr8(m, obj + 0x22u, 0x00000004u);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F302Eu);
    return;
}

/* ROM 0x0F34CC: type 0x1D */
static void case_0F34CC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0005u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x000F8214u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F34F6u);
    return;
}

/* ROM 0x0F4A2E: type 0x22 */
static void case_0F4A2E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000F4A6Cu, (uint16_t)0x00000003u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0F4D28: type 0x20 */
static void case_0F4D28(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0004u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x000F8214u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F4D52u);
    return;
}

/* ROM 0x0F54BA: type 0x24 */
static void case_0F54BA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000F54F8u, (uint16_t)0x00000003u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    Obj_StartDying(m, obj);
    return;
}

/* ROM 0x0F7BA6: type 0x2C */
static void case_0F7BA6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000F8214u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0004u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x20u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F7BD2u);
    return;
}

/* ROM 0x0F8284: type 0x8F */
static void case_0F8284(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000F8692u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000F854Eu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000F8586u;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000F8676u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr16(m, obj + 0x0Cu, 0x00000384u);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F82E0u);
    return;
}

/* ROM 0x0F87DA: type 0x90 */
static void case_0F87DA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000F8C1Eu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000F8ADAu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000F8B12u;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000F8C02u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr16(m, obj + 0x0Cu, 0x00000384u);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F8836u);
    return;
}

/* ROM 0x0F8D32: type 0xA3 */
static void case_0F8D32(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000F8EBAu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000F8EF2u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr32(m, obj + 0x00u, 0x000F8D54u);
    return;
}

/* ROM 0x0F920A: type 0xA4 */
static void case_0F920A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000F9390u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000F93C8u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr32(m, obj + 0x00u, 0x000F922Cu);
    return;
}

/* ROM 0x0F9596: type 0xA6 */
static void case_0F9596(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000F8692u, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000F986Cu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000F98A4u;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000F9A54u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x4Fu, 0x0000007Cu);
    wr8(m, obj + 0x1Du, 0x0000007Cu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr16(m, obj + 0x0Cu, 0x00000384u);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F95FEu);
    return;
}

/* ROM 0x0F9A6C: type 0xA7 */
static void case_0F9A6C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x000F8C1Eu, (uint16_t)0x00000005u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x000F9D9Eu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000F9DD6u;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x000F9F86u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x4Fu, 0x0000007Cu);
    wr8(m, obj + 0x1Du, 0x0000007Cu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr16(m, obj + 0x0Cu, 0x00000384u);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000F9AD4u);
    return;
}

/* ROM 0x0F9F9E: type 0x9D */
static void case_0F9F9E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000FA114u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000FA14Cu;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr32(m, obj + 0x00u, 0x000F9FCCu);
    return;
}

/* ROM 0x0FA564: type 0x9E */
static void case_0FA564(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000FA114u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x20u, 0x00000001u);
    a1 = 0x000FA14Cu;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr32(m, obj + 0x00u, 0x000F9FCCu);
    return;
}

/* ROM 0x0FE088: type 0x91 */
static void case_0FE088(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000FE350u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000FE388u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr32(m, obj + 0x00u, 0x000FE0AAu);
    return;
}

/* ROM 0x0FE702: type 0x92 */
static void case_0FE702(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000FE9CAu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000FEA02u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr32(m, obj + 0x00u, 0x000FE724u);
    return;
}

/* ROM 0x0FEF54: type 0x96 */
static void case_0FEF54(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000FF0F8u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000FF130u;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr32(m, obj + 0x00u, 0x000FEF76u);
    return;
}

/* ROM 0x0FF3CA: type 0x94 */
static void case_0FF3CA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000F824Cu;
    sub_0CEC4C(m, obj, a1);
    wr32(m, obj + 0x54u, 0xFFFFFFFFu);
    wr32(m, obj + 0x00u, 0x000FF3E4u);
    return;
}

/* ROM 0x0FF644: type 0x97 */
static void case_0FF644(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000FF828u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000FF898u;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000FF66Cu);
    return;
}

/* ROM 0x0FF968: type 0x98 */
static void case_0FF968(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x000FFB4Au;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x000FFBBAu;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000FF990u);
    return;
}

/* ROM 0x0FFC8A: type 0xA2 */
static void case_0FFC8A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x001002B0u;
    d1 = 0x00000002u;
    d0 = sub_0CF138(m, obj, a1, (uint16_t)d1, d0);
    if ((int16_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = rd32(m, F3_A5 + 0x00002862u);
    wr32(m, a1, 0x001000DEu);
    wr32(m, obj + 0x18u, 0x00E80000u);
    a1 = 0x000FFFE2u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x0010001Au;
    sub_0CF1D8(m, obj, a1);
    a1 = 0x001000D4u;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x1Du, 0x000000CAu);
    wr8(m, obj + 0x4Fu, 0x000000CAu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x000FFCF4u);
    return;
}

/* ROM 0x100BB6: type 0x9B */
static void case_100BB6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0014u);
    d7 = (d7 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x001086FCu;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d7);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x00100BDEu);
    return;
}

/* ROM 0x101A16: type 0x9C */
static void case_101A16(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0014u);
    d7 = (d7 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x001086FCu;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x06u, 0x00000001u);
    wr8(m, obj + 0x21u, (uint8_t)d7);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x00100BDEu);
    return;
}

/* ROM 0x101E1C: type 0xA5 */
static void case_101E1C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x00101FE6u, (uint16_t)0x00000003u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x00101F90u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x00101FC8u;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x00101E68u);
    return;
}

/* ROM 0x108D66: type 0x34 */
static void case_108D66(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x00108734u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0005u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x00108D8Eu);
    return;
}

/* ROM 0x10947E: type 0x35 */
static void case_10947E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x00108734u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0005u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x001094A6u);
    return;
}

/* ROM 0x109586: type 0x31 */
static void case_109586(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0005u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x00108734u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x001095B0u);
    return;
}

/* ROM 0x109AC2: type 0x36 */
static void case_109AC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0008u);
    d6 = (d6 & 0xFFFF0000u) | (uint16_t)d0;
    a1 = 0x00108734u;
    sub_0CEC4C(m, obj, a1);
    wr8(m, obj + 0x21u, (uint8_t)d6);
    wr8(m, obj + 0x22u, (uint8_t)d6);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x00109AECu);
    return;
}

/* ROM 0x10AA9A: type 0x38 */
static void case_10AA9A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x00108734u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0005u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x0010AAC2u);
    return;
}

/* ROM 0x10B7FE: type 0x39 */
static void case_10B7FE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x00108734u;
    sub_0CEC4C(m, obj, a1);
    d0 = (d0 & 0xFFFF0000u) | sub_0D0C7A(m);
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0004u);
    wr8(m, obj + 0x21u, (uint8_t)d0);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr32(m, obj + 0x14u, 0x01400000u);
    wr32(m, obj + 0x18u, 0x00740000u);
    wr32(m, obj + 0x68u, 0x0010B8F4u);
    wr32(m, obj + 0x00u, 0x0010B83Eu);
    return;
}

/* ROM 0x10BE20: type 0x3A */
static void case_10BE20(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x0010C160u, (uint16_t)0x00000009u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x0010C05Cu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x0010C0CCu;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x0010BE6Cu);
    return;
}

/* ROM 0x10C4FC: type 0x3B */
static void case_10C4FC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    d0 = Obj_Spawn(m, obj, 0x0010C800u, (uint16_t)0x00000009u, (uint16_t)0x00000002u, 0x00000000u);
    if ((int32_t)d0 < 0) { Obj_StartDying(m, obj); return; }
    a1 = 0x0010C734u;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x0010C76Cu;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x06u, 0x00000001u);
    wr32(m, obj + 0x00u, 0x0010C548u);
    return;
}

/* ROM 0x10E9BA: type 0x42 */
static void case_10E9BA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x0010ECACu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x0010ECE4u;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr32(m, obj + 0x00u, 0x0010E9E2u);
    return;
}

/* ROM 0x110300: type 0x43 */
static void case_110300(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0, d0 = 0, d1 = 0, d2 = 0, d6 = 0, d7 = 0;
    (void)a1; (void)d0; (void)d1; (void)d2; (void)d6; (void)d7;
    a1 = 0x0010ECACu;
    sub_0CEC4C(m, obj, a1);
    a1 = 0x001105E0u;
    sub_0CFA86(m, obj, a1);
    wr8(m, obj + 0x23u, 0x00000001u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~ 0x02u));
    wr32(m, obj + 0x00u, 0x00110328u);
    return;
}

/* ROM 0x0CC11C: wave spawner. Reads the record at +0x0C (x, y, palette,
 * type) and jumps through the pointer table 0x6C3E[type]. */
int Obj_CC11C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint8_t type = rd8(m, a1 + 5u);
    switch (type) {
    case 0x03u: break;
    case 0x05u: break;
    case 0x0Au: break;
    case 0x18u: break;
    case 0x1Bu: break;
    case 0x1Du: break;
    case 0x20u: break;
    case 0x22u: break;
    case 0x23u: break;
    case 0x24u: break;
    case 0x29u: break;
    case 0x2Au: break;
    case 0x2Bu: break;
    case 0x2Cu: break;
    case 0x2Du: break;
    case 0x30u: break;
    case 0x31u: break;
    case 0x34u: break;
    case 0x35u: break;
    case 0x36u: break;
    case 0x38u: break;
    case 0x39u: break;
    case 0x3Au: break;
    case 0x3Bu: break;
    case 0x42u: break;
    case 0x43u: break;
    case 0x60u: break;
    case 0x61u: break;
    case 0x62u: break;
    case 0x65u: break;
    case 0x66u: break;
    case 0x67u: break;
    case 0x69u: break;
    case 0x6Au: break;
    case 0x6Bu: break;
    case 0x6Du: break;
    case 0x6Eu: break;
    case 0x6Fu: break;
    case 0x78u: break;
    case 0x79u: break;
    case 0x7Au: break;
    case 0x7Bu: break;
    case 0x7Cu: break;
    case 0x7Fu: break;
    case 0x80u: break;
    case 0x82u: break;
    case 0x83u: break;
    case 0x85u: break;
    case 0x8Fu: break;
    case 0x90u: break;
    case 0x91u: break;
    case 0x92u: break;
    case 0x94u: break;
    case 0x96u: break;
    case 0x97u: break;
    case 0x98u: break;
    case 0x9Bu: break;
    case 0x9Cu: break;
    case 0x9Du: break;
    case 0x9Eu: break;
    case 0xA1u: break;
    case 0xA2u: break;
    case 0xA3u: break;
    case 0xA4u: break;
    case 0xA5u: break;
    case 0xA6u: break;
    case 0xA7u: break;
    case 0xACu: break;
    case 0xC1u: break;
    case 0xC2u: break;
    case 0xC3u: break;
    default:
        return -1;   /* case not translated yet (no side effects applied) */
    }

    wr32(m, obj + 0x14u, 0u - rd32(m, F3_A5 - 0x5DA8u));
    wr32(m, obj + 0x18u, 0u - rd32(m, F3_A5 - 0x5DA4u));
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x14u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x18u, rd16(m, a1)); a1 += 2u;
    wr8(m, obj + 0x4Eu, rd8(m, a1)); a1 += 2u;
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);

    switch (type) {
    case 0x6Eu: case_0D5EB2(m, obj); break;
    case 0x6Fu: case_0D616A(m, obj); break;
    case 0xACu: case_0D8ABA(m, obj); break;
    case 0xC1u: case_0D8BFE(m, obj); break;
    case 0xC2u: case_0D8C8C(m, obj); break;
    case 0xC3u: case_0D8CFC(m, obj); break;
    case 0x60u: case_0DC886(m, obj); break;
    case 0x78u: case_0DCDBA(m, obj); break;
    case 0x61u: case_0DD2EC(m, obj); break;
    case 0x79u: case_0DD6D4(m, obj); break;
    case 0x62u: case_0DDAB8(m, obj); break;
    case 0x7Bu: case_0DE1EC(m, obj); break;
    case 0x80u: case_0DF8BC(m, obj); break;
    case 0x82u: case_0E036E(m, obj); break;
    case 0xA1u: case_0E0466(m, obj); break;
    case 0x65u: case_0E11F4(m, obj); break;
    case 0x66u: case_0E1B5E(m, obj); break;
    case 0x67u: case_0E25D8(m, obj); break;
    case 0x83u: case_0E3FA6(m, obj); break;
    case 0x85u: case_0E4802(m, obj); break;
    case 0x69u: case_0E4D42(m, obj); break;
    case 0x7Cu: case_0E5326(m, obj); break;
    case 0x7Au: case_0E8476(m, obj); break;
    case 0x6Au: case_0E84EE(m, obj); break;
    case 0x6Bu: case_0E8DAE(m, obj); break;
    case 0x6Du: case_0E9D9A(m, obj); break;
    case 0x7Fu: case_0EA39E(m, obj); break;
    case 0x03u: case_0EBA6E(m, obj); break;
    case 0x2Bu: case_0EC636(m, obj); break;
    case 0x05u: case_0EC9DA(m, obj); break;
    case 0x29u: case_0ED1EA(m, obj); break;
    case 0x2Au: case_0EF33E(m, obj); break;
    case 0x18u: case_0F075E(m, obj); break;
    case 0x23u: case_0F18AA(m, obj); break;
    case 0x2Du: case_0F1D24(m, obj); break;
    case 0x1Bu: case_0F1E5C(m, obj); break;
    case 0x30u: case_0F28C0(m, obj); break;
    case 0x0Au: case_0F300A(m, obj); break;
    case 0x1Du: case_0F34CC(m, obj); break;
    case 0x22u: case_0F4A2E(m, obj); break;
    case 0x20u: case_0F4D28(m, obj); break;
    case 0x24u: case_0F54BA(m, obj); break;
    case 0x2Cu: case_0F7BA6(m, obj); break;
    case 0x8Fu: case_0F8284(m, obj); break;
    case 0x90u: case_0F87DA(m, obj); break;
    case 0xA3u: case_0F8D32(m, obj); break;
    case 0xA4u: case_0F920A(m, obj); break;
    case 0xA6u: case_0F9596(m, obj); break;
    case 0xA7u: case_0F9A6C(m, obj); break;
    case 0x9Du: case_0F9F9E(m, obj); break;
    case 0x9Eu: case_0FA564(m, obj); break;
    case 0x91u: case_0FE088(m, obj); break;
    case 0x92u: case_0FE702(m, obj); break;
    case 0x96u: case_0FEF54(m, obj); break;
    case 0x94u: case_0FF3CA(m, obj); break;
    case 0x97u: case_0FF644(m, obj); break;
    case 0x98u: case_0FF968(m, obj); break;
    case 0xA2u: case_0FFC8A(m, obj); break;
    case 0x9Bu: case_100BB6(m, obj); break;
    case 0x9Cu: case_101A16(m, obj); break;
    case 0xA5u: case_101E1C(m, obj); break;
    case 0x34u: case_108D66(m, obj); break;
    case 0x35u: case_10947E(m, obj); break;
    case 0x31u: case_109586(m, obj); break;
    case 0x36u: case_109AC2(m, obj); break;
    case 0x38u: case_10AA9A(m, obj); break;
    case 0x39u: case_10B7FE(m, obj); break;
    case 0x3Au: case_10BE20(m, obj); break;
    case 0x3Bu: case_10C4FC(m, obj); break;
    case 0x42u: case_10E9BA(m, obj); break;
    case 0x43u: case_110300(m, obj); break;
    default: break;
    }
    return 0;
}
