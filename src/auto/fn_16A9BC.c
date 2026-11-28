/* ROM 0x16A9BC: register-level wrapper of Stage_UploadBgTilemaps
 * (src/video/sprite.c) for its capture.
 * The upload itself is the existing C. The register bookkeeping below mirrors
 * the ROM skeleton (0x16A9BC..0x16AB24): per active layer A1/A2 are reloaded
 * from A5+0x4264.., each of the 16 tiles calls 0x16ABA2 which leaves
 * A0 = tile source, D0 = (D6&3)<<3, D1 = D7<<3 (word halves only, the movem
 * around the blit restores everything else), and both DBRA loops expire to
 * 0xFFFF. The capture (work/ram/fn_16A9BC, 1 pair) is a later-frame dump the
 * harness rejects, so this wrapper is not registered as a test. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_16A9BC(const f3mem_t *m, f3regs_t *r)
{
    static const struct { uint16_t flag_off, p1, p2; } layers[4] = {
        { 0x4284u, 0x4264u, 0x4268u }, { 0x4285u, 0x426Cu, 0x4270u },
        { 0x4286u, 0x4274u, 0x4278u }, { 0x4287u, 0x427Cu, 0x4280u },
    };
    Stage_UploadBgTilemaps(m);
    for (int l = 0; l < 4; l++) {
        if ((rd8(m, F3_A5 + layers[l].flag_off) & 1u) == 0u)   /* btst #0 ; beq */
            continue;
        uint32_t a1 = rd32(m, F3_A5 + layers[l].p1);           /* 0x16A9CE */
        uint32_t a2 = rd32(m, F3_A5 + layers[l].p2);           /* 0x16A9D2 */
        r->a[1] = a1;
        r->a[2] = a2;
        for (int16_t d6 = 3; d6 >= 0; d6--) {                  /* 0x16A9D6 */
            for (int16_t d7 = 3; d7 >= 0; d7--) {              /* 0x16A9DA */
                int16_t idx = (int16_t)((d6 << 2) + d7);
                int16_t t = (int16_t)rd16(m, a1 + (int32_t)idx * 2u);  /* 0x16ABAA */
                r->a[0] = rd32(m, a2 + (int32_t)(t << 2));     /* 0x16ABB0 */
                r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)((d6 & 3) << 3);
                r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)(d7 << 3);
                r->d[6] = (r->d[6] & 0xFFFF0000u) | (uint16_t)d6;
                r->d[7] = (r->d[7] & 0xFFFF0000u) | (uint16_t)d7;
            }
            r->d[7] = (r->d[7] & 0xFFFF0000u) | 0xFFFFu;       /* dbra d7 */
        }
        r->d[6] = (r->d[6] & 0xFFFF0000u) | 0xFFFFu;           /* dbra d6 */
    }
    return 0;                                                  /* 0x16AB24 rts */
}
