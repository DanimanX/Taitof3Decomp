#include "obj_187938.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

#define BOSS_4B26 (F3_A5 + 0x4B26u)
#define BOSS_4B3A (F3_A5 + 0x4B3Au)
#define BOSS_4B3B (F3_A5 + 0x4B3Bu)

static void bclr8(const f3mem_t *m, uint32_t a, uint8_t mask)
{ wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~mask)); }
static void bset8(const f3mem_t *m, uint32_t a, uint8_t mask)
{ wr8(m, a, (uint8_t)(rd8(m, a) | mask)); }

/* ROM 0x13F238 -> 0x0039D8 */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x17B02E: D0 = owner to credit, or -1 */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u)
        return rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (k == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x187ABE: core destroyed */
static void sub_187ABE(const f3mem_t *m, uint32_t obj)
{
    Obj_Spawn(m, obj, 0x0017AD18u, 0u, 1u, 0x00187D76u);
    Obj_Spawn(m, obj, 0x00179DEEu, 3u, 2u, 0u);
    play_sound(m, 0x0013u);
    wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - rd32(m, F3_A5 + 0x4B18u));
    bset8(m, BOSS_4B26, 0x10u);                 /* bfset {3:1} */
}

/* Movement-script commands reached through 0x178982 ({fn.l, dur.w, p0.w, p1.w}) */
static int script_cmd_known(uint32_t fn)
{
    return fn == 0x00178AC6u || fn == 0x00178B0Eu || fn == 0x00178C06u;
}

static void script_cmd(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    switch (fn) {
    case 0x00178AC6u:   /* set position (integer words), clear fractions */
        wr16(m, obj + 0x50u, rd16(m, a1 + 6u));
        wr16(m, obj + 0x54u, rd16(m, a1 + 8u));
        wr16(m, obj + 0x52u, 0u);
        wr16(m, obj + 0x56u, 0u);
        break;
    case 0x00178B0Eu:   /* set speed and heading */
        wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
        wr16(m, obj + 0x20u, rd16(m, a1 + 8u));
        break;
    case 0x00178C06u: { /* accelerate / turn, then move along the heading */
        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
        uint16_t d2 = (uint16_t)(rd16(m, obj + 0x20u) + rd16(m, a1 + 8u));
        wr16(m, obj + 0x42u, d0);
        wr16(m, obj + 0x20u, d2);
        int16_t sp = (int16_t)(uint16_t)(d0 + 0x20u);
        sp = (int16_t)(sp >> 6);                /* asr.w #6 */
        uint32_t ang = (uint32_t)(d2 >> 8);
        int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + ang * 2u);
        int16_t s = (int16_t)rd16(m, 0x00027E40u + ang * 2u);
        wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)((int32_t)sp * c));
        wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)((int32_t)sp * s));
        break;
    }
    default:
        break;
    }
}

/* ROM 0x178982: movement-script interpreter (10-byte entries).
 * dry != 0 only walks the script and reports whether every command it
 * would call is known (commands never touch +0x44/+0x48/+0x4A).
 * p44/p48/p4a = the current +0x44/+0x48/+0x4A values. */
static int sub_178982(const f3mem_t *m, uint32_t obj, int dry,
                      uint32_t p44, uint16_t p48, uint16_t p4a)
{
    uint32_t a1;
    for (;;) {
        a1 = p44;
        uint32_t fn = rd32(m, a1);
        if (fn == 0u)
            goto c8;
        if (dry) {
            if (!script_cmd_known(fn))
                return -1;
        } else {
            script_cmd(m, obj, fn);
        }
        if ((int16_t)p48 < 0)
            return 0;
        if (p48 == 0u) {
            a1 = p44 + 0x0Au;
            p44 = a1;
            p48 = rd16(m, a1 + 4u);
            if (!dry) {
                wr32(m, obj + 0x44u, p44);
                wr16(m, obj + 0x48u, p48);
            }
            continue;
        }
        p48 = (uint16_t)(p48 - 1u);
        if (!dry)
            wr16(m, obj + 0x48u, p48);
        if ((int16_t)p48 > 0)
            return 0;
        a1 = p44 + 0x0Au;
        if ((int32_t)rd32(m, a1) > 0)
            goto ee;
        goto c8;
    }
c8:
    if ((int16_t)p4a >= 0) {
        if (p4a == 0u)
            p4a = rd16(m, a1 + 4u);
        p4a = (uint16_t)(p4a - 1u);
        if (!dry)
            wr16(m, obj + 0x4Au, p4a);
        if (p4a == 0u) {
            a1 += 0x0Au;
            goto ee;
        }
    }
    a1 = rd32(m, a1 + 6u);                     /* 0x1789EA */
ee:
    if (!dry) {
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }
    return 0;
}

/* ROM 0x187DCE: phase 1 handler (index 1 of the table at +0x64).
 * phase1_check walks the script without side effects. */
static int phase1_check(const f3mem_t *m, uint32_t obj)
{
    if (!(rd8(m, BOSS_4B26) & 0x08u))   /* the script restarts at 0x187E26 */
        return sub_178982(m, obj, 1, 0x00187E26u, rd16(m, 0x00187E2Au), 0u);
    return sub_178982(m, obj, 1, rd32(m, obj + 0x44u), rd16(m, obj + 0x48u),
                      rd16(m, obj + 0x4Au));
}

static void phase1(const f3mem_t *m, uint32_t obj)
{
    if (!(rd8(m, BOSS_4B26) & 0x08u)) {
        bset8(m, BOSS_4B26, 0x08u);             /* bfset {4:1} */
        wr32(m, obj + 0x44u, 0x00187E26u);
        wr16(m, obj + 0x48u, rd16(m, 0x00187E2Au));
        wr16(m, obj + 0x4Au, 0u);
        bset8(m, BOSS_4B3B, 0x01u);             /* bfset 4B3A{15:1} */
        bset8(m, BOSS_4B3B, 0x40u);             /* bfset 4B3A{9:1} */
    }
    sub_178982(m, obj, 0, rd32(m, obj + 0x44u), rd16(m, obj + 0x48u),
               rd16(m, obj + 0x4Au));
    if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
        wr16(m, F3_A5 + 0x4B38u, 0x0002u);
        bclr8(m, BOSS_4B26, 0x08u);
        bclr8(m, BOSS_4B3B, 0x01u);
    }
}

/* ROM 0x187938: boss core */
int Obj_187938_Tick(const f3mem_t *m, uint32_t obj)
{
    /* pre-pass: which phase handler will run (only phase 1 is decompiled) */
    {
        uint32_t hp = rd32(m, obj + 0x24u);
        if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x3840)
            hp = 0u;
        int16_t ph = (int16_t)rd16(m, F3_A5 + 0x4B38u);
        if (ph != -1) {
            if (ph > 0 && (int32_t)hp <= 0)
                ph = 0;
            if (ph < 0)
                return -1;
            uint32_t fn = rd32(m, rd32(m, obj + 0x64u) + (uint32_t)ph * 4u);
            if (fn != 0x00187DCEu)
                return -1;
            /* 0x187ABE (run before it) only sets 4B26 bit {3:1}, which phase 1 ignores */
            if (phase1_check(m, obj) < 0)
                return -1;
        }
    }

    if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x3840)
        wr32(m, obj + 0x24u, 0u);
    wr32(m, F3_A5 + 0x4B0Cu, rd32(m, obj + 0x24u));

    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {
        Obj_Spawn(m, obj, 0x0017AE0Au, 0u, 1u, 0u);
        wr16(m, obj + 0x06u, 0x0001u);
        wr32(m, obj + 0x00u, 0x00187986u);
        return 0;
    }

    /* 0x18799A */
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) > 0 && (int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {
        wr16(m, F3_A5 + 0x4B38u, 0u);
        bclr8(m, BOSS_4B26, 0x08u);
        uint32_t own = sub_17B02E(m, obj);
        if ((int32_t)own >= 0)
            wr32(m, own + 0x48u, rd32(m, own + 0x48u) + 0x000186A0u);
        bclr8(m, BOSS_4B3A, 0x20u);
        bclr8(m, BOSS_4B3A, 0x10u);
        bclr8(m, BOSS_4B3A, 0x08u);
        bclr8(m, BOSS_4B3A, 0x04u);
        bclr8(m, BOSS_4B3A, 0x02u);
        bclr8(m, BOSS_4B3B, 0x80u);
    }

    /* 0x1879F0 */
    bclr8(m, BOSS_4B3A, 0x80u);
    bclr8(m, BOSS_4B3B, 0x80u);
    bclr8(m, BOSS_4B26, 0x10u);
    if (rd8(m, BOSS_4B26) & 0x20u) {
        bclr8(m, BOSS_4B26, 0x20u);
        sub_187ABE(m, obj);
    }

    /* 0x187A16 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) && !(rd8(m, obj + 0x23u) & 0x80u)) {
        bset8(m, obj + 0x23u, 0x80u);
        Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x00000080u);
    }

    /* 0x187A4E: phase handler from the table at +0x64 */
    phase1(m, obj);

    /* 0x187A5C */
    wr8(m, F3_A5 + 0x4B36u, 0u);
    if (rd8(m, F3_A5 + 0x4B28u) == rd8(m, obj + 0x1Du))
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B29u));
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B28u));
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    wr8(m, obj + 0x35u, 0u);
    wr8(m, F3_A5 + 0x4B36u, rd8(m, obj + 0x1Du));
    if (rd8(m, BOSS_4B3A) & 0x80u) {
        wr8(m, BOSS_4B3B, (uint8_t)(rd8(m, BOSS_4B3B) ^ 0x02u));   /* invert {14:1} */
        bset8(m, BOSS_4B3B, 0x04u);                               /* bfset {13:1} */
    }
    wr32(m, obj + 0x00u, 0x00187938u);      /* 0x187930 */
    return 0;
}
