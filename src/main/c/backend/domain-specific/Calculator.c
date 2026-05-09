#include "Calculator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownCalculatorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Calculator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCalculatorModule() {
	_logger = createLogger("Calculator");
	return _shutdownCalculatorModule;
}

/* PUBLIC FUNCTIONS */

ComputationResult executeCalculator(CompilerState * compilerState) {
	(void) compilerState;
	logDebugging(_logger, "Calculator stub: stage 2 frontend only.");
	ComputationResult result = { .succeeded = true, .value = 0 };
	return result;
}
