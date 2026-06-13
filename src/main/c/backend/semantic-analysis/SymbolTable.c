#include "SymbolTable.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownSymbolTableModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: SymbolTable...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSymbolTableModule() {
	_logger = createLogger("SymbolTable");
	return _shutdownSymbolTableModule;
}

/* PUBLIC FUNCTIONS */

SymbolTable * createSymbolTable() {
	SymbolTable * table = calloc(1, sizeof(SymbolTable));
	table->head = NULL;
	table->count = 0;
	return table;
}

void destroySymbolTable(SymbolTable * table) {
	if (table == NULL) {
		return;
	}
	SymbolEntry * entry = table->head;
	while (entry != NULL) {
		SymbolEntry * next = entry->next;
		free(entry);
		entry = next;
	}
	free(table);
}

bool defineWorkout(SymbolTable * table, char * name, Workout * workout) {
	if (resolveWorkout(table, name) != NULL) {
		return false;
	}
	SymbolEntry * entry = calloc(1, sizeof(SymbolEntry));
	entry->name = name;
	entry->workout = workout;
	entry->next = table->head;
	table->head = entry;
	table->count += 1;
	logDebugging(_logger, "Workout defined: \"%s\".", name);
	return true;
}

Workout * resolveWorkout(SymbolTable * table, const char * name) {
	for (SymbolEntry * entry = table->head; entry != NULL; entry = entry->next) {
		if (strcmp(entry->name, name) == 0) {
			return entry->workout;
		}
	}
	return NULL;
}
