#include <stdlib.h>
#include <stdio.h>

double *parse_text_matrix(const char *path, unsigned short *nrows, unsigned short *ncols);

int main() {
	const char* file_name = "dummy_4x4.txt";

	double *matrix;
	short nRows, nCols;

	matrix = parse_text_matrix(file_name, &nRows, &nCols);
	printf("Matrix is %i x %i.\n", nRows, nCols);

	char i,j;
	for(i = 0; i < nRows; i++){
		for(j = 0; j < nCols; j++){
			printf("%lf ", matrix[i * nCols + j]);
		}
		puts("");
	}

	return EXIT_SUCCESS;
}