// http://www.stuartcheshire.org/papers/COBSforToN.pdf

#include "cobs.h"

uint32_t cobs_encode(const uint8_t *src_ptr, uint32_t src_len, uint8_t *dst_buf_ptr, uint32_t dst_buf_len)
{
    uint8_t code = 1;
    uint32_t code_i = 0;
    uint32_t dst_i = 1; // First byte in the encoded stream is always a code.

    if ((src_ptr == 0) || (dst_buf_ptr == 0))
    {
        while (1) // Null pointer.
        {
        }
    }
    else
    {
        for (uint32_t src_i = 0; src_i < src_len; src_i++)
        {
            // Note that code_i is always less than dst_i.
            if (dst_i >= dst_buf_len)
            {
                while (1) // Output buffer overflow.
                {
                }
            }

            if (src_ptr[src_i] == 0)
            {
                dst_buf_ptr[code_i] = code;
                code_i = dst_i++;
                code = 1;
            }
            else
            {
                dst_buf_ptr[dst_i++] = src_ptr[src_i];
                code++;
                if (code == 0xFF)
                {
                    // Long string of non-zero bytes (>253), insert next code.
                    dst_buf_ptr[code_i] = code;
                    code_i = dst_i++;
                    code = 1;
                }
            }
        }
        dst_buf_ptr[code_i] = code;
    }

    return dst_i;
}

struct cobs_result cobs_decode(const uint8_t *src_ptr, uint32_t src_len, uint8_t *dst_buf_ptr, uint32_t dst_buf_len)
{
    uint8_t long_flag = 0;
    uint32_t code_i = 0;
    uint32_t dst_i = 0;
    struct cobs_result result = {0, COBS_OK};

    if ((src_ptr == 0) || (dst_buf_ptr == 0))
    {
        while (1) // Null pointer.
        {
        }
    }

    for (uint32_t src_i = 0; src_i < src_len; src_i++)
    {
        if (dst_i >= dst_buf_len)
        {
            while (1) // Output buffer overflow.
            {
            }
        }

        if (src_ptr[src_i] == 0)
        {
            // Input buffer contains a 0 for some reason.
            result.status = COBS_SRC_ZERO;
            return result;
        }

        if (src_i == code_i)
        {
            if ((src_i != 0) && (long_flag == 0))
            {
                dst_buf_ptr[dst_i++] = 0;
            }

            code_i = src_i + src_ptr[src_i];
            long_flag = (src_ptr[src_i] == 0xFF) ? 1 : 0;

            // The code byte should only ever point to at most one byte past
            // the end of the input buffer for the last segment.
            if (code_i > src_len)
            {
                // Input buffer too small for some reason.
                result.status = COBS_SRC_UNDERFLOW;
                return result;
            }
        }
        else
        {
            dst_buf_ptr[dst_i++] = src_ptr[src_i];
        }
    }

    result.len = dst_i;
    return result;
}
