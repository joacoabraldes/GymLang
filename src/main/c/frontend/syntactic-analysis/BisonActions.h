#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/* Value primitives. */

ValueItem * ValueItemSingleSemanticAction(int value);
ValueItem * ValueItemRangeSemanticAction(int from, int to);

ValueItemList * ValueItemListSingleSemanticAction(ValueItem * item);
ValueItemList * ValueItemListAppendSemanticAction(ValueItemList * list, ValueItem * item);

IntegerList * IntegerListSingleSemanticAction(int value);
IntegerList * IntegerListAppendSemanticAction(IntegerList * list, int value);

StringList * StringListSingleSemanticAction(char * value);
StringList * StringListAppendSemanticAction(StringList * list, char * value);

/* Exercise attributes. */

ExerciseAttribute * StringAttributeSemanticAction(AttributeKey key, char * value);
ExerciseAttribute * IntegerAttributeSemanticAction(AttributeKey key, int value);
ExerciseAttribute * ValueListAttributeSemanticAction(AttributeKey key, ValueItemList * list);
ExerciseAttribute * IntegerListAttributeSemanticAction(AttributeKey key, IntegerList * list);
ExerciseAttribute * TempoAttributeSemanticAction(int eccentric, int pause1, int concentric, int pause2);

AttributeList * AttributeListSingleSemanticAction(ExerciseAttribute * attribute);
AttributeList * AttributeListAppendSemanticAction(AttributeList * list, ExerciseAttribute * attribute);

/* Exercise / superset / warmup / items. */

Exercise * ExerciseSemanticAction(ExerciseTypeAst type, AttributeList * attributes);
ExerciseList * ExerciseListSingleSemanticAction(Exercise * exercise);
ExerciseList * ExerciseListAppendSemanticAction(ExerciseList * list, Exercise * exercise);

Warmup * WarmupSemanticAction(ExerciseList * exercises);

Superset * SupersetSemanticAction(int rounds, bool hasRest, int rest, Exercise * first, Exercise * second, ExerciseList * extras);

Item * ItemFromExerciseSemanticAction(Exercise * exercise);
Item * ItemFromSupersetSemanticAction(Superset * superset);

ItemList * ItemListSingleSemanticAction(Item * item);
ItemList * ItemListAppendSemanticAction(ItemList * list, Item * item);

/* Workouts. */

Workout * WorkoutSemanticAction(char * name, char * description, Warmup * warmup, ItemList * items);
WorkoutList * WorkoutListSingleSemanticAction(Workout * workout);
WorkoutList * WorkoutListAppendSemanticAction(WorkoutList * list, Workout * workout);

/* Schedule. */

WeekAssignment * WeekAssignmentSemanticAction(DayOfWeek day, char * workoutName);
WeekAssignmentList * WeekAssignmentListSingleSemanticAction(WeekAssignment * assignment);
WeekAssignmentList * WeekAssignmentListAppendSemanticAction(WeekAssignmentList * list, WeekAssignment * assignment);

WeekBlock * WeekBlockSemanticAction(WeekAssignmentList * assignments);
RecurringBlock * RecurringBlockSemanticAction(StringList * order, IntegerList * days);

Schedule * ScheduleFromWeekSemanticAction(WeekBlock * week);
Schedule * ScheduleFromRecurringSemanticAction(RecurringBlock * recurring);

/* Program. */

Program * ProgramSemanticAction(WorkoutList * workouts, Schedule * schedule);

#endif
