#include "obj_C74CC.h"
#include "../player/player_task.h"

/* ROM 0x0C74CC: continue panel of a player that is out, ticked from the
 * player lane (0x0C6B0C jsr (A2), A0 = player state, A1 = its input channel
 * +0x44). +0x22 picks the panel origin {col, row} in the word table 0x0C94BA
 * (full-format lea, ext word 0x0C74D2). Text is colour 7 in text RAM.
 *   A5+0x1D36 bit 7: blank rows 0..2.
 *   A5+0x08A0 bit 7: "GAME OVER" on row 2, then the coin line.
 *   +0x62 > 0:       "TO CONTINUE" on row 2; unless A5-0x5DC2 is set, blank
 *                    row 0 / "GAME OVER" row 1 / blank row 2 and done.
 *   else:            "TO START" on row 2, then the coin line.
 * Coin line (0x0C76C8): 0x0C77BE credit check, 0x0C7814 text on row 0, blank
 * row 1; with no credit message and the start button up (A1+1 bit 7 clear)
 * it clears rows 1..3, runs 0x0C7C60, queues sound 0x09AC and arms 0x0C7946
 * (sound helper not decompiled: -1 there). Otherwise re-arms 0x0C74CC. */
#define BLANK 0x000C9599u

int Obj_C74CC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint32_t a2 = 0x000C94BAu + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x22u) * 4u;
    uint16_t d2 = rd16(m, a2), d3 = rd16(m, a2 + 2u);

    if ((rd8(m, F3_A5 + 0x1D36u) & 0x80u) != 0u) {         /* 0x0C759C */
        (void)Player_PanelText(m, BLANK, d2, d3);
        (void)Player_PanelText(m, BLANK, d2, (uint16_t)(d3 + 1u));
        (void)Player_PanelText(m, BLANK, d2, (uint16_t)(d3 + 2u));
        goto rearm;
    }
    if ((rd8(m, F3_A5 + 0x08A0u) & 0x80u) != 0u) {         /* 0x0C7564 */
        (void)Player_PanelText(m, 0x000C954Du, d2, (uint16_t)(d3 + 2u));  /* GAME OVER */
    } else if ((int16_t)rd16(m, obj + 0x62u) > 0) {          /* 0x0C74F4 */
        (void)Player_PanelText(m, 0x000C9560u, d2, (uint16_t)(d3 + 2u));  /* TO CONTINUE */
        if (rd8(m, F3_A5 - 0x5DC2u) == 0u) {                 /* 0x0C762E */
            (void)Player_PanelText(m, BLANK, d2, d3);
            (void)Player_PanelText(m, 0x000C954Du, d2, (uint16_t)(d3 + 1u));
            (void)Player_PanelText(m, BLANK, d2, (uint16_t)(d3 + 2u));
            goto rearm;
        }
    } else {                                                 /* 0x0C752C */
        (void)Player_PanelText(m, 0x000C9586u, d2, (uint16_t)(d3 + 2u));  /* TO START */
    }

    /* 0x0C76C8 */
    uint8_t d4b = Player_CoinCheckA(m, obj);
    Player_FailText(m, d4b, d2, d3);
    d3 = (uint16_t)(d3 + 1u);
    (void)Player_PanelText(m, BLANK, d2, d3);
    if (d4b != 0u || (rd8(m, a1 + 1u) & 0x80u) != 0u)
        goto rearm;

    /* 0x0C7716: rows 1, 2, 3 blank (row 1 again) */
    (void)Player_PanelText(m, BLANK, d2, d3);
    (void)Player_PanelText(m, BLANK, d2, (uint16_t)(d3 + 1u));
    (void)Player_PanelText(m, BLANK, d2, (uint16_t)(d3 + 2u));
    if (Player_DefeatBookKeeping(m, obj) < 0)               /* 0x0C779E */
        return -1;
    return -1;  /* 0x0C77AE: jsr 0x09AC(+0x23, A5+0x32DE), then 0x0C793E arms 0x0C7946 */

rearm:
    wr32(m, obj + 0x00u, 0x000C74CCu);                       /* 0x0C74C4 */
    return 0;
}
