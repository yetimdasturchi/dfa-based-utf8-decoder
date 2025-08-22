#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "utf8.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"

static int failures = 0;
static int successes = 0;

static void print_bytes(const uint8_t *p, size_t n) {
    if (!p) { printf("  <null>\n"); return; }
    printf("  bytes(%zu):", n);
    for (size_t i = 0; i < n; i++) {
        if ((i % 16) == 0) printf("\n   ");
        printf(" %02X", p[i]);
    }
    printf("\n");
}

static void report_validation_state(const uint8_t *p, size_t n) {
    uint32_t state = UTF8_ACCEPT;
    state = is_utf8_len_state((uint8_t*)p, n, state);
    printf("  end_state=%u (%s)\n",
           state, (state == UTF8_ACCEPT ? "ACCEPT" : "NOT-ACCEPT"));
}

#define PASS(LABEL) do { \
    printf(COLOR_GREEN "[PASS]" COLOR_RESET " %s\n", LABEL); \
    successes++; \
} while (0)

#define FAIL(LABEL) do { \
    printf(COLOR_RED "[FAIL]" COLOR_RESET " %s\n", LABEL); \
    failures++; \
} while (0)

#define TEST_BOOL(LABEL, EXPR) do { \
    if ((EXPR)) PASS(LABEL); else { \
        FAIL(LABEL); \
        printf("  expected: true\n  actual  : false\n"); \
    } \
} while (0)

#define TEST_BOOL_FALSE(LABEL, EXPR) do { \
    if (!(EXPR)) PASS(LABEL); else { \
        FAIL(LABEL); \
        printf("  expected: false\n  actual  : true\n"); \
    } \
} while (0)

#define TEST_UTF8_NT(LABEL, STR, EXPECT_TRUE) do { \
    const uint8_t *s_ = (const uint8_t*)(STR); \
    size_t len_ = strlen((const char*)s_); \
    int got_ = is_utf8((uint8_t*)s_); \
    int exp_ = (EXPECT_TRUE); \
    if (got_ == exp_) { PASS(LABEL); } else { \
        FAIL(LABEL); \
        printf("  expected: %s\n  actual  : %s\n", exp_ ? "VALID" : "INVALID", got_ ? "VALID" : "INVALID"); \
        print_bytes(s_, len_); \
        report_validation_state(s_, len_); \
    } \
} while (0)

#define TEST_UTF8_LEN(LABEL, BUF, LEN, EXPECT_TRUE) do { \
    const uint8_t *b_ = (const uint8_t*)(BUF); \
    size_t n_ = (LEN); \
    int got_ = is_utf8_len((uint8_t*)b_, n_); \
    int exp_ = (EXPECT_TRUE); \
    if (got_ == exp_) { PASS(LABEL); } else { \
        FAIL(LABEL); \
        printf("  expected: %s\n  actual  : %s\n", exp_ ? "VALID" : "INVALID", got_ ? "VALID" : "INVALID"); \
        print_bytes(b_, n_); \
        report_validation_state(b_, n_); \
    } \
} while (0)

static void test_is_utf8_basic(void) {
    TEST_UTF8_NT("ASCII valid",                "Hello, world!", 1);
    TEST_UTF8_NT("Japanese valid こんにちは",  "こんにちは",     1);
    TEST_UTF8_NT("Emoji 😀 valid",             "\xF0\x9F\x98\x80", 1);
    TEST_UTF8_NT("Mixed string valid",         "ASCII + 😀 + café + 你好", 1);
}

static void test_is_utf8_invalid(void) {
    const uint8_t lone_cont[]     = { 0x80, 0x00 };
    const uint8_t overlong_slash[] = { 0xC0, 0xAF, 0x00 };
    const uint8_t trunc2[]        = { 0xC3, 0x00 };
    const uint8_t trunc4[]        = { 0xF0, 0x9F, 0x98, 0x00 };
    const uint8_t surrogate[]     = { 0xED, 0xA0, 0x80, 0x00 };
    const uint8_t too_large[]     = { 0xF4, 0x90, 0x80, 0x80, 0x00 };

    TEST_UTF8_NT ("Lone continuation invalid",              lone_cont,      0);
    TEST_UTF8_NT ("Overlong slash invalid",                 overlong_slash, 0);
    TEST_UTF8_NT ("Truncated 2-byte invalid",               trunc2,         0);
    TEST_UTF8_NT ("Truncated 4-byte invalid",               trunc4,         0);
    TEST_UTF8_NT ("Surrogate U+D800 invalid in UTF-8",      surrogate,      0);
    TEST_UTF8_NT ("Codepoint > U+10FFFF invalid",           too_large,      0);
}

static void test_is_utf8_len_partial(void) {
    const uint8_t ja[] = "こんにちは";
    TEST_UTF8_LEN("Ja first 2 bytes (incomplete) invalid",  ja, 2, 0);
    TEST_UTF8_LEN("Ja first 3 bytes (one char) valid",      ja, 3, 1);
    TEST_UTF8_LEN("Ja full length valid",                   ja, strlen((const char*)ja), 1);

    const uint8_t emoji[] = { 0xF0, 0x9F, 0x98, 0x80 };
    TEST_UTF8_LEN("Emoji 1 byte invalid",                   emoji, 1, 0);
    TEST_UTF8_LEN("Emoji 2 bytes invalid",                  emoji, 2, 0);
    TEST_UTF8_LEN("Emoji 3 bytes invalid",                  emoji, 3, 0);
    TEST_UTF8_LEN("Emoji 4 bytes valid",                    emoji, 4, 1);
}

static void test_streaming_state(void) {
    const uint8_t part1[] = "ASCII + ";
    const uint8_t part2[] = "\xF0\x9F\x98\x80 café";

    uint32_t state = UTF8_ACCEPT;
    state = is_utf8_len_state((uint8_t*)part1, strlen((const char*)part1), state);
    if (state == UTF8_ACCEPT) PASS("Streaming chunk1 end ACCEPT");
    else { FAIL("Streaming chunk1 end ACCEPT");
           print_bytes(part1, strlen((const char*)part1));
           report_validation_state(part1, strlen((const char*)part1));
    }

    state = is_utf8_len_state((uint8_t*)part2, strlen((const char*)part2), state);
    if (state == UTF8_ACCEPT) PASS("Streaming chunk2 end ACCEPT");
    else { FAIL("Streaming chunk2 end ACCEPT");
           print_bytes(part2, strlen((const char*)part2));
           report_validation_state(part2, strlen((const char*)part2));
    }

    const uint8_t e1[] = { 0xF0, 0x9F };
    const uint8_t e2[] = { 0x98, 0x80 };

    state = UTF8_ACCEPT;
    state = is_utf8_len_state((uint8_t*)e1, sizeof e1, state);
    if (state != UTF8_ACCEPT) PASS("Streaming split emoji part1 non-ACCEPT");
    else { FAIL("Streaming split emoji part1 non-ACCEPT");
           print_bytes(e1, sizeof e1);
           report_validation_state(e1, sizeof e1);
    }

    state = is_utf8_len_state((uint8_t*)e2, sizeof e2, state);
    if (state == UTF8_ACCEPT) PASS("Streaming split emoji part2 ACCEPT");
    else { FAIL("Streaming split emoji part2 ACCEPT");
           print_bytes(e2, sizeof e2);
           report_validation_state(e2, sizeof e2);
    }
}

#ifdef DEMO_FAIL
static void test_demo_failure(void) {
    const uint8_t good[] = "OK ✓";
    TEST_UTF8_NT("DEMO_FAIL: expect invalid but string is valid", good, 0);
}
#endif

int main(void) {
    test_is_utf8_basic();
    test_is_utf8_invalid();
    test_is_utf8_len_partial();
    test_streaming_state();
#ifdef DEMO_FAIL
    test_demo_failure();
#endif

    printf("\n===== TEST SUMMARY =====\n");
    printf("  Passed: %d\n", successes);
    printf("  Failed: %d\n", failures);

    if (failures) {
        printf("Some tests FAILED.\n");
        return EXIT_FAILURE;
    }
    printf("All tests PASSED.\n");
    return EXIT_SUCCESS;
}
