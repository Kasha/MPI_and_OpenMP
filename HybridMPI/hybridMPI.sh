export OMP_NUM_THREADS=12
export OMP_PLACES=cores
export OMP_PROC_BIND=close

mpirun -np 4 --map-by node:PE=1 ./hybridMPI 4 10 100 2 2
