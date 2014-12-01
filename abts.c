/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modify by vical@126.com 2013/11/4
 *
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <io.h>
#define DELIMITER '\\'
#else
#include <unistd.h>
#define DELIMITER '/'
#endif

#if defined(_MSC_VER)
#include <winsock2.h>
#define isatty _isatty
#define fileno _fileno
#define strdup _strdup
#pragma warning(disable:4996)
#else
#include <sys/time.h>
#endif

#include "abts.h"

struct abts_suite {
    char *name;
    int num_test;
    int num_failed;
    int not_run;
    int not_impl;
    struct abts_suite *next;
};

struct abts_case {
    int failed;
    abts_suite *parent;
};

#define ABTS_STAT_SIZE 6
static char status[ABTS_STAT_SIZE] = {'|', '/', '-', '|', '\\', '-'};
static int curr_char;
static int verbose = 0;
static int exclude = 0;
static int quiet = 0;
static int list_tests = 0;

static char **testnames = NULL;

static FILE *outfp = NULL;
static FILE *errfp = NULL;

/* Determine if the test should be run at all */
static int should_test_run(const char *testname) {
    int i, found = 0;
    if (list_tests == 1) {
        return 0;
    }
    if (testnames == NULL) { /* run all */
        return 1;
    }
	
	for (i = 0; testnames[i] != NULL; i++) {
        if (!strcmp(testnames[i], testname)) {
           found = 1;
		   break;
        }
    }
    if ((found && !exclude) || (!found && exclude)) {
        return 1;
    }
    return 0;
}

static void reset_status(void)
{
    curr_char = 0;
}

static void update_status(void)
{
    if (!quiet) {
        curr_char = (curr_char + 1) % ABTS_STAT_SIZE;
        fprintf(outfp, "\b%c", status[curr_char]);
        fflush(outfp);
    }
}

static void end_suite(abts_suite *suite)
{
    if (suite != NULL) {
        abts_suite *last = suite;
        if (!quiet) {
            fprintf(outfp, "\b");
            fflush(outfp);
        }
        if (last->num_failed == 0) {
            fprintf(outfp, "SUCCESS\n");
            fflush(outfp);
        }
        else {
            fprintf(outfp, "FAILED %d of %d\n", last->num_failed, last->num_test);
            fflush(outfp);
        }
    }
}

/* return the tail pointer */
abts_suite *abts_add_suite(abts_suite *suite, const char *suite_name_full)
{
    abts_suite *newsuite;
    char *p;
    const char *suite_name;
    curr_char = 0;
    
    /* Only end the suite if we actually ran it */
    if (suite && !suite->not_run) {
        end_suite(suite);
    }

    newsuite = malloc(sizeof(*newsuite));
    newsuite->num_test = 0;
    newsuite->num_failed = 0;
    newsuite->next = newsuite;
    /* suite_name_full may be an absolute path depending on __FILE__ expansion */
    suite_name = strrchr(suite_name_full, DELIMITER);
    if (suite_name) {
        suite_name++;
    } else {
	    suite_name = suite_name_full;
    }
    p = strrchr(suite_name, '.');
    if (p) {
        newsuite->name = memcpy(calloc(p - suite_name + 1, 1),
                                suite_name, p - suite_name);
    }
    else {
        newsuite->name = strdup(suite_name);
    }

    if (list_tests) {
        fprintf(outfp, "%s\n", newsuite->name);
    }
    
    newsuite->not_run = 0;

	/* append to tail */
    if (suite) {
    	newsuite->next = suite->next;
    	suite->next = newsuite;
    }    
	suite = newsuite;

    if (!should_test_run(newsuite->name)) {
        newsuite->not_run = 1;
        return suite;
    }

    reset_status();
    fprintf(outfp, "%-20s:  ", newsuite->name);
    update_status();
    fflush(outfp);

    return suite;
}

void abts_run_test(abts_suite *ts, abts_case_fntype f, void *arg)
{
    abts_case tc;

    if (!should_test_run(ts->name)) {
        return;
    }
    tc.failed = 0;
    tc.parent = ts;
    
    ts->num_test++;
    update_status();

    f(&tc, arg);
    
    if (tc.failed) {
        ts->num_failed++;
    }
}

static int report(abts_suite *suite)
{
    int count = 0;
    abts_suite *dptr;
    
    if (suite && !suite->not_run) {
        end_suite(suite);
    }

	if (suite) {
		for (dptr = suite->next; dptr != suite; dptr = dptr->next) {
	    	count += dptr->num_failed;
	    }
	    count += suite->num_failed;
    }

    if (list_tests) {
        return 0;
    }

    if (count == 0) {
        printf("All tests passed.\n");
        return 0;
    }

	dptr = suite ? suite->next : NULL;
    fprintf(outfp, "%-15s\t\tTotal\tFail\tFailed %%\n", "Failed Tests");
    fprintf(outfp, "===================================================\n");
    while (dptr != suite) {
        if (dptr->num_failed != 0) {
            float percent = ((float)dptr->num_failed / (float)dptr->num_test);
            fprintf(outfp, "%-15s\t\t%5d\t%4d\t%6.2f%%\n", dptr->name, 
                    dptr->num_test, dptr->num_failed, percent * 100);
        }
        dptr = dptr->next;
    }
    if (dptr && dptr->num_failed != 0) {
        float percent = ((float)dptr->num_failed / (float)dptr->num_test);
        fprintf(outfp, "%-15s\t\t%5d\t%4d\t%6.2f%%\n", dptr->name, 
                dptr->num_test, dptr->num_failed, percent * 100);
    }
    return 1;
}

void abts_log_message(const char *fmt, ...)
{
    va_list args;
    update_status();

    if (verbose) {
        va_start(args, fmt);
        vfprintf(errfp, fmt, args);
        va_end(args);
        fprintf(errfp, "\n");
        fflush(errfp);
    }
}
 
void abts_assert(abts_case *tc, int condition, const char *message, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (condition) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(errfp, "Line %d: %s\n", lineno, message ? message : "NULL");
        fflush(errfp);
    }
}

void abts_not_impl(abts_case *tc, const char *message, int lineno)
{
    update_status();

    tc->parent->not_impl++;
    if (verbose) {
        fprintf(errfp, "Line %d: %s\n", lineno,  message ? message : "NULL");
        fflush(errfp);
    }
}

int abts_main(int argc, char *argv[], abts_suite_fntype fns[]) 
{
    int i;
    int rv=0;
    int list_provided = 0;
    abts_suite *suite = NULL, *ps, *psbak;
   
	verbose = 1;

	outfp = stdout;
	errfp = stderr;

    quiet = !isatty(fileno(outfp));

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            verbose = 1;
            continue;
        }
        if (!strcmp(argv[i], "-x")) {
            exclude = 1;
            continue;
        }
        if (!strcmp(argv[i], "-l")) {
            list_tests = 1;
            continue;
        }
        if (!strcmp(argv[i], "-q")) {
            quiet = 1;
            continue;
        }
		if (!strcmp(argv[i], "-f")) {
			if (++i == argc) {
				fprintf(errfp, "Failed! option '-f' require a file name.\n", argv[i]);
				exit(1);
			}
			quiet = 1;
			outfp = fopen (argv[i], "a");
			if (!outfp) {
				fprintf (errfp, "Failed to open output file: '%s'\n", argv[i]);
				exit (1);
			}
			errfp = outfp;
			continue;
		}
		
        if (argv[i][0] == '-') {
            fprintf(errfp, "Invalid option: '%s'\n", argv[i]);
            exit(1);
        }
        list_provided = 1;
    }

    if (list_provided) {
        /* Waste a little space here, because it is easier than counting the
         * number of tests listed.  Besides it is at most three char *.
         */
        testnames = calloc(argc + 1, sizeof(char *));
        for (i = 1; i < argc; i++) {
            testnames[i - 1] = argv[i];
        }
    }

    for (i = 0; fns[i]; i++) {
        suite = fns[i] (suite);/* add_suite, run_test */
    }
    
	if (suite) {
		rv = report(suite);/* report all result */
	}

	if (outfp && outfp != stdout) {
		fclose(outfp);
	}

	if (suite) {
		for (ps=suite->next; ps!=suite && (psbak=ps->next, 1); ps = psbak) {
			free (ps->name);
			free (ps);
		}
		free (ps->name);
		free (ps);		
	}
	if (testnames) {
		free (testnames);
	}
	
    return rv;
}

unsigned abts_tick()
{
#if defined(_MSC_VER)
    return (unsigned)GetTickCount();

#else
	struct timeval tv={0};
	gettimeofday(&tv, NULL);
	return (unsigned)(tv.tv_sec*1000+tv.tv_usec / 1000);
	
#endif
}

