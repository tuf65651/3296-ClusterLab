#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Shmuel Jacobs
 * November 4, 2018
 * Software Design

 * Functions to write matrix to text, given dimensions and pointer to matrix.
 * Naive function. Will over-index if given the chance.
 */

FILE *write_text_matrix(const char* file_path, double *matrix_space, short nRows, short nCols){

	FILE *text_file = fopen(file_path, "w");
	if(text_file==NULL){
		puts("Unable to open file for write.");
		exit(1);
	}

	short row_written, current_col, matrix_index = 0;
	// Write matrix row by row
	for(row_written = 0; row_written < nRows;  row_written++) {
		// Get entries from row
		for(current_col = 0; current_col < nCols; current_col++){
			fprintf(text_file, "%f ", matrix_space[matrix_index++]);
		}
		// Move position back one, to write newline over last space.
		fseek(text_file, -1, SEEK_CUR);
		fputc('\n', text_file);
	} // Entire array written
	fclose(text_file);

	return text_file;
}