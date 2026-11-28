/* ROM 0x002E8C: copy a byte-counted block from A5-0x7E2C to A1+D1, wrapping D1 at 0x400 */
#include "ea_auto.h"

int Ea_002E8C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t a2 = a5 - 0x7E2Cu;                  /* 0x002E8C lea.l -$7e2c(a5), a2 */
    uint16_t d1 = (uint16_t)r->d[1];
    uint8_t count = rd8(m, a2);                  /* 0x002E92 move.b (a2)+, d0 */
    uint32_t src = a2 + 1;
    uint32_t dst_base = r->a[1];
    uint32_t dst_idx = d1;

    wr8(m, dst_base + (dst_idx & 0x3FFu), count); /* 0x002E94 move.b d0, (a1, d1.w) */
    dst_idx = (dst_idx + 1) & 0x3FFu;             /* 0x002EA0 addq.w #$1, d1 */
    
    uint8_t i = count - 1;                        /* 0x002E98 subq.w #$1, d0 */
    while (i > 0) {                               /* 0x002EA6 dbra d0, $2e9c */
        wr8(m, dst_base + (dst_idx & 0x3FFu), rd8(m, src)); /* 0x002E9C move.b (a2)+, (a1, d1.w) */
        src++;
        dst_idx = (dst_idx + 1) & 0x3FFu;         /* 0x002EA0 addq.w #$1, d1 */
        i--;
    }

    r->d[0] = 0x0000FFFFu;                       /* 0x002EA6 dbra d0 */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | dst_idx; /* 0x002EA0/0x002EA2 */
    r->a[2] = src;                               /* 0x002E92/0x002E9C (a2)+ */
    r->a[7] += 4u;                               /* 0x002EAA rts */
    return 0;
}
