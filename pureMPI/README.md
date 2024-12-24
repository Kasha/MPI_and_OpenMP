**HPC systems are often composed of clusters of nodes, where each node contains multiple cores and shared memory.**

**A Pure MPI architecture refers to a parallel computing setup where all parallelism is managed solely through the Message Passing Interface (MPI) without relying on shared memory threading models like OpenMP or others. In this model, the computational workload is divided among MPI processes, each of which operates independently and communicates with others explicitly through message passing**.

**PureMPI:**  
• Provide the ‘heat’ value and execution time for 100 iterations with  
both approaches using energy of 10  
• pureMPI.c: using 48 MPI ranks

**OS: Linux**  
**Build:**  
mpicc \-Wal  \-O2 \-I/usr/include \-o pureMPI pureMPI.c

**Run command:**./pureMPI.sh (give file an executable permissions: chmod u+x pureMPI.sh)

**pureMPI.sh file contains:**  
***mpirun \-np 4 ./pureMPI 4 10 100 2 2***  
**Settings of: only 4 MPI ranks (instead of 48), due to my machine hardware and Ubuntu limitations.**

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
***mpirun \-np 48 ./pureMPI 100 10 100 2 2***

**Results according to my machine limitations (as seen in screenshot):**  
Last heat: 302.828292  
Execution time: 0.000507 seconds  
\!\!\! Slower than pureOverlapMPI Faster than PureMPI (probably for higher grid value and better use of OpenMP \#pragma omp could produce better performance)  
