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
void compare_matrices(double *a, double *b, int nRows, int nCols);
const int master = 0; // id of master process

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
  double ans;

  int myid, numprocs;
  double starttime, endtime;
  MPI_Status statusa, statusb;
  /* insert other global variables here */
  int nIv1, nIv2;
  int i, j, numsent, sender;
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
    // TODO: finallize here so all processes end immediately
    exit(1);
  }
  // buffers for sending vector
  double *a_vec = (double*) malloc(sizeof(double) * nIv1);
  double* b_vec = (double*) malloc(sizeof(double) * nIv1); 
  cc1 = malloc(sizeof(double) * nrows * nrows);

  // Check for argument -- what should really be checked? Can't reach this without args.
  if (argc > 1) {
    // nrows = atoi(argv[1]);
    // ncols = nrows;
    if (myid == 0) {
      // Master Code goes here
      // aa = gen_matrix(nrows, ncols);
      // bb = gen_matrix(ncols, nrows);
       
      starttime = MPI_Wtime();
      /* Insert your master code here to store the product into cc1 */

      numsent = 0;
      // Synchronize calls
      //MPI_Bcast(cc1, nIv1 * ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
      
      // Iterate, as long as there's another process and another row for it.
      for (i = 0; i < min(numprocs-1, nrows); i++) {

        MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, 
           MPI_COMM_WORLD, &statusa);
        sender = statusa.MPI_SOURCE;
        cc1[statusa.MPI_TAG] = ans;

        if (numsent < nrows) {
          // For every entry 
          for (j = 0; j < nIv1; j++) {
            a_vec[j] = aa[i * ncols + j];
            b_vec[j] = bb[j * ncols + i];
          }  // Row of aa is copied into buffer
          // Send that buffer to all processes
          MPI_Send(a_vec, nIv1, MPI_DOUBLE, sender, i+1, MPI_COMM_WORLD);
          MPI_Send(b_vec, nIv1, MPI_DOUBLE, sender, i+1, MPI_COMM_WORLD);
          numsent++;
        } else {
          MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
        }
      }
      // signal end


      endtime = MPI_Wtime();
      printf("%f\n",(endtime - starttime));

      /**** Run old O(n^3) method for comparison ****/
      cc2  = malloc(sizeof(double) * nrows * nrows);
      mmult(cc2, aa, nrows, ncols, bb, ncols, nrows);
      compare_matrices(cc2, cc1, nrows, nrows);
      /**** Ignore this. It's independent ****/

    } else {

// Synchronize with buffer from master
      //MPI_Bcast(cc1, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
      if (myid <= nrows) { // Do nothing if matrix is already divvied up, else...
        while(1) { // Keep receiving and doing dot-products until getting status.MPI_TAG of 0

          MPI_Recv(a_vec, nIv1, MPI_DOUBLE, master, MPI_ANY_TAG, MPI_COMM_WORLD, &statusa);
          MPI_Recv(b_vec, nIv1, MPI_DOUBLE, master, MPI_ANY_TAG, MPI_COMM_WORLD, &statusb);
          if (statusa.MPI_TAG == 0 && statusb.MPI_TAG == 0){
            break;
          }
          short row = statusa.MPI_TAG;
          short col = statusb.MPI_TAG;

          ans = 0.0;
#pragma omp parallel
#pragma omp shared(ans) for reduction(+:ans)
          // Parallelize dot-product with threads, add arithmatic products to shared sum
    for (j = 0; j < nIv1; j++) {
            ans += a_vec[j] * b_vec[j];
          }
          // Report new sum to processes
          MPI_Send(&ans, 1, MPI_DOUBLE, master, row * ncols + col, MPI_COMM_WORLD);
        } //endwhile
      } // endif(myid <= nrows)
    }
  } else {
    fprintf(stderr, "Usage matrix_times_vector <size>\n");
  }
  MPI_Finalize();
  return 0;
}
