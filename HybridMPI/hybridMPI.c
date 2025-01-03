#include "mpi.h"
#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* row-major order */
#define ind(i,j) ((j)*(bx+2)+(i))


void setup(int rank, int proc, int argc, char **argv,
           int *n_ptr, int *energy_ptr, int *niters_ptr, int *px_ptr, int *py_ptr, int *final_flag);

void init_sources(int bx, int by, int offx, int offy, int n,
                  const int nsources, int sources[][2], int *locnsources_ptr, int locsources[][2]);

void update_grid(int bx, int by, double *aold, double *anew, double *heat_ptr);

void grid_first_touch_(double *array, int bx, int by) ;

int main(int argc, char **argv)
{
    int rank, size;
    int n, energy, niters, px, py;

    int rx, ry;
    int north, south, west, east;
    int bx, by, offx, offy;

    /* three heat sources */
    const int nsources = 3;
    int sources[nsources][2];
    int locnsources;            /* number of sources in my area */
    int locsources[nsources][2];        /* sources local to my rank */

    double start_time, end_time, mpi_time, openmp_time;

    int iter, i;

    double *aold, *anew, *tmp;

    double heat, rheat;

    int final_flag;

    // Measure time for MPI initialization and communication
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    start_time = MPI_Wtime(); /* take time */

    /* argument checking and setting */
    setup(rank, size, argc, argv, &n, &energy, &niters, &px, &py, &final_flag);

    if (final_flag == 1) {
        MPI_Finalize();
        exit(0);
    }

    /* determine my coordinates (x,y) -- rank=x*a+y in the 2d processor array */
    rx = rank % px;
    ry = rank / px;

    /* determine my four neighbors */
    north = (ry - 1) * px + rx;
    if (ry - 1 < 0)
        north = MPI_PROC_NULL;
    south = (ry + 1) * px + rx;
    if (ry + 1 >= py)
        south = MPI_PROC_NULL;
    west = ry * px + rx - 1;
    if (rx - 1 < 0)
        west = MPI_PROC_NULL;
    east = ry * px + rx + 1;
    if (rx + 1 >= px)
        east = MPI_PROC_NULL;

    /* decompose the domain */
    bx = n / px;        /* block size in x */
    by = n / py;        /* block size in y */
    offx = rx * bx;     /* offset in x */
    offy = ry * by;     /* offset in y */

    /* allocate working arrays & communication buffers */
    aold = (double *) malloc((bx + 2) * (by + 2) * sizeof(double));     /* 1-wide halo zones! */
    anew = (double *) malloc((bx + 2) * (by + 2) * sizeof(double));     /* 1-wide halo zones! */

    // First-touch initialization
    grid_first_touch_(aold, bx, by);
    grid_first_touch_(anew, bx, by);

    /* initialize three heat sources */
    init_sources(bx, by, offx, offy, n, nsources, sources, &locnsources, locsources);

    /* create north-south datatype */
    MPI_Datatype north_south_type;
    MPI_Type_contiguous(bx, MPI_DOUBLE, &north_south_type);
    MPI_Type_commit(&north_south_type);

    /* create east-west datatype */
    MPI_Datatype east_west_type;
    MPI_Type_vector(by, 1, bx + 2, MPI_DOUBLE, &east_west_type);
    MPI_Type_commit(&east_west_type);

    // Timing the OpenMP part
    openmp_time = omp_get_wtime();

    for (iter = 0; iter < niters; ++iter) {
        #pragma omp parallel for
        /* refresh heat sources */
        for (i = 0; i < locnsources; ++i) {
            aold[ind(locsources[i][0], locsources[i][1])] += energy;    /* heat source */
        }
        /*Allow only master thread to use MPI communication*/
        #pragma omp master
        {

            /* exchange data with neighbors */
            MPI_Request reqs[8];
            MPI_Isend(&aold[ind(1, 1)] /* north */ , 1, north_south_type, north, 9, MPI_COMM_WORLD,
                      &reqs[0]);
            MPI_Isend(&aold[ind(1, by)] /* south */ , 1, north_south_type, south, 9, MPI_COMM_WORLD,
                      &reqs[1]);
            MPI_Isend(&aold[ind(bx, 1)] /* east */ , 1, east_west_type, east, 9, MPI_COMM_WORLD,
                      &reqs[2]);
            MPI_Isend(&aold[ind(1, 1)] /* west */ , 1, east_west_type, west, 9, MPI_COMM_WORLD,
                      &reqs[3]);
            MPI_Irecv(&aold[ind(1, 0)] /* north */ , 1, north_south_type, north, 9, MPI_COMM_WORLD,
                      &reqs[4]);
            MPI_Irecv(&aold[ind(1, by + 1)] /* south */ , 1, north_south_type, south, 9, MPI_COMM_WORLD,
                      &reqs[5]);
            MPI_Irecv(&aold[ind(bx + 1, 1)] /* east */ , 1, east_west_type, east, 9, MPI_COMM_WORLD,
                      &reqs[6]);
            MPI_Irecv(&aold[ind(0, 1)] /* west */ , 1, east_west_type, west, 9, MPI_COMM_WORLD,
                      &reqs[7]);
            MPI_Waitall(8, reqs, MPI_STATUSES_IGNORE);
        }

        /* update grid points */
        update_grid(bx, by, aold, anew, &heat);

        /* swap working arrays */
        tmp = anew;
        anew = aold;
        aold = tmp;


    }

    // Record the time for MPI operations
    mpi_time = MPI_Wtime() - start_time; // Total time including communication

    openmp_time = omp_get_wtime() - openmp_time; // Time spent in OpenMP region
    /* free working arrays and communication buffers */
    free(aold);
    free(anew);

    MPI_Type_free(&east_west_type);
    MPI_Type_free(&north_south_type);

    /* get final heat in the system */
    MPI_Allreduce(&heat, &rheat, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    end_time = MPI_Wtime();  // Total execution time

    if (!rank)
    {
        printf("[%i] last heat: %f\n", rank, rheat);
        printf("Execution time with OpenMP: %f seconds\n", openmp_time);
        printf("MPI communication time: %f seconds\n", mpi_time);
        printf("Total execution time: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}

void setup(int rank, int proc, int argc, char **argv,
           int *n_ptr, int *energy_ptr, int *niters_ptr, int *px_ptr, int *py_ptr, int *final_flag)
{
    int n, energy, niters, px, py;

    (*final_flag) = 0;

    if (argc < 6) {
        if (!rank)
            printf("usage: pureMPI <n> <energy> <niters> <px> <py>\n");
        (*final_flag) = 1;
        return;
    }

    n = atoi(argv[1]);  /* nxn grid */
    energy = atoi(argv[2]);     /* energy to be injected per iteration */
    niters = atoi(argv[3]);     /* number of iterations */
    px = atoi(argv[4]); /* 1st dim processes */
    py = atoi(argv[5]); /* 2nd dim processes */

    if (px * py != proc) {
        fprintf(stderr, "px * py must equal to the number of processes.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);   /* abort if px or py are wrong */
    }
    if (n % px != 0) {
        fprintf(stderr, "grid size n must be divisible by px.\n");
        MPI_Abort(MPI_COMM_WORLD, 2);   /* abort px needs to divide n */
    }
    if (n % py != 0) {
        fprintf(stderr, "grid size n must be divisible by py.\n");
        MPI_Abort(MPI_COMM_WORLD, 3);   /* abort py needs to divide n */
    }

    (*n_ptr) = n;
    (*energy_ptr) = energy;
    (*niters_ptr) = niters;
    (*px_ptr) = px;
    (*py_ptr) = py;
}

void init_sources(int bx, int by, int offx, int offy, int n,
                  const int nsources, int sources[][2], int *locnsources_ptr, int locsources[][2])
{
    int i, locnsources = 0;
    sources[0][0] = n / 2;
    sources[0][1] = n / 2;
    sources[1][0] = n / 3;
    sources[1][1] = n / 3;
    sources[2][0] = n * 4 / 5;
    sources[2][1] = n * 8 / 9;
    #pragma omp parallel for
    for (i = 0; i < nsources; ++i) {    /* determine which sources are in my patch */
        int locx = sources[i][0] - offx;
        int locy = sources[i][1] - offy;
        if (locx >= 0 && locx < bx && locy >= 0 && locy < by) {
            locsources[locnsources][0] = locx + 1;      /* offset by halo zone */
            locsources[locnsources][1] = locy + 1;      /* offset by halo zone */
            locnsources++;
        }
    }

    (*locnsources_ptr) = locnsources;
}

/*Manage the parallel computation of heat distribution within the grid.*/
void update_grid(int bx, int by, double *aold, double *anew, double *heat_ptr)
{
    int i, j;
    double heat = 0.0;
    /* 2 Optimizations: Sum and  Wise parallel threads use for sum operation per chunk and not per iteration
    This clause specifies the variables that will be subject to reduction and the operation to be used.
    Each thread gets a private copy of the reduced variable, and at the end of the parallel region,
    these private copies are combined using the specified reduction operation.*/
    #pragma omp parallel for reduction(+:heat) schedule(static, 10)
    for (i = 1; i < bx + 1; ++i) {
        for (j = 1; j < by + 1; ++j) {
            anew[ind(i, j)] =
                anew[ind(i, j)] / 2.0 + (aold[ind(i - 1, j)] + aold[ind(i + 1, j)] +
                                         aold[ind(i, j - 1)] + aold[ind(i, j + 1)]) / 4.0 / 2.0;
            heat += anew[ind(i, j)];
        }
    }

    (*heat_ptr) = heat;
}

/*Ensure first-touch memory allocation for the dynamically allocated arrays,
initialize them in parallel, with each thread writing to the portion of the array that it will
 later access during computation. This ensures the memory pages are mapped to the NUMA node of the threads performing the initialization.*/
void grid_first_touch_(double *array, int bx, int by) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (int j = 0; j < by + 2; j++) {
        for (int i = 0; i < bx + 2; i++) {
            array[j * (bx + 2) + i] = 0.0; // First touch initializes the page
        }
    }
}
