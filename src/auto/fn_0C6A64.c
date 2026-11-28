/* ROM 0x0C6A64: kernel slot 5 (Task_PlayerControl) scene entry, up to the
 * TRAP #5 at 0x0C6A9A. The body is the player lane's once-per-scene setup:
 *
 *   0C6A64  bset.b #0,(0x1D1E,A5): Z = the old bit. When it was clear, set the
 *           demo-stage word (0x1D40) to 0x1D and clear the control flags (0x1D36)
 *   0C6A7A  Obj_Spawn(A0, fn 0x0C685A, group 7, n 2, arg 0) -- A0 is the caller's
 *           object, so the spawn runs whether or not the scene was fresh
 *   0C6A96  bsr.w 0x0C6B36: Player_SceneInit (frame counter, flags, palettes)
 *           followed by the two-player slot loop 0x0C6BD6
 *   0C6A9A  TRAP #5: the yield the capture stops at (resume PC 0x0C6A9C)
 *
 * Registers at the trap, from the ROM's last writers:
 *   D0  `move.w #0x3F,D0` + `dbra` in the 0x0C6DEC shot-pool reset -> 0x...FFFF
 *   D1  the 0x0C6BD6 slot-loop counter (`move.w #0,D1` at 0x0C6BE6, `addq.w`
 *       once per slot) -> 2
 *   A0  0x410354, the shot pool base the 0x0C6DEC loop leaves
 *   A1  0xC9262 + 6 per slot from 0x0C6BDC, or 0xC926E + 4*id when the slot took
 *       the 0x0C6CCA branch into 0x0C6D0E (`lea (0x253A,PC),A1` at 0x0C6D32)
 *   A2  the last `lea (0x6A,A0),A2` of 0x0C6CB4, i.e. last slot + 0x6A
 * The A5+0x1D38 word selects the player slot that gets 0x0C6D0E + 0x0C6CE6
 * (equal to the slot's +0x22 id); with it 0 the capture takes slot 0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../player/player_task.h"
#include "../video/palette.h"
#include "../video/sprite.h"

#define SLOT0  0x0041B190u                     /* 0x0C6BD6 lea.l ($41b190).l */
#define SLOT_SZ 0x8Au                          /* 0x0C6CD2 lea (0x8A,A0),A0 */
#define SLOT_DATA 0x000C9262u                  /* 0x0C6BDC lea (0x2684,PC),A1 */

/* ROM 0x0C8E52: one gauge field of a player slot. A2 is the descriptor
 * {count.w, step2.w, step4.w, .., map.l@0xA, src.l@0xE}; D1.w is the field
 * value's remainder and D2.w its quotient. It blits the gauge's tile block at
 * 0x61C000 + {0xC914A[id*4] + step2} * 2 + {0xC914A[2+id] << 7}, then queues
 * two palette uploads: the filled part (src descriptor+0xE, `remainder` words)
 * and the empty part (src 0xC9242, count - remainder words). */
static void gauge_0C8E52(const f3mem_t *m, uint32_t slot, uint32_t tbl,
                         uint16_t quo, uint16_t rem)
{
    uint16_t id = rd16(m, slot + 0x22u);                      /* 0x0C8E56 */
    uint32_t a0 = rd32(m, tbl + 0x0Au);                       /* 0x0C8E5A */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)(id * 4u);    /* 0x0C8E5E lea (A0,D0.w*4) */
    uint32_t src = rd32(m, a0 + (uint32_t)(int32_t)(int16_t)(uint16_t)(quo * 8u)); /* 0x0C8E62 */
    a0 = 0x000C914Au;                                         /* 0x0C8E66 */
    uint16_t d3 = rd16(m, a0 + (uint32_t)(uint16_t)(id * 4u)); /* 0x0C8E6C */
    uint16_t d0 = rd16(m, a0 + 2u + (uint32_t)id);            /* 0x0C8E70 $2(A0,D0.w) */
    d3 = (uint16_t)(d3 + rd16(m, tbl + 4u));                  /* 0x0C8E74 */
    d3 = (uint16_t)(d3 << 1);                                 /* 0x0C8E78 */
    d0 = (uint16_t)(d0 << 7);                                 /* 0x0C8E7A */
    d3 = (uint16_t)(d3 + d0);                                 /* 0x0C8E7C */
    Gfx_DrawTilemapBlock(m, src,
                         0x0061C000u + (uint32_t)(int32_t)(int16_t)d3, 0u); /* 0x0C8E8E */

    a0 = 0x000C915Eu;                                         /* 0x0C8E98 */
    d0 = rd16(m, slot + 0x22u);                               /* 0x0C8E9E */
    d0 = rd16(m, a0 + 2u + (uint32_t)d0);                     /* 0x0C8EA2 */
    d3 = (uint16_t)(rd16(m, tbl + 2u) << 2);                  /* 0x0C8EA6 */
    d0 = (uint16_t)(d0 + d3);                                 /* 0x0C8EAC */
    d3 = rem;                                                 /* 0x0C8EAE move.w D1,D3 */
    if (d3 != 0u)                                             /* 0x0C8EB0 beq */
        Pal_QueueUpload(m, rd32(m, tbl + 0x0Eu), d0, d3);     /* 0x0C8EBC */
    uint16_t d4 = (uint16_t)(rd16(m, tbl) - d3);              /* 0x0C8EC6/0x0C8EC8 */
    d3 = (uint16_t)(d3 << 2);                                 /* 0x0C8ECA */
    d0 = (uint16_t)(d0 + d3);                                 /* 0x0C8ECC */
    Pal_QueueUpload(m, 0x000C9242u, d0, d4);                  /* 0x0C8ED8 */
}

/* ROM 0x0C8E1A: the slot's two gauge fields (+0x56 and +0x58), each divided by
 * the divisor word of its descriptor (3 and 5 in the ROM tables). */
static void gauges_0C8E1A(const f3mem_t *m, uint32_t slot)
{
    uint16_t v = rd16(m, slot + 0x56u);                       /* 0x0C8E28 */
    uint16_t div = rd16(m, 0x000C9008u);                      /* 0x0C8E2C divu.w (A2) */
    gauge_0C8E52(m, slot, 0x000C9008u, (uint16_t)(v / div), (uint16_t)(v % div));

    v = rd16(m, slot + 0x58u);                                /* 0x0C8E3E */
    div = rd16(m, 0x000C9022u);                               /* 0x0C8E42 */
    gauge_0C8E52(m, slot, 0x000C9022u, (uint16_t)(v / div), (uint16_t)(v % div));
}

/* ROM 0x0C8BCE: clear the slot's status window in text RAM: five longs at
 * 0x61C000 + 0xC915A[id] and one at +0x80. The PC-relative base of the
 * `lea (0x57C,PC,D0.w*2),A2` at 0x0C8BDC resolves to the per-player pair
 * 0xC915A = {0x0D80, 0x0DA8} (work/ram/fnt_0C6A64 clears exactly
 * 0x61CD80 and 0x61CE00 for player 0). */
static void clear_window_0C8BCE(const f3mem_t *m, uint32_t slot)
{
    uint16_t id = rd16(m, slot + 0x22u);                   /* 0x0C8BD2 */
    uint32_t a1 = 0x0061C000u                              /* 0x0C8BD6 */
                + (uint32_t)(int32_t)(int16_t)rd16(m, 0x000C915Au + 2u * id); /* 0x0C8BDC/0x0C8BE4 */
    for (unsigned k = 0; k < 5u; k++) {                    /* 0x0C8BF0 dbra D0 */
        wr32(m, a1 + 0x80u, 0u);                           /* 0x0C8BEA clr.l (0x80,A1) */
        wr32(m, a1, 0u);                                   /* 0x0C8BEE clr.l (A1)+ */
        a1 += 4u;
    }
}

/* 0x0C6BD6..0x0C6CE4: the two player slots and the shot-pool reset. Returns
 * the slot count. */
static unsigned player_slots_0C6BD6(const f3mem_t *m)
{
    uint32_t a0 = SLOT0;
    uint32_t a1 = SLOT_DATA;
    uint16_t sel = rd16(m, F3_A5 + 0x1D38u);   /* 0x0C6BE2 move.w (0x1d38,A5),D0 */
    uint16_t d1 = 0u;                          /* 0x0C6BE6 move.w #0,D1 */
    unsigned slots = 0u;

    for (;;) {
        /* 0x0C6BEE / 0x0C6C0C: two 0x0CABAC objects in group 7 (n = 2), args
         * 0xFFF4 and 0x1B; the parent is the player slot. */
        (void)Obj_Spawn(m, a0, 0x000CABACu, 7u, 2u, 0x0000FFF4u);
        (void)Obj_Spawn(m, a0, 0x000CABACu, 7u, 2u, 0x0000001Bu);

        wr32(m, a0 + 0x44u, rd32(m, a1));      /* 0x0C6C2E move.l (A1)+,(0x44,A0) */
        a1 += 4u;
        wr8(m, a0 + 0x0Cu, rd8(m, a1));        /* 0x0C6C32 move.b (A1)+ */
        a1 += 1u;
        wr8(m, a0 + 0x0Du, rd8(m, a1));        /* 0x0C6C36 move.b (A1)+ */
        a1 += 1u;

        wr32(m, a0, 0x000C7480u);              /* 0x0C6C3A move.l #0xC7480,(A0) */
        wr16(m, a0 + 0x22u, d1);               /* 0x0C6C40 move.w D1,(0x22,A0) */
        wr8(m, a0 + 0x04u, 0u);                /* 0x0C6C44 sf.b */
        wr8(m, a0 + 0x34u, 0xFFu);             /* 0x0C6C48 st.b */
        wr8(m, a0 + 0x1Cu, 0u);                /* 0x0C6C4C sf.b */
        wr16(m, a0 + 0x1Eu, 0u);               /* 0x0C6C50 clr.w */
        wr16(m, a0 + 0x4Cu, 0xFFFFu);          /* 0x0C6C54 */
        wr16(m, a0 + 0x54u, 0u);               /* 0x0C6C5A */
        wr16(m, a0 + 0x56u, 0u);               /* 0x0C6C60 */
        wr16(m, a0 + 0x58u, 0u);               /* 0x0C6C66 */
        wr32(m, a0 + 0x48u, 0u);               /* 0x0C6C6C clr.l */
        wr16(m, a0 + 0x4Eu, 0u);               /* 0x0C6C74 */
        wr16(m, a0 + 0x42u, 0x000Cu);          /* 0x0C6C7A */
        wr16(m, a0 + 0x5Au, 0x000Fu);          /* 0x0C6C80 */
        wr16(m, a0 + 0x5Cu, 0x000Cu);          /* 0x0C6C86 */
        wr16(m, a0 + 0x62u, 0xFFFFu);          /* 0x0C6C8C */
        wr8(m, a0 + 0x69u, 0u);                /* 0x0C6C92 */
        wr8(m, a0 + 0x68u, 0u);                /* 0x0C6C96 */
        wr16(m, a0 + 0x66u, 0xFFFFu);          /* 0x0C6C9A */
        /* 0x0C6CA0 bfclr (0x66,A0){0:1}: a memory bit field counts from the MSB
         * of the first byte, so this clears bit 7 of the high byte. */
        wr8(m, a0 + 0x66u, (uint8_t)(rd8(m, a0 + 0x66u) & 0x7Fu));

        wr16(m, a0 + 0x7Au, rd16(m, a0 + 0x56u));  /* 0x0C6CAA */
        wr16(m, a0 + 0x7Cu, 0xFFFFu);              /* 0x0C6CAE */
        wr16(m, a0 + 0x6Au, rd16(m, a0 + 0x58u));  /* 0x0C6CB8 */
        wr16(m, a0 + 0x6Cu, 0xFFFFu);              /* 0x0C6CBC */

        if (rd16(m, a0 + 0x22u) == sel) {      /* 0x0C6CC2 cmp.w (0x22,A0),D0 */
            (void)Player_Respawn(m, a0);       /* 0x0C6CCA bsr 0x0C6D0E */
            gauges_0C8E1A(m, a0);              /* 0x0C6DC2 inside 0x0C6D0E */
            clear_window_0C8BCE(m, a0);        /* 0x0C6DC6 inside 0x0C6D0E */
            {   /* 0x0C6DCA subi.w #0x708,(0x3318,A5) ; bge ; clr.w */
                uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x3318u) - 0x0708u);
                wr16(m, F3_A5 + 0x3318u, (int16_t)v >= 0 ? v : 0u);
            }
            {   /* 0x0C6DE0: 0x0C660A adds D0 = 0x50000 to the bomb-damage long
                 * of the slot's player id (`add.l D0,(0x41A980,D1.w*4)`, D1 =
                 * A0+0x22); the helper saves and restores D1/A0. */
                uint32_t bd = 0x0041A980u + 4u * (uint32_t)rd16(m, a0 + 0x22u);
                wr32(m, bd, rd32(m, bd) + 0x00050000u);
            }
            Player_ArmRespawn(m, a0);          /* 0x0C6CCE bsr 0x0C6CE6 */
        }

        a0 += SLOT_SZ;                         /* 0x0C6CD2 lea (0x8A,A0),A0 */
        slots++;
        d1++;                                  /* 0x0C6CD6 addq.w #1,D1 */
        if ((int16_t)d1 >= 2)                  /* 0x0C6CD8 cmpi.w #2 / blt */
            break;
    }
    Player_ShotPoolReset(m);                   /* 0x0C6CE0 bsr 0x0C6DEC */
    return slots;
}


int Fn_0C6A64(const f3mem_t *m, f3regs_t *r)
{
    uint8_t old = rd8(m, F3_A5 + 0x1D1Eu);
    wr8(m, F3_A5 + 0x1D1Eu, (uint8_t)(old | 0x01u));     /* 0x0C6A64 bset.b #0 */
    if (!(old & 0x01u)) {                                /* 0x0C6A6A bne 0x0C6A7A */
        wr16(m, F3_A5 + 0x1D40u, 0x001Du);               /* 0x0C6A6E */
        wr8(m, F3_A5 + 0x1D36u, 0u);                     /* 0x0C6A74 */
    }

    /* 0x0C6A7A..0x0C6A92: Obj_Spawn(A0, fn 0x0C685A, group 7, n 2, arg 0). */
    r->d[0] = Obj_Spawn(m, r->a[0], 0x000C685Au, 7u, 2u, 0u); /* D0 = the object */
    /* ... whose high word (0x0041) is what the 0x0C6BE2 `move.w (0x1D38,A5),D0`
     * keeps: the trap's D0 is 0x0041FFFF in work/ram/fnt_0C6A64. */

    (void)Player_SceneInit(m);                           /* 0x0C6A96 bsr 0x0C6B36 */
    unsigned slots = player_slots_0C6BD6(m);

    /* Registers at the 0x0C6A9A TRAP #5 (see the file comment). */
    uint16_t sel = rd16(m, F3_A5 + 0x1D38u);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0xFFFFu;        /* 0x0C6DF2 move.w #0x3F + dbra */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)slots; /* 0x0C6BE6/0x0C6CD6 */
    r->a[0] = 0x00410354u;                               /* 0x0C6DEC lea.l */
    r->a[1] = (sel < slots) ? (0x000C926Eu + 4u * (uint32_t)sel) /* 0x0C6D32 */
                            : (SLOT_DATA + 6u * (uint32_t)slots);
    r->a[2] = (SLOT0 + SLOT_SZ * (uint32_t)(slots - 1u)) + 0x6Au; /* 0x0C6CB4 */
    return 0;                                            /* 0x0C6A9A trap #5 */
}
