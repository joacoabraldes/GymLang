#include "SemanticAnalyzer.h"
#include "SymbolTable.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;
static int _errors = 0;

void _shutdownSemanticAnalyzerModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: SemanticAnalyzer...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSemanticAnalyzerModule() {
	_logger = createLogger("SemanticAnalyzer");
	return _shutdownSemanticAnalyzerModule;
}

/* The scope where an exercise lives changes which attributes it must define. */
typedef enum {
	SCOPE_WORKOUT,
	SCOPE_WARMUP,
	SCOPE_SUPERSET
} ExerciseScope;

#define ATTRIBUTE_COUNT (ATTR_DROPSET_REPS + 1)

#define reportError(...) do { _errors += 1; logError(_logger, __VA_ARGS__); } while (0)

/* PRIVATE FUNCTIONS */

static const char * _attributeName(AttributeKey key) {
	switch (key) {
		case ATTR_NAME: return "name";
		case ATTR_SETS: return "sets";
		case ATTR_REPS: return "reps";
		case ATTR_WEIGHT: return "weight";
		case ATTR_REST: return "rest";
		case ATTR_TEMPO: return "tempo";
		case ATTR_DURATION: return "duration";
		case ATTR_INTENSITY: return "intensity";
		case ATTR_DISTANCE: return "distance";
		case ATTR_DROPSET: return "dropset";
		case ATTR_DROPSET_REPS: return "dropset_reps";
		default: return "unknown";
	}
}

static const char * _exerciseTypeName(ExerciseTypeAst type) {
	switch (type) {
		case EXERCISE_STRENGTH: return "strength";
		case EXERCISE_CARDIO: return "cardio";
		case EXERCISE_HYPERTROPHY: return "hypertrophy";
		default: return "unknown";
	}
}

static const char * _dayName(DayOfWeek day) {
	switch (day) {
		case DAY_MON: return "mon";
		case DAY_TUE: return "tue";
		case DAY_WED: return "wed";
		case DAY_THU: return "thu";
		case DAY_FRI: return "fri";
		case DAY_SAT: return "sat";
		case DAY_SUN: return "sun";
		default: return "unknown";
	}
}

static int _valueItemListLength(const ValueItemList * list) {
	int length = 0;
	for (const ValueItemList * node = list; node != NULL; node = node->next) {
		length += 1;
	}
	return length;
}

static int _integerListLength(const IntegerList * list) {
	int length = 0;
	for (const IntegerList * node = list; node != NULL; node = node->next) {
		length += 1;
	}
	return length;
}

static bool _isAttributeAllowed(ExerciseTypeAst type, AttributeKey key) {
	switch (key) {
		case ATTR_NAME:
			return true;
		case ATTR_SETS:
		case ATTR_REPS:
		case ATTR_WEIGHT:
		case ATTR_REST:
		case ATTR_TEMPO:
		case ATTR_DROPSET:
		case ATTR_DROPSET_REPS:
			return type == EXERCISE_STRENGTH || type == EXERCISE_HYPERTROPHY;
		case ATTR_DURATION:
		case ATTR_INTENSITY:
		case ATTR_DISTANCE:
			return type == EXERCISE_CARDIO;
		default:
			return false;
	}
}

static void _validateValueRanges(ExerciseAttribute ** found, const char * typeName) {
	if (found[ATTR_SETS] != NULL && found[ATTR_SETS]->intValue < 1) {
		reportError("\"sets\" must be at least 1, found %d.", found[ATTR_SETS]->intValue);
	}
	if (found[ATTR_DURATION] != NULL && found[ATTR_DURATION]->intValue < 1) {
		reportError("\"duration\" must be at least 1 minute, found %d.", found[ATTR_DURATION]->intValue);
	}
	if (found[ATTR_DISTANCE] != NULL && found[ATTR_DISTANCE]->intValue < 1) {
		reportError("\"distance\" must be at least 1 meter, found %d.", found[ATTR_DISTANCE]->intValue);
	}
	if (found[ATTR_INTENSITY] != NULL) {
		int intensity = found[ATTR_INTENSITY]->intValue;
		if (intensity < 1 || intensity > 10) {
			reportError("\"intensity\" must be between 1 and 10, found %d.", intensity);
		}
	}
	if (found[ATTR_REPS] != NULL) {
		for (ValueItemList * node = found[ATTR_REPS]->valueList; node != NULL; node = node->next) {
			ValueItem * item = node->item;
			if (item->isRange && (item->from < 1 || item->from > item->to)) {
				reportError("\"reps\" range %d-%d is invalid for a %s exercise.", item->from, item->to, typeName);
			}
			else if (!item->isRange && item->from < 1) {
				reportError("\"reps\" must be at least 1, found %d.", item->from);
			}
		}
	}
	if (found[ATTR_WEIGHT] != NULL) {
		for (ValueItemList * node = found[ATTR_WEIGHT]->valueList; node != NULL; node = node->next) {
			ValueItem * item = node->item;
			if (item->isRange && item->from > item->to) {
				reportError("\"weight\" range %d-%d is invalid.", item->from, item->to);
			}
		}
	}
	if (found[ATTR_DROPSET_REPS] != NULL) {
		for (IntegerList * node = found[ATTR_DROPSET_REPS]->intList; node != NULL; node = node->next) {
			if (node->value < 1) {
				reportError("\"dropset_reps\" must be at least 1, found %d.", node->value);
			}
		}
	}
}

static void _validateExercise(Exercise * exercise, ExerciseScope scope, int rounds) {
	ExerciseTypeAst type = exercise->type;
	const char * typeName = _exerciseTypeName(type);

	int counts[ATTRIBUTE_COUNT] = {0};
	ExerciseAttribute * found[ATTRIBUTE_COUNT] = {NULL};
	for (AttributeList * node = exercise->attributes; node != NULL; node = node->next) {
		ExerciseAttribute * attribute = node->attribute;
		counts[attribute->key] += 1;
		if (found[attribute->key] == NULL) {
			found[attribute->key] = attribute;
		}
		if (!_isAttributeAllowed(type, attribute->key)) {
			reportError("Attribute \"%s\" is not valid for a %s exercise.", _attributeName(attribute->key), typeName);
		}
	}
	for (int key = 0; key < ATTRIBUTE_COUNT; ++key) {
		if (counts[key] > 1) {
			reportError("Attribute \"%s\" is repeated %d times in the same exercise.", _attributeName(key), counts[key]);
		}
	}

	if (counts[ATTR_NAME] == 0) {
		reportError("Every exercise must define a \"name\".");
	}
	if (type == EXERCISE_CARDIO) {
		if (counts[ATTR_DURATION] == 0) {
			reportError("A cardio exercise must define a \"duration\".");
		}
	}
	else {
		if (counts[ATTR_REPS] == 0) {
			reportError("A %s exercise must define \"reps\".", typeName);
		}
		if (scope != SCOPE_SUPERSET && counts[ATTR_SETS] == 0) {
			reportError("A %s exercise must define \"sets\".", typeName);
		}
	}

	_validateValueRanges(found, typeName);

	int expected = -1;
	const char * seriesName = "sets";
	if (scope == SCOPE_SUPERSET) {
		expected = rounds;
		seriesName = "rounds";
	}
	else if (found[ATTR_SETS] != NULL) {
		expected = found[ATTR_SETS]->intValue;
	}
	if (expected > 0) {
		if (found[ATTR_REPS] != NULL) {
			int length = _valueItemListLength(found[ATTR_REPS]->valueList);
			if (length > 1 && length != expected) {
				reportError("The \"reps\" list has %d values but the exercise has %d %s.", length, expected, seriesName);
			}
		}
		if (found[ATTR_WEIGHT] != NULL) {
			int length = _valueItemListLength(found[ATTR_WEIGHT]->valueList);
			if (length > 1 && length != expected) {
				reportError("The \"weight\" list has %d values but the exercise has %d %s.", length, expected, seriesName);
			}
		}
	}

	if (found[ATTR_DROPSET_REPS] != NULL && found[ATTR_DROPSET] == NULL) {
		reportError("\"dropset_reps\" requires a \"dropset\" to be defined.");
	}
	if (found[ATTR_DROPSET] != NULL && found[ATTR_DROPSET_REPS] != NULL) {
		int drops = _integerListLength(found[ATTR_DROPSET]->intList);
		int dropReps = _integerListLength(found[ATTR_DROPSET_REPS]->intList);
		if (drops != dropReps) {
			reportError("\"dropset\" has %d values but \"dropset_reps\" has %d.", drops, dropReps);
		}
	}
}

static void _validateWorkout(Workout * workout) {
	if (workout->warmup != NULL) {
		for (ExerciseList * node = workout->warmup->exercises; node != NULL; node = node->next) {
			_validateExercise(node->exercise, SCOPE_WARMUP, 0);
		}
	}
	for (ItemList * node = workout->items; node != NULL; node = node->next) {
		Item * item = node->item;
		if (item->type == ITEM_EXERCISE) {
			_validateExercise(item->exercise, SCOPE_WORKOUT, 0);
		}
		else {
			Superset * superset = item->superset;
			if (superset->rounds < 1) {
				reportError("A superset must have at least one round, found %d.", superset->rounds);
			}
			for (ExerciseList * inner = superset->exercises; inner != NULL; inner = inner->next) {
				_validateExercise(inner->exercise, SCOPE_SUPERSET, superset->rounds);
			}
		}
	}
}

static void _validateWeek(WeekBlock * week, SymbolTable * table) {
	bool assigned[DAY_SUN + 1] = {false};
	for (WeekAssignmentList * node = week->assignments; node != NULL; node = node->next) {
		WeekAssignment * assignment = node->assignment;
		if (assigned[assignment->day]) {
			reportError("Day \"%s\" is assigned more than once in the week plan.", _dayName(assignment->day));
		}
		assigned[assignment->day] = true;
		if (resolveWorkout(table, assignment->workoutName) == NULL) {
			reportError("The week plan references an undefined workout: \"%s\".", assignment->workoutName);
		}
	}
}

static void _validateRecurring(RecurringBlock * recurring, SymbolTable * table) {
	for (StringList * node = recurring->order; node != NULL; node = node->next) {
		if (resolveWorkout(table, node->value) == NULL) {
			reportError("The recurring plan references an undefined workout: \"%s\".", node->value);
		}
	}
	int days = _integerListLength(recurring->days);
	if (days != 2) {
		reportError("\"days\" must contain exactly two values (training and rest), found %d.", days);
	}
	else if (recurring->days->value < 1) {
		reportError("The recurring plan must have at least one training day.");
	}
}

/* PUBLIC FUNCTIONS */

bool executeSemanticAnalysis(CompilerState * compilerState) {
	_errors = 0;
	Program * program = (Program *) compilerState->abstractSyntaxtTree;
	if (program == NULL) {
		logError(_logger, "There is no program to analyze.");
		return false;
	}
	SymbolTable * table = createSymbolTable();
	for (WorkoutList * node = program->workouts; node != NULL; node = node->next) {
		Workout * workout = node->workout;
		if (!defineWorkout(table, workout->name, workout)) {
			reportError("Workout \"%s\" is defined more than once.", workout->name);
		}
		_validateWorkout(workout);
	}
	if (program->schedule != NULL) {
		if (program->schedule->type == SCHEDULE_WEEK) {
			_validateWeek(program->schedule->week, table);
		}
		else {
			_validateRecurring(program->schedule->recurring, table);
		}
	}
	destroySymbolTable(table);
	if (_errors == 0) {
		logInformation(_logger, "Semantic analysis succeeded.");
		return true;
	}
	logError(_logger, "Semantic analysis rejected the program with %d error(s).", _errors);
	return false;
}
