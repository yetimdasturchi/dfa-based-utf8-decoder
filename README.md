
# UTF-8 DFA Validator & Decoder

A lightweight, fast, and robust **UTF-8 validation and decoding library** in C.  
It is based on **Bjoern Höhrmann’s DFA-based UTF-8 decoder**, which uses a small state machine table to efficiently validate and decode UTF-8 byte sequences.

---

## Features

- **Validates** whether a string or buffer is proper UTF-8
- **Decodes** byte sequences into Unicode code points
- **Incremental / streaming** decoding support
- **Tiny & fast**: table-driven DFA, minimal branching

---

## API

Header: [`utf8.h`](utf8.h)  
Implementation: [`utf8.c`](utf8.c)

```c
#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

int is_utf8(uint8_t *s);
int is_utf8_len(uint8_t *s, size_t len);
uint32_t is_utf8_len_state(uint8_t *s, size_t len, uint32_t state);
```

## Functions
- `is_utf8(uint8_t *s)` - Check if a null-terminated string is valid UTF-8.

- `is_utf8_len(uint8_t *s, size_t len)` - Check if a buffer of given length is valid UTF-8.

- `is_utf8_len_state(uint8_t *s, size_t len, uint32_t state)` - Incrementally validate a buffer starting from a given state (useful for streaming input). Returns the final DFA state (UTF8_ACCEPT if valid).

## Example Usage
```c
#include <stdio.h>
#include "utf8.h"

int main(void) {
    uint8_t valid[] = "こんにちは";
    uint8_t invalid[] = {0xC3, 0x28, 0x00};

    if (is_utf8(valid)) {
        printf("Valid UTF-8 string!\\n");
    }

    if (!is_utf8(invalid)) {
        printf("Invalid UTF-8 string!\\n");
    }

    return 0;
}
```
## Compile and run:

```bash
cc -std=c11 -Wall -Wextra -O2 utf8.c example.c -o example
./example
```

Output:

```
Valid UTF-8 string!
Invalid UTF-8 string!
```

## Tests
A test suite is included in [`main.c`](main.c)  .   It covers valid/invalid strings, partial sequences, emoji, and streaming chunked decoding.

```bash
cc -std=c11 -Wall -Wextra -O2 utf8.c main.c -o test
./test
```
**Example output:**

```
[PASS] ASCII valid
[PASS] Japanese valid こんにちは
[PASS] Emoji 😀 valid
...
===== TEST SUMMARY =====
  Passed: 21
  Failed: 0
All tests PASSED.
```

**Run with demo failure mode:**

To see how failures are reported (with hex dump and DFA state), compile with -DDEMO_FAIL:

```
cc -std=c11 -Wall -Wextra -O2 -DDEMO_FAIL utf8.c main.c -o test
./test
```

Example output:

```
[FAIL] DEMO_FAIL: expect invalid but string is valid
  expected: INVALID
  actual  : VALID
  bytes(5):
   4F 4B 20 E2 9C 93
  end_state=0 (ACCEPT)

===== TEST SUMMARY =====
  Passed: 20
  Failed: 1
Some tests FAILED.
```
## Why DFA-based?
- Very compact: a single lookup table (utf8d[]) encodes both byte classes and state transitions.

- Very fast: decoding is mostly table lookups, minimizing branches.

- Streaming-friendly: you can feed data in chunks and keep track of state between calls.

For more details, see [Bjoern Höhrmann’s](https://bjoern.hoehrmann.de/utf-8/decoder/dfa/) original description.
