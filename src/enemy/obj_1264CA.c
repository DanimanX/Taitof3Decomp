#include "obj_1264CA.h"
#include "obj_11B262.h"

/* ROM 0x1264CA: steering tick. The 0x11B288/0x11B2CA/0x11B262 trio, then the
 * +0x06 countdown: expiry sets A5+0x30EE{0xD} and hands over to 0x1264F8
 * with a 0x50-frame timer. */
int Obj_1264CA_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1264CA jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1264D0 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1264D6 jsr 0x11B262 */

    /* 0x1264DC: subq.w #1 -> bgt 0x1264C2 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001264CAu);         /* 0x1264C2 */
        return 0;
    }
    /* 0x1264E2: bfset (0x30EE,A5){0xD:1} -> bit 5 of A5+0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Du / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Du % 8u))));
    }
    wr16(m, obj + 0x06u, 0x0050u);                 /* 0x1264E8; Z clear */
    wr32(m, obj + 0x00u, 0x001264F8u);             /* 0x1264F0 */
    return 0;
}
