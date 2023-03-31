/*
 * Copyright 2023 alex@staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INT128_WIN_UINT128_WIN_H
#define INT128_WIN_UINT128_WIN_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <intrin.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INT128_WIN_HEX_STR_SIZE 35

typedef struct uint128_win {
  uint64_t low;  
  uint64_t high;  
} uint128_win;

static inline void uint128_win_byte_to_hex(uint8_t byte, char* dest) {
  const char digits[17] = "0123456789abcdef";
  uint8_t idx1 = (byte >> 4) & 0xf;
  char ch1 = digits[idx1];
  dest[0] = ch1;
  uint8_t idx2 = byte & 0xf;
  char ch2 = digits[idx2];
  dest[1] = ch2;
}

static inline void uint128_win_uint64_to_hex(uint64_t value, char* hex_dest) {
  for (size_t i = 0; i < 8; i++) {
    uint64_t shifted = value >> (((8 - i - 1) * 8));
    uint8_t byte = (uint8_t) shifted & 0xff;
    uint128_win_byte_to_hex(byte, hex_dest + (i * 2));
  }
}

static inline int uint128_win_to_hex(uint128_win value, char* hex_dest) {
  if (NULL == hex_dest) {
    return -1;
  }
  hex_dest[0] = '0';
  hex_dest[1] = 'x';
  uint128_win_uint64_to_hex(value.high, hex_dest + 2);
  uint128_win_uint64_to_hex(value.low, hex_dest + 2 + 16);
  hex_dest[INT128_WIN_HEX_STR_SIZE - 1] = '\0';
  return 0;
}

static inline int uint128_win_count_leading_zeros(uint64_t value) {
  unsigned long result = 0;
  if (_BitScanReverse64(&result, value)) {
    return 63 - result;
  }
  return 64;
}

static inline int uint128_win_last_set_bit_pos(uint128_win value) {
  if (value.high > 0) {
    return 127 - uint128_win_count_leading_zeros(value.high);
  }
  return 63 - uint128_win_count_leading_zeros(value.low);
}

static inline int uint128_win_compare(uint128_win left, uint128_win right) {
  if (left.high > right.high) {
    return 1;
  }
  if (left.high < right.high) {
    return -1;
  }
  if (left.low > right.low) {
    return 1;
  }
  if (left.low < right.low) {
    return - 1;
  }
  return 0;
}

static inline uint128_win uint128_win_add(uint128_win left, uint128_win right) {
  uint64_t high = left.high + right.high;
  uint64_t low = left.low + right.low;
  if (low < left.low) {
    high += 1;
  }
  return (uint128_win) { .low = low, .high = high };
}

static inline uint128_win uint128_win_subtract(uint128_win left, uint128_win right) {
  uint64_t high = left.high - right.high;
  uint64_t low = left.low - right.low;
  if (low > left.low) {
    high -= 1;
  }
  return (uint128_win) { .low = low, .high = high };
}

static inline uint128_win uint128_win_multiply(uint128_win left, uint128_win right) {
  uint64_t carry = 0;
  uint64_t low = _umul128(left.low, right.low, &carry);
  uint64_t high = (left.low * right.high) + (left.high * right.low) + carry;
  return (uint128_win) { .low = low, .high = high };
}

static inline uint128_win uint128_win_shift_left(uint128_win value, int amount) {
  if (amount >= 64) {
    uint64_t high = value.low << (amount - 64);
    return (uint128_win) {.low = 0, .high = high};
  } else if (0 == amount) {
    return value;
  } else {
    uint64_t high = (value.high << amount) | (value.low >> (64 - amount));
    uint64_t low = value.low << amount;
    return (uint128_win) {.low = low, .high = high};
  }
}

static inline uint128_win uint128_win_shift_right(uint128_win value, int amount) {
  if (amount >= 64) {
    uint64_t low = value.high >> (amount - 64);
    return (uint128_win) {.low = low, .high = 0};
  } else if (0 == amount) {
    return value;
  } else {
    uint64_t high = value.high >> amount;
    uint64_t low = (value.low >> amount) | (value.high << (64 - amount));
    return (uint128_win) {.low = low, .high = high};
  }
}

static inline uint128_win uint128_win_divide(uint128_win dividend, uint128_win divisor, uint128_win* remainder) {
  uint128_win zero = {.low = 0, .high = 0};

  if (0 == divisor.low && 0 == divisor.high) {
    if (NULL != remainder) {
      *remainder = zero;
    }
    return zero;
  }

  if (1 == uint128_win_compare(divisor, dividend)) {
    if (NULL != remainder) {
      *remainder = dividend;
    }
    return (uint128_win) {.low = 0, .high = 0};
  }

  if (0 == uint128_win_compare(divisor, dividend)) {
    if (NULL != remainder) {
      *remainder = (uint128_win) {.low = 0, .high = 0};
    }
    return (uint128_win) {.low = 1, .high = 0};
  }

  uint128_win denominator = divisor;
  uint128_win quotient = zero;

  // Left aligns the MSB of the denominator and the dividend.
  const int shift = uint128_win_last_set_bit_pos(dividend) - uint128_win_last_set_bit_pos(denominator);
  denominator = uint128_win_shift_left(denominator, shift);

  // Uses shift-subtract algorithm to divide dividend by denominator. The
  // remainder will be left in dividend.
  for (int i = 0; i <= shift; ++i) {
    quotient = uint128_win_shift_left(quotient, 1);
    if (uint128_win_compare(dividend, denominator) >= 0) {
      dividend = uint128_win_subtract(dividend, denominator);
      quotient.low = quotient.low | 1;
    }
    denominator = uint128_win_shift_right(denominator, 1);
  }

  if (NULL != remainder) {
    *remainder = dividend;
  }
  return quotient;
}


#ifdef __cplusplus
}
#endif

#endif // INT128_WIN_UINT128_WIN_H