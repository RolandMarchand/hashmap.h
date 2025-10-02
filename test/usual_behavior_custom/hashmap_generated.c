/* #include "hashmap_generated.h"
 * 
 * #define FNV1A_32_PRIME 0x01000193U
 * #define FNV1A_32_INITIAL_VAL 0x811c9dc5U
 * 
 * unsigned long fnv1a_32_str(const char *str)
 * {
 * 	const unsigned char *ustr = (const unsigned char *)str;
 * 	unsigned long hval = FNV1A_32_INITIAL_VAL;
 * 
 * 	for (; ustr[0] != '\0'; ustr++) {
 * 		hval ^= (unsigned long)ustr[0];
 * 		hval *= FNV1A_32_PRIME;
 * 		hval &= 0xFFFFFFFFUL;
 * 	}
 * 
 * 	return hval;
 * } */
typedef int unused;
