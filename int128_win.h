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

#ifndef INT128_WIN_INT128_WIN_H
#define INT128_WIN_INT128_WIN_H

#include "uint128_win.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct int128_win {
  uint64_t low;  
  int64_t high;  
} int128_win;

static inline int128_win int128_win_from_int64(int64_t value) {
  uint64_t low = (uint64_t) value;
  int64_t high = 0;
  if (value < 0) {
    high = ~((int64_t) 0);
  }
  return (int128_win) { .low = low, .high = high };
}

static inline int64_t int128_win_bitcast_to_signed(uint64_t value) {
  // Casting an unsigned integer to a signed integer of the same
  // width is implementation defined behavior if the source value would not fit
  // in the destination type. We step around it with a roundtrip bitwise not
  // operation to make sure this function remains constexpr. Clang, GCC, and
  // MSVC optimize this to a no-op on x86-64.
  if (value & (((uint64_t) 1) << 63)) {
    return ~((int64_t)(~value));
  } else {
    return (int64_t) value;
  }
}

static inline int int128_win_compare(int128_win left, int128_win right) {
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

static inline int128_win int128_win_add(int128_win left, int128_win right) {
  int64_t high = left.high + right.high;
  uint64_t low = left.low + right.low;
  if (low < left.low) {
    high += 1;
  }
  return (int128_win) { .low = low, .high = high };
}

static inline int128_win int128_win_subtract(int128_win left, int128_win right) {
  int64_t high = left.high - right.high;
  uint64_t low = left.low - right.low;
  if (low > left.low) {
    high -= 1;
  }
  return (int128_win) { .low = low, .high = high };
}

static inline int128_win int128_win_multiply(int128_win left, int128_win right) {
  uint128_win uleft = { .low = left.low, .high = (uint64_t) left.high };
  uint128_win uright = { .low = right.low, .high = (uint64_t) right.high };
  uint128_win ures = uint128_win_multiply(uleft, uright);
  uint64_t low = ures.low;
  int64_t high = int128_win_bitcast_to_signed(ures.high);
  return (int128_win) { .low = low, .high = high };
}

static inline uint128_win uint128_win_negate(uint128_win value) {
  uint64_t low = (~value.low) + 1;
  uint64_t high = (~value.high) + (unsigned long)(value.low == 0);
  return (uint128_win) { .low = low, .high = high };
}

static inline uint128_win int128_win_unsigned_absolute_value(int128_win value) {
  // Cast to uint128 before possibly negating because -Int128Min() is undefined.
  uint128_win positive = { .low = value.low, .high = (uint64_t) value.high };
  if (value.high < 0) {
    uint128_win negative = uint128_win_negate(positive);
    return negative;
  } else {
    return positive;
  } 
}

static inline int128_win int128_win_divide(int128_win dividend, int128_win divisor, int128_win* remainder) {
  uint128_win udividend = int128_win_unsigned_absolute_value(dividend);
  uint128_win udivisor = int128_win_unsigned_absolute_value(divisor);
  uint128_win uremainder_positive = { .low = 0, .high = 0 };
  uint128_win uquotient_positive = uint128_win_divide(udividend, udivisor, &uremainder_positive);
  uint128_win uquotient = uquotient_positive;
  if ((dividend.high < 0) != (divisor.high < 0)) {
    uquotient = uint128_win_negate(uquotient_positive);
  }
  uint128_win uremainder = uremainder_positive;
  if (dividend.high < 0) {
    uremainder = uint128_win_negate(uremainder_positive);
  }
  int64_t quotient_high = int128_win_bitcast_to_signed(uquotient.high);
  int64_t remainder_high = int128_win_bitcast_to_signed(uremainder.high);
  if (NULL != remainder) {
    *remainder = (int128_win) { .low = uremainder.low, .high = remainder_high };
  }
  return (int128_win) { .low = uquotient.low, .high = quotient_high };
}

#ifdef __cplusplus
}
#endif

#endif // INT128_WIN_INT128_WIN_H