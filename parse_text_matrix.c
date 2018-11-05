/**
 * Shmuel Jacobs
 * November 4, 2018
 * Software Design

 * Functions to Read in matrix, determines dimensions, stores in newly allocated
   memory and returns pointer.

 *  Fragile Implementation: Will exit under any of the following conditions:
 	- matrix is more than 20 entries wide
 	- matrix is more than 20 rows tall
 	- matrix file contains more than 255 characters in a line (counting spaces)
 	- file contains non-numeric characters or extra spacing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char DEBUG = 1;

/**
 * Call on text file.
 * Parameters:
 * path - string containing path to text file
 * nrows - pointer to variable for number of rows in matrix
 * ncols - pointer to variable for number of columns in matrix

 */
double *parse_text_matrix(char *path, unsigned short *nrows, unsigned short *ncols) {
	FILE *matrix_file = fopen(path, "r");
	// String holds most recent line read.
	char this_line[256];
	// Variable holds dimension counts, to be placed in pointers later.
	unsigned short count_rows = 0;

	// Allocate space for entries retrieved.
	double *matrix_space = (double*) malloc(sizeof(double) * 400);
	unsigned short matrix_index = 0;
	if(matrix_space == NULL){
		puts("Failed to allocate space for matrix.");
		exit(1);
	}

	// Get first line and find dimension
	fgets(this_line,255,matrix_file);
	count_rows++;
	
	/* Iterate through entries first in row */
	char *cell_str = strtok(this_line," ");

	while(cell_str != NULL){
		// Put entry into matrix
		matrix_space[matrix_index++] = atof(cell_str);
		if(DEBUG) {
			printf("Just read in %s.\n", cell_str);
		}

		cell_str = strtok(NULL, " ");
	} // Finished with first row -- current index is number of columns
	*ncols = matrix_index;

	// Iterate through remaining rows of matrix
	for( ; fgets(this_line,255,matrix_file); count_rows++ ) {
		char *cell_str = strtok(this_line," ");
		// Iterate through all entries.
		while(cell_str != NULL){
			// Put entry into matrix
			matrix_space[matrix_index++] = atof(cell_str);

			if(DEBUG) {
				printf("Just read in %s.\n", cell_str);
			}

			cell_str = strtok(NULL, " ");
		} 
	}

	*nrows = count_rows;
	return matrix_space;
}