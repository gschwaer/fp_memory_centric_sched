#include "system_config.h"

// SetWay, bits [31:4]
// Contains two fields:
// * Way, bits[31:32-A], the number of the way to operate on.
// * Set, bits[B-1:L], the number of the set to operate on.
// Bits[L-1:4] are RES 0.
// A = Log_2(ASSOCIATIVITY), L = Log_2(LINELEN), B = (L + S), S = Log_2(NSETS).
// ASSOCIATIVITY, LINELEN (line length, in bytes), and NSETS (number of sets) have their usual
// meanings and are the values for the cache level being operated on. The values of A and S are
// rounded up to the next integer.
// A = log_2(16) = 4
// L = log_2(64) = 6
// S = log_2(2048) = 11
// B = L + S = 6 + 11 = 17
// => Way, bits[31:28]
// => Set, bits[16:6]
// +Level, bits[3:1] <- Cache level to operate on, minus 1. For example, this field is 0 for operations on L1 cache, or 1 for operations on L2 cache.
// Cache level: L1=0, L2=1
#define WAYS_OFF 28
#define SETS_OFF 6
#define LEVEL ((2-1) << 1) /* L2 */
