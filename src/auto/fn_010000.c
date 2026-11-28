/* ROM 0x010000: UNVERIFIED (no capture): reset vector target -- hardware init,
 * RAM test, RAM/line-RAM clear, display + sound init, then the jump into the
 * kernel cold boot at 0x001DB8.
 *
 *   010000  ori.w #0x700,SR      (supervisor, IRQ mask 7: SR is not in f3regs_t)
 *   010004  A5 = 0x408000        (the global base every later routine uses)
 *   01000A  byte writes to the F3 control/IO ports 0x4A0000, 0x4A0006,
 *           0x4A0016 (0xFF), 0x4A0004 and 0x4A0014 (all write-only MMIO)
 *   010032  word 0x278B to the timer 0x4C0000, word 0 to video reg 0x66001E
 *   010042  D0 = 0, then zero the ten video registers 0x660000..0x66001A
 *   010080  A4 = 0x01008A (PC-relative), jmp 0x000402: the RAM test returns
 *           through jmp (A4), i.e. at 0x01008A below.
 *   01008A  clear work RAM 0x400000..0x420000 (0x8000 longs of 0, one
 *           0x4A0000 strobe per long)
 *   0100A8  word 0 to A5-0x5DAA, A5-0x5DC7 = 0xFF
 *   0100B2  line-set RAM 0x621000: word 0xF0 then 0x7FF words of 0
 *   0100D4  jsr 0x004D06 (EEPROM settings), 0x005B0C (line RAM), 0x005A3A
 *           (character font), 0x001C2C (scroll base table), 0x005A12 (all
 *           layers), 0x001070 (sprite buffers), 0x00117E (sprites), 0x0014DA
 *           (sound bank 0), 0x003960 (sound DPRAM reset), 0x000BA2 (sound
 *           chip init, translated below) and 0x000D5A (dip/config init,
 *           translated below)
 *   010100  jmp 0x001DB8           (kernel cold boot: loads the register set
 *                                   from ROM 0x2326, spawns Task0 at 0x010172
 *                                   through TRAP #1 and enters the scheduler;
 *                                   it never returns, so the translation ends
 *                                   here)
 *
 * Both jmps are tail jumps: these addresses are not called with a return
 * address of their own.
 *
 * The jsr return addresses of the register-level callees (0x004D06, 0x005A3A,
 * 0x001C2C, 0x000BA2) are not written to the stack: those helpers leave SP at
 * their entry value instead of popping the way their own rts does. The frames
 * are below SP, nothing on this path reads them (the last jump goes to the
 * kernel cold boot and never returns), and every register and every byte of
 * RAM/gfx outside the stack window is written exactly as the ROM does.
 *
 * 0x000D5A reads the switch port 0x4A0002, which the model leaves unmapped
 * (returns 0), so Fn_010000 always ends in the SERVICE SW ERROR branch into
 * 0x01010A here, as on a cabinet with the service switches off. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../audio/sound_cmd.h"
#include "../kernel/soundque.h"    /* Sound_EnqueueExact (0x000910) */
#include "../video/sprite.h"

/* 0x000910 (Sound_EnqueueExact) leaves D0 = the shifted command, D1.w =
 * 0xFFFF, A0 = the serial shadow A5-0x5DCA and A1 = 0x4A0013; the shared helper
 * models only the port and the shadow, so the leftovers are set here (the same
 * convention as push_cmd in fn_000B30.c). */
static void enqueue_word(const f3mem_t *m, f3regs_t *r, uint16_t cmd)
{
    uint32_t a5 = r->a[5];
    r->a[7] -= 2u;                              /* move.w #cmd,-(A7) */
    wr16(m, r->a[7], cmd);
    Sound_EnqueueExact(m, cmd);                 /* bsr 0x000910 */
    r->a[7] += 2u;                              /* addq.l #2,A7 */
    r->d[0] = set_word(r->d[0], (uint16_t)(cmd >> 9)); /* 0x093E dbra D1 (9x lsr.w) */
    r->d[1] = set_word(r->d[1], 0xFFFFu);
    r->a[0] = a5 - 0x5DCAu;
    r->a[1] = 0x004A0013u;
}

/* 0x000C8A with the byte the ROM pushes: it consumes the argument from the
 * stack (SP+4) and returns with SP past it. */
static void c8a_byte(const f3mem_t *m, f3regs_t *r, uint8_t value, uint32_t ret)
{
    r->a[7] -= 2u;                              /* 0x000BCC move.b #x,-(A7) */
    wr8(m, r->a[7], value);
    push_long(m, r, ret);                       /* bsr 0x000C8A */
    (void)Fn_000C8A(m, r);
    r->a[7] += 2u;                              /* 0x000BD4 addq.l #2,A7 */
}

/* ROM 0x000BA2: sound-chip init.
 *   000BA2  D0 = 0 ; word 0 to the ring write/read indices A5-0x7F80/0x7F7E
 *   000BAC  bset #0 then #1 in the serial shadow byte A5-0x5DCA
 *   000BB8  bsr 0x000910 with 0x3FF, then with 0x3FE (two serial words)
 *   000BCC  bsr 0x000C8A with 0xE0
 *   000BD6  D0 = sign-extended word at ROM 0x1FFFFE (dip switch), then
 *           000C30(1, D0) -- queue the switch value as one decimal pair
 *   000BEA  bsr 0x000C8A with 0xFF
 * No register is saved; the shadow/DPRAM effects come from the shared
 * register-level helpers. */
static void sound_init_000BA2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    r->d[0] = 0;                                         /* 0x000BA2 moveq #0,D0 */
    wr16(m, a5 - 0x7F80u, 0u);                           /* 0x000BA4 move.w D0 */
    wr16(m, a5 - 0x7F7Eu, 0u);                           /* 0x000BA8 */
    wr8(m, a5 - 0x5DCAu, (uint8_t)(rd8(m, a5 - 0x5DCAu) | 0x01u)); /* 0x000BAC */
    wr8(m, a5 - 0x5DCAu, (uint8_t)(rd8(m, a5 - 0x5DCAu) | 0x02u)); /* 0x000BB2 */

    enqueue_word(m, r, 0x03FFu);                         /* 0x000BB8/0x000BBC */
    enqueue_word(m, r, 0x03FEu);                         /* 0x000BC2/0x000BC6 */
    c8a_byte(m, r, 0xE0u, 0x000BD4u);                    /* 0x000BCC/0x000BD0 */

    r->d[0] = (uint32_t)(int32_t)(int16_t)rd16(m, 0x001FFFFEu); /* 0x000BD6 */
    push_word(m, r, 1u);                                 /* 0x000BDE move.w #1 */
    push_long(m, r, r->d[0]);                            /* 0x000BE2 move.l D0 */
    push_long(m, r, 0x000BE8u);                          /* bsr 0x000C30 */
    (void)Fn_000C30(m, r);
    r->a[7] += 6u;                                       /* 0x000BE8 addq.l #6 */

    c8a_byte(m, r, 0xFFu, 0x000BF2u);                    /* 0x000BEA/0x000BEE */
    /* 0x000BF4 rts */
}

/* ROM 0x000D5A: copy the cabinet switches into the RAM config block.
 *   000D5A  jsr 0x000BA2 (above)
 *   000D5E  A5-0x5DE0 = 0xFF, A5-0x7F6A / A5-0x7F66 = 0x1E (lives / credits)
 *   000D70  D2b = byte at ROM 0x1FFFFF (dip switch), D1 = high then low
 *           nibble of A5-0x5DC9 into A5-0x7F5C / A5-0x7F5A, A5-0x7F5E = 0
 *   000D8E  only when that switch reads 2: A5-0x7F5E = min(non-zero high
 *           nibble of A5-0x5DC8, A5-0x7F5C)
 *   000DAA  when the switch reads 2: copy A5-0x7F5C..0x7F5E to
 *           A5-0x7F58..0x7F5A and point A5-0x7F62 at 0x400092; otherwise
 *           split the nibbles of A5-0x5DC8 into A5-0x7F58/0x7F56 and point
 *           A5-0x7F62 at 0x400094
 *   000DDC  if (byte at IO 0x4A0002 & 0x0E) != 0x0E: A0 = 0x000F91
 *           ("   SERVICE SW ERROR   ") and jmp 0x01010A (the test-mode entry,
 *           which never returns); else rts. */
static void config_init_000D5A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint8_t sw;

    sound_init_000BA2(m, r);                             /* 0x000D5A jsr 0x000BA2 */
    wr8(m, a5 - 0x5DE0u, 0xFFu);                         /* 0x000D5E */
    wr8(m, a5 - 0x7F6Au, 0x1Eu);                         /* 0x000D64 */
    wr8(m, a5 - 0x7F66u, 0x1Eu);                         /* 0x000D6A */

    sw = rd8(m, 0x001FFFFFu);                            /* 0x000D70 move.b */
    r->d[2] = (r->d[2] & 0xFFFFFF00u) | sw;
    r->d[1] = (uint32_t)(rd8(m, a5 - 0x5DC9u) >> 4) & 0x0Fu; /* 0x000D76 bfextu {0:4} */
    wr16(m, a5 - 0x7F5Cu, (uint16_t)r->d[1]);            /* 0x000D7C */
    r->d[1] = (uint32_t)rd8(m, a5 - 0x5DC9u) & 0x0Fu;    /* 0x000D80 bfextu {4:4} */
    wr16(m, a5 - 0x7F5Au, (uint16_t)r->d[1]);            /* 0x000D86 */
    wr16(m, a5 - 0x7F5Eu, 0u);                           /* 0x000D8A clr.w */

    if (sw == 2u) {                                      /* 0x000D8E cmpi.b #2 */
        uint16_t live = (uint16_t)(rd8(m, a5 - 0x5DC8u) >> 4); /* 0x000D94 */
        r->d[1] = live;
        if (live == 0u ||                              /* 0x000D9A beq 0x000DA2 */
            (int16_t)live >= (int16_t)rd16(m, a5 - 0x7F5Cu)) /* 0x000D9C/0x000DA0 */
            r->d[1] = rd16(m, a5 - 0x7F5Cu);           /* 0x000DA2 move.w */
        wr16(m, a5 - 0x7F5Eu, (uint16_t)r->d[1]);      /* 0x000DA6 */
    }

    if (sw == 2u) {                                      /* 0x000DAA */
        wr32(m, a5 - 0x7F58u, rd32(m, a5 - 0x7F5Cu));  /* 0x000DB0 move.l */
        wr32(m, a5 - 0x7F62u, 0x00400092u);            /* 0x000DB6 */
    } else {
        wr16(m, a5 - 0x7F58u, (uint16_t)(rd8(m, a5 - 0x5DC8u) >> 4)); /* 0x000DC0 */
        wr16(m, a5 - 0x7F56u, (uint16_t)(rd8(m, a5 - 0x5DC8u) & 0x0Fu)); /* 0x000DCA */
        wr32(m, a5 - 0x7F62u, 0x00400094u);            /* 0x000DD4 */
    }

    r->d[3] = (r->d[3] & 0xFFFFFF00u) | (uint8_t)(rd8(m, 0x004A0002u) & 0x0Eu);
    if ((uint8_t)r->d[3] != 0x0Eu) {                     /* 0x000DE6 cmpi.b #0xe */
        r->a[0] = 0x00000F91u;                           /* 0x000DEE lea (0x1A1,PC) */
        (void)Fn_01010A(m, r);                           /* 0x000DF2 jmp 0x01010A */
    }
    /* 0x000DF8 rts */
}

int Fn_010000(const f3mem_t *m, f3regs_t *r)
{
    /* 0x010000 ori.w #0x700,SR: supervisor mode, not modelled (no SR). */

    r->a[5] = F3_A5;                                /* 0x010004 lea (0x408000).l */
    wr8(m, 0x004A0000u, 0x00u);                     /* 0x01000A */
    wr8(m, 0x004A0006u, 0x00u);                     /* 0x010012 */
    wr8(m, 0x004A0016u, 0xFFu);                     /* 0x01001A */
    wr8(m, 0x004A0004u, 0x00u);                     /* 0x010022 */
    wr8(m, 0x004A0014u, 0x00u);                     /* 0x01002A */
    wr16(m, 0x004C0000u, 0x278Bu);                  /* 0x010032 */
    wr16(m, 0x0066001Eu, 0x0000u);                  /* 0x01003A */

    r->d[0] = 0;                                    /* 0x010042 moveq #0,D0 */
    static const uint32_t video[] = {
        0x00660000u, 0x00660002u, 0x00660004u, 0x00660006u, 0x00660008u,
        0x0066000Au, 0x0066000Cu, 0x0066000Eu, 0x00660018u, 0x0066001Au,
    };
    for (unsigned i = 0; i < sizeof video / sizeof video[0]; ++i) /* 0x010044..0x01007A */
        wr16(m, video[i], (uint16_t)r->d[0]);

    r->a[4] = 0x0001008Au;                          /* 0x010080 lea (0x8,PC),A4 */
    if (RamTest_000402(m, r) != 0)
        return 0;                                   /* the ROM spins in the error screen */

    /* 0x01008A: clear work RAM with longwords of D1 = 0. */
    r->a[0] = 0x00400000u;                          /* 0x01008A lea */
    r->d[0] = 0x00020000u >> 2;                     /* 0x010090/0x010096 lsr.l #2 */
    r->d[1] = 0;                                    /* 0x010098 moveq #0,D1 */
    do {
        wr8(m, 0x004A0000u, 0x00u);                  /* 0x01009A */
        wr32(m, r->a[0], r->d[1]);                   /* 0x0100A2 move.l D1,(A0)+ */
        r->a[0] += 4u;
        r->d[0] -= 1u;                               /* 0x0100A4 subq.l #1 */
    } while (r->d[0] != 0u);                         /* 0x0100A6 bne 0x01009A */

    wr16(m, r->a[5] - 0x5DAAu, 0u);                  /* 0x0100A8 */
    wr8(m, r->a[5] - 0x5DC7u, 0xFFu);                /* 0x0100AE st.b */

    /* 0x0100B2: line-set RAM 0x621000 gets word 0xF0 then 0x7FF words of 0
     * (0x1000/2 - 2 = 0x7FE, dbra => 0x7FF iterations). */
    r->a[0] = 0x00621000u;                           /* 0x0100B2 lea */
    wr16(m, r->a[0], 0x00F0u);                       /* 0x0100B8 move.w #0xf0,(A0)+ */
    r->a[0] += 2u;
    r->d[0] = (uint16_t)((0x1000u >> 1) - 2u);       /* 0x0100BC..0x0100C2 */
    r->d[1] = 0;                                     /* 0x0100C4 moveq #0,D1 */
    do {
        wr8(m, 0x004A0000u, 0x00u);                  /* 0x0100C6 */
        wr16(m, r->a[0], (uint16_t)r->d[1]);         /* 0x0100CE move.w D1,(A0)+ */
        r->a[0] += 2u;
        r->d[0] = set_word(r->d[0], (uint16_t)((uint16_t)r->d[0] - 1u));
    } while ((uint16_t)r->d[0] != 0xFFFFu);          /* 0x0100D0 dbra D0 */

    (void)Fn_004D06(m, r);                           /* 0x0100D4 jsr 0x00004D06 */
    Gfx_InitLineRam(m);                              /* 0x0100D8 jsr 0x00005B0C */
    (void)Fn_005A3A(m, r);                           /* 0x0100DC jsr 0x00005A3A */
    (void)Fn_001C2C(m, r);                           /* 0x0100E0 jsr 0x00001C2C */
    Gfx_ClearAllLayers(m);                           /* 0x0100E4 jsr 0x00005A12 */
    Sprite_InitBuffers(m);                           /* 0x0100E8 jsr 0x00001070 */
    Sprite_ClearAll(m);                              /* 0x0100EC jsr 0x0000117E */
    SoundBank0_0014DA_Regs(m, r);                    /* 0x0100F0 jsr 0x000014DA */
    Sound_ResetDpram(m);                             /* 0x0100F4 jsr 0x00003960 */
    sound_init_000BA2(m, r);                         /* 0x0100F8 jsr 0x00000BA2 */
    config_init_000D5A(m, r);                        /* 0x0100FC jsr 0x00000D5A */

    /* 0x010100 jmp 0x001DB8: the kernel cold boot never returns. */
    return 0;
}
