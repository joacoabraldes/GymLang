#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyValueItem(ValueItem * valueItem) {
	if (valueItem != NULL) {
		free(valueItem);
	}
}

void destroyValueItemList(ValueItemList * list) {
	while (list != NULL) {
		ValueItemList * next = list->next;
		destroyValueItem(list->item);
		free(list);
		list = next;
	}
}

void destroyIntegerList(IntegerList * list) {
	while (list != NULL) {
		IntegerList * next = list->next;
		free(list);
		list = next;
	}
}

void destroyStringList(StringList * list) {
	while (list != NULL) {
		StringList * next = list->next;
		if (list->value != NULL) {
			free(list->value);
		}
		free(list);
		list = next;
	}
}

void destroyExerciseAttribute(ExerciseAttribute * attribute) {
	if (attribute == NULL) {
		return;
	}
	switch (attribute->valueType) {
		case VAL_STRING:
			if (attribute->stringValue != NULL) {
				free(attribute->stringValue);
			}
			break;
		case VAL_INTEGER_LIST:
			destroyIntegerList(attribute->intList);
			break;
		case VAL_VALUE_LIST:
			destroyValueItemList(attribute->valueList);
			break;
		case VAL_INTEGER:
		case VAL_TEMPO:
		default:
			break;
	}
	free(attribute);
}

void destroyAttributeList(AttributeList * list) {
	while (list != NULL) {
		AttributeList * next = list->next;
		destroyExerciseAttribute(list->attribute);
		free(list);
		list = next;
	}
}

void destroyExercise(Exercise * exercise) {
	if (exercise == NULL) {
		return;
	}
	destroyAttributeList(exercise->attributes);
	free(exercise);
}

void destroyExerciseList(ExerciseList * list) {
	while (list != NULL) {
		ExerciseList * next = list->next;
		destroyExercise(list->exercise);
		free(list);
		list = next;
	}
}

void destroyWarmup(Warmup * warmup) {
	if (warmup == NULL) {
		return;
	}
	destroyExerciseList(warmup->exercises);
	free(warmup);
}

void destroySuperset(Superset * superset) {
	if (superset == NULL) {
		return;
	}
	destroyExerciseList(superset->exercises);
	free(superset);
}

void destroyItem(Item * item) {
	if (item == NULL) {
		return;
	}
	switch (item->type) {
		case ITEM_EXERCISE:
			destroyExercise(item->exercise);
			break;
		case ITEM_SUPERSET:
			destroySuperset(item->superset);
			break;
	}
	free(item);
}

void destroyItemList(ItemList * list) {
	while (list != NULL) {
		ItemList * next = list->next;
		destroyItem(list->item);
		free(list);
		list = next;
	}
}

void destroyWorkout(Workout * workout) {
	if (workout == NULL) {
		return;
	}
	if (workout->name != NULL) {
		free(workout->name);
	}
	if (workout->description != NULL) {
		free(workout->description);
	}
	destroyWarmup(workout->warmup);
	destroyItemList(workout->items);
	free(workout);
}

void destroyWorkoutList(WorkoutList * list) {
	while (list != NULL) {
		WorkoutList * next = list->next;
		destroyWorkout(list->workout);
		free(list);
		list = next;
	}
}

void destroyWeekAssignment(WeekAssignment * assignment) {
	if (assignment == NULL) {
		return;
	}
	if (assignment->workoutName != NULL) {
		free(assignment->workoutName);
	}
	free(assignment);
}

void destroyWeekAssignmentList(WeekAssignmentList * list) {
	while (list != NULL) {
		WeekAssignmentList * next = list->next;
		destroyWeekAssignment(list->assignment);
		free(list);
		list = next;
	}
}

void destroyWeekBlock(WeekBlock * block) {
	if (block == NULL) {
		return;
	}
	destroyWeekAssignmentList(block->assignments);
	free(block);
}

void destroyRecurringBlock(RecurringBlock * block) {
	if (block == NULL) {
		return;
	}
	destroyStringList(block->order);
	destroyIntegerList(block->days);
	free(block);
}

void destroySchedule(Schedule * schedule) {
	if (schedule == NULL) {
		return;
	}
	switch (schedule->type) {
		case SCHEDULE_WEEK:
			destroyWeekBlock(schedule->week);
			break;
		case SCHEDULE_RECURRING:
			destroyRecurringBlock(schedule->recurring);
			break;
	}
	free(schedule);
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program == NULL) {
		return;
	}
	destroyWorkoutList(program->workouts);
	destroySchedule(program->schedule);
	free(program);
}
