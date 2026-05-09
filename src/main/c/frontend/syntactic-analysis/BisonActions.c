#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* Value primitives. */

ValueItem * ValueItemSingleSemanticAction(int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ValueItem * item = calloc(1, sizeof(ValueItem));
	item->from = value;
	item->to = value;
	item->isRange = false;
	return item;
}

ValueItem * ValueItemRangeSemanticAction(int from, int to) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ValueItem * item = calloc(1, sizeof(ValueItem));
	item->from = from;
	item->to = to;
	item->isRange = true;
	return item;
}

ValueItemList * ValueItemListSingleSemanticAction(ValueItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ValueItemList * list = calloc(1, sizeof(ValueItemList));
	list->item = item;
	list->next = NULL;
	return list;
}

ValueItemList * ValueItemListAppendSemanticAction(ValueItemList * list, ValueItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ValueItemList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(ValueItemList));
	tail->next->item = item;
	tail->next->next = NULL;
	return list;
}

IntegerList * IntegerListSingleSemanticAction(int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IntegerList * list = calloc(1, sizeof(IntegerList));
	list->value = value;
	list->next = NULL;
	return list;
}

IntegerList * IntegerListAppendSemanticAction(IntegerList * list, int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IntegerList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(IntegerList));
	tail->next->value = value;
	tail->next->next = NULL;
	return list;
}

StringList * StringListSingleSemanticAction(char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	StringList * list = calloc(1, sizeof(StringList));
	list->value = value;
	list->next = NULL;
	return list;
}

StringList * StringListAppendSemanticAction(StringList * list, char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	StringList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(StringList));
	tail->next->value = value;
	tail->next->next = NULL;
	return list;
}

/* Exercise attributes. */

ExerciseAttribute * StringAttributeSemanticAction(AttributeKey key, char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExerciseAttribute * attribute = calloc(1, sizeof(ExerciseAttribute));
	attribute->key = key;
	attribute->valueType = VAL_STRING;
	attribute->stringValue = value;
	return attribute;
}

ExerciseAttribute * IntegerAttributeSemanticAction(AttributeKey key, int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExerciseAttribute * attribute = calloc(1, sizeof(ExerciseAttribute));
	attribute->key = key;
	attribute->valueType = VAL_INTEGER;
	attribute->intValue = value;
	return attribute;
}

ExerciseAttribute * ValueListAttributeSemanticAction(AttributeKey key, ValueItemList * list) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExerciseAttribute * attribute = calloc(1, sizeof(ExerciseAttribute));
	attribute->key = key;
	attribute->valueType = VAL_VALUE_LIST;
	attribute->valueList = list;
	return attribute;
}

ExerciseAttribute * IntegerListAttributeSemanticAction(AttributeKey key, IntegerList * list) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExerciseAttribute * attribute = calloc(1, sizeof(ExerciseAttribute));
	attribute->key = key;
	attribute->valueType = VAL_INTEGER_LIST;
	attribute->intList = list;
	return attribute;
}

ExerciseAttribute * TempoAttributeSemanticAction(int eccentric, int pause1, int concentric, int pause2) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExerciseAttribute * attribute = calloc(1, sizeof(ExerciseAttribute));
	attribute->key = ATTR_TEMPO;
	attribute->valueType = VAL_TEMPO;
	attribute->tempo.eccentric = eccentric;
	attribute->tempo.pause1 = pause1;
	attribute->tempo.concentric = concentric;
	attribute->tempo.pause2 = pause2;
	return attribute;
}

AttributeList * AttributeListSingleSemanticAction(ExerciseAttribute * attribute) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	AttributeList * list = calloc(1, sizeof(AttributeList));
	list->attribute = attribute;
	list->next = NULL;
	return list;
}

AttributeList * AttributeListAppendSemanticAction(AttributeList * list, ExerciseAttribute * attribute) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	AttributeList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(AttributeList));
	tail->next->attribute = attribute;
	tail->next->next = NULL;
	return list;
}

/* Exercise / superset / warmup / items. */

Exercise * ExerciseSemanticAction(ExerciseTypeAst type, AttributeList * attributes) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Exercise * exercise = calloc(1, sizeof(Exercise));
	exercise->type = type;
	exercise->attributes = attributes;
	return exercise;
}

ExerciseList * ExerciseListSingleSemanticAction(Exercise * exercise) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExerciseList * list = calloc(1, sizeof(ExerciseList));
	list->exercise = exercise;
	list->next = NULL;
	return list;
}

ExerciseList * ExerciseListAppendSemanticAction(ExerciseList * list, Exercise * exercise) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExerciseList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(ExerciseList));
	tail->next->exercise = exercise;
	tail->next->next = NULL;
	return list;
}

Warmup * WarmupSemanticAction(ExerciseList * exercises) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Warmup * warmup = calloc(1, sizeof(Warmup));
	warmup->exercises = exercises;
	return warmup;
}

Superset * SupersetSemanticAction(int rounds, bool hasRest, int rest, Exercise * first, Exercise * second, ExerciseList * extras) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Superset * superset = calloc(1, sizeof(Superset));
	superset->rounds = rounds;
	superset->hasRest = hasRest;
	superset->rest = hasRest ? rest : 0;
	ExerciseList * exercises = ExerciseListSingleSemanticAction(first);
	ExerciseListAppendSemanticAction(exercises, second);
	if (extras != NULL) {
		ExerciseList * tail = exercises;
		while (tail->next != NULL) {
			tail = tail->next;
		}
		tail->next = extras;
	}
	superset->exercises = exercises;
	return superset;
}

Item * ItemFromExerciseSemanticAction(Exercise * exercise) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Item * item = calloc(1, sizeof(Item));
	item->type = ITEM_EXERCISE;
	item->exercise = exercise;
	return item;
}

Item * ItemFromSupersetSemanticAction(Superset * superset) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Item * item = calloc(1, sizeof(Item));
	item->type = ITEM_SUPERSET;
	item->superset = superset;
	return item;
}

ItemList * ItemListSingleSemanticAction(Item * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ItemList * list = calloc(1, sizeof(ItemList));
	list->item = item;
	list->next = NULL;
	return list;
}

ItemList * ItemListAppendSemanticAction(ItemList * list, Item * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ItemList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(ItemList));
	tail->next->item = item;
	tail->next->next = NULL;
	return list;
}

/* Workouts. */

Workout * WorkoutSemanticAction(char * name, char * description, Warmup * warmup, ItemList * items) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Workout * workout = calloc(1, sizeof(Workout));
	workout->name = name;
	workout->description = description;
	workout->warmup = warmup;
	workout->items = items;
	return workout;
}

WorkoutList * WorkoutListSingleSemanticAction(Workout * workout) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	WorkoutList * list = calloc(1, sizeof(WorkoutList));
	list->workout = workout;
	list->next = NULL;
	return list;
}

WorkoutList * WorkoutListAppendSemanticAction(WorkoutList * list, Workout * workout) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	WorkoutList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(WorkoutList));
	tail->next->workout = workout;
	tail->next->next = NULL;
	return list;
}

/* Schedule. */

WeekAssignment * WeekAssignmentSemanticAction(DayOfWeek day, char * workoutName) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	WeekAssignment * assignment = calloc(1, sizeof(WeekAssignment));
	assignment->day = day;
	assignment->workoutName = workoutName;
	return assignment;
}

WeekAssignmentList * WeekAssignmentListSingleSemanticAction(WeekAssignment * assignment) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	WeekAssignmentList * list = calloc(1, sizeof(WeekAssignmentList));
	list->assignment = assignment;
	list->next = NULL;
	return list;
}

WeekAssignmentList * WeekAssignmentListAppendSemanticAction(WeekAssignmentList * list, WeekAssignment * assignment) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	WeekAssignmentList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = calloc(1, sizeof(WeekAssignmentList));
	tail->next->assignment = assignment;
	tail->next->next = NULL;
	return list;
}

WeekBlock * WeekBlockSemanticAction(WeekAssignmentList * assignments) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	WeekBlock * block = calloc(1, sizeof(WeekBlock));
	block->assignments = assignments;
	return block;
}

RecurringBlock * RecurringBlockSemanticAction(StringList * order, IntegerList * days) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RecurringBlock * block = calloc(1, sizeof(RecurringBlock));
	block->order = order;
	block->days = days;
	return block;
}

Schedule * ScheduleFromWeekSemanticAction(WeekBlock * week) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Schedule * schedule = calloc(1, sizeof(Schedule));
	schedule->type = SCHEDULE_WEEK;
	schedule->week = week;
	return schedule;
}

Schedule * ScheduleFromRecurringSemanticAction(RecurringBlock * recurring) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Schedule * schedule = calloc(1, sizeof(Schedule));
	schedule->type = SCHEDULE_RECURRING;
	schedule->recurring = recurring;
	return schedule;
}

/* Program. */

Program * ProgramSemanticAction(WorkoutList * workouts, Schedule * schedule) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->workouts = workouts;
	program->schedule = schedule;
	if (_compilerState != NULL) {
		_compilerState->abstractSyntaxtTree = program;
	}
	return program;
}
