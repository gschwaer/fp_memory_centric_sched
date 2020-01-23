#ifndef HERCULES_UTILITIES_H
#define HERCULES_UTILITIES_H

#ifdef HERCULES_DEBUG
# define DEBUG_PRINT(x) printk x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

#endif /* HERCULES_UTILITIES_H */
