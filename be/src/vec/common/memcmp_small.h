// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
// This file is copied from
// https://github.com/ClickHouse/ClickHouse/blob/master/src/Common/MemcmpSmall.h
// and modified by Doris

#pragma once

#include <algorithm>
#include <cstdint>

namespace doris::vectorized::detail {
#include "common/compile_check_begin.h"
inline int cmp(uint8_t a, uint8_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

inline int cmp(size_t a, size_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}
} // namespace doris::vectorized::detail

/// We can process uninitialized memory in the functions below.
/// Results don't depend on the values inside uninitialized memory but Memory Sanitizer cannot see it.
/// Disable optimized functions if compile with Memory Sanitizer.

#if (defined(__SSE2__) && !defined(__aarch64__)) && !defined(MEMORY_SANITIZER)
#include "util/sse_util.hpp"

/** All functions works under the following assumptions:
  * - it's possible to read up to 15 excessive bytes after end of 'a' and 'b' region;
  * - memory regions are relatively small and extra loop unrolling is not worth to do.
  */

inline int memcmp_small_allow_overflow15(const uint8_t* a, size_t a_size, const uint8_t* b,
                                         size_t b_size) {
    size_t min_size = std::min(a_size, b_size);

    for (size_t offset = 0; offset < min_size; offset += 16) {
        uint16_t mask = static_cast<uint16_t>(_mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset)))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);

            if (offset >= min_size) break;

            return doris::vectorized::detail::cmp(a[offset], b[offset]);
        }
    }

    return doris::vectorized::detail::cmp(a_size, b_size);
}

/** Variant when memory regions have same size.
  * TODO Check if the compiler can optimize previous function when the caller pass identical sizes.
  */
inline int memcmp_small_allow_overflow15(const uint8_t* a, const uint8_t* b, size_t size) {
    for (size_t offset = 0; offset < size; offset += 16) {
        uint16_t mask = static_cast<uint16_t>(_mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset)))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);

            if (offset >= size) return 0;

            return doris::vectorized::detail::cmp(a[offset], b[offset]);
        }
    }

    return 0;
}

/** Compare memory regions for equality.
  */
inline bool memequal_small_allow_overflow15(const uint8_t* a, size_t a_size, const uint8_t* b,
                                            size_t b_size) {
    if (a_size != b_size) return false;

    for (size_t offset = 0; offset < a_size; offset += 16) {
        uint16_t mask = static_cast<uint16_t>(_mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset)))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);
            return offset >= a_size;
        }
    }

    return true;
}

/** Variant when the caller know in advance that the size is a multiple of 16.
  */
inline int memcmp_small_multiple_of16(const uint8_t* a, const uint8_t* b, size_t size) {
    for (size_t offset = 0; offset < size; offset += 16) {
        uint16_t mask = static_cast<uint16_t>(_mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset)))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);
            return doris::vectorized::detail::cmp(a[offset], b[offset]);
        }
    }

    return 0;
}

/** Variant when the size is 16 exactly.
  */
inline int memcmp16(const uint8_t* a, const uint8_t* b) {
    uint16_t mask = static_cast<uint16_t>(_mm_movemask_epi8(
            _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a)),
                           _mm_loadu_si128(reinterpret_cast<const __m128i*>(b)))));
    mask = ~mask;

    if (mask) {
        auto offset = __builtin_ctz(mask);
        return doris::vectorized::detail::cmp(a[offset], b[offset]);
    }

    return 0;
}

/** Variant when the size is 16 exactly.
  */
inline bool memequal16(const void* a, const void* b) {
    return 0xFFFF ==
           _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a)),
                                            _mm_loadu_si128(reinterpret_cast<const __m128i*>(b))));
}

/** Compare memory region to zero */
inline bool memory_is_zero_small_allow_overflow15(const void* data, size_t size) {
    const __m128i zero16 = _mm_setzero_si128();

    for (size_t offset = 0; offset < size; offset += 16) {
        uint16_t mask = static_cast<uint16_t>(_mm_movemask_epi8(
                _mm_cmpeq_epi8(zero16, _mm_loadu_si128(reinterpret_cast<const __m128i*>(
                                               reinterpret_cast<const char*>(data) + offset)))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);
            return offset >= size;
        }
    }

    return true;
}

#else

#include <cstring>

inline int memcmp_small_allow_overflow15(const uint8_t* a, size_t a_size, const uint8_t* b,
                                         size_t b_size) {
    if (auto res = memcmp(a, b, std::min(a_size, b_size)))
        return res;
    else
        return doris::vectorized::detail::cmp(a_size, b_size);
}

inline int memcmp_small_allow_overflow15(const uint8_t* a, const uint8_t* b, size_t size) {
    return memcmp(a, b, size);
}

inline bool memequal_small_allow_overflow15(const uint8_t* a, size_t a_size, const uint8_t* b,
                                            size_t b_size) {
    return a_size == b_size && 0 == memcmp(a, b, a_size);
}

inline int memcmp_small_multiple_of16(const uint8_t* a, const uint8_t* b, size_t size) {
    return memcmp(a, b, size);
}

inline int memcmp16(const uint8_t* a, const uint8_t* b) {
    return memcmp(a, b, 16);
}

inline bool memequal16(const void* a, const void* b) {
    return 0 == memcmp(a, b, 16);
}

inline bool memory_is_zero_small_allow_overflow15(const void* data, size_t size) {
    const char* pos = reinterpret_cast<const char*>(data);
    const char* end = pos + size;

    for (; pos < end; ++pos)
        if (*pos) return false;

    return true;
}

#endif

#include "common/compile_check_end.h"
