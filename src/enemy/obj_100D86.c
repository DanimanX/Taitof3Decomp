#include "obj_100D86.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x100D86: debug-freeze gate. While the freeze bit (A5+0x1D58 bit 1) is
 * set and +0x14 is on screen the object just keeps its flash byte from +0x4F.
 * Otherwise every tick integrates the pose (+0x50/+0x54 into +0x3A/+0x3E, the
 * second with the global A5+0x4228 subtracted), runs the off-screen flag,
 * copies the pose to +0x14/+0x18 and the script/collision/colour/draw tail.
 * An object with +0x24 <= 0 (no health left) spawns the 0xE6082 fx child in
 * groups 2..0xB instead, and the freeze gate does the same. */

int Obj_100D86_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x100D86: btst.b #1,(0x1D58,A5) ; beq 0x100DAA ;
     * cmpi.w #-0xA,(0x14,A0) ; blt 0x100DAA ; cmpi.w #0x11C ; bgt 0x100DAA */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            /* 0x100D9E: move.b (0x4F,A0),(0x1D,A0) ; jmp 0x100E64 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x100E64: moveq #0x64,D0 ; jsr 0xD0E30 ; bra 0xCEE9A */
            Obj_RelayTimer(m, 0x00000064u);
            Obj_DieWithBonusDbgChk(m, obj, 0x00000064u);
            return 0;
        }
    }

    /* 0x100DAA: tst.l (0x24,A0) ; ble.w 0x100E08 */
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        /* 0x100DB2: btst.b #5,(-0x144B,A5) ; bne.w 0x100E10 */
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
            /* 0x100DBC: move.l (0x50,A0),D0 ; add.l D0,(0x3A,A0) */
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
            /* 0x100DC4: move.l (0x54,A0),D0 ; sub.l (0x4228,A5),D0 ;
             * add.l D0,(0x3E,A0) */
            wr32(m, obj + 0x3Eu,
                 rd32(m, obj + 0x3Eu) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));

            /* 0x100DD0: jsr 0xCF85A (flag off screen) */
            Obj_FlagOffscreen(m, obj);

            /* 0x100DD6: btst.b #3,(0x7,A0) ; bne.l 0xCEE74 (start dying) */
            if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }

            /* 0x100DE2 / 0x100DE8: copy the integrated pose to +0x14/+0x18 */
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

            /* 0x100DEE..0x100E00 */
            Obj_StepScript(m, obj);          /* jsr 0xD0690 */
            Obj_0D0DC2(m, obj);              /* jsr 0xD0DC2 */
            Obj_BlinkColour(m, obj);         /* jsr 0xCBE34 */
            Obj_DrawSprite(m, obj);          /* jsr 0x1282 */
            return 0;
        }
        /* 0x100DB8: bne.w 0x100E10 -- freeze gate, fall through */
    } else {
        /* 0x100E08: moveq #0x64,D0 ; jsr 0xD0E04 ; falls into 0x100E10 */
        Obj_0D0E04(m, obj, 0x00000064u);
    }

    /* 0x100E10: pea #0 ; move.w #2,-(SP) ; move.w #9,-(SP) ;
     * move.l #0xE6082,-(SP) ; jsr 0x1057C ; lea (0xC,SP),SP */
    uint32_t child = Obj_Spawn(m, obj, 0x000E6082u, 0x0009u, 0x0002u, 0u);
    /* 0x100E2C: tst.l D0 ; bmi.l 0xCEE48 */
    if ((int32_t)child < 0) {
        Obj_0CEE48(m, obj);
        return 0;
    }

    /* 0x100E34: movea.l D0,A1 -- copy the pose, adding the .w offsets at
     * +0xC / +0xE, and the +0x58/+0x60 fields */
    wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + rd16(m, obj + 0x0Cu)));
    wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + rd16(m, obj + 0x0Eu)));
    wr16(m, child + 0x58u, rd16(m, obj + 0x58u));
    wr16(m, child + 0x60u, rd16(m, obj + 0x60u));

    /* 0x100E5E: bra.l 0xCEE48 */
    Obj_0CEE48(m, obj);
    return 0;
}
