/* ROM 0x0C64FC: register-level wrapper of Coll_PlayersVsHazards (src/collision/collision.h) for its capture.
 * No arguments: the two fixed player blocks at 0x41B190 (step 0x8A) are tested
 * against the hazard list at COLL_LIST3 (0x41ACE0, cursor COLL_LIST3_END). The
 * RAM effects come from Coll_PlayersVsHazards; the registers at rts are:
 *   D0 = 0x41ACE0 (0x0C64FC `move.l #$41ace0,d0`, the list base it compares with).
 *   A0 = 0x41B2A4, 0x41B190 + 2*0x8A (0x0C6502 lea + two `lea 0x8a(a0),a0`), and
 *   A1 = 0x41A988, 0x41A980 + 2*4 (0x0C6508 lea + two `lea 4(a1),a1`): both step
 *        once per `dbra d5` iteration (two, 0x0C6604).
 *   A2 = the hazard-list cursor read at 0x0C650E (0x41ACE0 or 0x41ACEC here).
 *   A3 = 0x41ACE0: each player iteration reloads A3 from A2 (0x0C6548) and walks it
 *        down in steps of 4 until `cmpa.l d0,a3 ; ble` (0x0C654A), which always
 *        lands on the list base; if both players are skipped by 0x0C6516/0x0C651E
 *        the entry value survives.
 *   D5 = 0xFFFF in the low word only (0x0C6512 `move.w #$1,d5` + two `dbra`).
 *   D1..D4 = the box of the last player iteration that was not skipped (0x0C6528..
 *       0x0C6544, word-sized adds, so the entry's upper halves survive). A skipped
 *        player leaves them alone.
 * D6 is written by the inner loop but its value is the same before and after in
 * every captured pair, so it is left untouched here. */
#include "fn_auto.h"
#include "../collision/collision.h"

#define E_POS_X  0x14
#define E_POS_Y  0x18
#define E_HBX0   0x2C
#define E_HBX1   0x2E
#define E_HBY0   0x30
#define E_HBY1   0x32
#define E_FLAGS  0x34
#define E_SKIP   0x65
#define PLAYERS  0x41B190u
#define PLAYER_SIZE 0x8Au

int Fn_0C64FC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t top = rd32(m, COLL_LIST3_END);                 /* 0x0C650E movea.l $906(a5),a2 */
    uint32_t box = 0;
    int scanned = 0;

    for (int i = 0; i < 2; i++) {                           /* 0x0C6512 move.w #$1,d5 */
        uint32_t pl = PLAYERS + (uint32_t)i * PLAYER_SIZE;
        if (rd8(m, pl + E_FLAGS) || (rd8(m, pl + E_SKIP) & 0x04u))
            continue;                                       /* 0x0C6516 / 0x0C651E */
        box = pl;
        scanned = 1;
    }

    Coll_PlayersVsHazards(m);

    r->d[0] = 0x0041ACE0u;                                  /* 0x0C64FC */
    r->a[0] = PLAYERS + 2u * PLAYER_SIZE;                   /* 0x0C6502 + 2 * lea 0x8a */
    r->a[1] = COLL_LIST1;                                   /* 0x0C6508 + 2 * lea 4 */
    r->a[2] = top;                                          /* 0x0C650E */
    if (scanned)
        r->a[3] = COLL_LIST3;                               /* 0x0C6548 walk down to the base */
    r->d[5] = (r->d[5] & 0xFFFF0000u) | 0x0000FFFFu;        /* 0x0C6604 dbra */

    if (scanned) {
        int16_t x = (int16_t)rd16(m, box + E_POS_X), y = (int16_t)rd16(m, box + E_POS_Y);
        r->d[1] = (r->d[1] & 0xFFFF0000u)
                | (uint16_t)(int16_t)(uint16_t)(rd16(m, box + E_HBX0) + x);   /* 0x0C6528 */
        r->d[2] = (r->d[2] & 0xFFFF0000u)
                | (uint16_t)(int16_t)(uint16_t)(rd16(m, box + E_HBX1) + x);   /* 0x0C652C */
        r->d[3] = (r->d[3] & 0xFFFF0000u)
                | (uint16_t)(int16_t)(uint16_t)(rd16(m, box + E_HBY0) + y);   /* 0x0C6538 */
        r->d[4] = (r->d[4] & 0xFFFF0000u)
                | (uint16_t)(int16_t)(uint16_t)(rd16(m, box + E_HBY1) + y);   /* 0x0C653C */
    }
    return 0;
}
