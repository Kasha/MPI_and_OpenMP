**HPC systems are often composed of clusters of nodes, where each node contains multiple cores and shared memory.**  
**MPI handles communication across nodes (distributed memory), while OpenMP manages parallelism within a node (shared memory).**

**Hybrid MPI \+ OpenMP intra-node and inter-node parallelism including hardware setup optimization.**

**HybridMPI instructions:**  
hybridMPI.c which includes hybrid multithread implementation  
• Reproduce identical results to pureMPI.c  
• Each node (group of cores) will be assigned a single MPI process  
• Calculation within each node will use multiple threads when possible  
• Allow only master thread to use MPI communication  
• Results of the ‘heat’ value and execution time for 100 iterations with  
both approaches using energy of 10  
• pureMPI.c: using 48 MPI ranks  
• hybridMPI.c: using 4 MPI ranks with 12 threads each

**OS: Linux**  
**Build:**  
mpicc \-Wall \-fopenmp  \-O2 \-I/usr/include \-o hybridMPI hybridMPI .c

**Run command:**./hybridMPI.sh (give file an executable permissions: chmod u+x hybridMP.sh)  
**hybridMPI.sh file contains:**  
export OMP\_NUM\_THREADS=12  
export OMP\_PLACES=cores  
export OMP\_PROC\_BIND=close

***mpirun \-np 4 \--map-by node:PE=1 ./hybridMPI 4 10 100 2 2***  
**Settings of 4 MPI ranks mapped to node with one core and up to 12 threads due to my machine hardware and Ubuntu limitations.**

**Expected arguments:**  
 n \= atoi(argv\[1\]);  /\* nxn grid \*/  
 energy \= atoi(argv\[2\]);     /\* energy to be injected per iteration \*/  
 niters \= atoi(argv\[3\]);     /\* number of iterations \*/  
 px \= atoi(argv\[4\]); /\* 1st dim processes \*/  
 py \= atoi(argv\[5\]); /\* 2nd dim processes \*/

int n \= 100; // Grid size per process   
Int energy \= 10; // Initial energy value   
int niters \= 100; // Number of iterations  
int px \= 2, py \= 2; // Grid dimensions in processes

**Expected running command:**  
***mpirun \-np 4 \--map-by ppr:1:node ./hybridMPI 100 10 100***   
**Settings of: 4 MPI ranks with 12 threads. Each MPI process is assigned to a single node with attached group of cores**

**Settings and Optimizations:**  
**Environment variables:**

* OMP\_NUM\_THREADS=12 \- Each MPI process spawns 12 OpenMP threads  
* OMP\_PLACES=cores \- Each thread is bound to a single physical core. Optimizes performance by minimizing contention.  
* OMP\_PROC\_BIND=close \- Place threads close to each other to improve cache locality  
  **Time measuring:**  
* Omp\_get\_wtime for OpenMP \-  time measuring, starts after MPI init and settings.  
* MPI\_Wtime() \- MPI,  time measuring, starts after MPI init.  
  **Performance:**  
* \--report-bindings: I used this parameter with mpirun, for MPI runtime outputs the binding information for each process

**Code:**

* first\_touch\_grid \- (replacing memset): first-touch memory allocation for the dynamically allocated arrays `aold` and `anew`. Initializing them in parallel, with each thread writing to the portion of the array that it will later access during computation.   
* **\#pragma omp parallel for: for refreshing heat sources inside the niters loop**  
* **\#pragma omp master:** Allow only master thread to use MPI communication  
* **\#pragma omp parallel for reduction(+:heat) schedule(static, 10):** Inside update\_grid heat parallel calculation (every 10 iterations are assigned to the same thread, contains copy of heat variable and on completion summarize them)  
* I also tried using MPI\_Init\_Thread which produced a higher duration time by 1 second. I added back to MPI\_Init method for the MPI communication initialization

**Results according to my machine limitations (as seen in screenshot):**  
Last heat: 302.828292  
Execution time: 0.44241 seconds  
\!\!\! Slower than PureMPI and pureOverlapMPI  (probably for higher grid value and better use of OpenMP \#pragma omp could produce better performance)