#include "Generator.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdio.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule() {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

#define OUTPUT_PATH "output.html"
#define DEFAULT_REST_SECONDS 90
#define DEFAULT_SECONDS_PER_REP 4

typedef struct {
	long volume;
	int sets;
	long seconds;
} WorkoutStats;

/* PRIVATE FUNCTIONS */

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

static ExerciseAttribute * _findAttribute(const Exercise * exercise, AttributeKey key) {
	for (AttributeList * node = exercise->attributes; node != NULL; node = node->next) {
		if (node->attribute->key == key) {
			return node->attribute;
		}
	}
	return NULL;
}

static const char * _exerciseName(const Exercise * exercise) {
	ExerciseAttribute * name = _findAttribute(exercise, ATTR_NAME);
	return name != NULL ? name->stringValue : "Ejercicio";
}

static const char * _exerciseTypeName(ExerciseTypeAst type) {
	switch (type) {
		case EXERCISE_STRENGTH: return "strength";
		case EXERCISE_CARDIO: return "cardio";
		case EXERCISE_HYPERTROPHY: return "hypertrophy";
		default: return "unknown";
	}
}

static int _representativeValue(const ValueItem * item) {
	if (item->isRange) {
		return (item->from + item->to) / 2;
	}
	return item->from;
}

static int _valueAtIndex(const ValueItemList * list, int index) {
	int length = _valueItemListLength(list);
	if (length == 0) {
		return 0;
	}
	if (index >= length) {
		index = length - 1;
	}
	int current = 0;
	for (const ValueItemList * node = list; node != NULL; node = node->next, ++current) {
		if (current == index) {
			return _representativeValue(node->item);
		}
	}
	return 0;
}

static int _integerAtIndex(const IntegerList * list, int index) {
	int current = 0;
	const IntegerList * last = list;
	for (const IntegerList * node = list; node != NULL; node = node->next, ++current) {
		last = node;
		if (current == index) {
			return node->value;
		}
	}
	return last != NULL ? last->value : 0;
}

static void _accumulateExercise(const Exercise * exercise, int series, int restBetween, bool countVolume, WorkoutStats * stats) {
	if (exercise->type == EXERCISE_CARDIO) {
		ExerciseAttribute * duration = _findAttribute(exercise, ATTR_DURATION);
		if (duration != NULL) {
			stats->seconds += (long) duration->intValue * 60;
		}
		return;
	}
	ExerciseAttribute * reps = _findAttribute(exercise, ATTR_REPS);
	ExerciseAttribute * weight = _findAttribute(exercise, ATTR_WEIGHT);
	ExerciseAttribute * tempo = _findAttribute(exercise, ATTR_TEMPO);
	int secondsPerRep = DEFAULT_SECONDS_PER_REP;
	if (tempo != NULL) {
		Tempo value = tempo->tempo;
		int total = value.eccentric + value.pause1 + value.concentric + value.pause2;
		if (total > 0) {
			secondsPerRep = total;
		}
	}
	for (int set = 0; set < series; ++set) {
		int repetitions = reps != NULL ? _valueAtIndex(reps->valueList, set) : 0;
		int load = weight != NULL ? _valueAtIndex(weight->valueList, set) : 0;
		stats->seconds += (long) repetitions * secondsPerRep;
		if (countVolume) {
			stats->volume += (long) repetitions * load;
		}
	}
	if (series > 1) {
		stats->seconds += (long) (series - 1) * restBetween;
	}
	if (countVolume) {
		stats->sets += series;
	}
	ExerciseAttribute * dropset = _findAttribute(exercise, ATTR_DROPSET);
	if (countVolume && dropset != NULL) {
		ExerciseAttribute * dropsetReps = _findAttribute(exercise, ATTR_DROPSET_REPS);
		int lastReps = reps != NULL ? _valueAtIndex(reps->valueList, series - 1) : 0;
		int index = 0;
		for (IntegerList * node = dropset->intList; node != NULL; node = node->next, ++index) {
			int repetitions = dropsetReps != NULL ? _integerAtIndex(dropsetReps->intList, index) : lastReps;
			stats->volume += (long) repetitions * node->value;
			stats->seconds += (long) repetitions * secondsPerRep;
		}
	}
}

static int _exerciseSeries(const Exercise * exercise) {
	ExerciseAttribute * sets = _findAttribute(exercise, ATTR_SETS);
	return sets != NULL ? sets->intValue : 1;
}

static WorkoutStats _computeWorkoutStats(const Workout * workout) {
	WorkoutStats stats = {0, 0, 0};
	if (workout->warmup != NULL) {
		for (ExerciseList * node = workout->warmup->exercises; node != NULL; node = node->next) {
			_accumulateExercise(node->exercise, _exerciseSeries(node->exercise), 60, false, &stats);
		}
	}
	for (ItemList * node = workout->items; node != NULL; node = node->next) {
		Item * item = node->item;
		if (item->type == ITEM_EXERCISE) {
			ExerciseAttribute * rest = _findAttribute(item->exercise, ATTR_REST);
			int restBetween = rest != NULL ? rest->intValue : DEFAULT_REST_SECONDS;
			_accumulateExercise(item->exercise, _exerciseSeries(item->exercise), restBetween, true, &stats);
		}
		else {
			Superset * superset = item->superset;
			for (ExerciseList * inner = superset->exercises; inner != NULL; inner = inner->next) {
				_accumulateExercise(inner->exercise, superset->rounds, 0, true, &stats);
			}
			if (superset->rounds > 1) {
				stats.seconds += (long) (superset->rounds - 1) * (superset->hasRest ? superset->rest : DEFAULT_REST_SECONDS);
			}
		}
	}
	return stats;
}

/* HTML emission. */

static void _emitEscaped(FILE * out, const char * text) {
	if (text == NULL) {
		return;
	}
	for (const char * character = text; *character != '\0'; ++character) {
		switch (*character) {
			case '&': fputs("&amp;", out); break;
			case '<': fputs("&lt;", out); break;
			case '>': fputs("&gt;", out); break;
			case '"': fputs("&quot;", out); break;
			case '\'': fputs("&#39;", out); break;
			default: fputc((unsigned char) *character, out);
		}
	}
}

static void _emitJsonString(FILE * out, const char * text) {
	fputc('"', out);
	if (text != NULL) {
		for (const char * character = text; *character != '\0'; ++character) {
			switch (*character) {
				case '"': fputs("\\\"", out); break;
				case '\\': fputs("\\\\", out); break;
				case '<': fputs("\\u003c", out); break;
				case '\n': fputs("\\n", out); break;
				default: fputc((unsigned char) *character, out);
			}
		}
	}
	fputc('"', out);
}

static void _emitValueList(FILE * out, const ValueItemList * list) {
	bool first = true;
	for (const ValueItemList * node = list; node != NULL; node = node->next) {
		if (!first) {
			fputs(", ", out);
		}
		first = false;
		if (node->item->isRange) {
			fprintf(out, "%d-%d", node->item->from, node->item->to);
		}
		else {
			fprintf(out, "%d", node->item->from);
		}
	}
}

static void _emitIntegerList(FILE * out, const IntegerList * list) {
	bool first = true;
	for (const IntegerList * node = list; node != NULL; node = node->next) {
		if (!first) {
			fputs(", ", out);
		}
		first = false;
		fprintf(out, "%d", node->value);
	}
}

static void _emitAttributeRow(FILE * out, const char * label, const Exercise * exercise, AttributeKey key) {
	ExerciseAttribute * attribute = _findAttribute(exercise, key);
	if (attribute == NULL) {
		return;
	}
	fputs("<li><span class=\"key\">", out);
	_emitEscaped(out, label);
	fputs("</span><span class=\"value\">", out);
	switch (attribute->valueType) {
		case VAL_INTEGER:
			fprintf(out, "%d", attribute->intValue);
			break;
		case VAL_VALUE_LIST:
			_emitValueList(out, attribute->valueList);
			break;
		case VAL_INTEGER_LIST:
			_emitIntegerList(out, attribute->intList);
			break;
		case VAL_TEMPO:
			fprintf(out, "%d-%d-%d-%d", attribute->tempo.eccentric, attribute->tempo.pause1, attribute->tempo.concentric, attribute->tempo.pause2);
			break;
		case VAL_STRING:
			_emitEscaped(out, attribute->stringValue);
			break;
	}
	fputs("</span></li>\n", out);
}

static void _emitExercise(FILE * out, const Exercise * exercise, int series, bool inSuperset) {
	const char * type = _exerciseTypeName(exercise->type);
	fprintf(out, "<div class=\"exercise %s\">\n", type);
	fputs("<header class=\"exercise-head\"><span class=\"name\">", out);
	_emitEscaped(out, _exerciseName(exercise));
	fprintf(out, "</span><span class=\"badge badge-%s\">%s</span></header>\n", type, type);
	fputs("<ul class=\"attributes\">\n", out);
	_emitAttributeRow(out, "Series", exercise, ATTR_SETS);
	_emitAttributeRow(out, "Repeticiones", exercise, ATTR_REPS);
	_emitAttributeRow(out, "Peso (kg)", exercise, ATTR_WEIGHT);
	_emitAttributeRow(out, "Descanso (s)", exercise, ATTR_REST);
	_emitAttributeRow(out, "Tempo", exercise, ATTR_TEMPO);
	_emitAttributeRow(out, "Duracion (min)", exercise, ATTR_DURATION);
	_emitAttributeRow(out, "Intensidad", exercise, ATTR_INTENSITY);
	_emitAttributeRow(out, "Distancia (m)", exercise, ATTR_DISTANCE);
	_emitAttributeRow(out, "Dropset (kg)", exercise, ATTR_DROPSET);
	_emitAttributeRow(out, "Dropset reps", exercise, ATTR_DROPSET_REPS);
	fputs("</ul>\n", out);

	if (exercise->type != EXERCISE_CARDIO) {
		ExerciseAttribute * reps = _findAttribute(exercise, ATTR_REPS);
		ExerciseAttribute * weight = _findAttribute(exercise, ATTR_WEIGHT);
		ExerciseAttribute * rest = _findAttribute(exercise, ATTR_REST);
		const char * label = inSuperset ? "Ronda" : "Serie";
		fputs("<div class=\"sets\">\n", out);
		for (int set = 0; set < series; ++set) {
			int repetitions = reps != NULL ? _valueAtIndex(reps->valueList, set) : 0;
			fprintf(out, "<label class=\"set\"><input type=\"checkbox\"><span>%s %d &middot; %d reps", label, set + 1, repetitions);
			if (weight != NULL) {
				fprintf(out, " &middot; %d kg", _valueAtIndex(weight->valueList, set));
			}
			fputs("</span></label>\n", out);
		}
		fputs("</div>\n", out);
		if (!inSuperset && rest != NULL) {
			fprintf(out, "<button class=\"timer\" data-rest=\"%d\">Descanso %d s</button>\n", rest->intValue, rest->intValue);
		}
	}
	fputs("</div>\n", out);
}

static void _emitSuperset(FILE * out, const Superset * superset) {
	fputs("<div class=\"superset\">\n", out);
	if (superset->hasRest) {
		fprintf(out, "<header class=\"superset-head\">Superset &middot; %d rondas &middot; descanso %d s</header>\n", superset->rounds, superset->rest);
	}
	else {
		fprintf(out, "<header class=\"superset-head\">Superset &middot; %d rondas</header>\n", superset->rounds);
	}
	for (ExerciseList * node = superset->exercises; node != NULL; node = node->next) {
		_emitExercise(out, node->exercise, superset->rounds, true);
	}
	int restBetween = superset->hasRest ? superset->rest : 0;
	if (restBetween > 0) {
		fprintf(out, "<button class=\"timer\" data-rest=\"%d\">Descanso entre rondas %d s</button>\n", restBetween, restBetween);
	}
	fputs("</div>\n", out);
}

static void _emitWorkout(FILE * out, const Workout * workout) {
	fputs("<article class=\"workout\" data-name=\"", out);
	_emitEscaped(out, workout->name);
	fputs("\">\n<h2>", out);
	_emitEscaped(out, workout->name);
	fputs("</h2>\n", out);
	if (workout->description != NULL) {
		fputs("<p class=\"description\">", out);
		_emitEscaped(out, workout->description);
		fputs("</p>\n", out);
	}
	if (workout->warmup != NULL) {
		fputs("<section class=\"block warmup\"><h3>Entrada en calor</h3>\n", out);
		for (ExerciseList * node = workout->warmup->exercises; node != NULL; node = node->next) {
			_emitExercise(out, node->exercise, _exerciseSeries(node->exercise), false);
		}
		fputs("</section>\n", out);
	}
	fputs("<section class=\"block main\"><h3>Bloque principal</h3>\n", out);
	for (ItemList * node = workout->items; node != NULL; node = node->next) {
		if (node->item->type == ITEM_EXERCISE) {
			_emitExercise(out, node->item->exercise, _exerciseSeries(node->item->exercise), false);
		}
		else {
			_emitSuperset(out, node->item->superset);
		}
	}
	fputs("</section>\n", out);

	WorkoutStats stats = _computeWorkoutStats(workout);
	fputs("<footer class=\"summary\">\n", out);
	fprintf(out, "<div><span class=\"metric\">%d</span><span class=\"caption\">series</span></div>\n", stats.sets);
	fprintf(out, "<div><span class=\"metric\">%ld</span><span class=\"caption\">kg de volumen</span></div>\n", stats.volume);
	fprintf(out, "<div><span class=\"metric\">%ld</span><span class=\"caption\">min estimados</span></div>\n", (stats.seconds + 59) / 60);
	fputs("</footer>\n</article>\n", out);
}

static const char * _weekdayLabel(int index) {
	switch (index) {
		case 0: return "Lunes";
		case 1: return "Martes";
		case 2: return "Miercoles";
		case 3: return "Jueves";
		case 4: return "Viernes";
		case 5: return "Sabado";
		case 6: return "Domingo";
		default: return "";
	}
}

static void _buildWeekSchedule(const WeekBlock * week, const char * schedule[7]) {
	for (int day = 0; day < 7; ++day) {
		schedule[day] = NULL;
	}
	for (WeekAssignmentList * node = week->assignments; node != NULL; node = node->next) {
		schedule[node->assignment->day] = node->assignment->workoutName;
	}
}

static void _buildRecurringSchedule(const RecurringBlock * recurring, const char * schedule[7]) {
	int orderLength = 0;
	const char * order[64];
	for (StringList * node = recurring->order; node != NULL && orderLength < 64; node = node->next) {
		order[orderLength++] = node->value;
	}
	int training = recurring->days != NULL ? recurring->days->value : 0;
	int rest = (recurring->days != NULL && recurring->days->next != NULL) ? recurring->days->next->value : 0;
	int cycle = training + rest;
	for (int day = 0; day < 7; ++day) {
		int position = cycle > 0 ? day % cycle : 0;
		if (cycle > 0 && position < training && orderLength > 0) {
			int completedCycles = day / cycle;
			schedule[day] = order[(completedCycles * training + position) % orderLength];
		}
		else {
			schedule[day] = NULL;
		}
	}
}

static void _emitScheduleSection(FILE * out, ScheduleType type, const char * schedule[7]) {
	fputs("<section class=\"schedule\">", out);
	if (type == SCHEDULE_WEEK) {
		fputs("<h2>Planificacion semanal</h2>", out);
	}
	else {
		fputs("<h2>Planificacion recurrente</h2>", out);
	}
	fputs("<p id=\"today\" class=\"today\"></p>\n<div class=\"week\">\n", out);
	for (int day = 0; day < 7; ++day) {
		fprintf(out, "<div class=\"day-cell\" data-index=\"%d\"><span class=\"day\">", day);
		if (type == SCHEDULE_WEEK) {
			fputs(_weekdayLabel(day), out);
		}
		else {
			fprintf(out, "Dia %d", day + 1);
		}
		fputs("</span><span class=\"target\">", out);
		if (schedule[day] != NULL) {
			_emitEscaped(out, schedule[day]);
		}
		else {
			fputs("Descanso", out);
		}
		fputs("</span></div>\n", out);
	}
	fputs("</div></section>\n", out);
}

static void _emitHead(FILE * out) {
	fputs(
		"<!DOCTYPE html>\n"
		"<html lang=\"es\">\n"
		"<head>\n"
		"<meta charset=\"UTF-8\">\n"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"<title>GymLang</title>\n"
		"<style>\n"
		":root{--bg:#0f1115;--card:#191c24;--soft:#222633;--text:#e6e8ee;--muted:#8b93a7;--strength:#4c8dff;--cardio:#ff8c42;--hypertrophy:#3ec98a;--accent:#ffd166;}\n"
		"*{box-sizing:border-box;}\n"
		"body{margin:0;font-family:'Segoe UI',system-ui,sans-serif;background:var(--bg);color:var(--text);}\n"
		"header.top{position:sticky;top:0;z-index:10;display:flex;flex-wrap:wrap;gap:16px;align-items:center;justify-content:space-between;padding:16px 24px;background:#0c0e12;border-bottom:1px solid var(--soft);}\n"
		"header.top h1{margin:0;font-size:22px;letter-spacing:1px;}\n"
		".clock-box{display:flex;flex-direction:column;align-items:flex-end;gap:4px;font-size:13px;color:var(--muted);}\n"
		".clock-box #clock{font-size:18px;color:var(--accent);font-variant-numeric:tabular-nums;}\n"
		".speed{display:flex;align-items:center;gap:8px;}\n"
		"main{max-width:1024px;margin:0 auto;padding:24px;}\n"
		"h2{font-size:20px;margin:28px 0 12px;}\n"
		"h3{font-size:15px;text-transform:uppercase;letter-spacing:1px;color:var(--muted);margin:18px 0 10px;}\n"
		".schedule .week{display:grid;grid-template-columns:repeat(7,minmax(100px,1fr));gap:10px;}\n"
		".today{font-size:15px;color:var(--accent);font-weight:600;margin:4px 0 12px;}\n"
		".day-cell{background:var(--card);border:1px solid var(--soft);border-radius:10px;padding:12px;display:flex;flex-direction:column;gap:6px;}\n"
		".day-cell .day{font-size:12px;color:var(--muted);text-transform:uppercase;}\n"
		".day-cell .target{font-weight:600;}\n"
		".day-cell.active{border-color:var(--accent);box-shadow:0 0 0 1px var(--accent);}\n"
		".workout{background:var(--card);border:1px solid var(--soft);border-radius:14px;padding:20px;margin:18px 0;}\n"
		".workout h2{margin-top:0;}\n"
		".description{color:var(--muted);margin:4px 0 12px;}\n"
		".exercise{background:var(--soft);border-radius:10px;padding:14px;margin:10px 0;border-left:4px solid var(--muted);}\n"
		".exercise.strength{border-left-color:var(--strength);}\n"
		".exercise.cardio{border-left-color:var(--cardio);}\n"
		".exercise.hypertrophy{border-left-color:var(--hypertrophy);}\n"
		".exercise-head{display:flex;align-items:center;justify-content:space-between;gap:8px;}\n"
		".exercise-head .name{font-weight:600;font-size:16px;}\n"
		".badge{font-size:11px;text-transform:uppercase;letter-spacing:1px;padding:2px 8px;border-radius:999px;color:#0c0e12;}\n"
		".badge-strength{background:var(--strength);}\n"
		".badge-cardio{background:var(--cardio);}\n"
		".badge-hypertrophy{background:var(--hypertrophy);}\n"
		".attributes{list-style:none;margin:10px 0 0;padding:0;display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:4px 16px;}\n"
		".attributes li{display:flex;justify-content:space-between;font-size:13px;border-bottom:1px dotted #333a4a;padding:2px 0;}\n"
		".attributes .key{color:var(--muted);}\n"
		".sets{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px;}\n"
		".set{display:flex;align-items:center;gap:6px;background:#10131a;padding:6px 10px;border-radius:8px;font-size:13px;cursor:pointer;}\n"
		".set input:checked + span{text-decoration:line-through;color:var(--muted);}\n"
		".superset{border:1px dashed #3a4156;border-radius:10px;padding:12px;margin:10px 0;}\n"
		".superset-head{font-weight:600;color:var(--accent);margin-bottom:6px;}\n"
		".timer{margin-top:10px;background:var(--accent);color:#0c0e12;border:none;border-radius:8px;padding:8px 14px;font-weight:600;cursor:pointer;}\n"
		".timer.running{background:var(--cardio);}\n"
		".summary{display:flex;gap:24px;margin-top:16px;padding-top:14px;border-top:1px solid var(--soft);}\n"
		".summary .metric{font-size:22px;font-weight:700;display:block;}\n"
		".summary .caption{font-size:12px;color:var(--muted);}\n"
		".rest-banner{background:var(--card);border:1px solid var(--soft);border-radius:14px;padding:48px 24px;margin:18px 0;text-align:center;font-size:20px;color:var(--muted);}\n"
		"[hidden]{display:none !important;}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<header class=\"top\">\n"
		"<h1>GymLang</h1>\n"
		"<div class=\"speed\"><label for=\"speed\">Velocidad</label><input id=\"speed\" type=\"range\" min=\"1\" max=\"1440\" value=\"60\"><span id=\"speedValue\">60 min/s</span></div>\n"
		"<div class=\"clock-box\"><span>Reloj simulado</span><span id=\"clock\">--</span></div>\n"
		"</header>\n"
		"<main>\n",
		out);
}

static void _emitScripts(FILE * out, ScheduleType type, bool hasSchedule, const char * schedule[7]) {
	fputs("<script>\n", out);
	fputs("var GYM = {type:", out);
	if (!hasSchedule) {
		fputs("\"none\"", out);
	}
	else if (type == SCHEDULE_WEEK) {
		fputs("\"week\"", out);
	}
	else {
		fputs("\"recurring\"", out);
	}
	fputs(",schedule:[", out);
	if (hasSchedule) {
		for (int day = 0; day < 7; ++day) {
			if (day > 0) {
				fputc(',', out);
			}
			if (schedule[day] != NULL) {
				_emitJsonString(out, schedule[day]);
			}
			else {
				fputs("null", out);
			}
		}
	}
	fputs("]};\n", out);
	fputs(
		"(function(){\n"
		"var speed=document.getElementById('speed');\n"
		"var speedValue=document.getElementById('speedValue');\n"
		"var clock=document.getElementById('clock');\n"
		"var today=document.getElementById('today');\n"
		"var restBanner=document.getElementById('rest-banner');\n"
		"var workouts=Array.prototype.slice.call(document.querySelectorAll('.workout'));\n"
		"var days=['Lunes','Martes','Miercoles','Jueves','Viernes','Sabado','Domingo'];\n"
		"var minutes=0;var last=null;var timers=[];var currentDay=-1;\n"
		"function setupTimers(){\n"
		"  document.querySelectorAll('.timer').forEach(function(button){\n"
		"    button.addEventListener('click',function(){\n"
		"      var seconds=parseInt(button.getAttribute('data-rest'),10);\n"
		"      timers.push({button:button,remaining:seconds,label:button.textContent});\n"
		"      button.classList.add('running');\n"
		"    });\n"
		"  });\n"
		"}\n"
		"function updateTimers(elapsedSeconds){\n"
		"  for(var i=timers.length-1;i>=0;i--){\n"
		"    var timer=timers[i];timer.remaining-=elapsedSeconds;\n"
		"    if(timer.remaining<=0){timer.button.textContent=timer.label;timer.button.classList.remove('running');timers.splice(i,1);}\n"
		"    else{timer.button.textContent='Descansando '+Math.ceil(timer.remaining)+' s';}\n"
		"  }\n"
		"}\n"
		"function applyDay(index){\n"
		"  document.querySelectorAll('.day-cell').forEach(function(cell){\n"
		"    cell.classList.toggle('active',parseInt(cell.getAttribute('data-index'),10)===index);\n"
		"  });\n"
		"  var target=GYM.schedule[index];\n"
		"  if(target){\n"
		"    workouts.forEach(function(card){card.hidden=card.getAttribute('data-name')!==target;});\n"
		"    if(restBanner){restBanner.hidden=true;}\n"
		"    if(today){today.textContent='Hoy: '+target;}\n"
		"  }else{\n"
		"    workouts.forEach(function(card){card.hidden=true;});\n"
		"    if(restBanner){restBanner.hidden=false;}\n"
		"    if(today){today.textContent='Hoy toca descansar';}\n"
		"  }\n"
		"}\n"
		"function format(){\n"
		"  var totalDays=Math.floor(minutes/1440);var minuteOfDay=Math.floor(minutes%1440);\n"
		"  var hh=String(Math.floor(minuteOfDay/60)).padStart(2,'0');var mm=String(minuteOfDay%60).padStart(2,'0');\n"
		"  return days[((totalDays%7)+7)%7]+' '+hh+':'+mm;\n"
		"}\n"
		"function loop(now){\n"
		"  if(last===null){last=now;}\n"
		"  var deltaSeconds=(now-last)/1000;last=now;\n"
		"  var perSecond=parseFloat(speed.value);\n"
		"  speedValue.textContent=perSecond+' min/s';\n"
		"  minutes+=deltaSeconds*perSecond;\n"
		"  clock.textContent=format();\n"
		"  if(GYM.type!=='none'){\n"
		"    var index=((Math.floor(minutes/1440)%7)+7)%7;\n"
		"    if(index!==currentDay){currentDay=index;applyDay(index);}\n"
		"  }\n"
		"  updateTimers(deltaSeconds*perSecond*60);\n"
		"  requestAnimationFrame(loop);\n"
		"}\n"
		"setupTimers();\n"
		"if(GYM.type!=='none'){applyDay(0);currentDay=0;}\n"
		"requestAnimationFrame(loop);\n"
		"})();\n"
		"</script>\n"
		"</body>\n"
		"</html>\n",
		out);
}

/* PUBLIC FUNCTIONS */

void executeGenerator(CompilerState * compilerState) {
	Program * program = (Program *) compilerState->abstractSyntaxtTree;
	if (program == NULL) {
		logError(_logger, "There is no program to generate.");
		return;
	}
	FILE * out = fopen(OUTPUT_PATH, "w");
	if (out == NULL) {
		logError(_logger, "Could not open the output file: %s.", OUTPUT_PATH);
		return;
	}
	_emitHead(out);
	const char * schedule[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	bool hasSchedule = program->schedule != NULL;
	ScheduleType type = SCHEDULE_WEEK;
	if (hasSchedule) {
		type = program->schedule->type;
		if (type == SCHEDULE_WEEK) {
			_buildWeekSchedule(program->schedule->week, schedule);
		}
		else {
			_buildRecurringSchedule(program->schedule->recurring, schedule);
		}
		_emitScheduleSection(out, type, schedule);
		fputs("<div id=\"rest-banner\" class=\"rest-banner\" hidden>Hoy toca descansar</div>\n", out);
	}
	fputs("<section class=\"workouts\">\n", out);
	for (WorkoutList * node = program->workouts; node != NULL; node = node->next) {
		_emitWorkout(out, node->workout);
	}
	fputs("</section>\n</main>\n", out);
	_emitScripts(out, type, hasSchedule, schedule);
	fclose(out);
	logInformation(_logger, "Generated %s.", OUTPUT_PATH);
}
