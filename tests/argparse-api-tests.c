/*
   @mindmaze_header@
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "mmargparse.h"
#include "mmpredefs.h"

#include <check.h>
#include <stdlib.h>

#define LOREM_IPSUM "Lorem ipsum dolor sit amet, consectetur adipiscing"   \
"elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." \
"Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi " \
"ut aliquip ex ea commodo consequat..."

#define NARGV_MAX	10

struct argv_case {
	char* argv[NARGV_MAX];
	const char* expval;
};


/**
 * argv_len() - return the number of argument in a NULL terminated array
 * @argv:       NULL terminated array of argument strings
 *
 * Return: the number of element in @argv, excluding NULL termination
 */
static
int argv_len(char* argv[])
{
	int len = 0;

	while (argv[len] != NULL)
		len++;

	return len;
}

/**************************************************************************
 *                                                                        *
 *                           argument parsing tests                       *
 *                                                                        *
 **************************************************************************/
#define STRVAL_UNSET    u8"unset_value"
#define STRVAL_DEFAULT  u8"default_value"
#define STRVAL1         u8"skljfhls"
#define STRVAL2         u8"é(è-d--(è"
#define STRVAL3         u8"!:;mm"
#define STRVAL4         u8"µ%POPIP"

static
struct mmarg_opt cmdline_optv[] = {
	{"d|distractor", MMOPT_OPTSTR, "default_distractor", {NULL}, NULL},
	{"s|set", MMOPT_OPTSTR, STRVAL_DEFAULT, {NULL}, NULL},
};

static const
struct argv_case parsing_order_cases[] = {
	{{"prg_name", "-s", STRVAL1, NULL}, STRVAL1},
	{{"prg_name", "-s", STRVAL1, "-s", STRVAL2, NULL}, STRVAL2},
	{{"prg_name", "-s", STRVAL1, "-s", NULL}, STRVAL_DEFAULT},
	{{"prg_name", "--set="STRVAL3, NULL}, STRVAL3},
	{{"prg_name", "--set", STRVAL3, NULL}, STRVAL_DEFAULT},
	{{"prg_name", "-s", STRVAL1, "--set="STRVAL3, NULL}, STRVAL3},
	{{"prg_name", "-s", STRVAL1, "an_argument", "--set="STRVAL3, NULL}, STRVAL1},
	{{"prg_name", "-s", STRVAL1, "--", "--set="STRVAL3, NULL}, STRVAL1},
	{{"prg_name", "--set="STRVAL3, "-s", STRVAL4, NULL}, STRVAL4},
	{{"prg_name", "-d", STRVAL1, "-s", STRVAL2, NULL}, STRVAL2},
	{{"prg_name", "-s", STRVAL1, "-d", STRVAL2, NULL}, STRVAL1},
	{{"prg_name", "-s", STRVAL1, "--set="STRVAL4, NULL}, STRVAL4},
	{{"prg_name", NULL}, STRVAL_UNSET},
};

static
int parse_option_cb(const struct mmarg_opt* opt, union mmarg_val value, void* data)
{
	const char** strval = data;

	if (mmarg_opt_get_key(opt) == 's')
		*strval = value.str;

	return 0;
}


START_TEST(parsing_order_cb)
{
	const char* str_value = STRVAL_UNSET;
	struct mmarg_parser parser = {
		.optv = cmdline_optv,
		.num_opt = MM_NELEM(cmdline_optv),
		.cb = parse_option_cb,
		.cb_data = &str_value,
	};
	int arg_index, argc;
	const struct argv_case* case_data = &parsing_order_cases[_i];
	char** argv = (char**)case_data->argv;

	argc =  argv_len(argv);
	arg_index = mmarg_parse(&parser, argc, argv);

	ck_assert_int_ge(arg_index, 0);
	ck_assert_str_eq(str_value, case_data->expval);
}
END_TEST

/**************************************************************************
 *                                                                        *
 *                        argument validation tests                       *
 *                                                                        *
 **************************************************************************/
static
struct {
	long long ll;
	unsigned long long ull;
	int i;
	unsigned int ui;
	const char* str;
} optdata = {0};

static
struct mmarg_opt argval_valid_tests_optv[] = {
	{"set-ll", MMOPT_NEEDLLONG, NULL, {.llptr = &optdata.ll}, NULL},
	{"set-ull", MMOPT_NEEDULLONG, NULL, {.ullptr = &optdata.ull},
	 "Use this option to ull to @VAL_ULL. Recall value is @VAL_ULL."},
	{"i|set-i", MMOPT_NEEDINT, NULL, {.iptr = &optdata.i}, NULL},
	{"set-ui", MMOPT_NEEDUINT, NULL, {.uiptr = &optdata.ui}, NULL},
	{"set-str", MMOPT_NEEDSTR, NULL, {.sptr = &optdata.str}, NULL},
};


static const
struct argv_case help_argv_cases[] = {
	{.argv = {"prg_name", "-h"}},
	{.argv = {"prg_name", "--help"}},
	{.argv = {"prg_name", "--help", "hello"}},
	{.argv = {"prg_name", "--set-ll=-1", "--help", "hello"}},
	{.argv = {"prg_name", "-h", "hello"}},
	{.argv = {"prg_name", "--set-ll=-1", "-h", "hello"}},
};

START_TEST(print_help)
{
	struct mmarg_parser parser = {
		.flags = MMARG_PARSER_NOEXIT,
		.doc = LOREM_IPSUM,
		.optv = argval_valid_tests_optv,
		.num_opt = MM_NELEM(argval_valid_tests_optv),
	};
	int argc, rv;
	char** argv = (char**)help_argv_cases[_i].argv;

	argc = argv_len(argv);
	rv = mmarg_parse(&parser, argc, argv);

	ck_assert_int_eq(rv, MMARGPARSE_STOP);
}
END_TEST

#define INT_TOOBIG      "2147483648"            // INT32_MAX+1
#define INT_TOOSMALL    "-2147483649"           // INT32_MIN-1
#define UINT_TOOBIG     "4294967296"            // UINT32_MAX+1
#define LLONG_TOOBIG    "9223372036854775808"   // INT64_MAX+1
#define LLONG_TOOSMALL  "-9223372036854775809"  // INT64_MIN-1
#define ULLONG_TOOBIG   "18446744073709551616"  // UINT64_MAX+1

static const
struct argv_case error_argv_cases[] = {
	{.argv = {"prg_name", "-k"}},
	{.argv = {"prg_name", "-i"}},
	{.argv = {"prg_name", "-i", "-o"}},
	{.argv = {"prg_name", "---set-ll=-1"}},
	{.argv = {"prg_name", "--unknown-opt"}},
	{.argv = {"prg_name", "--set-ll=-1",  "--unknown-opt"}},
	{.argv = {"prg_name", "-i", "42",  "--unknown-opt"}},
	{.argv = {"prg_name", "--set-i=not_a_number"}},
	{.argv = {"prg_name", "--set-i=21_noise"}},
	{.argv = {"prg_name", "--set-i="INT_TOOBIG}},
	{.argv = {"prg_name", "--set-i="INT_TOOSMALL}},
	{.argv = {"prg_name", "--set-ui="UINT_TOOBIG}},
	{.argv = {"prg_name", "--set-ui=-1"}},
	{.argv = {"prg_name", "--set-ll="LLONG_TOOBIG}},
	{.argv = {"prg_name", "--set-ll="LLONG_TOOSMALL}},
	{.argv = {"prg_name", "--set-ull="ULLONG_TOOBIG}},
	{.argv = {"prg_name", "--set-ull=-1"}},
};

START_TEST(parsing_error)
{
	struct mmarg_parser parser = {
		.flags = MMARG_PARSER_NOEXIT,
		.optv = argval_valid_tests_optv,
		.num_opt = MM_NELEM(argval_valid_tests_optv),
	};
	int argc, rv;
	char** argv = (char**)error_argv_cases[_i].argv;

	argc = argv_len(argv);
	rv = mmarg_parse(&parser, argc, argv);

	ck_assert_int_eq(rv, MMARGPARSE_ERROR);
}
END_TEST


static const
struct argv_case success_argv_cases[] = {
	{.argv = {"prg_name", "an_arg", "--unknown-opt"}, "an_arg"},
	{.argv = {"prg_name", "--", "--unknown-opt"}, "--unknown-opt"},
	{.argv = {"prg_name", "-", "--unknown-opt"}, "-"},
	{.argv = {"prg_name", "-i", "42", "--", "--unknown-opt"}, "--unknown-opt"},
	{.argv = {"prg_name", "-i", "42", "another --arg"}, "another --arg"},
};

START_TEST(parsing_success)
{
	struct mmarg_parser parser = {
		.flags = MMARG_PARSER_NOEXIT,
		.optv = argval_valid_tests_optv,
		.num_opt = MM_NELEM(argval_valid_tests_optv),
	};
	int argc, rv;
	char** argv = (char**)success_argv_cases[_i].argv;

	argc = argv_len(argv);
	rv = mmarg_parse(&parser, argc, argv);

	ck_assert_int_ge(rv, 0);
	ck_assert_str_eq(argv[rv], success_argv_cases[_i].expval);
}
END_TEST


/**************************************************************************
 *                                                                        *
 *                  option name and shortkey parsing tests                *
 *                                                                        *
 **************************************************************************/
struct {
	struct mmarg_opt opt;
	char key;
	char* name;
} parse_optname_cases [] = {
	{{.name = "d"}, .key = 'd', .name = NULL},
	{{.name = "choice"}, .key = 0, .name = "choice"},
	{{.name = "d|choice"}, .key = 'd', .name = "choice"},
	{{.name = "a-choice"}, .key = 0, .name = "a-choice"},
	{{.name = "a|a-choice"}, .key = 'a', .name = "a-choice"},
};


START_TEST(get_key)
{
	int key;
	const struct mmarg_opt* opt = &parse_optname_cases[_i].opt;

	key = mmarg_opt_get_key(opt);

	ck_assert_int_eq(key, parse_optname_cases[_i].key);
}
END_TEST


START_TEST(get_name)
{
	const char *name, *ref_name;
	const struct mmarg_opt* opt = &parse_optname_cases[_i].opt;

	name = mmarg_opt_get_name(opt);
	ref_name = parse_optname_cases[_i].name;

	if (ref_name)
		ck_assert_str_eq(name, ref_name);
	else
		ck_assert(name == NULL);
}
END_TEST


/**************************************************************************
 *                                                                        *
 *                          Test suite setup                              *
 *                                                                        *
 **************************************************************************/
LOCAL_SYMBOL
TCase* create_argparse_tcase(void)
{
	TCase *tc = tcase_create("argparse");

	tcase_add_loop_test(tc, get_key, 0, MM_NELEM(parse_optname_cases));
	tcase_add_loop_test(tc, get_name, 0, MM_NELEM(parse_optname_cases));
	tcase_add_loop_test(tc, parsing_order_cb, 0, MM_NELEM(parsing_order_cases));
	tcase_add_loop_test(tc, print_help, 0, MM_NELEM(help_argv_cases));
	tcase_add_loop_test(tc, parsing_error, 0, MM_NELEM(error_argv_cases));
	tcase_add_loop_test(tc, parsing_success, 0, MM_NELEM(success_argv_cases));

	return tc;
}