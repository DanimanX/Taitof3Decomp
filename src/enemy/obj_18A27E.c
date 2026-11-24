/* ROM 0x18A27E: object timer that re-arms its delay of 8 then merges state. */
#include "obj_18A27E.h"

#define A5_B36   (F3_A5 + 0x4B36u)
#define A5_B28   (F3_A5 + 0x4B28u)
#define A5_B29   (F3_A5 + 0x4B29u)
#define A5_B26   (F3_A5 + 0x4B26u)
#define A5_B10   (F3_A5 + 0x4B10u)
#define A5_B0C   (F3_A5 + 0x4B0Cu)
#define A5_B18   (F3_A5 + 0x4B18u)
#define A5_B74   (F3_A5 + 0x4B74u)

/* ROM 0x189AF6 */
static void Obj_18A27E_Sub_189AF6(const f3mem_t *m)
{
    wr8(m, A5_B26, (uint8_t)(rd8(m, A5_B26) & (uint8_t)~0x10u));   /* bfclr bit 4 */
    if ((int32_t)rd32(m, A5_B10) > (int32_t)rd32(m, A5_B0C)) {
        wr32(m, A5_B10, rd32(m, A5_B10) - rd32(m, A5_B18));
        wr8(m, A5_B26, (uint8_t)(rd8(m, A5_B26) | 0x10u));         /* bfset bit 4 */
    }
}

/* ROM 0x189A8E */
static void Obj_18A27E_Sub_189A8E(const f3mem_t *m, uint32_t obj)
{
    wr8(m, A5_B36, 0);
    if (rd8(m, A5_B28) == rd8(m, obj + 0x1Du)) {
        wr8(m, obj + 0x1Du, rd8(m, A5_B29));
    } else if ((int8_t)rd8(m, obj + 0x35u) != 0) {
        wr8(m, obj + 0x1Du, rd8(m, A5_B28));
    } else {
        wr8(m, obj + 0x1Du, rd8(m, A5_B36));
    }
    wr8(m, obj + 0x35u, 0);
    wr8(m, A5_B36, rd8(m, obj + 0x1Du));
    Obj_18A27E_Sub_189AF6(m);
}

/* ROM 0x18A27E */
int Obj_18A27E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {               /* subq.w #1 */
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x18A27Eu);                                   /* re-arm self */
        return 0;
    }
    wr16(m, obj + 0x06u, 0);                                       /* subq.w #1 */
    Obj_18A27E_Sub_189A8E(m, obj);
    wr32(m, obj, 0x001899B0u);
    return 0;
}
