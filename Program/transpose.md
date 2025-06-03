```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xtensa/tie/xt_hifi5.h>
#define SIZE 8
int compare(int16_t mat1[SIZE][SIZE],int16_t mat2[SIZE][SIZE]){
	for(int r =0;r < SIZE;r++){
		for(int c = 0;c < SIZE;c++){
			if(mat1[r][c] != mat2[r][c]) return 0;
		}
	}
	return 1;
}
void transposeScalar(int16_t mat[SIZE][SIZE]){
	for(int r = 0;r < SIZE;r++){
		for(int c = r + 1;c < SIZE;c++){
			int temp = mat[r][c];
			mat[r][c] = mat[c][r];
			mat[c][r] = temp;
		}
	}
}
void print_ae_int16x4(ae_int16x4 vec) {
    int16_t temp[4];
    AE_S16X4_I(vec, (ae_int16x4*)temp, 0);
    printf("[%04x, %04x, %04x, %04x]", temp[0], temp[1], temp[2], temp[3]);
}
void transposeSIMD(int rows, int cols, int16_t mat[SIZE][SIZE], int16_t output[8][8]) {
    int block_rows = (rows / 4) * 4;
    int block_cols = (cols / 4) * 4;
    for (int r = 0; r < block_rows; r += 4) {
        for (int col = 0; col < block_cols; col += 4) {
            ae_int16x4 row0, row1, row2, row3;
            ae_int16x4 a, b, c, d,e,f,g,h;
            ae_int16x4 *ptr0 = (ae_int16x4*)&mat[r+0][col];
            ae_int16x4 *ptr1 = (ae_int16x4*)&mat[r+1][col];
            ae_int16x4 *ptr2 = (ae_int16x4*)&mat[r+2][col];
            ae_int16x4 *ptr3 = (ae_int16x4*)&mat[r+3][col];
            row0 = AE_L16X4_I(ptr0, 0);
            row1 = AE_L16X4_I(ptr1, 0);
            row2 = AE_L16X4_I(ptr2, 0);
            row3 = AE_L16X4_I(ptr3, 0);
            a = AE_SEL16_7362(row0, row2);
            b = AE_SEL16_5140(row0, row2);
            c = AE_SEL16_7362(row1, row3);
            d = AE_SEL16_5140(row1, row3);
            e = AE_SEL16_7362(a, c);
            f = AE_SEL16_5140(a, c);
            g = AE_SEL16_7362(b, d);
            h = AE_SEL16_5140(b, d);
            ae_int16x4 *out_ptr0 = (ae_int16x4*)&output[col+0][r];
            ae_int16x4 *out_ptr1 = (ae_int16x4*)&output[col+1][r];
            ae_int16x4 *out_ptr2 = (ae_int16x4*)&output[col+2][r];
            ae_int16x4 *out_ptr3 = (ae_int16x4*)&output[col+3][r];
            AE_S16X4_I(e, out_ptr0, 0);
            AE_S16X4_I(f, out_ptr1, 0);
            AE_S16X4_I(g, out_ptr2, 0);
            AE_S16X4_I(h, out_ptr3, 0);
        }
    }
    for (int r = 0; r < rows; r++) {
        for (int c = block_cols; c < cols; c++) {
            output[c][r] = mat[r][c];
        }
    }

    for (int r = block_rows; r < rows; r++) {
        for (int c = 0; c < block_cols; c++) {
            output[c][r] = mat[r][c];
        }
    }

}

int main() {
	int16_t input[8][8] = {
		{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
		{0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10},
		{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18},
		{0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20},
		{0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28},
		{0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30},
		{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38},
		{0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40}
	};
	int16_t output[8][8];
	transposeSIMD(8,8,input,output);
	transposeScalar(input);
	if(compare(input,output)){
		printf("They are Equal\n");
	}
	else{
		printf("They are not Equal\n");
	}
    return 0;
}
``` 