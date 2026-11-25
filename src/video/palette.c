#include "palette.h"
#include "../objects/obj_common.h"

/* ROM 0x001660: queue a palette upload {count.w, src.l, dst.w} at A5-0x6750 (max 0x20) */
void Pal_QueueUpload(const f3mem_t *m, uint32_t src, uint16_t dst, uint16_t count)
{
    int16_t idx = (int16_t)rd16(m, F3_A5 - 0x5DF0u);
    if (idx >= 0x20)
        return;
    uint32_t slot = (F3_A5 - 0x6750u) + (uint32_t)((uint16_t)idx << 3);
    wr16(m, slot + 0u, count);
    wr32(m, slot + 2u, src);
    wr16(m, slot + 6u, dst);
    wr16(m, F3_A5 - 0x5DEEu, 2u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(idx + 1));
}

/* ROM 0x001774: clear all 16 palette-ramp slots at A5-0x6650 and flag A5-0x5DE8 */
void Pal_ClearRamps(const f3mem_t *m)
{
    uint32_t a0 = F3_A5 - 0x6650u;
    for (int i = 0; i < 16; i++) {
        wr16(m, a0, 0u);
        a0 += 6u;
    }
    wr16(m, F3_A5 - 0x5DE8u, 0u);
}

/* ROM 0x16A258: blend 64 bytes from src1 (weight ~d0) and src2 (weight d0) into dst */
void Pal_Blend64(const f3mem_t *m, uint32_t dst, uint32_t src1, uint32_t src2, uint16_t weight)
{
    uint16_t d1 = weight;
    uint16_t d0 = (uint16_t)~weight;
    for (int i = 0x3F; i >= 0; i--) {
        uint32_t c1 = (uint32_t)rd8(m, src1 + (uint32_t)i) << 8;
        uint32_t c2 = (uint32_t)rd8(m, src2 + (uint32_t)i) << 8;
        uint32_t sum = c1 * (uint32_t)d0 + c2 * (uint32_t)d1;
        uint16_t q = (uint16_t)(sum / 0xFFFFu);
        wr8(m, dst + (uint32_t)i, (uint8_t)(q >> 8));
    }
}

static void pal_blend_upload_16(const f3mem_t *m, uint32_t src1_base, uint32_t src2,
                                uint32_t dst_wram, uint16_t w_base, uint16_t pal_dst,
                                int32_t w_step)
{
    for (uint32_t i = 0; i < 16u; i++) {
        uint32_t s1 = (w_step < 0) ? src1_base : (src1_base + (i & 1u) * 0x40u);
        uint32_t a0 = dst_wram + i * 0x40u;
        uint16_t w = (w_step < 0)
            ? (uint16_t)((int32_t)w_base + (int32_t)i * w_step)
            : (uint16_t)(w_base + (uint16_t)((i >> 1) * (uint32_t)w_step));
        Pal_Blend64(m, a0, s1, src2, w);
        Pal_QueueUpload(m, a0, (uint16_t)(pal_dst + i * 0x40u), 0x10u);
    }
}

/* ROM 0x150156 */
void Pal_UploadFade150156(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x00150F3Cu, 0x00150DBCu, 0x0040B7AEu, 0x3C00u, 0x3400u, 0x0780);
}

/* ROM 0x150578 */
void Pal_UploadFade150578(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x00150F3Cu, 0x00150DBCu, 0x0040B3AEu, 0x7800u, 0x3800u, 0x0780);
}

/* ROM 0x15099A */
void Pal_UploadFade15099A(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x00150F3Cu, 0x00150DBCu, 0x0040B7AEu, 0xB400u, 0x3C00u, 0x0780);
}

/* ROM 0x15458C */
void Pal_UploadFade15458C(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x000247C0u, 0x00155898u, 0x0040B3AEu, 0x0000u, 0x3000u, 0x0780);
}

/* ROM 0x1549AE */
void Pal_UploadFade1549AE(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x000247C0u, 0x00155898u, 0x0040B7AEu, 0x3C00u, 0x3400u, 0x0780);
}

/* ROM 0x154DD0 */
void Pal_UploadFade154DD0(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x000247C0u, 0x00155898u, 0x0040B3AEu, 0x7800u, 0x3800u, 0x0780);
}

/* ROM 0x1551F2 */
void Pal_UploadFade1551F2(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x000247C0u, 0x00155898u, 0x0040B7AEu, 0xB400u, 0x3C00u, 0x0780);
}

/* ROM 0x163D1A */
void Pal_UploadFade163D1A(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x00164B2Au, 0x00164B6Au, 0x0040B72Eu, 0xFFF0u, 0x3800u, -0x1110);
}

/* ROM 0x16413C */
void Pal_UploadFade16413C(const f3mem_t *m)
{
    pal_blend_upload_16(m, 0x00164BAAu, 0x00164BEAu, 0x0040B72Eu, 0xFFF0u, 0x3C00u, -0x1110);
}

/* ROM 0x001866 */
void Pal_FadeOut001866(const f3mem_t *m, uint32_t a1, uint32_t d0, uint32_t d1, uint32_t d2)
{
    (void)Pal_QueueRamp(m, 0x0000190Eu, a1, d0, d1, d2);
}

/* ROM 0x001890 */
void Pal_FadeIn001890(const f3mem_t *m, uint32_t a0, uint32_t d0, uint32_t d1, uint32_t d2)
{
    (void)Pal_QueueRamp(m, a0, 0x0000190Eu, d0, d1, d2);
}

/* ROM 0x146006: stage palette fade-in (0x11) / fade-out (0x51) */
void Stage_Fade146006(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd16(m, F3_A5 + 0x3384u) & 0x007Fu);
    if (d0 == 0x0011u) {
        Pal_FadeIn001890(m, 0x0002131Cu, 0x18DCu, 0x30u, 7u);
        Pal_FadeIn001890(m, 0x0001BCF4u, 0x77F4u, 0x30u, 3u);
        Pal_FadeIn001890(m, 0x0001BCF4u, 0x45F4u, 0x30u, 3u);
        Pal_FadeIn001890(m, 0x0001BCF4u, 0x40F4u, 0x30u, 3u);
        Pal_FadeIn001890(m, 0x0001BCF4u, 0x4174u, 0x30u, 3u);
    } else if (d0 == 0x0051u) {
        Pal_FadeOut001866(m, 0x0002131Cu, 0x18DCu, 0x30u, 7u);
        Pal_FadeOut001866(m, 0x0001BCF4u, 0x77F4u, 0x30u, 3u);
        Pal_FadeOut001866(m, 0x0001BCF4u, 0x45F4u, 0x30u, 3u);
        Pal_FadeOut001866(m, 0x0001BCF4u, 0x40F4u, 0x30u, 3u);
        Pal_FadeOut001866(m, 0x0001BCF4u, 0x4174u, 0x30u, 3u);
    }
}
