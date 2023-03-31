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

#include "uint128_win.h"
#include "int128_win.h"
#include <stdio.h>
#include <stdlib.h>

#define int128_assert(x) if (!(x)) {\
  printf("ASSERTION FAILED: line: %d", __LINE__);\
  exit(1);\
}

const uint128_win zero = {.low = 0, .high = 0};
const uint128_win one = {.low = 1, .high = 0};
const uint128_win two = {.low = 2, .high = 0};
const uint128_win four = {.low = 4, .high = 0};
const uint128_win low_max = {.low = UINT64_MAX, .high = 0};
const uint128_win high_one = {.low = 0, .high = 1};
const uint128_win high_max = {.low = 0, .high = UINT64_MAX};
const uint128_win max = {.low = UINT64_MAX, .high = UINT64_MAX};

static const char digits[16] = "0123456789abcdef";

static void byte_to_binary(uint8_t byte, char* dest) {
  for (size_t i = 0; i < 8; i++) {
    uint8_t shifted = byte >> (8 - i - 1);
    uint8_t bit = shifted & 0x1;
    unsigned char ch = digits[bit];
    dest[i] = ch;
  }
}

static void byte_to_hex(uint8_t byte, char* dest) {
  uint8_t idx1 = (byte >> 4) & 0xf;
  char ch1 = digits[idx1];
  dest[0] = ch1;
  uint8_t idx2 = byte & 0xf;
  char ch2 = digits[idx2];
  dest[1] = ch2;
}

static void print_uint64_debug(FILE* out, uint64_t value, int num_start) {
  char buf_hex[3];
  memset(buf_hex, '\0', sizeof(buf_hex));
  char buf_binary[9];
  memset(buf_binary, '\0', sizeof(buf_binary));
  for (size_t i = 0; i < 8; i++) {
    uint64_t shifted = value >> (((8 - i - 1) * 8));
    uint8_t byte = (uint8_t) shifted & 0xff;
    byte_to_binary(byte, buf_binary);
    byte_to_hex(byte, buf_hex);
    fprintf(out, "%2zu: ", num_start + i);
    fputs(buf_binary, out);
    fputs(" 0x", out);
    fputs(buf_hex, out);
    fputc('\n', out);
  }
}

static void uint64_to_hex(uint64_t value, char* hex_dest) {
  for (size_t i = 0; i < 8; i++) {
    uint64_t shifted = value >> (((8 - i - 1) * 8));
    uint8_t byte = (uint8_t) shifted & 0xff;
    byte_to_hex(byte, hex_dest + (i * 2));
  }
}

void uint128_win_print_debug(uint128_win value, FILE* file) {
  char buf[INT128_WIN_HEX_STR_SIZE];
  uint128_win_to_hex(value, buf);
  fprintf(file, "%s\n", buf);
  print_uint64_debug(file, value.high, 0);
  print_uint64_debug(file, value.low, 8);
}

int uint128_win_from_hex(const char* hex_src, uint128_win* value_out) {
  if (NULL == hex_src || NULL == value_out) {
    return -1;
  }
  if (!('0' == hex_src[0] && 'x' == hex_src[1] &&
      '\0' == hex_src[INT128_WIN_HEX_STR_SIZE - 1])) {
    return 1;
  }
  for (size_t i = 2; i < INT128_WIN_HEX_STR_SIZE - 1; i++) {
    bool found = false;
    for (size_t j = 0; j < sizeof(digits); j++) {
      if (digits[j] == hex_src[i]) {
        found = true;
        break;
      }
    }
    if (!found) {
      return 1;
    }
  }
  uint64_t high = 0;
  for (size_t i = 2; i < 18; i++) {
    for (size_t j = 0; j < sizeof(digits); j++) {
      if (digits[j] == hex_src[i]) {
        high = high << 4;
        high = high | j;
        break;
      }
    }
  }
  uint64_t low = 0;
  for (size_t i = 18; i < INT128_WIN_HEX_STR_SIZE - 1; i++) {
    for (size_t j = 0; j < sizeof(digits); j++) {
      if (digits[j] == hex_src[i]) {
        low = low << 4;
        low = low | j;
        break;
      }
    }
  }
  uint128_win res = { .high = high, .low = low };
  *value_out = res;
  return 0;
}

int128_win int128_win_create(uint128_win value) {
  uint64_t low = value.low;
  int64_t high = (int64_t) value.high;
  return (int128_win) { .low = low, .high = high };
}

int128_win int128_win_create_negative(uint128_win value) {
  uint128_win negative = uint128_win_negate(value);
  uint64_t low = negative.low;
  int64_t high = (int64_t) negative.high;
  return (int128_win) { .low = low, .high = high };
}

void test_from_hex() {
  uint128_win parsed = { .low = 0, .high = 0 };

  int err_null_1 = uint128_win_from_hex(NULL, NULL);
  int128_assert(err_null_1);
  int err_null_2 = uint128_win_from_hex(NULL, &parsed);
  int128_assert(err_null_2);
  int err_null_3 = uint128_win_from_hex("foo", NULL);
  int128_assert(err_null_3);
  int err_format = uint128_win_from_hex("ff", &parsed);
  int128_assert(err_format);

  int err_zero = uint128_win_from_hex("0x00000000000000000000000000000000", &parsed);
  int128_assert(!err_zero);
  int128_assert(0 == uint128_win_compare((uint128_win) {.low = 0, .high = 0}, parsed));

  int err_one = uint128_win_from_hex("0x00000000000000000000000000000001", &parsed);
  int128_assert(!err_one);
  int128_assert(0 == uint128_win_compare((uint128_win) {.low = 1, .high = 0}, parsed));

  int err_max = uint128_win_from_hex("0xffffffffffffffffffffffffffffffff", &parsed);
  int128_assert(!err_max);
  int128_assert(0 == uint128_win_compare((uint128_win) {.low = UINT64_MAX, .high = UINT64_MAX}, parsed));
}

void test_to_hex() {
  char buf[INT128_WIN_HEX_STR_SIZE];
  uint128_win parsed = {.low = 0, .high = 0};

  int err_null = uint128_win_to_hex(zero, NULL);
  int128_assert(err_null);

  int err_zero = uint128_win_to_hex(zero, buf);
  int128_assert(!err_zero);
  int err_zero_parse = uint128_win_from_hex(buf, &parsed);
  int128_assert(!err_zero_parse);
  int128_assert(0 == uint128_win_compare(zero, parsed));

  int err_one = uint128_win_to_hex(one, buf);
  int128_assert(!err_one);
  int err_one_parse = uint128_win_from_hex(buf, &parsed);
  int128_assert(!err_one_parse);
  int128_assert(0 == uint128_win_compare(one, parsed));

  int err_max = uint128_win_to_hex(max, buf);
  int128_assert(!err_max);
  int err_max_parse = uint128_win_from_hex(buf, &parsed);
  int128_assert(!err_max_parse);
  int128_assert(0 == uint128_win_compare(max, parsed));
}

void test_compare() {
  int128_assert(0 == uint128_win_compare(zero, zero));
  int128_assert(0 == uint128_win_compare(low_max, low_max));
  int128_assert(0 == uint128_win_compare(max, max));
  int128_assert(-1 == uint128_win_compare(zero, one));
  int128_assert(1 == uint128_win_compare(one, zero));
  int128_assert(-1 == uint128_win_compare(one, low_max));
  int128_assert(1 == uint128_win_compare(low_max, one));
  int128_assert(-1 == uint128_win_compare(low_max, high_one));
  int128_assert(1 == uint128_win_compare(high_one, low_max));
  int128_assert(-1 == uint128_win_compare(high_one, max));
  int128_assert(1 == uint128_win_compare(max, high_one));
}

void test_add() {
  int128_assert(0 == uint128_win_compare(uint128_win_add(zero, zero), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_add(zero, one), one));
  int128_assert(0 == uint128_win_compare(uint128_win_add(one, zero), one));
  int128_assert(0 == uint128_win_compare(uint128_win_add(one, low_max), high_one));
  int128_assert(0 == uint128_win_compare(uint128_win_add(max, one), zero));
}

void test_subtract() {
  int128_assert(0 == uint128_win_compare(uint128_win_subtract(zero, zero), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_subtract(one, zero), one));
  int128_assert(0 == uint128_win_compare(uint128_win_subtract(one, one), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_subtract(high_one, one), low_max));
  int128_assert(0 == uint128_win_compare(uint128_win_subtract(high_one, low_max), one));
  int128_assert(0 == uint128_win_compare(uint128_win_subtract(zero, one), max));
}

void test_multiply() {
  const uint128_win big1 = {.low = 2, .high = 3};
  const uint128_win big2 = {.low = 5, .high = 7};
  const uint128_win big_res = {.low = 10, .high = 29};
  const uint128_win max_res = {.low = UINT64_MAX - 1, .high = UINT64_MAX};

  int128_assert(0 == uint128_win_compare(uint128_win_multiply(zero, zero), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(one, zero), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(zero, one), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(one, one), one));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(two, one), two));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(two, two), four));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(big1, big2), big_res));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(max, zero), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(max, one), max));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(max, two), max_res));
}

void test_divide() {
  const uint128_win big1 = {.low = 2, .high = 3};
  const uint128_win big2 = {.low = 9, .high = 0};
  const uint128_win big_dividend = {.low = 18, .high = 27};
  const uint128_win rem_dividend = {.low = 20, .high = 27};

  int128_assert(0 == uint128_win_compare(uint128_win_divide(zero, one, NULL), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_divide(zero, two, NULL), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_divide(two, one, NULL), two));
  int128_assert(0 == uint128_win_compare(uint128_win_divide(two, two, NULL), one));
  int128_assert(0 == uint128_win_compare(uint128_win_divide(four, two, NULL), two));
  int128_assert(0 == uint128_win_compare(uint128_win_multiply(big1, big2), big_dividend));
  int128_assert(0 == uint128_win_compare(uint128_win_divide(big_dividend, big1, NULL), big2));
  int128_assert(0 == uint128_win_compare(uint128_win_divide(big_dividend, big2, NULL), big1));
  int128_assert(0 == uint128_win_compare(uint128_win_divide(max, one, NULL), max));
  uint128_win remainder = zero;
  uint128_win result = uint128_win_divide(rem_dividend, big2, &remainder);
  int128_assert(0 == uint128_win_compare(result, big1));
  int128_assert(0 == uint128_win_compare(remainder, two));
}

void test_shift_left() {
  const uint128_win one_127_res = {.low = 0, .high = 9223372036854775808};

  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(zero, 0), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(zero, 1), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(zero, 127), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(one, 0), one));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(one, 1), two));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(one, 2), four));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(one, 64), high_one));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(one, 127), one_127_res));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(max, 64), high_max));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_left(max, 127), one_127_res));
}

void test_shift_right() {
  const uint128_win one_127_src = {.low = 0, .high = 9223372036854775808};

  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(zero, 0), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(zero, 1), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(zero, 127), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(one, 0), one));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(one, 1), zero));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(two, 1), one));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(four, 1), two));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(high_one, 64), one));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(one_127_src, 127), one));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(max, 64), low_max));
  int128_assert(0 == uint128_win_compare(uint128_win_shift_right(max, 127), one));
}

void test_compare_signed() {
  int128_win signed_zero = int128_win_create(zero);
  int128_win signed_one = int128_win_create(one);
  int128_win minus_one = int128_win_create_negative(one);
  int128_win signed_low_max = int128_win_create(low_max);
  int128_win signed_high_one = int128_win_create(high_one);
  int128_win minus_low_max = int128_win_create_negative(low_max);
  int128_win minus_high_one = int128_win_create_negative(high_one);

  int128_assert(0 == int128_win_compare(signed_zero, signed_zero));
  int128_assert(1 == int128_win_compare(signed_one, signed_zero));
  int128_assert(-1 == int128_win_compare(minus_one, signed_zero));
  int128_assert(1 == int128_win_compare(signed_high_one, signed_low_max));
  int128_assert(-1 == int128_win_compare(minus_high_one, minus_low_max));
}

void test_add_signed() {
  int128_win signed_zero = int128_win_create(zero);
  int128_win signed_one = int128_win_create(one);
  int128_win minus_one = int128_win_create_negative(one);
  int128_win signed_low_max = int128_win_create(low_max);
  int128_win signed_high_one = int128_win_create(high_one);
  int128_win minus_low_max = int128_win_create_negative(low_max);
  int128_win minus_high_one = int128_win_create_negative(high_one);

  int128_assert(0 == int128_win_compare(int128_win_add(signed_zero, signed_zero), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_zero, signed_one), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_one, signed_zero), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_one, minus_one), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_zero, minus_one), minus_one));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_one, signed_low_max), signed_high_one));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_high_one, minus_one), signed_low_max));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_high_one, minus_low_max), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_add(signed_low_max, minus_high_one), minus_one));
}

void test_subtract_signed() {
  int128_win signed_zero = int128_win_create(zero);
  int128_win signed_one = int128_win_create(one);
  int128_win minus_one = int128_win_create_negative(one);
  int128_win signed_two = int128_win_create(two);
  int128_win minus_two = int128_win_create_negative(two);
  int128_win signed_low_max = int128_win_create(low_max);
  int128_win signed_high_one = int128_win_create(high_one);
  int128_win minus_low_max = int128_win_create_negative(low_max);
  int128_win minus_high_one = int128_win_create_negative(high_one);

  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_zero, signed_zero), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_zero, signed_one), minus_one));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_one, signed_zero), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_one, minus_one), signed_two));
  int128_assert(0 == int128_win_compare(int128_win_subtract(minus_one, signed_one), minus_two));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_zero, minus_one), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_high_one, signed_one), signed_low_max));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_low_max, minus_one), signed_high_one));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_high_one, signed_low_max), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_subtract(signed_low_max, signed_high_one), minus_one));
  int128_assert(0 == int128_win_compare(int128_win_subtract(minus_low_max, signed_one), minus_high_one));
}

void test_multiply_signed() {
  int128_win signed_zero = int128_win_create(zero);
  int128_win signed_one = int128_win_create(one);
  int128_win minus_one = int128_win_create_negative(one);
  int128_win signed_two = int128_win_create(two);
  int128_win minus_two = int128_win_create_negative(two);
  int128_win signed_four = int128_win_create(four);
  int128_win minus_four = int128_win_create_negative(four);
  const uint128_win big1 = {.low = 2, .high = 3};
  int128_win signed_big1 = int128_win_create(big1);
  int128_win minus_big1 = int128_win_create_negative(big1);
  const uint128_win big2 = {.low = 5, .high = 7};
  int128_win signed_big2 = int128_win_create(big2);
  int128_win minus_big2 = int128_win_create_negative(big2);
  const uint128_win big_res = {.low = 10, .high = 29};
  int128_win signed_big_res = int128_win_create(big_res);
  int128_win minus_big_res = int128_win_create_negative(big_res);

  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_zero, signed_zero), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_one, signed_zero), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_multiply(minus_one, signed_zero), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_zero, signed_one), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_one, signed_one), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_one, minus_one), minus_one));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_two, signed_one), signed_two));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_two, minus_one), minus_two));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_two, signed_two), signed_four));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_two, minus_two), minus_four));
  int128_assert(0 == int128_win_compare(int128_win_multiply(minus_two, minus_two), signed_four));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_big1, signed_big2), signed_big_res));
  int128_assert(0 == int128_win_compare(int128_win_multiply(signed_big1, minus_big2), minus_big_res));
  int128_assert(0 == int128_win_compare(int128_win_multiply(minus_big1, signed_big2), minus_big_res));
  int128_assert(0 == int128_win_compare(int128_win_multiply(minus_big1, minus_big2), signed_big_res));
}

void test_divide_signed() {
  int128_win signed_zero = int128_win_create(zero);
  int128_win signed_one = int128_win_create(one);
  int128_win minus_one = int128_win_create_negative(one);
  int128_win signed_two = int128_win_create(two);
  int128_win minus_two = int128_win_create_negative(two);
  int128_win signed_four = int128_win_create(four);
  const uint128_win big1 = {.low = 2, .high = 3};
  int128_win signed_big1 = int128_win_create(big1);
  int128_win minus_big1 = int128_win_create_negative(big1);
  const uint128_win big2 = {.low = 9, .high = 0};
  int128_win signed_big2 = int128_win_create(big2);
  int128_win minus_big2 = int128_win_create_negative(big2);
  const uint128_win big_dividend = {.low = 18, .high = 27};
  int128_win signed_big_dividend = int128_win_create(big_dividend);
  int128_win minus_big_dividend = int128_win_create_negative(big_dividend);
  const uint128_win rem_dividend = {.low = 20, .high = 27};
  int128_win signed_rem_dividend = int128_win_create(rem_dividend);
  int128_win minus_rem_dividend = int128_win_create_negative(rem_dividend);

  int128_assert(0 == int128_win_compare(int128_win_divide(signed_zero, signed_one, NULL), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_zero, minus_one, NULL), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_zero, signed_two, NULL), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_zero, minus_two, NULL), signed_zero));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_two, signed_one, NULL), signed_two));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_two, minus_one, NULL), minus_two));
  int128_assert(0 == int128_win_compare(int128_win_divide(minus_two, signed_one, NULL), minus_two));
  int128_assert(0 == int128_win_compare(int128_win_divide(minus_two, minus_one, NULL), signed_two));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_two, signed_two, NULL), signed_one));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_two, minus_two, NULL), minus_one));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_four, minus_two, NULL), minus_two));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_big_dividend, signed_big1, NULL), signed_big2));
  int128_assert(0 == int128_win_compare(int128_win_divide(minus_big_dividend, signed_big2, NULL), minus_big1));
  int128_assert(0 == int128_win_compare(int128_win_divide(signed_big_dividend, minus_big2, NULL), minus_big1));
  int128_win signed_remainder = signed_zero;
  int128_win signed_result = int128_win_divide(signed_rem_dividend, signed_big2, &signed_remainder);
  int128_assert(0 == int128_win_compare(signed_result, signed_big1));
  int128_assert(0 == int128_win_compare(signed_remainder, signed_two));
  int128_win minus_remainder = signed_zero;
  int128_win minus_result = int128_win_divide(minus_rem_dividend, signed_big2, &minus_remainder);
  int128_assert(0 == int128_win_compare(minus_result, minus_big1));
  int128_assert(0 == int128_win_compare(minus_remainder, minus_two));
}

int main() {

  test_from_hex();
  test_to_hex();
  test_compare();
  test_add();
  test_subtract();
  test_multiply();
  test_divide();
  test_shift_left();
  test_shift_right();

  test_compare_signed();
  test_add_signed();
  test_subtract_signed();
  test_multiply_signed();
  test_divide_signed();

  return 0;
}