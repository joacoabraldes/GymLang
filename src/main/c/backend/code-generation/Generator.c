#include "Generator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule() {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

/* PUBLIC FUNCTIONS */

void executeGenerator(CompilerState * compilerState) {
	(void) compilerState;
	logDebugging(_logger, "Generator stub: stage 2 frontend only.");
}
