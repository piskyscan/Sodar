/*
Command line processing for Sodar
 */

#include <stdlib.h>
#include <error.h>
#include <argp.h>
#include <argz.h>
#include "Sodar.h"

const char *argp_program_version = "Sodar 1.0";
const char *argp_program_bug_address =
		"<piskyscan@piskyscan.com>";

/* Program documentation. */
static char doc[] =
		"Sodar -- program to try to measure sound direction\nUses phase difference between two microphones to estimate direction";

/* A description of the arguments we accept. */
static char args_doc[] = "";

//"-d device -h hertz -f frame_size -w width_apart -t time_to_run -i time_to_ignore -c correlation";

/* Keys for options without short-options. */
#define OPT_ABORT  1            /* –abort */

/* The options we understand. */
static struct argp_option options[] =
{
		{"device",   'd', "DEVICE", OPTION_ARG_OPTIONAL,"Device to use (default)"},
		{"hertz",   'h', "HERTZ", OPTION_ARG_OPTIONAL,"Hertz to run at (44000)"},
		{"frames",   'f', "FRAMES", OPTION_ARG_OPTIONAL, "Frane Size (default 4096)"},
		{"width",   'w', "WIDTH", OPTION_ARG_OPTIONAL, "Speaker distance apart"},
		{"time",   't', "TIME", OPTION_ARG_OPTIONAL,"Time to run for (forever)"},
		{"ignore",   'i', "IGNORE", OPTION_ARG_OPTIONAL,"Initial time to ignore (0.3s)"},
		{"correlation",   'c', "CORRELATION", OPTION_ARG_OPTIONAL,"Minimum correlation to report (0.8)"},
		{"output",   'o', "FILE",  0,"Output to FILE instead of standard output" },
		{ 0 }
};


/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key)
	{
	case 'r':
		arguments->repeat_count = arg ? atoi (arg) : 10;
		break;


	case 'd':
		arguments->device = arg ? arg : "default";
		break;

	case 'h':
		arguments->hertz = arg ? atoi(arg) : 44000;
		break;

	case 'f':
		arguments->frames = arg ? atoi(arg) : 4096;
		break;

	case 'w':
		arguments->width = arg ? atoi(arg) : 50;
		break;

	case 'i':
		arguments->ignore = arg ? atof(arg) : 0.3;
		break;

	case 'c':
		arguments->correlation = arg ? atof(arg) : 0.8;
		break;


	case OPT_ABORT:
		arguments->abort = 1;
		break;

	case ARGP_KEY_NO_ARGS:
		// no args is fine
		// argp_usage (state);
		break;

	case ARGP_KEY_ARG:
		/* Here we know that state->arg_num == 0, since we
         force argument parsing to end before any more arguments can
         get here. */
		arguments->arg1 = arg;

		/* Now we consume all the rest of the arguments.
         state->next is the index in state->argv of the
         next argument to be parsed, which is the first string
         we’re interested in, so we can just use
         &state->argv[state->next] as the value for
         arguments->strings.

         In addition, by setting state->next to the end
         of the arguments, we can force argp to stop parsing here and
         return. */
		arguments->strings = &state->argv[state->next];
		state->next = state->argc;

		break;

	default:
		argp_usage (state);
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, 0 /* args_doc */,0 /* doc*/ };

int main (int argc, char **argv)
{
	int i, j;
	struct arguments arguments;

	/* Default values. */
	arguments.silent = 0;
	arguments.verbose = 0;
	arguments.output_file = "-";
	arguments.repeat_count = 1;
	arguments.abort = 0;
	arguments.device = "default";
	arguments.hertz = 44000;
	arguments.frames = 4096;
	arguments.correlation = 0.8;
	arguments.width = 50;
	arguments.time = 0.3;
	double ignore;



	/* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
	if (argp_parse (&argp, argc, argv, 0, 0, &arguments))
	{
		exit(1);
	}

	if (arguments.abort)
		error (10, 0, "ABORTED");

	main_process(&arguments);


	exit (0);
}
