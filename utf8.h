#ifndef UTF8_DECODE_H
#define UTF8_DECODE_H

	#include <inttypes.h>
	#include <stddef.h>

	#define UTF8_ACCEPT 0
	#define UTF8_REJECT 1

	extern int is_utf8(uint8_t* s);
	extern int is_utf8_len(uint8_t *s, size_t len);
	extern uint32_t is_utf8_len_state(uint8_t *s, size_t len, uint32_t state);

#endif
