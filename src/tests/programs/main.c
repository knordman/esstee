/*
Copyright (C) 2015 Kristian Nordman

This file is part of esstee. 

esstee is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

esstee is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with esstee.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Loads a program, runs queries pre-run, runs N cycles, runs queries
 * post-run */

#include <esstee/esstee.h>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
extern int yydebug;

#define BISON_DEBUG 1
#define PRE_QUERIES 2
#define POST_QUERIES 3
#define PROGRAM 4
#define RUN_CYCLES 5
#define FILE 6
#define QUIET_PRE_RUN 7

static struct option long_options[] = {
    {"bison-debug", no_argument, NULL, BISON_DEBUG},
    {"pre-run-queries", required_argument, NULL, PRE_QUERIES},
    {"quiet-pre-run", no_argument, NULL, QUIET_PRE_RUN},
    {"post-run-queries", required_argument, NULL, POST_QUERIES},
    {"program", required_argument, NULL, PROGRAM},
    {"run-cycles", required_argument, NULL, RUN_CYCLES},
    {"file", required_argument, NULL, FILE},
    {0, 0, 0, 0}
};


static void print_all_errors(struct st_t *st) {
    const struct st_issue_t *i = NULL;
    while((i = st_fetch_issue(st, ESSTEE_FILTER_ANY_ISSUE)) != NULL)
    {
	fprintf(stderr, "%s\n", i->message);
	if(i->has_sub_issues)
	{
	    const struct st_issue_t *si = NULL;
	    while((si = st_fetch_sub_issue(st, i, ESSTEE_FILTER_ANY_ISSUE)) != NULL)
	    {
		fprintf(stderr, "%s\n", si->message);
	    }
	}

	if(i->locations)
	{
	    struct st_location_t *itr = NULL;
	    for(itr = i->locations; itr != NULL; itr = itr->next)
	    {
		fprintf(stderr, "@ L(%d:%d) C(%d:%d)\n",
			itr->first_line,
			itr->last_line,
			itr->first_column,
			itr->last_column);
	    }
	}
    }
}

int main(int argc, char * const argv[])
{
    /* Default options */
    yydebug = 0;
    const char *program = NULL;
    const char *pre_run_queries = NULL;
    const char *post_run_queries = NULL;
    const char *file = NULL;
    int quiet_pre_run = 0;
    int run_cycles = 1;
    
    int argument_parsing = 1;
    while(argument_parsing)
    {
	int c = getopt_long_only(argc, argv, "", long_options, NULL);

	switch(c)
	{
	case -1:
	    argument_parsing = 0;
	    break;
	    
	case BISON_DEBUG:
	    yydebug = 1;
	    break;

	case PRE_QUERIES:
	    pre_run_queries = optarg;
	    break;

	case POST_QUERIES:
	    post_run_queries = optarg;
	    break;

	case PROGRAM:
	    program = optarg;
	    break;

	case RUN_CYCLES:
	    run_cycles = atoi(optarg);

	case FILE:
	    file = optarg;
	    break;

	case QUIET_PRE_RUN:
	    quiet_pre_run = 1;
	    break;
	    
	default:
	    break;
	}
    }

    /* Check if arguments are somewhat sane */
    if(!file) {
	fprintf(stderr, "no source file given (--file)\n");
	return EXIT_FAILURE;
    }

    if(!program) {
	fprintf(stderr, "no start program given (--program)\n");
	return EXIT_FAILURE;
    }

    /* Fire away */
    struct st_t *st = st_new_instance(1024);
    if(!st) {
	fprintf(stderr, "error creating esstee instance\n");
	return EXIT_FAILURE;
    }

    fprintf(stderr, "loading '%s' ... ", file);
    int load_result = st_load_file(st, file);
    if(load_result != ESSTEE_OK) {
	fprintf(stderr, "failed\n");
	print_all_errors(st);
	return EXIT_FAILURE;
    }
    else {
	fprintf(stderr, "ok\n");
    }

    int link_result = st_link(st);
    if(link_result != ESSTEE_OK) {
	print_all_errors(st);
	return EXIT_FAILURE;
    }

    const struct st_location_t *cursor = st_start(st, program);
    if(!cursor) {
	fprintf(stderr, "could not start '%s'\n", program);
	print_all_errors(st);
	return EXIT_FAILURE;
    }

    char output_buffer[1000];
    
    if(pre_run_queries) {
	fprintf(stderr, "running pre queries\n");
	int query_result = st_query(st,
				    output_buffer,
				    1000,
				    pre_run_queries);

	if(query_result < 0) {
	    print_all_errors(st);
	    return EXIT_FAILURE;
	}

	if(!quiet_pre_run)
	{
	    printf("%s\n", output_buffer);
	}
    }

    int cycle_result = ESSTEE_OK;
    for(int i = 0; i < run_cycles; i++) {
	cycle_result = st_run_cycle(st, 20);

	if(cycle_result != ESSTEE_OK)
	{
	    break;
	}
    }

    if(cycle_result != ESSTEE_OK)
    {
	print_all_errors(st);
	return EXIT_FAILURE;
    }
    
    if(post_run_queries) {
	fprintf(stderr, "running post queries\n");
	int query_result = st_query(st,
				    output_buffer,
				    1000,
				    post_run_queries);

	if(query_result < 0) {
	    print_all_errors(st);
	    return EXIT_FAILURE;
	}
	
	printf("%s\n", output_buffer);
    }

    return EXIT_SUCCESS;
}
