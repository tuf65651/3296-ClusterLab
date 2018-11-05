#include <stdlib.h>
#include <stdio.h>

FILE *write_text_matrix(const char* file_path, double *matrix_space, short nRows, short nCols);

int main(){
	double matrix_space[18] = {1, 2, 4, 5, 2.3, 2.4, 2.5, 2.6, 3.0, 0, 3, 0, 4, 0, 0, 0, .02, .02127};

	const char * file_path = "matrix_out.txt";
	short numRows = 3, numCol = 6;

	write_text_matrix(file_path, matrix_space, numRows, numCol);

	return EXIT_SUCCESS;
}