#include "obj_14D75A.h"

/* Local callees of 0x14D75A -- only these two, both included in the bundle. */

/* ROM 0x17668A: for D7 = D7w..D6w (signed, D7 <= D6) replace bits 31..28 of
 * word (A0 + D7*2) with the low nibble of D0w. Note (0x0,A0,D7w*2) is A0-32.
 * bfins D0,D1{0x1c:0x4} on the LONG register D1 fills MSB-first from bit 28,
 * i.e. the high nibble of the word at (A0 + D7w*2); D1l is untouched and
 * replaced by the following move.w, so only the high nibble is affected. */
static void sub_17668A(const f3mem_t *m, uint32_t a0, uint16_t d0w,
                       uint16_t d6w, uint16_t d7w)
{
    while ((int16_t)d7w <= (int16_t)d6w) {
        uint32_t ea = a0 + (uint32_t)(int32_t)(int16_t)d7w * 2u;
        uint16_t d1w = rd16(m, ea);
        d1w = (uint16_t)((d1w & 0x0FFFu) | (uint16_t)((d0w & 0x000Fu) << 12));
        wr16(m, ea, d1w);
        d7w = (uint16_t)(d7w + 1u);
    }
}

/* ROM 0x17669E: for D7 = D7w..D6w store D0w at (A0 + D7*2) and the unchanged
 * D1w at (A1 + D7*2). */
static void sub_17669E(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint16_t d0w, uint16_t d1w, uint16_t d6w, uint16_t d7w)
{
    while ((int16_t)d7w <= (int16_t)d6w) {
        uint32_t off = (uint32_t)(int32_t)(int16_t)d7w * 2u;
        wr16(m, a0 + off, d0w);
        wr16(m, a1 + off, d1w);
        d7w = (uint16_t)(d7w + 1u);
    }
}

/* ROM 0x14D75A: line-buffer wipe. The A5-relative bias (A5-0x5D56) is added
 * to every base. Fills 0xE8..0 line RAM words 0..0xE7, then 0xE7..0 line RAM
 * words 0..0xE7 and character RAM words 0..0xE7 with 0xDDD9 / 0x0000 (both
 * pushed constants are popped back with +D6/+D7, D0w = -0x2227 is the nibble
 * source, D1w = 0x02FF), and re-arms itself at 0x14D81A. */
int Obj_14D75A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t bias = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    /* 0x14D75A: D6w = 0, D7w = 0xE7, D0w = 0xA -> 0x62B200 + bias (even words) */
    sub_17668A(m, 0x0062B200u + bias, 0x000Au, 0x00E7u, 0x0000u);

    /* 0x14D786: D6w = 0, D7w = 0xE7, D0w = 0x8 -> 0x62B400 + bias */
    sub_17668A(m, 0x0062B400u + bias, 0x0008u, 0x00E7u, 0x0000u);

    /* 0x14D7AE: D6w = 0, D7w = 0xE7, D0w = 0x4 -> 0x62B600 + bias */
    sub_17668A(m, 0x0062B600u + bias, 0x0004u, 0x00E7u, 0x0000u);

    /* 0x14D7D6: D6w = 0x00E7 and D7w = 0; D0w = 0xDDD9, D1w = 0x02FF ->
     * 0x627600 + bias := 0xDDD9 and 0x626000 + bias := 0x02FF, words 0..0xE7 */
    sub_17669E(m, 0x00627600u + bias, 0x00626000u + bias,
               0xDDD9u, 0x02FFu, 0x00E7u, 0x0000u);

    /* 0x14D812: re-arm at 0x14D81A */
    wr32(m, obj + 0x00u, 0x0014D81Au);
    return 0;
}
