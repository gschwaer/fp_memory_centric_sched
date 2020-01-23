#ifndef DPREM_H
#define DPREM_H

#include "ee_compiler.h"

FUNC(void, OS_CODE)
DPREM_init
(
		void
);

FUNC(void, OS_CODE)
DPREM_begin_memory_phase
(
		void
);

FUNC(void, OS_CODE)
DPREM_end_memory_phase
(
		void
);

#endif /* DPREM_H */
