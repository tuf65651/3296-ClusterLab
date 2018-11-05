#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include "parse_text_matrix.c"
#include "write_text_matrix.c"
#define min(x, y) ((x)<(y)?(x):(y))

double* gen_matrix(int n, int m);
int mmult(double *c, double *a, int aRows, int aCols, double *b, int bRows, int bCols);
void compare_matrix(double *a, double *b, int nRows, int nCols);

/** 
    Program to multiply a matrix times a matrix using both
    mpi to distribute the computation among nodes and omp
    to distribute the computation among threads.

    Use -a at cli to provide left side matrix.
    Use -b at cli to provide right side matrix.
*/

int main(int argc, char* argv[])
{
  /* Declarations */
  int nrows, ncols;
  double *aa;	/* the A matrix */
  double *bb;	/* the B matrix */
  double *cc1;	/* A x B computed using the omp-mpi code you write */
  double *cc2;	/* A x B computed using the conventional algorithm */
  int myid, numprocs;
  double starttime, endtime;
  MPI_Status status;
  /* insert other global variables here */
  int nIv1, nIv2;
  /* Begin processes */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  /**** Try to populate aa and bb ****/

  // Search for first matrix
  short arg_search;
  for(arg_search = 1; arg_search < argc; arg_search++){
    if( !strcmp(argv[arg_search], "-a") ) break; // found it
  }
  // If loop completed, exit. No path was provided.
  if(arg_search==argc) {
    puts("Must provide -a <<path to matrix text file>>");
    exit(2);
  }
  aa = parse_text_matrix(argv[++arg_search], &nrows, &nIv1);

  // Search for second matrix, validating nIv
  for(arg_search = 1; arg_search < argc; arg_search++){
    if( !strcmp(argv[arg_search], "-b") ) break;
  }
  // If loop completed, exit. No path was provided.
  if(arg_search==argc) {
    puts("Must provide -b <<path to matrix text file>>");
    exit(2);
  }
  bb = parse_text_matrix(argv[++arg_search], &nIv2, &ncols);

  if(nIv1 != nIv2){
    puts("Incompatible matrices provided.");
    exit(1);
  }

  // Check for argument -- what should really be checked? Can't reach this without args.
  if (argc > 1) {
    // nrows = atoi(argv[1]);
    // ncols = nrows;
    if (myid == 0) {
      // Master Code goes here
      // aa = gen_matrix(nrows, ncols);
      // bb = gen_matrix(ncols, nrows);
      cc1 = malloc(sizeof(double) * nrows * nrows); 
      starttime = MPI_Wtime();
      /* Insert your master code here to store the product into cc1 */
      endtime = MPI_Wtime();
      printf("%f\n",(endtime - starttime));
      cc2  = malloc(sizeof(double) * nrows * nrows);
      mmult(cc2, aa, nrows, ncols, bb, ncols, nrows);
      compare_matrices(cc2, cc1, nrows, nrows);
    } else {
      // Slave Code goes here
    }
  } else {
    fprintf(stderr, "Usage matrix_times_vector <size>\n");
  }
  MPI_Finalize();
  return 0;
}
