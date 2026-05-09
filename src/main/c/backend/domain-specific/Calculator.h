#ifndef CALCULATOR_HEADER
#define CALCULATOR_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/** Initialize module's internal state. */
ModuleDestructor initializeCalculatorModule();

/**
 * The result of a computation. It's considered valid only if "succeeded" is
 * true.
 */
typedef struct {
	bool succeeded;
	int value;
} ComputationResult;

/**
 * Stage 2 stub: the frontend already validates the program; this entry point
 * always succeeds. Domain-specific computation (volume totals, estimated
 * training time, etc.) belongs to a later stage.
 */
ComputationResult executeCalculator(CompilerState * compilerState);

#endif
