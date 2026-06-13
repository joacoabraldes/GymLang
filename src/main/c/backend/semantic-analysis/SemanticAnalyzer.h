#ifndef SEMANTIC_ANALYZER_HEADER
#define SEMANTIC_ANALYZER_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/** Initialize module's internal state. */
ModuleDestructor initializeSemanticAnalyzerModule();

/**
 * Walks the AST validating that the program is meaningful for the gym domain:
 * unique workout names, type-consistent exercises, coherent set/rep/weight
 * lists and schedules that only reference defined workouts. Returns true when
 * the program is accepted, and false when at least one rule is violated.
 */
bool executeSemanticAnalysis(CompilerState * compilerState);

#endif
