/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include <string.h>

#include "common/cs_time.h"
#include "common/cs_varint.h"
#include "common/mg_str.h"
#include "common/str_util.h"
#include "common/test_util.h"

int num_tests;

static const char *test_c_snprintf(void) {
  char buf[100];

  memset(buf, 'x', sizeof(buf));
  ASSERT_EQ(c_snprintf(buf, sizeof(buf), "boo"), 3);
  ASSERT_STREQ(buf, "boo");

  memset(buf, 'x', sizeof(buf));
  ASSERT_EQ(c_snprintf(buf, 0, "boo"), 3);
  ASSERT_EQ(buf[0], 'x');

  memset(buf, 'x', sizeof(buf));
  ASSERT_EQ(
      c_snprintf(buf, sizeof(buf), "/%s%c [%.*s]", "foo", 'q', 3, "123456789"),
      11);
  ASSERT_STREQ(buf, "/fooq [123]");

  memset(buf, 'x', sizeof(buf));
  ASSERT(c_snprintf(buf, sizeof(buf), "%d %x %04x %lu %ld", -2194, 0xabcdef01,
                    11, (unsigned long) 354523323, (long) 17) > 0);
  ASSERT_STREQ(buf, "-2194 abcdef01 000b 354523323 17");

  memset(buf, 'x', sizeof(buf));
  /*  ASSERT_EQ(c_snprintf(buf, sizeof(buf), "%*sfoo", 3, ""), 3); */
  c_snprintf(buf, sizeof(buf), "%*sfoo", 3, "");
  ASSERT_STREQ(buf, "   foo");

  return NULL;
}

static const char *test_cs_varint(void) {
  uint8_t buf[100];

  int llen_enc;
  int llen_dec;
  ASSERT_EQ((llen_enc = cs_varint_encode(127, buf)), 1);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 127);
  ASSERT_EQ(llen_dec, llen_enc);

  ASSERT_EQ((llen_enc = cs_varint_encode(128, buf)), 2);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 128);
  ASSERT_EQ(llen_dec, llen_enc);

  ASSERT_EQ((llen_enc = cs_varint_encode(0xfffffff, buf)), 4);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 0xfffffff);
  ASSERT_EQ(llen_dec, llen_enc);

  ASSERT_EQ((llen_enc = cs_varint_encode(0x7fffffff, buf)), 5);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 0x7fffffff);
  ASSERT_EQ(llen_dec, llen_enc);

  ASSERT_EQ((llen_enc = cs_varint_encode(0xffffffff, buf)), 5);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 0xffffffff);
  ASSERT_EQ(llen_dec, llen_enc);

  ASSERT_EQ((llen_enc = cs_varint_encode(0xfffffffff, buf)), 6);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 0xfffffffff);
  ASSERT_EQ(llen_dec, llen_enc);

  ASSERT_EQ((llen_enc = cs_varint_encode(0xfffffffffffffff, buf)), 9);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 0xfffffffffffffff);
  ASSERT_EQ(llen_dec, llen_enc);

  ASSERT_EQ((llen_enc = cs_varint_encode(0xffffffffffffffff, buf)), 10);
  ASSERT_EQ(cs_varint_decode(buf, &llen_dec), 0xffffffffffffffff);
  ASSERT_EQ(llen_dec, llen_enc);

  return NULL;
}

static const char *test_cs_timegm(void) {
  struct tm t;
  time_t now = time(NULL);
  gmtime_r(&now, &t);
  ASSERT_EQ((double) timegm(&t), cs_timegm(&t));

  return NULL;
}

static const char *test_mg_match_prefix(void) {
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str(""), mg_mk_str("")), 0);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("|"), mg_mk_str("")), 0);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("xy|"), mg_mk_str("")), 0);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("xy|"), mg_mk_str("xy")), 2);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("*"), mg_mk_str("")), 0);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a"), mg_mk_str("a")), 1);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a"), mg_mk_str("xyz")), -1);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("abc"), mg_mk_str("abc")), 3);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("abc"), mg_mk_str("abcdef")), 3);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("abc*"), mg_mk_str("abcdef")), 6);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f"), mg_mk_str("abcdef")), 6);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f"), mg_mk_str("abcdefZZ")), 6);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f"), mg_mk_str("abcdeZZ")), -1);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f|de*"), mg_mk_str("abcdef")), 6);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f|de*|xy"), mg_mk_str("defgh")), 5);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f,de*"), mg_mk_str("abcdef")), 6);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f|de*,xy"), mg_mk_str("defgh")), 5);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("a*f,de*,xy"), mg_mk_str("defgh")), 5);
  ASSERT_EQ(mg_match_prefix_n(mg_mk_str("/a/b/*/d"), mg_mk_str("/a/b/c/d")), 8);
  return NULL;
}

static const char *test_mg_mk_str(void) {
  const char *foo = "foo";
  struct mg_str s0 = MG_NULL_STR;
  ASSERT_EQ(s0.len, 0);
  ASSERT(s0.p == NULL);
  struct mg_str s0a = mg_mk_str(NULL);
  ASSERT_EQ(s0a.len, 0);
  ASSERT(s0a.p == NULL);
  struct mg_str s1 = MG_MK_STR("foo");
  ASSERT_EQ(s1.len, 3);
  ASSERT(s1.p == foo);
  struct mg_str s2 = mg_mk_str("foo");
  ASSERT_EQ(s2.len, 3);
  ASSERT(s2.p == foo);
  struct mg_str s3 = mg_mk_str_n("foo", 2);
  ASSERT_EQ(s3.len, 2);
  ASSERT(s3.p == foo);
  struct mg_str s4 = mg_mk_str_n("foo", 0);
  ASSERT_EQ(s4.len, 0);
  ASSERT(s4.p == foo);
  return NULL;
}

static const char *test_mg_strdup(void) {
  struct mg_str s0 = MG_NULL_STR;
  struct mg_str s1 = MG_MK_STR("foo");

  struct mg_str s2 = mg_strdup(s1);
  ASSERT_EQ(s1.len, s2.len);
  ASSERT(s1.p != s2.p);
  ASSERT_EQ(memcmp(s2.p, s1.p, s1.len), 0);

  struct mg_str s3 = mg_strdup(s0);
  ASSERT_EQ(s3.len, 0);
  ASSERT(s3.p == NULL);

  struct mg_str s4 = mg_mk_str_n("foo", 0);
  struct mg_str s5 = mg_strdup(s4);
  ASSERT_EQ(s5.len, 0);
  ASSERT(s5.p == NULL);

  free((void *) s2.p);
  return NULL;
}

static const char *test_mg_strchr(void) {
  struct mg_str s0 = MG_NULL_STR;
  struct mg_str s1 = mg_mk_str_n("foox", 3);
  struct mg_str s2 = mg_mk_str_n("foox\0yz", 7);

  ASSERT(mg_strchr(s0, 'f') == NULL);

  ASSERT(mg_strchr(s1, 'f') == s1.p);
  ASSERT(mg_strchr(s1, 'o') == s1.p + 1);
  ASSERT(mg_strchr(s1, 'x') == NULL);

  ASSERT(mg_strchr(s2, 'y') == s2.p + 5);
  ASSERT(mg_strchr(s2, 'z') == s2.p + 6);

  return NULL;
}

static const char *test_mg_strstr(void) {
  struct mg_str s0 = MG_NULL_STR;
  struct mg_str s1 = MG_MK_STR("foobario");
  struct mg_str s2 = s1;
  s2.len -= 2;

  ASSERT(mg_strstr(s0, s0) == NULL);
  ASSERT(mg_strstr(s1, s0) == s1.p);
  ASSERT(mg_strstr(s0, s1) == NULL);
  ASSERT(mg_strstr(s1, s1) == s1.p);
  ASSERT(mg_strstr(s1, mg_mk_str("foo")) == s1.p);
  ASSERT(mg_strstr(s1, mg_mk_str("oo")) == s1.p + 1);
  ASSERT(mg_strstr(s1, mg_mk_str("bar")) == s1.p + 3);
  ASSERT(mg_strstr(s1, mg_mk_str("bario")) == s1.p + 3);
  ASSERT(mg_strstr(s2, mg_mk_str("bar")) == s1.p + 3);
  ASSERT(mg_strstr(s2, mg_mk_str("bario")) == NULL);

  return NULL;
}

static const char *run_tests(const char *filter, double *total_elapsed) {
  RUN_TEST(test_c_snprintf);
  RUN_TEST(test_cs_varint);
  RUN_TEST(test_cs_timegm);
  RUN_TEST(test_mg_match_prefix);
  RUN_TEST(test_mg_mk_str);
  RUN_TEST(test_mg_strdup);
  RUN_TEST(test_mg_strchr);
  RUN_TEST(test_mg_strstr);
  return NULL;
}

int main(int argc, char *argv[]) {
  const char *fail_msg;
  const char *filter = argc > 1 ? argv[1] : "";
  double total_elapsed = 0.0;

  fail_msg = run_tests(filter, &total_elapsed);
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", num_tests);

  if (fail_msg != NULL) {
    /* Prevent leak analyzer from running: there will be "leaks" because of
     * premature return from the test, and in this case we don't care. */
    _exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
