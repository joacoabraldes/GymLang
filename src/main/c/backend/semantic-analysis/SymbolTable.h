#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/** Initialize module's internal state. */
ModuleDestructor initializeSymbolTableModule();

/**
 * A single entry of the symbol table: the workout name acts as the key, and
 * the workout node is kept so later phases can resolve a reference back to its
 * definition.
 */
typedef struct SymbolEntry {
	char * name;
	Workout * workout;
	struct SymbolEntry * next;
} SymbolEntry;

typedef struct {
	SymbolEntry * head;
	int count;
} SymbolTable;

/** Creates an empty symbol table. */
SymbolTable * createSymbolTable();

/** Releases the table and its entries (workout names are not owned). */
void destroySymbolTable(SymbolTable * table);

/**
 * Registers a workout under its name. Returns false when the name was already
 * defined, leaving the previous binding untouched.
 */
bool defineWorkout(SymbolTable * table, char * name, Workout * workout);

/** Returns the workout bound to the name, or NULL when it is undefined. */
Workout * resolveWorkout(SymbolTable * table, const char * name);

#endif
