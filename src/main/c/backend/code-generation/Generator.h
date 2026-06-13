#ifndef GENERATOR_HEADER
#define GENERATOR_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/** Initialize module's internal state. */
ModuleDestructor initializeGeneratorModule();

/**
 * Translates the validated AST into a self-contained, interactive HTML file
 * that renders every workout, its blocks and the training schedule, together
 * with the session summary (volume, total sets and estimated time).
 */
void executeGenerator(CompilerState * compilerState);

#endif
