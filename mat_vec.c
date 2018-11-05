#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
int main (int argc, char* argv[]) {
  int myid, numprocs;
  char me[255];
//  int n;

  // Create Identity Matrix 6x6
  short I6[36];
  int i;
  for(i = 0; i < 6; i++){
    if(i/6 = i%6){
      I6[i] = 1;
    } else {
      I6[i] = 0;
    }
  }

  // Create 6x6 1's to make ops conspicuous.
  short A[36];
  for(i = 0; i < 36; i++) A[i] = 1;

  // Create buffer for answer
  short C[36];
  short *matrices[3] = {I3, A, C};

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  gethostname(me, 254);
  
  // Determine which row of matrix A to find dot products
  short my_col_A = myid;
  // Columns kill performance -- only scan once
  short col_vec[6], dot_prod, row;
  for(row = 0; row < 6; row++){
    col_vec[row] = A[6 * row + my_col_A];
  }

  dot_prod = 0;
  for(row = 0; row < 6; row++){
    dot_prod += I6[6 * myrow + row] * col_vec[row];
  }
  
//  printf("Hello from %s I am process %d of %d\n", me, myid, numprocs);
//  if (myid == 0) {
//    n = 12345;
//  }
//  printf("Call to MPI_Bcast n==%d on %s myid=%d\n", n, me, myid);
//  MPI_Bcast(&C, 1, MPI_INT, 0, MPI_COMM_WORLD);
//  printf("Return from MPI_Bcast n==%d on %s myid=%d\n", n, me, myid);
  MPI_Finalize();
  return 0;
}

