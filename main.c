#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <omp.h>
#include <numa.h>
#include <numaif.h>

#ifndef HOST_NAME_MAX
	#define HOST_NAME_MAX 64
#endif

#ifndef PAGE_SIZE
	#define PAGE_SIZE 4096
#endif

extern int sched_getcpu(void);

int main(int argc, char **argv){
	FILE *f;
	int r, *v;
	long long int i, j, tbegin, tend;
	long long int max, tmax;
	long long int mem, memr, tmem, tmemr;
	int tid, cid, nid, nthreads, page, numa_node = -1;
	double tstart, tfinish;
	char hostname[HOST_NAME_MAX + 1], hostdate[1024], fname[1024];
	struct stat st = {0};
	time_t t;

	time(&t);
	strftime(hostdate, 1023, "%d.%m.%Y.%H.%M.%S", localtime(&t));
	gethostname(hostname, HOST_NAME_MAX);
	sprintf(fname, "output/lbench.%s.%s.csv", hostname, hostdate);

	if(argc != 3){
		fprintf(stderr, "Usage: %s <nthreads> <mem_GB>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	nthreads = atoi(argv[1]);
	
	memr  = atoll(argv[2]); // GB
	mem   = memr * 1024 * 1024 * 1024; // B

	tmem  = ceil(mem / nthreads / 1024) * 1024; // B
	tmemr = tmem / 1024 / 1024; // MB

	max = mem / sizeof(int);
	tmax = tmem / sizeof(int);

	page = PAGE_SIZE / sizeof(int);

	v = (int *) calloc(max, sizeof(int));
	if(!v){
		fprintf(stderr, "lbench is not able to allocate %lld GB\n", mem);
		exit(EXIT_FAILURE);
	}

	get_mempolicy(&numa_node, NULL, 0, (void*)v, MPOL_F_NODE | MPOL_F_ADDR);

	printf("%s - %s\n", hostname, hostdate);
	printf("Output file: %s\n\n", fname);

	printf("mem numa node: %d\n", numa_node);
	printf("mem allocated: %lld GB - %lld B\n", memr, mem);
	printf("elements allocated: %lld\n", max);
	printf("nthreads: %d\n\n", nthreads);

	if(nthreads > 1){
		printf("mem allocated per thread: %lld MB - %lld B\n", tmemr, tmem);
		printf("elements allocated per thread: %lld\n\n", tmax);
	}

	if(stat("output", &st) == -1)
    	if(mkdir("output", 0700) == -1){
    		fprintf(stderr, "error creating output directory\n");
			exit(EXIT_FAILURE);
    	}

	f = fopen(fname, "w");
	if(!f){
		fprintf(stderr, "error creating file output\nfile: %s", fname);
		exit(EXIT_FAILURE);
	}

	fprintf(f, "#hostname %s\n", hostname);
	fprintf(f, "#nthreads %d\n", nthreads);
	fprintf(f, "#memoryGB %lld\n", memr);
	fprintf(f, "#memoryNode %d\n", numa_node);
	fprintf(f, "bench,node,time\n");
	#pragma omp parallel num_threads(nthreads) default(shared) private(tid, cid, nid, tbegin, tend, tstart, tfinish, i, j, r)
	{

		tid = omp_get_thread_num();
		#pragma omp critical
		cid = sched_getcpu();
		nid = numa_node_of_cpu(cid);
		tbegin = tid * tmax;
		tend = tbegin + tmax;

		fprintf(stderr, "(t%-2d, c%-2d, n%d) - (%lld, %lld)\n\n", tid,  cid, nid, tbegin, tend);

		r = 1;

		tstart = omp_get_wtime();
		for(i = tbegin; i < tend; i++)
			v[i] = r;
		tfinish = omp_get_wtime();

		fprintf(stderr, "(t%-2d, c%-2d, n%d) - seq_write: \t%lf\n", tid, cid, nid, tfinish - tstart);
		#pragma omp critical
		fprintf(f, "seq_write,%d,%lf\n", nid, tfinish - tstart);

		tstart = omp_get_wtime();
		for(i = tbegin; i < tend; i++)
			r = v[i];
		tfinish = omp_get_wtime();

		fprintf(stderr, "(t%-2d, c%-2d, n%d) - seq_read: \t%lf\n", tid, cid, nid, tfinish - tstart);
		#pragma omp critical
		fprintf(f, "seq_read,%d,%lf\n", nid, tfinish - tstart);

		r = 2;

		j = 0;
		tstart = omp_get_wtime();
		for(i = 0; i < tmax; i++){
			j = tbegin + (i * page) % tmax;
			v[j] = r;
		}
		tfinish = omp_get_wtime();

		fprintf(stderr, "(t%-2d, c%-2d, n%d) - rand_write: \t%lf\n", tid, cid, nid, tfinish - tstart);
		#pragma omp critical
		fprintf(f, "rand_write,%d,%lf\n", nid, tfinish - tstart);

		j = 0;
		tstart = omp_get_wtime();
		for(i = 0; i < tmax; i++){
			j = tbegin + (i * page) % tmax;
			r = v[j];
		}
		tfinish = omp_get_wtime();

		fprintf(stderr, "(t%-2d, c%-2d, n%d) - rand_read: \t%lf\n", tid, cid, nid, tfinish - tstart);
		#pragma omp critical
		fprintf(f, "rand_read,%d,%lf\n", nid, tfinish - tstart);
	}

	fclose(f);
	free(v);
	return 0;
}
