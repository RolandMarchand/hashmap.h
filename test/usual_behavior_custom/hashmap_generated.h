#ifndef HASHMAP_GENERATED_H
#define HASHMAP_GENERATED_H

#define COMPARISON_CALLBACK strcmp
#define HASH_CALLBACK fnv1a_32_str

unsigned long fnv1a_32_str(const char *str);

#define HASHMAP_LONG_JUMP_NO_ABORT
#include "hashmap.h"

#define FNV1A_32_PRIME 0x01000193U
#define FNV1A_32_INITIAL_VAL 0x811c9dc5U

unsigned long fnv1a_32_str(const char *str)
{
	const unsigned char *ustr = (const unsigned char *)str;
	unsigned long hval = FNV1A_32_INITIAL_VAL;

	for (; ustr[0] != '\0'; ustr++) {
		hval ^= (unsigned long)ustr[0];
		hval *= FNV1A_32_PRIME;
		hval &= 0xFFFFFFFFUL;
	}

	return hval;
}


#endif /* HASHMAP_GENERATED_H */
