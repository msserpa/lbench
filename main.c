#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv){
	int r, *v;
	long long int i, j, tid, tbegin, tend;
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

	tstart = omp_get_wtime();
	#pragma omp parallel num_threads(nthreads) default(shared) private(tid, tbegin, tend, i, r)
	{
		tid = omp_get_thread_num();
		tbegin = tid * tmax;
		tend = tbegin + tmax;

		r = 1;
		for(i = tbegin; i < tend; i++)
			v[i] = r;
	}
	tfinish = omp_get_wtime();
	fprintf(stdout, "seq_write: \t%lf\n", tfinish - tstart);
	fprintf(stderr, "%lf\n", tfinish - tstart);


	tstart = omp_get_wtime();
	#pragma omp parallel num_threads(nthreads) default(shared) private(tid, tbegin, tend, i, r)
	{
		tid = omp_get_thread_num();
		tbegin = tid * tmax;
		tend = tbegin + tmax;

		r = 2;
		for(i = tbegin; i < tend; i++)
			r = v[i];
	}
	tfinish = omp_get_wtime();
	fprintf(stdout, "seq_read: \t%lf\n", tfinish - tstart);
	fprintf(stderr, "%lf\n", tfinish - tstart);

	tstart = omp_get_wtime();
	#pragma omp parallel num_threads(nthreads) default(shared) private(tid, tbegin, tend, i, j, r)
	{
		tid = omp_get_thread_num();
		tbegin = tid * tmax;
		tend = tbegin + tmax;

		j = 0;
		r = 3;
		for(i = 0; i < tmax; i++){
			j = tbegin + (i * page) % tmax;
			v[j] = r;
		}
	}
	tfinish = omp_get_wtime();
	fprintf(stdout, "rand_write: \t%lf\n", tfinish - tstart);
	fprintf(stderr, "%lf\n", tfinish - tstart);

	tstart = omp_get_wtime();
	#pragma omp parallel num_threads(nthreads) default(shared) private(tid, tbegin, tend, i, j, r)
	{
		tid = omp_get_thread_num();
		tbegin = tid * tmax;
		tend = tbegin + tmax;
		
		j = 0;
		r = 4;
		for(i = 0; i < tmax; i++){
			j = tbegin + (i * page) % tmax;
			r = v[j];
		}
	}
	tfinish = omp_get_wtime();
	fprintf(stdout, "rand_read: \t%lf\n", tfinish - tstart);
	fprintf(stderr, "%lf\n", tfinish - tstart);

	free(v);
	return 0;
}