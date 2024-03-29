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

	if(argc != 4){
		fprintf(stderr, "Usage: %s <nthreads> <mem_MB> <test>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nthreads = atoi(argv[1]);
	
	memr  = atoll(argv[2]); // MB
	mem   = memr * 1024 * 1024; // B

	tmem  = ceil(mem / nthreads / 1024) * 1024; // B
	tmemr = tmem / 1024 / 1024; // MB

	max = mem / sizeof(int);
	tmax = tmem / sizeof(int);

	page = PAGE_SIZE / sizeof(int);

	v = (int *) calloc(max, sizeof(int));
	if(!v){
		fprintf(stderr, "lbench is not able to allocate %lld B\n", mem);
		exit(EXIT_FAILURE);
	}
	memset(v, 1, max);

	get_mempolicy(&numa_node, NULL, 0, (void*)v, MPOL_F_NODE | MPOL_F_ADDR);

	printf("%s - %s\n", hostname, hostdate);
	printf("Output file: %s\n\n", fname);

	printf("mem numa node: %d\n", numa_node);
	printf("mem allocated: %lld MB - %lld B\n", memr, mem);
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
	fprintf(f, "#memoryMB %lld\n", memr);
	fprintf(f, "#memoryNode %d\n", numa_node);
	fprintf(f, "bench,nid,mem,time\n");

	
	if(argv[3][0] == '-' && argv[3][1] == 's' && argv[3][2] == 'w'){
		#pragma omp parallel num_threads(nthreads) default(shared) private(tid, cid, nid, tbegin, tend, tstart, tfinish, i, j, r)
		{

			tid = omp_get_thread_num();
			#pragma omp critical
			cid = sched_getcpu();
			nid = numa_node_of_cpu(cid);
			tbegin = tid * tmax;
			tend = tbegin + tmax;
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - (%lld, %lld)\n\n", tid,  cid, nid, tbegin, tend);
			#endif
			r = 1;

			tstart = omp_get_wtime();
			for(i = tbegin; i < tend; i++)
				v[i] = r;
			tfinish = omp_get_wtime();
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - seq_write: \t%lf\n", tid, cid, nid, tfinish - tstart);
			#endif
			#pragma omp critical
			fprintf(f, "seq_write,%d,%d,%lf\n", nid, numa_node, tfinish - tstart);
		}
	}else if(argv[3][0] == '-' && argv[3][1] == 's' && argv[3][2] == 'r'){
		#pragma omp parallel num_threads(nthreads) default(shared) private(tid, cid, nid, tbegin, tend, tstart, tfinish, i, j, r)
		{

			tid = omp_get_thread_num();
			#pragma omp critical
			cid = sched_getcpu();
			nid = numa_node_of_cpu(cid);
			tbegin = tid * tmax;
			tend = tbegin + tmax;
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - (%lld, %lld)\n\n", tid,  cid, nid, tbegin, tend);
			#endif
			r = 1;

			tstart = omp_get_wtime();
			for(i = tbegin; i < tend; i++)
				r = v[i];
			tfinish = omp_get_wtime();
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - seq_read: \t%lf\n", tid, cid, nid, tfinish - tstart);
			#endif
			#pragma omp critical
			fprintf(f, "seq_read,%d,%d,%lf\n", nid, numa_node, tfinish - tstart);
		}
	}else if(argv[3][0] == '-' && argv[3][1] == 'r' && argv[3][2] == 'w'){
		#pragma omp parallel num_threads(nthreads) default(shared) private(tid, cid, nid, tbegin, tend, tstart, tfinish, i, j, r)
		{

			tid = omp_get_thread_num();
			#pragma omp critical
			cid = sched_getcpu();
			nid = numa_node_of_cpu(cid);
			tbegin = tid * tmax;
			tend = tbegin + tmax;
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - (%lld, %lld)\n\n", tid,  cid, nid, tbegin, tend);
			#endif
			r = 1;
			j = 0;

			tstart = omp_get_wtime();
			for(i = 0; i < tmax; i++){
				j = tbegin + (i * page) % tmax;
				v[j] = r;
			}
			tfinish = omp_get_wtime();
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - rand_write: \t%lf\n", tid, cid, nid, tfinish - tstart);
			#endif
			#pragma omp critical
			fprintf(f, "rand_write,%d,%d,%lf\n", nid, numa_node, tfinish - tstart);
		}
	}else if(argv[3][0] == '-' && argv[3][1] == 'r' && argv[3][2] == 'r'){
		#pragma omp parallel num_threads(nthreads) default(shared) private(tid, cid, nid, tbegin, tend, tstart, tfinish, i, j, r)
		{

			tid = omp_get_thread_num();
			#pragma omp critical
			cid = sched_getcpu();
			nid = numa_node_of_cpu(cid);
			
			tbegin = tid * tmax;
			tend = tbegin + tmax;
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - (%lld, %lld)\n\n", tid,  cid, nid, tbegin, tend);
			#endif
			r = 1;
			j = 0;

			tstart = omp_get_wtime();
			for(i = 0; i < tmax; i++){
				j = tbegin + (i * page) % tmax;
				r = v[j];
			}
			tfinish = omp_get_wtime();
			#ifdef DEBUG
				fprintf(stderr, "(t%-2d, c%-2d, n%d) - rand_read: \t%lf\n", tid, cid, nid, tfinish - tstart);
			#endif
			#pragma omp critical
			{
			fprintf(f, "rand_read,%d,%d,%lf\n", nid, numa_node, tfinish - tstart);
			}
		}
        }else if(argv[3][0] == '-' && argv[3][1] == 'r' && argv[3][2] == 'm'){
                #pragma omp parallel num_threads(nthreads) default(shared) private(tbegin, tend, tid, cid, nid, tstart, tfinish, i, j, r)
                {

                        tid = omp_get_thread_num();
                        #pragma omp critical
                        cid = sched_getcpu();
                        nid = numa_node_of_cpu(cid);
			
			tbegin = tid * tmax;
			tend = tbegin + tmax;
                        #ifdef DEBUG
                                fprintf(stderr, "(t%-2d, c%-2d, n%d) - (%lld, %lld)\n\n", tid,  cid, nid, tbegin, tend);
                        #endif
                        r = 1;
                        j = 0;
                        tstart = omp_get_wtime();
                        for(i = 0; i < tmax; i++){
//				printf("%d, %lld\n", tid, tbegin + j); 
                                r = v[tbegin + j];
				j += 270336; j = j % (tend - tbegin);
                        }
			tfinish = omp_get_wtime();
                        #ifdef DEBUG
                                fprintf(stderr, "(t%-2d, c%-2d, n%d) - rand_read_mod: \t%lf\n", tid, cid, nid, tfinish - tstart);
                        #endif
                        #pragma omp critical
                        {
                        fprintf(f, "rand_read_mod,%d,%d,%lf\n", nid, numa_node, tfinish - tstart);
                        }
                }

	}else{
		fprintf(stderr, "Avaiable tests are:\n");
		fprintf(stderr, "\t-sw sequential write\n");
		fprintf(stderr, "\t-sr sequential read\n");
		fprintf(stderr, "\t-rw rand write\n");
		fprintf(stderr, "\t-rr rand read\n\n");
	}

	fclose(f);
	free(v);
	return 0;
}
