#include "../smartcuckoo.h"

#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <assert.h>
#include <errno.h>

#include <string>
#include "bench_common.h"

#include <inttypes.h>
#include <stdint.h>
#include <vector>
#include <chrono>

using namespace std;

/* default parameter settings */
static size_t key_len;
static size_t val_len;
static size_t num_queries;
static size_t num_records;
static char* loadfile = NULL;
static char* runfile = NULL;

typedef struct {
    size_t num_insert;
    size_t num_update;
    size_t num_read;
    double tput;
    double time;
} stat_t;

/* Calculate the second difference*/
static double timeval_diff(struct timeval *start, 
	struct timeval *end)
{
    double r = end->tv_sec - start->tv_sec;

    /* Calculate the microsecond difference */
    if (end->tv_usec > start->tv_usec)
	r += (end->tv_usec - start->tv_usec)/1000000.0;
    else if (end->tv_usec < start->tv_usec)
	r -= (start->tv_usec - end->tv_usec)/1000000.0;
    return r;
}

/* init all queries from the ycsb trace file before issuing them */
static query *load_trace(char* filename, int phase)
{
    FILE *input;

    input = fopen(filename, "rb");
    if (input == NULL) {
	perror("can not open file");
	perror(filename);
	exit(1);
    }

    int n;
    n = fread(&key_len, sizeof(key_len), 1, input);
    if (n != 1)
	perror("fread error");
    n = fread(&val_len, sizeof(val_len), 1, input);
    if (n != 1)
	perror("fread error");
    n = fread(&num_records, sizeof(num_records), 1, input);
    if (n != 1)
	perror("fread error");
    n = fread(&num_queries, sizeof(num_queries), 1, input);
    if (n != 1)
	perror("fread error");

    printf("trace(%s):\n", filename);
    printf("\tkey_len = %zu\n", key_len);
    printf("\tval_len = %zu\n", val_len);
    printf("\tnum_records = %zu\n", num_records);
    printf("\tnum_queries = %zu\n", num_queries);
    printf("\n");

    size_t cnt = 0;
    if (phase == 0) {
	cnt = num_records;
    } else {
	cnt = num_queries;
    }

    query *queries = (query *)malloc(sizeof(query) * cnt);
    if (queries == NULL) {
	perror("not enough memory to init queries\n");
	exit(-1);
    }

    size_t num_read;
    size_t offset = 0;
    while ( (num_read = fread(queries + sizeof(query) * offset, sizeof(query), cnt, input)) > 0) {
	    offset += num_read;
    }
    if (offset < cnt) {
	fprintf(stderr, "num_read: %zu\n", offset);
	perror("can not read all queries\n");
	fclose(input);
	exit(-1);
    }

    fclose(input);
    printf("queries_init...done\n");
    return queries;
}

void run_trace(query *queries, int phase)
{
    printf("run_trace...begin\n");

    struct timeval tv_s, tv_e;
    stat_t result;

    result.num_insert = 0;
    result.num_update = 0;
    result.num_read = 0;
    result.tput = 0.0;
    result.time = 0.0;

    size_t cnt = 0;
    if (phase == 0) cnt = num_records;
    else cnt = num_queries;

    gettimeofday(&tv_s, NULL);  // start timing
    for (size_t i = 0 ; i < cnt; i++) {
	enum query_types type = queries[i].type;
	char *key = queries[i].hashed_key;
	char buf[val_len];

	if (type == query_insert) {
	    insert(std::string(key, key_len));
	    result.num_insert++;
	} else if (type == query_update) {
	    insert(std::string(key, key_len));
	    result.num_update++;
	} else if (type == query_read) {
	    search(std::string(key, key_len));
	    result.num_read++;
	} else {
	    fprintf(stderr, "unknown query type\n");
	}
    }
    gettimeofday(&tv_e, NULL);  // stop timing
    result.time = timeval_diff(&tv_s, &tv_e);

    size_t nops = result.num_insert + result.num_update + result.num_read;
    result.tput = nops / result.time;

    printf("run %zu #operations in %.2f sec \n",
	    nops, result.time);
    printf("#insert = %zu, #update = %zu, #read = %zu\n", 
	    result.num_insert, 
	    result.num_update, 
	    result.num_read);
    printf("tput = %.2f\n",  result.tput);
    printf("\n");

    printf("run_trace...done\n");
    return;
}

static void usage(char* binname)
{
    printf("%s [-l trace][-h]\n", binname);
    printf("\t-l load trace; required!\n");
    printf("\t-r run trace; required!\n"); 
    printf("\t-h  : show usage\n");
}


int main(int argc, char **argv)
{
    if (argc <= 1) {
	usage(argv[0]);
	exit(-1);
    }

    char ch;
    while ((ch = getopt(argc, argv, "l:r:")) != -1) {
	switch (ch) {
	    case 'l': loadfile   = optarg; break;
	    case 'r': runfile    = optarg; break;
	    case 'h': usage(argv[0]); exit(0); break;
	    default:
		      usage(argv[0]);
		      exit(-1);
	}
    }

    if ( loadfile == NULL || runfile == NULL) {
	usage(argv[0]);
	exit(-1);
    }

    // init smartcuckoo
    init();
    
    printf("==================load phase begin=================\n");

    query *loadTraces = load_trace(loadfile, 0);
    run_trace(loadTraces, 0);
    free(loadTraces);

    printf("==================load phase end=================\n");
    
    printf("==================run phase begin=================\n");

    query *runTraces = load_trace(runfile, 1);
    run_trace(runTraces, 1);
    free(runTraces);

    printf("==================run phase end=================\n");

    return 0;
}
