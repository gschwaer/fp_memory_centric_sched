#ifndef DPREM_H
#define DPREM_H

#include "ee_compiler.h"
#include "ee_platform_types.h"

FUNC(void, OS_CODE)
DPREM_init
(
		void
);

FUNC(OsEE_reg, OS_CODE)
DPREM_begin_memory_phase
(
		void
);

FUNC(OsEE_reg, OS_CODE)
DPREM_end_memory_phase
(
		void
);

FUNC(void, OS_CODE)
DPREM_end_execution_phase(
		OsEE_reg
);

FUNC(void, OS_CODE)
DPREM_debug(
		void
);

#endif /* DPREM_H */
