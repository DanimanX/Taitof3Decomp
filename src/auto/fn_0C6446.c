/* ROM 0x0C6446: register-level wrapper of Coll_PlayerShotsVsEnemies (src/collision/collision.h) for its capture.
 * No arguments: the routine walks the fixed shot list at COLL_LIST2 (0x41ABE0,
 * cursor COLL_LIST2_END) against the shootable list at COLL_LIST1 (0x41A988,
 * cursor COLL_LIST1_END). Every RAM effect comes from Coll_PlayerShotsVsEnemies;
 * this file only rebuilds the register file the ROM leaves at its rts:
 *   D0 = the shots-end cursor loaded at 0x0C6446, A0 = the same pointer, which is
 *        also where the outer loop's post-incremented (a0) ends (0x0C645C).
 *   D1 = 0x41A988, the constant loaded at 0x0C644A (`move.l #$41a988,d1`).
 *   A2 = the last shot read by the outer loop, D2..D5 = its box: the word-sized
 *        adds at 0x0C645E..0x0C647A combine hitbox +0x2C/+0x2E/+0x30/+0x32 with
 *        position +0x14/+0x18, so the entry's upper halves survive.
 *   A1/A3/D6 are the residue of the inner loop (0x0C647E..0x0C64F6). Its exit
 *        value depends on which of the four box comparisons fails, so
 *        shot_scan_tail() replays only the pointer walk and the two `btst` flags
 *        tests for the LAST shot: it reads memory and writes nothing, so the
 *        collision logic itself is never duplicated.
 * When the list holds no shot the outer loop is never entered (0x0C6456
 * `cmpa.l d0,a0 ; bge`), so A1/A2/A3 and D2..D6 keep their entry values. */
#include "fn_auto.h"
#include "../collision/collision.h"

#define E_POS_X 0x14
#define E_POS_Y 0x18
#define E_DMG   0x28
#define E_HBX0  0x2C
#define E_HBX1  0x2E
#define E_HBY0  0x30
#define E_HBY1  0x32
#define E_FLAGS 0x34

typedef struct { uint32_t a1, a3, d6; } tail_t;

/* 0x0C6482..0x0C64F6 for the last shot. Word comparisons are signed (cmp.w), and
 * each `move.w $xx(a3),d6 / add.w ..,d6` keeps D6's upper half. */
static tail_t shot_scan_tail(const f3mem_t *m, uint32_t shot, int16_t d2, int16_t d3,
                             int16_t d4, int16_t d5, uint32_t a1, uint32_t a3, uint32_t d6)
{
    tail_t t = { a1, a3, d6 };
    for (;;) {
        if ((int32_t)a1 <= (int32_t)COLL_LIST1)             /* 0x0C6482 ble -> next shot */
            break;
        a1 -= 4;                                            /* 0x0C6486 movea.l -(a1),a3 */
        t.a3 = rd32(m, a1);
        int16_t v = (int16_t)(uint16_t)(rd16(m, t.a3 + E_HBX1) + rd16(m, t.a3 + E_POS_X));
        t.d6 = (t.d6 & 0xFFFF0000u) | (uint16_t)v;          /* 0x0C6488 */
        if (v < d2) continue;                               /* 0x0C6490 blt */
        v = (int16_t)(uint16_t)(rd16(m, t.a3 + E_HBX0) + rd16(m, t.a3 + E_POS_X));
        t.d6 = (t.d6 & 0xFFFF0000u) | (uint16_t)v;          /* 0x0C6494 */
        if (v > d3) continue;                               /* 0x0C649C bgt */
        v = (int16_t)(uint16_t)(rd16(m, t.a3 + E_HBY1) + rd16(m, t.a3 + E_POS_Y));
        t.d6 = (t.d6 & 0xFFFF0000u) | (uint16_t)v;          /* 0x0C64A0 */
        if (v < d4) continue;                               /* 0x0C64A8 blt */
        v = (int16_t)(uint16_t)(rd16(m, t.a3 + E_HBY0) + rd16(m, t.a3 + E_POS_Y));
        t.d6 = (t.d6 & 0xFFFF0000u) | (uint16_t)v;          /* 0x0C64AC */
        if (v > d5) continue;                               /* 0x0C64B4 bgt */
        uint8_t f = rd8(m, t.a3 + E_FLAGS);                 /* 0x0C64CA / 0x0C64DC / 0x0C64EE */
        if (!(f & 0x01))
            t.d6 = rd32(m, t.a3 + E_DMG);                   /* 0x0C64D4 move.l $28(a3),d6 */
        if (!(f & 0x02))
            t.d6 = rd32(m, shot + E_DMG);                   /* 0x0C64E6 move.l $28(a2),d6 */
        if (!(f & 0x40))
            break;                                          /* 0x0C64F6 shot consumed */
    }
    t.a1 = a1;
    return t;
}

int Fn_0C6446(const f3mem_t *m, f3regs_t *r)
{
    uint32_t end = rd32(m, COLL_LIST2_END);                 /* 0x0C6446 move.l $902(a5),d0 */
    uint32_t enemy_end = rd32(m, COLL_LIST1_END);            /* 0x0C647E movea.l $8fe(a5),a1 */
    int have_shot = (int32_t)end > (int32_t)COLL_LIST2;
    uint32_t shot = have_shot ? rd32(m, end - 4u) : 0u;      /* 0x0C645C last (a0)+ load */
    int16_t d2 = 0, d3 = 0, d4 = 0, d5 = 0;

    if (have_shot) {
        int16_t x = (int16_t)rd16(m, shot + E_POS_X), y = (int16_t)rd16(m, shot + E_POS_Y);
        d2 = (int16_t)(uint16_t)(rd16(m, shot + E_HBX0) + x);   /* 0x0C645E */
        d3 = (int16_t)(uint16_t)(rd16(m, shot + E_HBX1) + x);   /* 0x0C6462 */
        d4 = (int16_t)(uint16_t)(rd16(m, shot + E_HBY0) + y);   /* 0x0C646E */
        d5 = (int16_t)(uint16_t)(rd16(m, shot + E_HBY1) + y);   /* 0x0C6472 */
    }

    Coll_PlayerShotsVsEnemies(m);

    r->d[0] = end;                                          /* 0x0C6446 */
    r->a[0] = end;                                          /* 0x0C645C (a0)+ chain */
    r->d[1] = COLL_LIST1;                                   /* 0x0C644A */
    if (have_shot) {
        r->a[2] = shot;
        r->d[2] = (r->d[2] & 0xFFFF0000u) | (uint16_t)d2;
        r->d[3] = (r->d[3] & 0xFFFF0000u) | (uint16_t)d3;
        r->d[4] = (r->d[4] & 0xFFFF0000u) | (uint16_t)d4;
        r->d[5] = (r->d[5] & 0xFFFF0000u) | (uint16_t)d5;
        tail_t t = shot_scan_tail(m, shot, d2, d3, d4, d5, enemy_end, r->a[3], r->d[6]);
        r->a[1] = t.a1;
        r->a[3] = t.a3;
        r->d[6] = t.d6;
    }
    return 0;
}
