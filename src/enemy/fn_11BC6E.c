#include "fn_11BC6E.h"
#include "../objects/obj_pool.h"
#include "obj_115B3E.h"     /* Rand_0CC2BC */

/* ROM 0x11BC6E:
 * 0x11BC6E push D0; move.w #0xFA,D0 ; jsr 0x0CC2BC ; tst.w D0 ; bne 0x11BCAC
 * 0x11BC82 list A5+0x1D2C..(0x1D34-1): for each pointer, 0x11BC90
 *   move.l (A1)+,-(SP) ; move.w #1,-(SP) ; move.w #0,-(SP) ;
 *   pea 0x11BCB2 ; jsr 0x01057C ; lea (0xC,SP),SP
 * 0x11BCAC pop D0 ; rts.
 * The captures never pass the 0x0CC2BC gate, so the spawn loop is checked
 * against the ROM bytes only (parent = A0, arg = list entry). */
void Obj_11BC6E_SpawnWave(const f3mem_t *m, uint32_t a0)
{
    (void)Rng_NextMem;  /* 0x0CC2BC only reads A5+0x878 / A5+0x87E */
    if (Rand_0CC2BC(m, 0x00FAu) != 0u)
        return;                                         /* tst.w D0 ; bne */
    uint32_t a1 = F3_A5 + 0x1D2Cu;
    int16_t n = (int16_t)(rd16(m, F3_A5 + 0x1D34u) - 1u);
    if (n < 0)
        return;                                         /* subq.w #1 ; blt */
    for (;;) {
        uint32_t arg = rd32(m, a1);
        a1 += 4u;
        (void)Obj_Spawn(m, a0, 0x0011BCB2u, 0u, 1u, arg);  /* parent = caller A0 */
        n--;
        if (n < 0)
            break;                                      /* bne back to 0x11BC8A */
    }
}
