/*
 * Copyright 2023, alex at staticlibs.net
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
#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#undef NDEBUG
#endif // NDEBUG

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

void test_from_hex() {
  uint128_win parsed = { .low = 0, .high = 0 };

  int err_null_1 = uint128_win_from_hex(NULL, NULL);
  assert(err_null_1);
  int err_null_2 = uint128_win_from_hex(NULL, &parsed);
  assert(err_null_2);
  int err_null_3 = uint128_win_from_hex("foo", NULL);
  assert(err_null_3);
  int err_format = uint128_win_from_hex("ff", &parsed);
  assert(err_format);

  int err_zero = uint128_win_from_hex("0x00000000000000000000000000000000", &parsed);
  assert(!err_zero);
  assert(0 == uint128_win_compare((uint128_win) {.low = 0, .high = 0}, parsed));

  int err_one = uint128_win_from_hex("0x00000000000000000000000000000001", &parsed);
  assert(!err_one);
  assert(0 == uint128_win_compare((uint128_win) {.low = 1, .high = 0}, parsed));

  int err_max = uint128_win_from_hex("0xffffffffffffffffffffffffffffffff", &parsed);
  assert(!err_max);
  assert(0 == uint128_win_compare((uint128_win) {.low = UINT64_MAX, .high = UINT64_MAX}, parsed));
}

void test_to_hex() {
  char buf[INT128_WIN_HEX_STR_SIZE];
  uint128_win parsed = {.low = 0, .high = 0};

  int err_null = uint128_win_to_hex(zero, NULL);
  assert(err_null);

  int err_zero = uint128_win_to_hex(zero, buf);
  assert(!err_zero);
  int err_zero_parse = uint128_win_from_hex(buf, &parsed);
  assert(!err_zero_parse);
  assert(0 == uint128_win_compare(zero, parsed));

  int err_one = uint128_win_to_hex(one, buf);
  assert(!err_one);
  int err_one_parse = uint128_win_from_hex(buf, &parsed);
  assert(!err_one_parse);
  assert(0 == uint128_win_compare(one, parsed));

  int err_max = uint128_win_to_hex(max, buf);
  assert(!err_max);
  int err_max_parse = uint128_win_from_hex(buf, &parsed);
  assert(!err_max_parse);
  assert(0 == uint128_win_compare(max, parsed));
}

void test_compare() {
  assert(0 == uint128_win_compare(zero, zero));
  assert(0 == uint128_win_compare(low_max, low_max));
  assert(0 == uint128_win_compare(max, max));
  assert(-1 == uint128_win_compare(zero, one));
  assert(1 == uint128_win_compare(one, zero));
  assert(-1 == uint128_win_compare(one, low_max));
  assert(1 == uint128_win_compare(low_max, one));
  assert(-1 == uint128_win_compare(low_max, high_one));
  assert(1 == uint128_win_compare(high_one, low_max));
  assert(-1 == uint128_win_compare(high_one, max));
  assert(1 == uint128_win_compare(max, high_one));
}

void test_add() {
  assert(0 == uint128_win_compare(uint128_win_add(zero, zero), zero));
  assert(0 == uint128_win_compare(uint128_win_add(zero, one), one));
  assert(0 == uint128_win_compare(uint128_win_add(one, zero), one));
  assert(0 == uint128_win_compare(uint128_win_add(one, low_max), high_one));
  assert(0 == uint128_win_compare(uint128_win_add(max, one), zero));
}

void test_subtract() {
  assert(0 == uint128_win_compare(uint128_win_subtract(zero, zero), zero));
  assert(0 == uint128_win_compare(uint128_win_subtract(one, zero), one));
  assert(0 == uint128_win_compare(uint128_win_subtract(one, one), zero));
  assert(0 == uint128_win_compare(uint128_win_subtract(high_one, one), low_max));
  assert(0 == uint128_win_compare(uint128_win_subtract(high_one, low_max), one));
  assert(0 == uint128_win_compare(uint128_win_subtract(zero, one), max));
}

void test_multiply() {
  const uint128_win big1 = {.low = 2, .high = 3};
  const uint128_win big2 = {.low = 5, .high = 7};
  const uint128_win big_res = {.low = 10, .high = 29};
  const uint128_win max_res = {.low = UINT64_MAX - 1, .high = UINT64_MAX};

  assert(0 == uint128_win_compare(uint128_win_multiply(zero, zero), zero));
  assert(0 == uint128_win_compare(uint128_win_multiply(one, zero), zero));
  assert(0 == uint128_win_compare(uint128_win_multiply(zero, one), zero));
  assert(0 == uint128_win_compare(uint128_win_multiply(one, one), one));
  assert(0 == uint128_win_compare(uint128_win_multiply(two, one), two));
  assert(0 == uint128_win_compare(uint128_win_multiply(two, two), four));
  assert(0 == uint128_win_compare(uint128_win_multiply(big1, big2), big_res));
  assert(0 == uint128_win_compare(uint128_win_multiply(max, zero), zero));
  assert(0 == uint128_win_compare(uint128_win_multiply(max, one), max));
  assert(0 == uint128_win_compare(uint128_win_multiply(max, two), max_res));
}

void test_divide() {
  const uint128_win big1 = {.low = 2, .high = 3};
  const uint128_win big2 = {.low = 9, .high = 0};
  const uint128_win big_dividend = {.low = 18, .high = 27};
  const uint128_win rem_dividend = {.low = 20, .high = 27};
  const uint128_win max_res = {.low = UINT64_MAX - 1, .high = UINT64_MAX};

  assert(0 == uint128_win_compare(uint128_win_divide(zero, one, NULL), zero));
  assert(0 == uint128_win_compare(uint128_win_divide(zero, two, NULL), zero));
  assert(0 == uint128_win_compare(uint128_win_divide(two, one, NULL), two));
  assert(0 == uint128_win_compare(uint128_win_divide(two, two, NULL), one));
  assert(0 == uint128_win_compare(uint128_win_divide(four, two, NULL), two));
  assert(0 == uint128_win_compare(uint128_win_multiply(big1, big2), big_dividend));
  assert(0 == uint128_win_compare(uint128_win_divide(big_dividend, big1, NULL), big2));
  assert(0 == uint128_win_compare(uint128_win_divide(big_dividend, big2, NULL), big1));
  assert(0 == uint128_win_compare(uint128_win_multiply(max, zero), zero));
  assert(0 == uint128_win_compare(uint128_win_multiply(max, one), max));
  assert(0 == uint128_win_compare(uint128_win_multiply(max, two), max_res));
  uint128_win remainder = zero;
  uint128_win result = uint128_win_divide(rem_dividend, big2, &remainder);
  assert(0 == uint128_win_compare(result, big1));
  assert(0 == uint128_win_compare(remainder, two));
}

void test_shift_left() {
  const uint128_win one_127_res = {.low = 0, .high = 9223372036854775808};

  assert(0 == uint128_win_compare(uint128_win_shift_left(zero, 0), zero));
  assert(0 == uint128_win_compare(uint128_win_shift_left(zero, 1), zero));
  assert(0 == uint128_win_compare(uint128_win_shift_left(zero, 127), zero));
  assert(0 == uint128_win_compare(uint128_win_shift_left(one, 0), one));
  assert(0 == uint128_win_compare(uint128_win_shift_left(one, 1), two));
  assert(0 == uint128_win_compare(uint128_win_shift_left(one, 2), four));
  assert(0 == uint128_win_compare(uint128_win_shift_left(one, 64), high_one));
  assert(0 == uint128_win_compare(uint128_win_shift_left(one, 127), one_127_res));
  assert(0 == uint128_win_compare(uint128_win_shift_left(max, 64), high_max));
  assert(0 == uint128_win_compare(uint128_win_shift_left(max, 127), one_127_res));
}

void test_shift_right() {
  const uint128_win one_127_src = {.low = 0, .high = 9223372036854775808};

  assert(0 == uint128_win_compare(uint128_win_shift_right(zero, 0), zero));
  assert(0 == uint128_win_compare(uint128_win_shift_right(zero, 1), zero));
  assert(0 == uint128_win_compare(uint128_win_shift_right(zero, 127), zero));
  assert(0 == uint128_win_compare(uint128_win_shift_right(one, 0), one));
  assert(0 == uint128_win_compare(uint128_win_shift_right(one, 1), zero));
  assert(0 == uint128_win_compare(uint128_win_shift_right(two, 1), one));
  assert(0 == uint128_win_compare(uint128_win_shift_right(four, 1), two));
  assert(0 == uint128_win_compare(uint128_win_shift_right(high_one, 64), one));
  assert(0 == uint128_win_compare(uint128_win_shift_right(one_127_src, 127), one));
  assert(0 == uint128_win_compare(uint128_win_shift_right(max, 64), low_max));
  assert(0 == uint128_win_compare(uint128_win_shift_right(max, 127), one));
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

  return 0;
}