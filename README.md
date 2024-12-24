[MPI\_and\_OpenMP](https://github.com/Kasha/MPI_and_OpenMP)

**PureMPI**  
**A** **Pure MPI architecture** refers to a parallel computing setup where all parallelism is managed solely through the **Message Passing Interface (MPI)** without relying on shared memory threading models like OpenMP or others. In this model, the computational workload is divided among **MPI processes**, each of which operates independently and communicates with others explicitly through message passing.

**OpenMP** is a higher-level API for parallel programming, designed specifically for shared memory systems.

It simplifies parallel programming by allowing you to use **compiler directives (pragmas)** to parallelize loops and sections of code.

**3 Sample Projects:**  
Computation of  Heat Equation with 2D Decomposition.  
The computational domain (e.g., a plate) is divided into a grid of points, and derivatives are approximated using finite differences.

**Decomposition for Parallel Computation:**  
For parallel processing (e.g., MPI or Hybrid MPI+OpenMP), the computational domain is **decomposed into subdomains**.  
**2D decomposition**: Each process handles a rectangular block of the grid  
Boundary Exchange (Halo Exchange)  
To compute the stencil values at the boundaries of each subdomain, neighboring processes exchange **halo data** (boundary rows/columns).

HPC systems are often composed of clusters of nodes, where each node contains multiple cores and shared memory.  
MPI handles communication across nodes (distributed memory), while OpenMP manages parallelism within a node (shared memory).  
\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_

The following code examples demonstrate how to parallelize the heat equation using PureMPI and Hybrid MPI \+ OpenMP intra-node and inter-node parallelism including hardware setup optimization.

[pureMPI](https://github.com/Kasha/MPI_and_OpenMP/tree/main/pureMPI) 

[hybridMPI](https://github.com/Kasha/MPI_and_OpenMP/tree/main/HybridMPI)

[pureOverlappedMPI](https://github.com/Kasha/MPI_and_OpenMP/tree/main/pureOverlappedMPI)