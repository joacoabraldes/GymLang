%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */
	int integer;
	char * string;
	TokenLabel token;

	/** Non-terminals. */
	Program * program;
	WorkoutList * workoutList;
	Workout * workout;
	Warmup * warmup;
	ItemList * itemList;
	Item * item;
	Exercise * exercise;
	ExerciseList * exerciseList;
	Superset * superset;
	AttributeList * attributeList;
	ExerciseAttribute * attribute;
	ValueItem * valueItem;
	ValueItemList * valueItemList;
	IntegerList * integerList;
	StringList * stringList;
	ExerciseTypeAst exerciseType;
	DayOfWeek dayOfWeek;
	Schedule * schedule;
	WeekBlock * weekBlock;
	RecurringBlock * recurringBlock;
	WeekAssignment * weekAssignment;
	WeekAssignmentList * weekAssignmentList;
}

/* Terminals. */

%token <integer> INTEGER
%token <string> STRING
%token <string> STRENGTH
%token <string> CARDIO
%token <string> HYPERTROPHY

%token <token> WORKOUT
%token <token> EXERCISE
%token <token> SUPERSET
%token <token> WARMUP
%token <token> WEEK
%token <token> RECURRING

%token <token> NAME_KEY
%token <token> DESCRIPTION_KEY
%token <token> TYPE_KEY
%token <token> SETS_KEY
%token <token> REPS_KEY
%token <token> WEIGHT_KEY
%token <token> REST_KEY
%token <token> TEMPO_KEY
%token <token> DURATION_KEY
%token <token> INTENSITY_KEY
%token <token> DISTANCE_KEY
%token <token> DROPSET_KEY
%token <token> DROPSET_REPS_KEY
%token <token> ROUNDS_KEY
%token <token> ORDER_KEY
%token <token> DAYS_KEY

%token <token> MON_KEY
%token <token> TUE_KEY
%token <token> WED_KEY
%token <token> THU_KEY
%token <token> FRI_KEY
%token <token> SAT_KEY
%token <token> SUN_KEY

%token <token> OPEN_BRACE
%token <token> CLOSE_BRACE
%token <token> EQUALS
%token <token> SEMICOLON
%token <token> COMMA
%token <token> DASH

%token <token> IGNORED
%token <token> UNKNOWN

/* Non-terminals. */

%type <program> program
%type <workoutList> workouts
%type <workout> workout
%type <string> name_attr
%type <string> description_opt
%type <warmup> warmup_opt
%type <warmup> warmup
%type <exerciseList> exercise_list
%type <itemList> items
%type <item> item
%type <exercise> exercise
%type <exerciseType> exercise_type
%type <attributeList> exercise_attrs
%type <attribute> exercise_attr
%type <valueItem> value_item
%type <valueItemList> value_item_list
%type <integerList> integer_list
%type <stringList> string_list
%type <string> string_val
%type <superset> superset
%type <exerciseList> extra_exercises
%type <schedule> schedule_opt
%type <schedule> schedule
%type <weekBlock> week_block
%type <weekAssignment> week_assignment
%type <weekAssignmentList> week_assignments
%type <dayOfWeek> day_key
%type <recurringBlock> recurring_block

/* Cleanup destructors for error recovery / AST shutdown. */

%destructor { if ($$ != NULL) free($$); } <string>
%destructor { destroyValueItem($$); } <valueItem>
%destructor { destroyValueItemList($$); } <valueItemList>
%destructor { destroyIntegerList($$); } <integerList>
%destructor { destroyStringList($$); } <stringList>
%destructor { destroyExerciseAttribute($$); } <attribute>
%destructor { destroyAttributeList($$); } <attributeList>
%destructor { destroyExercise($$); } <exercise>
%destructor { destroyExerciseList($$); } <exerciseList>
%destructor { destroyWarmup($$); } <warmup>
%destructor { destroySuperset($$); } <superset>
%destructor { destroyItem($$); } <item>
%destructor { destroyItemList($$); } <itemList>
%destructor { destroyWorkout($$); } <workout>
%destructor { destroyWorkoutList($$); } <workoutList>
%destructor { destroyWeekAssignment($$); } <weekAssignment>
%destructor { destroyWeekAssignmentList($$); } <weekAssignmentList>
%destructor { destroyWeekBlock($$); } <weekBlock>
%destructor { destroyRecurringBlock($$); } <recurringBlock>
%destructor { destroySchedule($$); } <schedule>

%%

program
	: workouts schedule_opt												{ $$ = ProgramSemanticAction($1, $2); }
	;

workouts
	: workout															{ $$ = WorkoutListSingleSemanticAction($1); }
	| workouts workout													{ $$ = WorkoutListAppendSemanticAction($1, $2); }
	;

workout
	: WORKOUT OPEN_BRACE name_attr description_opt warmup_opt items CLOSE_BRACE
																		{ $$ = WorkoutSemanticAction($3, $4, $5, $6); }
	;

name_attr
	: NAME_KEY EQUALS string_val SEMICOLON								{ $$ = $3; }
	;

description_opt
	: %empty															{ $$ = NULL; }
	| DESCRIPTION_KEY EQUALS string_val SEMICOLON						{ $$ = $3; }
	;

warmup_opt
	: %empty															{ $$ = NULL; }
	| warmup															{ $$ = $1; }
	;

warmup
	: WARMUP OPEN_BRACE exercise_list CLOSE_BRACE						{ $$ = WarmupSemanticAction($3); }
	;

exercise_list
	: exercise															{ $$ = ExerciseListSingleSemanticAction($1); }
	| exercise_list exercise											{ $$ = ExerciseListAppendSemanticAction($1, $2); }
	;

items
	: item																{ $$ = ItemListSingleSemanticAction($1); }
	| items item														{ $$ = ItemListAppendSemanticAction($1, $2); }
	;

item
	: exercise															{ $$ = ItemFromExerciseSemanticAction($1); }
	| superset															{ $$ = ItemFromSupersetSemanticAction($1); }
	;

exercise
	: EXERCISE OPEN_BRACE TYPE_KEY EQUALS exercise_type SEMICOLON exercise_attrs CLOSE_BRACE
																		{ $$ = ExerciseSemanticAction($5, $7); }
	;

exercise_type
	: STRENGTH															{ free($1); $$ = EXERCISE_STRENGTH; }
	| CARDIO															{ free($1); $$ = EXERCISE_CARDIO; }
	| HYPERTROPHY														{ free($1); $$ = EXERCISE_HYPERTROPHY; }
	;

exercise_attrs
	: exercise_attr														{ $$ = AttributeListSingleSemanticAction($1); }
	| exercise_attrs exercise_attr										{ $$ = AttributeListAppendSemanticAction($1, $2); }
	;

exercise_attr
	: NAME_KEY EQUALS string_val SEMICOLON								{ $$ = StringAttributeSemanticAction(ATTR_NAME, $3); }
	| SETS_KEY EQUALS INTEGER SEMICOLON									{ $$ = IntegerAttributeSemanticAction(ATTR_SETS, $3); }
	| REPS_KEY EQUALS value_item_list SEMICOLON							{ $$ = ValueListAttributeSemanticAction(ATTR_REPS, $3); }
	| WEIGHT_KEY EQUALS value_item_list SEMICOLON						{ $$ = ValueListAttributeSemanticAction(ATTR_WEIGHT, $3); }
	| REST_KEY EQUALS INTEGER SEMICOLON									{ $$ = IntegerAttributeSemanticAction(ATTR_REST, $3); }
	| TEMPO_KEY EQUALS INTEGER DASH INTEGER DASH INTEGER DASH INTEGER SEMICOLON
																		{ $$ = TempoAttributeSemanticAction($3, $5, $7, $9); }
	| DURATION_KEY EQUALS INTEGER SEMICOLON								{ $$ = IntegerAttributeSemanticAction(ATTR_DURATION, $3); }
	| INTENSITY_KEY EQUALS INTEGER SEMICOLON							{ $$ = IntegerAttributeSemanticAction(ATTR_INTENSITY, $3); }
	| DISTANCE_KEY EQUALS INTEGER SEMICOLON								{ $$ = IntegerAttributeSemanticAction(ATTR_DISTANCE, $3); }
	| DROPSET_KEY EQUALS integer_list SEMICOLON							{ $$ = IntegerListAttributeSemanticAction(ATTR_DROPSET, $3); }
	| DROPSET_REPS_KEY EQUALS integer_list SEMICOLON					{ $$ = IntegerListAttributeSemanticAction(ATTR_DROPSET_REPS, $3); }
	;

value_item_list
	: value_item														{ $$ = ValueItemListSingleSemanticAction($1); }
	| value_item_list COMMA value_item									{ $$ = ValueItemListAppendSemanticAction($1, $3); }
	;

value_item
	: INTEGER															{ $$ = ValueItemSingleSemanticAction($1); }
	| INTEGER DASH INTEGER												{ $$ = ValueItemRangeSemanticAction($1, $3); }
	;

integer_list
	: INTEGER															{ $$ = IntegerListSingleSemanticAction($1); }
	| integer_list COMMA INTEGER										{ $$ = IntegerListAppendSemanticAction($1, $3); }
	;

string_list
	: string_val														{ $$ = StringListSingleSemanticAction($1); }
	| string_list COMMA string_val										{ $$ = StringListAppendSemanticAction($1, $3); }
	;

string_val
	: STRING															{ $$ = $1; }
	| STRENGTH															{ $$ = $1; }
	| CARDIO															{ $$ = $1; }
	| HYPERTROPHY														{ $$ = $1; }
	;

superset
	: SUPERSET OPEN_BRACE ROUNDS_KEY EQUALS INTEGER SEMICOLON exercise exercise extra_exercises CLOSE_BRACE
																		{ $$ = SupersetSemanticAction($5, false, 0, $7, $8, $9); }
	| SUPERSET OPEN_BRACE ROUNDS_KEY EQUALS INTEGER SEMICOLON REST_KEY EQUALS INTEGER SEMICOLON exercise exercise extra_exercises CLOSE_BRACE
																		{ $$ = SupersetSemanticAction($5, true, $9, $11, $12, $13); }
	;

extra_exercises
	: %empty															{ $$ = NULL; }
	| extra_exercises exercise											{ $$ = ($1 == NULL) ? ExerciseListSingleSemanticAction($2) : ExerciseListAppendSemanticAction($1, $2); }
	;

schedule_opt
	: %empty															{ $$ = NULL; }
	| schedule															{ $$ = $1; }
	;

schedule
	: week_block														{ $$ = ScheduleFromWeekSemanticAction($1); }
	| recurring_block													{ $$ = ScheduleFromRecurringSemanticAction($1); }
	;

week_block
	: WEEK OPEN_BRACE week_assignments CLOSE_BRACE						{ $$ = WeekBlockSemanticAction($3); }
	;

week_assignments
	: week_assignment													{ $$ = WeekAssignmentListSingleSemanticAction($1); }
	| week_assignments week_assignment									{ $$ = WeekAssignmentListAppendSemanticAction($1, $2); }
	;

week_assignment
	: day_key EQUALS string_val SEMICOLON								{ $$ = WeekAssignmentSemanticAction($1, $3); }
	;

day_key
	: MON_KEY															{ $$ = DAY_MON; }
	| TUE_KEY															{ $$ = DAY_TUE; }
	| WED_KEY															{ $$ = DAY_WED; }
	| THU_KEY															{ $$ = DAY_THU; }
	| FRI_KEY															{ $$ = DAY_FRI; }
	| SAT_KEY															{ $$ = DAY_SAT; }
	| SUN_KEY															{ $$ = DAY_SUN; }
	;

recurring_block
	: RECURRING OPEN_BRACE ORDER_KEY EQUALS string_list SEMICOLON DAYS_KEY EQUALS integer_list SEMICOLON CLOSE_BRACE
																		{ $$ = RecurringBlockSemanticAction($5, $9); }
	;

%%
