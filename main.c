#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

extern int sched_getcpu(void);

int main(int argc, char **argv){
	FILE *f;
	int r, *v;
	long long int i, j, tid, cid, tbegin, tend;
	long long int max, tmax;
	long long int mem, memr, tmem, tmemr;
	int nthreads, page;
	double tstart, tfinish;

	if(argc != 3){
		fprintf(stderr, "Usage: %s <nthreads> <mem_per_thread_GB>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	nthreads = atoi(argv[1]);
	
	tmemr = atoll(argv[2]);
	memr = tmemr * nthreads;
	
	tmem = tmemr * 1024 * 1024 * 1024;
	mem = tmem * nthreads;

	max = mem / sizeof(int);
	tmax = max / nthreads;

	page = 4 * 1024 / sizeof(int);

	v = (int *) calloc(max, sizeof(int));
	if(!v){
		fprintf(stderr, "lbench is not able to allocate %lld GB\n", mem);
		exit(EXIT_FAILURE);
	}

	printf("mem allocated: %lld GB - %lld B\n", memr, mem);
	printf("elements allocated: %lld\n", max);
	printf("nthreads: %d\n\n", nthreads);

	if(nthreads > 1){
		printf("mem allocated per thread: %lld GB - %lld B\n", tmemr, tmem);
		printf("elements allocated per thread: %lld\n\n", tmax);
	}


	f = fopen("log.csv", "w");

	#pragma omp parallel num_threads(nthreads) default(shared) private(tid, cid, tbegin, tend, tstart, tfinish, i, j, r)
	{

		tid = omp_get_thread_num();
		#pragma omp critical
		cid = sched_getcpu();
		tbegin = tid * tmax;
		tend = tbegin + tmax;

		fprintf(stderr, "t%lld(%lld) - (%lld, %lld)\n\n", tid, cid, tbegin, tend);

		r = 1;

		tstart = omp_get_wtime();
		for(i = tbegin; i < tend; i++)
			v[i] = r;
		tfinish = omp_get_wtime();

		fprintf(stderr, "t%lld(%lld) seq_write: \t%lf\n", tid, cid, tfinish - tstart);

		tstart = omp_get_wtime();
		for(i = tbegin; i < tend; i++)
			r = v[i];
		tfinish = omp_get_wtime();

		fprintf(stderr, "t%lld(%lld) seq_read: \t%lf\n", tid, cid, tfinish - tstart);

		r = 2;

		j = 0;
		tstart = omp_get_wtime();
		for(i = 0; i < tmax; i++){
			j = tbegin + (i * page) % tmax;
			if(j < tbegin || j >= tend)
				printf("%lld - %lld\n", tid, j);
			v[j] = r;
		}
		tfinish = omp_get_wtime();

		fprintf(stderr, "t%lld(%lld) rand_write: \t%lf\n", tid, cid, tfinish - tstart);

		j = 0;
		tstart = omp_get_wtime();
		for(i = 0; i < tmax; i++){
			j = tbegin + (i * page) % tmax;
			r = v[j];
		}
		tfinish = omp_get_wtime();

		fprintf(stderr, "t%lld(%lld) rand_read: \t%lf\n", tid, cid, tfinish - tstart);		
	}

	fclose(f);
	free(v);
	return 0;
}