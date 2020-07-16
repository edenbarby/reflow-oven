#ifndef COBS_H
#define COBS_H

#include <stdint.h>

#define CODS_ENCODE_BUF_MAX(SRC_LEN) ((SRC_LEN) + (((SRC_LEN) + 253UL) / 254UL))
#define CODS_DECODE_BUF_MAX(SRC_LEN) (((SRC_LEN) == 0) ? (0UL) : ((SRC_LEN)-1UL))

enum cobs_status
{
    COBS_OK,
    COBS_SRC_ZERO,
    COBS_SRC_UNDERFLOW
};

struct cobs_result
{
    uint32_t len;
    enum cobs_status status;
};

uint32_t cobs_encode(const uint8_t *src_ptr, uint32_t src_len, uint8_t *dst_buf_ptr, uint32_t dst_buf_len);
struct cobs_result cobs_decode(const uint8_t *src_ptr, uint32_t src_len, uint8_t *dst_buf_ptr, uint32_t dst_buf_len);

#endif // COBS_H
