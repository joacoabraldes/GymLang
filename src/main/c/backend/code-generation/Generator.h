#ifndef GENERATOR_HEADER
#define GENERATOR_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/** Initialize module's internal state. */
ModuleDestructor initializeGeneratorModule();

/**
 * Stage 2 stub: HTML generation is deferred to a later stage.
 */
void executeGenerator(CompilerState * compilerState);

#endif
