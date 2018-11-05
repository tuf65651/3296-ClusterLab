#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define min(x, y) ((x)<(y)?(x):(y))

/** 
    Program to multiply a matrix times a vector using both
    mpi to distribute the computation among nodes and omp
    to distribute the computation among threads.
*/
const char DEBUG = 1;

int main(int argc, char* argv[])
{
  /* Variables */
  int nrows, ncols;
  double *aa, *b, *c;
  double *buffer, ans;
  double *times;
  double total_times;
  int run_index;
  int nruns;
  int myid, master, numprocs;
  double starttime, endtime;
  MPI_Status status;
  int i, j, numsent, sender;
  int anstype, row;

  // Seed random
  srand(time(0));

  // MPI Init
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  // Check for argument before entering program.
  if (argc > 1) {

    // Define square matrix, of size ordered on cmdln.
    nrows = atoi(argv[1]);
    ncols = nrows;

    // Allocate space for matrix, vector, and output vector.
    aa = (double*)malloc(sizeof(double) * nrows * ncols);
    b = (double*)malloc(sizeof(double) * ncols);
    c = (double*)malloc(sizeof(double) * nrows);


    buffer = (double*)malloc(sizeof(double) * ncols);
    master = 0;
    if (myid == master) {
      // Master Code goes here
      for (i = 0; i < nrows; i++) { // For every row in aa
      	for (j = 0; j < ncols; j++) { // For every entry in row
      	  aa[i*ncols + j] = (double)rand()/RAND_MAX; // Set to random value.
      	}
      } // post: Matrix aa is set.

      // Get timestamp for starttime
      starttime = MPI_Wtime();
      numsent = 0;
      /* Send row to all processes.
        Send whole vector, length of vector, indication that data is doubles,
        send from master process, send to all MPI processes. */

	if(DEBUG){
		printf("Main process reached Broadcast\n");
	}
	
      MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);

      // Iterate, as long as there's another process and another row for it.
      for (i = 0; i < min(numprocs-1, nrows); i++) {
        // For every entry 
      	for (j = 0; j < ncols; j++) {
      	  buffer[j] = aa[i * ncols + j];
      	}  // Row of aa is copied into buffer
        // Send that buffer to all processes
      	MPI_Send(buffer, ncols, MPI_DOUBLE, i+1, i+1, MPI_COMM_WORLD);
      	numsent++;
      }

      // Iterate through rows of aa
      for (i = 0; i < nrows; i++) {
        // Get the vector that was broadcast
      	MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, 
      		 MPI_COMM_WORLD, &status);
      	sender = status.MPI_SOURCE;
      	anstype = status.MPI_TAG;
      	c[anstype-1] = ans;

        // If not all rows have been sent
      	if (numsent < nrows) {
          // Copy next row into buffer
      	  for (j = 0; j < ncols; j++) {
      	    buffer[j] = aa[numsent*ncols + j];
      	  }  
          // Send row in buffer
      	  MPI_Send(buffer, ncols, MPI_DOUBLE, sender, numsent+1, 
      		   MPI_COMM_WORLD);
          // Mark next row sent
      	  numsent++;
      	} else { // All rows have been sent
      	  MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
      	}
      } 

      // Master process ending. Get time interval.
      endtime = MPI_Wtime();
      printf("%f\n",(endtime - starttime));

    } else {
      /* Slave Code goes here */

      // Synchronize with buffer from master
      MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
      if (myid <= nrows) {
      	while(1) { // Keep receiving and doing dot-products until getting status.MPI_TAG of 0
      	  MPI_Recv(buffer, ncols, MPI_DOUBLE, master, MPI_ANY_TAG, 
      		   MPI_COMM_WORLD, &status);
      	  if (status.MPI_TAG == 0){
      	    break;
          }
      	  row = status.MPI_TAG;
      	  ans = 0.0;
#pragma omp parallel
#pragma omp shared(ans) for reduction(+:ans)
          // Parallelize dot-product with threads, add arithmatic products to shared sum
	  for (j = 0; j < ncols; j++) {
      	    ans += buffer[j] * b[j];
        	}
          // Report new sum to processes
      	  MPI_Send(&ans, 1, MPI_DOUBLE, master, row, MPI_COMM_WORLD);
      	} //endwhile
      } // endif(myid <= nrows)
    } // end slave code
  } else {
    fprintf(stderr, "Usage matrix_times_vector <size>\n");
  }
  // All processes meet, and main process exits.
  MPI_Finalize();
  return 0;
}
