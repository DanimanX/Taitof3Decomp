/* ROM 0x111EA8: random escort spawner (A0 = object; every register, SP
 * included, is saved by the leading movem and restored by the trailing one).
 * A 0x001D8A draw scaled by 2 is compared with 0x8000 + A5+0x87E; when the
 * threshold is smaller the routine builds one 0x0D9056 and one 0x0CDCA0
 * group-9 object (ngroups 2) whose shared argument is assembled by bitfields:
 * byte3 = 0x30 + 0x18*(0xFF - A0+0x1F)/0xFF, byte2 = 0x70, byte1 =
 * (rand & 0x7F) - 0x40, byte0 = the low byte of 0x8000 + A5+0x87E. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"       /* Obj_Spawn */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x111F2C: D0 = (D0.w * (~A0+0x1F & 0xFF)) divu 0xFF; quotient in the low
 * word, remainder in the high word (D2's high word is kept). */
static uint32_t sub_111F2C(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint16_t d2 = (uint16_t)(~(uint16_t)rd8(m, obj + 0x1Fu));
    d2 = (uint16_t)(d2 & 0x00FFu);                      /* andi.w #0xff */
    uint32_t prod = (uint32_t)((uint16_t)d0 * d2);      /* mulu.w */
    uint16_t div = 0x00FFu;
    uint16_t quot = (uint16_t)(prod / div);             /* divu.w: quotient low */
    uint16_t rem = (uint16_t)(prod % div);              /* ... remainder high */
    return ((uint32_t)rem << 16) | quot;
}

int Fn_111EA8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t d0 = Rng_NextMem(m);                       /* 0x111EAC jsr 0x001D8A */
    d0 = (uint32_t)((uint16_t)d0 * 2u);                 /* 0x111EB2 mulu.w #2,D0 */
    uint32_t d1 = 0;                                    /* 0x111EB6 moveq #0,D1 */
    d1 = setw(d1, rd16(m, F3_A5 + 0x087Eu));            /* 0x111EB8 */
    d1 += 0x00008000u;                                  /* 0x111EBC addi.l */
    if ((int32_t)d1 <= (int32_t)d0)                     /* 0x111EC2 cmp.l/ble */
        return 0;

    d0 = sub_111F2C(m, obj, setw(d0, 0x0018u));         /* 0x111EC8..0x111ECC */
    d0 = setw(d0, (uint16_t)(d0 + 0x0030u));            /* 0x111ED0 addi.w */
    d1 = (d1 & 0x00FFFFFFu) | ((d0 & 0x000000FFu) << 24); /* 0x111ED4 bfins {0:8} */
    d0 = setw(d0, 0x0070u);                             /* 0x111ED8 */
    d1 = (d1 & 0xFF00FFFFu) | ((d0 & 0x000000FFu) << 16); /* 0x111EDC bfins {8:8} */

    d0 = Rng_NextMem(m);                                /* 0x111EE0 */
    d0 = setw(d0, (uint16_t)(d0 & 0x007Fu));            /* 0x111EE6 andi.w */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 - 0x0040u));  /* 0x111EEA subi.w */
    d1 = (d1 & 0xFFFF00FFu) | ((d0 & 0x000000FFu) << 8);  /* 0x111EEE bfins {0x10:8} */

    (void)Obj_Spawn(m, obj, 0x000D9056u, 9u, 2u, d1);   /* 0x111EF2..0x111F08 */
    (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, d1);   /* 0x111F0C..0x111F22 */
    return 0;                                           /* 0x111F26 movem restores all */
}
