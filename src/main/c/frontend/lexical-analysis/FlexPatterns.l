%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus BraceLexemeAction(TokenLabel label);
extern CompilationStatus EnterValueLexemeAction(FlexContext context);
extern CompilationStatus LeaveValueLexemeAction();
extern CompilationStatus IntegerLexemeAction();
extern CompilationStatus StringLexemeAction();
extern CompilationStatus TypeKeywordLexemeAction(TokenLabel label);
extern CompilationStatus CommaLexemeAction();
extern CompilationStatus DashLexemeAction();
extern CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus LeaveMultilineCommentLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus UnknownLexemeAction();
extern CompilationStatus EOFLexemeAction();

%}

/**
 * Flex contexts (start conditions).
 */
%x VALUE
%x MULTILINE_COMMENT

/**
 * Reusable patterns.
 */
whitespace			[[:space:]]+

%%

	/* Multiline comments — valid in any context. */
"/*"										{ return EnterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"						{ return LeaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+					{ return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>.						{ return IgnoredLexemeAction(); }

	/* Block keywords (INITIAL). */
"workout"									{ return KeywordLexemeAction(WORKOUT); }
"exercise"									{ return KeywordLexemeAction(EXERCISE); }
"superset"									{ return KeywordLexemeAction(SUPERSET); }
"warmup"									{ return KeywordLexemeAction(WARMUP); }
"week"										{ return KeywordLexemeAction(WEEK); }
"recurring"									{ return KeywordLexemeAction(RECURRING); }

	/* Attribute keys (INITIAL).
	   Order matters when one is a prefix of another (e.g. dropset_reps before dropset). */
"dropset_reps"								{ return KeywordLexemeAction(DROPSET_REPS_KEY); }
"dropset"									{ return KeywordLexemeAction(DROPSET_KEY); }
"description"								{ return KeywordLexemeAction(DESCRIPTION_KEY); }
"name"										{ return KeywordLexemeAction(NAME_KEY); }
"type"										{ return KeywordLexemeAction(TYPE_KEY); }
"sets"										{ return KeywordLexemeAction(SETS_KEY); }
"reps"										{ return KeywordLexemeAction(REPS_KEY); }
"weight"									{ return KeywordLexemeAction(WEIGHT_KEY); }
"rest"										{ return KeywordLexemeAction(REST_KEY); }
"tempo"										{ return KeywordLexemeAction(TEMPO_KEY); }
"duration"									{ return KeywordLexemeAction(DURATION_KEY); }
"intensity"									{ return KeywordLexemeAction(INTENSITY_KEY); }
"distance"									{ return KeywordLexemeAction(DISTANCE_KEY); }
"rounds"									{ return KeywordLexemeAction(ROUNDS_KEY); }
"order"										{ return KeywordLexemeAction(ORDER_KEY); }
"days"										{ return KeywordLexemeAction(DAYS_KEY); }

	/* Day-of-week keys (INITIAL). */
"mon"										{ return KeywordLexemeAction(MON_KEY); }
"tue"										{ return KeywordLexemeAction(TUE_KEY); }
"wed"										{ return KeywordLexemeAction(WED_KEY); }
"thu"										{ return KeywordLexemeAction(THU_KEY); }
"fri"										{ return KeywordLexemeAction(FRI_KEY); }
"sat"										{ return KeywordLexemeAction(SAT_KEY); }
"sun"										{ return KeywordLexemeAction(SUN_KEY); }

	/* Block delimiters and value entry. */
"{"											{ return BraceLexemeAction(OPEN_BRACE); }
"}"											{ return BraceLexemeAction(CLOSE_BRACE); }
"="											{ return EnterValueLexemeAction(VALUE); }

	/* VALUE context. */
<VALUE>";"									{ return LeaveValueLexemeAction(); }
<VALUE>","									{ return CommaLexemeAction(); }
<VALUE>"-"									{ return DashLexemeAction(); }
<VALUE>[0-9]+								{ return IntegerLexemeAction(); }
<VALUE>"strength"							{ return TypeKeywordLexemeAction(STRENGTH); }
<VALUE>"cardio"								{ return TypeKeywordLexemeAction(CARDIO); }
<VALUE>"hypertrophy"						{ return TypeKeywordLexemeAction(HYPERTROPHY); }
<VALUE>[A-Za-z][^,;{}=]*					{ return StringLexemeAction(); }
<VALUE>{whitespace}							{ return IgnoredLexemeAction(); }
<VALUE>.									{ return UnknownLexemeAction(); }

	/* Whitespace and unknown — INITIAL. */
{whitespace}								{ return IgnoredLexemeAction(); }
.											{ return UnknownLexemeAction(); }

<<EOF>>										{ return EOFLexemeAction(); }

%%

#include "FlexActions.h"
#include "FlexExport.h"
