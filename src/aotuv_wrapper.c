/*
 * aotuv_wrapper.c
 *
 * Wrapper mỏng quanh libvorbisenc (bản đã patch aoTuV beta6.03) để expose
 * một hàm encode đơn giản: nhận PCM 16-bit interleaved + config, trả về
 * buffer .ogg hoàn chỉnh. Dùng qua Module.ccall/cwrap từ JS.
 *
 * Đây KHÔNG phải hàng production polish — là bản tối thiểu để chứng minh
 * pipeline chạy được. Cần review kỹ trước khi dùng thật (xem NOTES.md).
 */

#include <stdlib.h>
#include <string.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <ogg/ogg.h>

/* Buffer động để gom output .ogg */
typedef struct {
    unsigned char *data;
    long size;
    long capacity;
} ByteBuffer;

static void bb_init(ByteBuffer *bb) {
    bb->capacity = 1 << 16;
    bb->data = (unsigned char *)malloc(bb->capacity);
    bb->size = 0;
}

static void bb_append(ByteBuffer *bb, const unsigned char *src, long len) {
    if (bb->size + len > bb->capacity) {
        while (bb->size + len > bb->capacity) bb->capacity *= 2;
        bb->data = (unsigned char *)realloc(bb->data, bb->capacity);
    }
    memcpy(bb->data + bb->size, src, len);
    bb->size += len;
}

/*
 * aotuv_encode_wav_to_ogg
 *
 * pcm_data:     con trỏ tới PCM 16-bit signed little-endian, interleaved
 * num_samples:  số sample PER CHANNEL (không phải tổng số int16)
 * channels:     1 hoặc 2
 * sample_rate:  vd 44100, 48000
 * quality:      -0.1 .. 1.0 (thang VBR của Vorbis, giống -q của oggenc)
 * out_len:      [out] độ dài buffer .ogg trả về
 *
 * return: con trỏ buffer .ogg (do hàm này malloc — JS phải gọi free() qua
 *         Module._free() sau khi copy dữ liệu ra, tránh leak memory trong wasm heap)
 */
unsigned char *aotuv_encode_wav_to_ogg(
    const short *pcm_data,
    int num_samples,
    int channels,
    long sample_rate,
    float quality,
    int *out_len
) {
    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;

    vorbis_info vi;
    vorbis_comment vc;
    vorbis_dsp_state vd;
    vorbis_block vb;

    ByteBuffer out;
    bb_init(&out);

    vorbis_info_init(&vi);

    /* vorbis_encode_init_vbr là API chuẩn của libvorbisenc, aoTuV không đổi
       chữ ký hàm này -- chỉ đổi phần lõi tâm lý âm học (psychoacoustic) bên trong. */
    if (vorbis_encode_init_vbr(&vi, channels, (long)sample_rate, quality) != 0) {
        vorbis_info_clear(&vi);
        free(out.data);
        *out_len = 0;
        return NULL;
    }

    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "aoTuV_wasm_wrapper");

    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    ogg_stream_init(&os, rand());

    /* Header packets */
    {
        ogg_packet header, header_comm, header_code;
        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header);
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        while (ogg_stream_flush(&os, &og)) {
            bb_append(&out, og.header, og.header_len);
            bb_append(&out, og.body, og.body_len);
        }
    }

    /* Feed PCM theo từng block, dùng vorbis_analysis_buffer để tránh copy thừa */
    {
        int frames_per_chunk = 1024;
        int pos = 0;
        int eos = 0;

        while (!eos) {
            int chunk = frames_per_chunk;
            if (pos + chunk > num_samples) chunk = num_samples - pos;

            if (chunk <= 0) {
                /* Báo hiệu hết dữ liệu */
                vorbis_analysis_wrote(&vd, 0);
            } else {
                float **buffer = vorbis_analysis_buffer(&vd, chunk);
                for (int i = 0; i < chunk; i++) {
                    for (int c = 0; c < channels; c++) {
                        short s = pcm_data[(pos + i) * channels + c];
                        buffer[c][i] = (float)s / 32768.0f;
                    }
                }
                vorbis_analysis_wrote(&vd, chunk);
                pos += chunk;
            }

            while (vorbis_analysis_blockout(&vd, &vb) == 1) {
                vorbis_analysis(&vb, NULL);
                vorbis_bitrate_addblock(&vb);

                while (vorbis_bitrate_flushpacket(&vd, &op)) {
                    ogg_stream_packetin(&os, &op);

                    while (!eos) {
                        int result = ogg_stream_pageout(&os, &og);
                        if (result == 0) break;
                        bb_append(&out, og.header, og.header_len);
                        bb_append(&out, og.body, og.body_len);
                        if (ogg_page_eos(&og)) eos = 1;
                    }
                }
            }

            if (chunk <= 0 && !eos) {
                /* Đã báo EOS ở vorbis_analysis_wrote(&vd,0) nhưng chưa thấy
                   page eos -- flush thêm 1 lần để chắc chắn thoát vòng lặp */
                while (ogg_stream_flush(&os, &og)) {
                    bb_append(&out, og.header, og.header_len);
                    bb_append(&out, og.body, og.body_len);
                }
                eos = 1;
            }
        }
    }

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    *out_len = (int)out.size;
    return out.data; /* JS chịu trách nhiệm free() */
}
