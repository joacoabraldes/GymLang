#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/* Forward typedefs (mutual references). */

typedef enum ExerciseTypeAst ExerciseTypeAst;
typedef enum DayOfWeek DayOfWeek;
typedef enum AttributeKey AttributeKey;
typedef enum AttributeValueType AttributeValueType;
typedef enum ItemType ItemType;
typedef enum ScheduleType ScheduleType;

typedef struct ValueItem ValueItem;
typedef struct Tempo Tempo;
typedef struct ValueItemList ValueItemList;
typedef struct IntegerList IntegerList;
typedef struct StringList StringList;
typedef struct ExerciseAttribute ExerciseAttribute;
typedef struct AttributeList AttributeList;
typedef struct Exercise Exercise;
typedef struct ExerciseList ExerciseList;
typedef struct Warmup Warmup;
typedef struct Superset Superset;
typedef struct Item Item;
typedef struct ItemList ItemList;
typedef struct Workout Workout;
typedef struct WorkoutList WorkoutList;
typedef struct WeekAssignment WeekAssignment;
typedef struct WeekAssignmentList WeekAssignmentList;
typedef struct WeekBlock WeekBlock;
typedef struct RecurringBlock RecurringBlock;
typedef struct Schedule Schedule;
typedef struct Program Program;

/* Enumerations. */

enum ExerciseTypeAst {
	EXERCISE_STRENGTH,
	EXERCISE_CARDIO,
	EXERCISE_HYPERTROPHY
};

enum DayOfWeek {
	DAY_MON,
	DAY_TUE,
	DAY_WED,
	DAY_THU,
	DAY_FRI,
	DAY_SAT,
	DAY_SUN
};

enum AttributeKey {
	ATTR_NAME,
	ATTR_SETS,
	ATTR_REPS,
	ATTR_WEIGHT,
	ATTR_REST,
	ATTR_TEMPO,
	ATTR_DURATION,
	ATTR_INTENSITY,
	ATTR_DISTANCE,
	ATTR_DROPSET,
	ATTR_DROPSET_REPS
};

enum AttributeValueType {
	VAL_INTEGER,
	VAL_STRING,
	VAL_INTEGER_LIST,
	VAL_VALUE_LIST,
	VAL_TEMPO
};

enum ItemType {
	ITEM_EXERCISE,
	ITEM_SUPERSET
};

enum ScheduleType {
	SCHEDULE_WEEK,
	SCHEDULE_RECURRING
};

/* Value primitives. */

struct ValueItem {
	int from;
	int to;
	bool isRange;
};

struct Tempo {
	int eccentric;
	int pause1;
	int concentric;
	int pause2;
};

/* Linked-list nodes. */

struct ValueItemList {
	ValueItem * item;
	ValueItemList * next;
};

struct IntegerList {
	int value;
	IntegerList * next;
};

struct StringList {
	char * value;
	StringList * next;
};

/* Exercise attributes. */

struct ExerciseAttribute {
	AttributeKey key;
	AttributeValueType valueType;
	union {
		int intValue;
		char * stringValue;
		IntegerList * intList;
		ValueItemList * valueList;
		Tempo tempo;
	};
};

struct AttributeList {
	ExerciseAttribute * attribute;
	AttributeList * next;
};

/* Exercise. */

struct Exercise {
	ExerciseTypeAst type;
	AttributeList * attributes;
};

struct ExerciseList {
	Exercise * exercise;
	ExerciseList * next;
};

/* Workout building blocks. */

struct Warmup {
	ExerciseList * exercises;
};

struct Superset {
	int rounds;
	bool hasRest;
	int rest;
	ExerciseList * exercises;
};

struct Item {
	ItemType type;
	union {
		Exercise * exercise;
		Superset * superset;
	};
};

struct ItemList {
	Item * item;
	ItemList * next;
};

struct Workout {
	char * name;
	char * description;
	Warmup * warmup;
	ItemList * items;
};

struct WorkoutList {
	Workout * workout;
	WorkoutList * next;
};

/* Schedule blocks. */

struct WeekAssignment {
	DayOfWeek day;
	char * workoutName;
};

struct WeekAssignmentList {
	WeekAssignment * assignment;
	WeekAssignmentList * next;
};

struct WeekBlock {
	WeekAssignmentList * assignments;
};

struct RecurringBlock {
	StringList * order;
	IntegerList * days;
};

struct Schedule {
	ScheduleType type;
	union {
		WeekBlock * week;
		RecurringBlock * recurring;
	};
};

/* Program (root). */

struct Program {
	WorkoutList * workouts;
	Schedule * schedule;
};

/* Recursive destructors. */

void destroyValueItem(ValueItem * valueItem);
void destroyValueItemList(ValueItemList * list);
void destroyIntegerList(IntegerList * list);
void destroyStringList(StringList * list);
void destroyExerciseAttribute(ExerciseAttribute * attribute);
void destroyAttributeList(AttributeList * list);
void destroyExercise(Exercise * exercise);
void destroyExerciseList(ExerciseList * list);
void destroyWarmup(Warmup * warmup);
void destroySuperset(Superset * superset);
void destroyItem(Item * item);
void destroyItemList(ItemList * list);
void destroyWorkout(Workout * workout);
void destroyWorkoutList(WorkoutList * list);
void destroyWeekAssignment(WeekAssignment * assignment);
void destroyWeekAssignmentList(WeekAssignmentList * list);
void destroyWeekBlock(WeekBlock * block);
void destroyRecurringBlock(RecurringBlock * block);
void destroySchedule(Schedule * schedule);
void destroyProgram(Program * program);

#endif
