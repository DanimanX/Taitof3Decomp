/* ROM 0x13467E: segmented boss part initializer (A0 = object, +0x0C = descriptor).
 * Resets fields via Obj_ResetFields (0x0CBD86), sets +0x28.w = 1, clears +0x4F,
 * sets bits 1 and 3 of +0x34, loads callback A2 and fields +0x5A/+0x5E/+0x62 from
 * descriptor (+0x0C), invokes A2, and transitions to 0x1346B6. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

int Fn_13467E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t desc = rd32(m, obj + 0x0Cu);
    uint32_t a2 = rd32(m, desc);

    switch (a2) {
    case 0x001352FEu:
    case 0x00135340u:
    case 0x0013536Au:
    case 0x00135446u:
    case 0x00135452u:
    case 0x001354CAu:
    case 0x00135554u:
    case 0x001355DEu:
    case 0x00135604u:
    case 0x001356C2u:
    case 0x001356CAu:
    case 0x00135762u:
    case 0x00135788u:
    case 0x00135846u:
    case 0x0013586Cu:
    case 0x0013594Au:
    case 0x00135970u:
    case 0x00135A98u:
    case 0x00135B3Cu:
    case 0x00135C7Au:
    case 0x00135CE8u:
    case 0x00135E1Cu:
    case 0x00136006u:
        break;
    default:
        return -1;
    }

    Obj_ResetFields(m, obj);                                 /* 0x13467E -> 0x0CBD86 */
    wr16(m, obj + 0x28u, 1u);                                /* 0x134684 */
    wr8(m, obj + 0x4Fu, 0u);                                 /* 0x13468A */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* 0x13468E */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u)); /* 0x134694 */

    uint32_t a1 = desc + 4u;                                 /* 0x13469A..0x13469E */
    wr32(m, obj + 0x5Au, rd32(m, a1)); a1 += 4u;             /* 0x1346A0 */
    wr32(m, obj + 0x5Eu, rd32(m, a1)); a1 += 4u;             /* 0x1346A4 */
    wr8(m, obj + 0x62u, rd8(m, a1)); a1 += 1u;               /* 0x1346A8 */
    r->a[1] = a1;
    r->a[2] = a2;

    switch (a2) {                                            /* 0x1346AC */
    case 0x001352FEu:
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x08u));
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u));
        break;
    case 0x00135340u:
        wr8(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x20u, 0x8000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x001361AEu);
        break;
    case 0x0013536Au:
        wr8(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x20u, 0x8000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x001361BCu);
        break;
    case 0x00135446u:
    case 0x00135452u:
        wr8(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x20u, 0x4000u);
        break;
    case 0x001354CAu:
        wr16(m, obj + 0x3Eu, 0x000Au);
        break;
    case 0x00135554u:
        wr16(m, obj + 0x3Eu, 0x0014u);
        break;
    case 0x001355DEu:
        wr16(m, obj + 0x20u, 0xA000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x0013641Eu);
        break;
    case 0x00135604u:
        wr16(m, obj + 0x20u, 0xA000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x0013642Cu);
        break;
    case 0x001356C2u:
    case 0x001356CAu:
        wr16(m, obj + 0x20u, 0x4000u);
        break;
    case 0x00135762u:
        wr16(m, obj + 0x20u, 0xA000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x00136676u);
        break;
    case 0x00135788u:
        wr16(m, obj + 0x20u, 0xA000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x00136684u);
        break;
    case 0x00135846u:
        wr16(m, obj + 0x20u, 0x6000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x001367A2u);
        break;
    case 0x0013586Cu:
        wr16(m, obj + 0x20u, 0x6000u);
        r->d[0] = Obj_Spawn(m, obj, 0x0013467Eu, 3u, 2u, 0x001367B0u);
        break;
    case 0x0013594Au:
    case 0x00135970u:
        wr16(m, obj + 0x20u, 0x4000u);
        r->d[0] = Obj_Spawn(m, obj, 0x00134844u, 3u, 2u, 0x00136C82u);
        break;
    case 0x00135A98u:
    case 0x00135C7Au:
        wr16(m, obj + 0x3Eu, 0u);
        break;
    case 0x00135B3Cu:
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
        wr16(m, obj + 0x24u, 0x0064u);
        wr8(m, obj + 0x06u, 0u);
        break;
    case 0x00135E1Cu:
        wr16(m, obj + 0x20u, 0xC000u);
        break;
    case 0x00135CE8u:
    case 0x00136006u:
        break;
    }

    wr32(m, obj, 0x001346B6u);                               /* 0x1346AE */
    return 0;
}
