#include "obj_15A1CA.h"

static void sub_176832(const f3mem_t *m, uint32_t a0_base, uint32_t a1_base,
                       int16_t d7w, int16_t d6w, uint32_t a5_4298,
                       int32_t d5, int32_t d3, uint16_t a5_42a0)
{
    int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a0 = (uint32_t)((int32_t)a0_base + off);
    uint32_t a1 = (uint32_t)((int32_t)a1_base + off);
    wr32(m, F3_A5 + 0x4298u, a5_4298);
    wr16(m, F3_A5 + 0x42A0u, a5_42a0);
    int16_t step = (d7w <= d6w) ? 1 : -1;
    for (;;) {
        int16_t z = (int16_t)rd16(m, F3_A5 + 0x4298u);
        int16_t q = (int16_t)((int32_t)-0xA000 / (int16_t)(0x00A0 + z));
        int16_t d0w = (int16_t)(q + 0x0100);
        int16_t d2w = (int16_t)(0x0024 - (int16_t)rd16(m, F3_A5 + 0x42A0u));
        int32_t prod = ((int32_t)d0w * (int32_t)d2w) << 6;
        int16_t row = (int16_t)(prod / 0x0032);
        row ^= 0x003F;
        wr16(m, (uint32_t)((int32_t)a0 + (int32_t)d7w * 2), (uint16_t)row);
        wr8(m, (uint32_t)((int32_t)a1 + (int32_t)d7w * 2), (uint8_t)d0w);
        d5 += d3;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + (uint32_t)d5);
        d7w = (int16_t)(d7w + step);
        if (step >= 0) {
            if (d7w > d6w) break;
        } else {
            if (d6w > d7w) break;
        }
    }
}

static void sub_1768CA(const f3mem_t *m, uint32_t a0_base, int16_t d7w, int16_t d6w,
                       int32_t a5_4298, int32_t d3, int32_t d4)
{
    int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a0 = (uint32_t)((int32_t)a0_base + off);
    if ((int16_t)rd16(m, F3_A5 - 0x5D58u) != 0) {
        a5_4298 = -a5_4298;
        d3 = -d3;
        d4 = -d4;
    }
    wr32(m, F3_A5 + 0x4298u, (uint32_t)a5_4298);
    int16_t step = (d7w <= d6w) ? 1 : -1;
    for (;;) {
        uint16_t d2w = rd16(m, F3_A5 + 0x4298u);
        uint32_t addr = (uint32_t)((int32_t)a0 + (int32_t)d7w * 2);
        uint16_t d0w = rd16(m, addr);
        d0w = (uint16_t)((d0w & ~0x01FFu) | (d2w & 0x01FFu));
        wr16(m, addr, d0w);
        d3 += d4;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + (uint32_t)d3);
        d7w = (int16_t)(d7w + step);
        if (step >= 0) {
            if (d7w > d6w) break;
        } else {
            if (d6w > d7w) break;
        }
    }
}

/* ROM 0x15A1CA: road/ground perspective line-scroll setup */
int Obj_15A1CA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0w = (uint16_t)rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x44u, d0w);
    wr16(m, obj + 0x48u, (uint16_t)(d0w - 1u));
    int16_t diff = (int16_t)(rd16(m, obj + 0x44u) - rd16(m, obj + 0x48u));
    int32_t d0 = (int32_t)diff << 7;
    int32_t d1 = d0 << 8;
    d0 = -d0;
    sub_1768CA(m, 0x00624400u, 0, 0x00E7, d1, d0, 0);
    diff = (int16_t)(rd16(m, obj + 0x44u) - rd16(m, obj + 0x48u));
    uint32_t d0u = (uint32_t)((uint16_t)diff) * 0xA000u;
    sub_176832(m, 0x0062A400u, 0x00628400u, 0x00E7, 0, d0u, 0, 0, 2u);
    wr32(m, obj + 0x00u, 0x0015A264u);
    return 0;
}
