```c
#include<stdio.h>
#include <stdint.h>
#include <xtensa/config/core-isa.h>
#include "xtensa/tie/xt_hifi5.h"
#define SIZE 12
#define ALIGN(x) __attribute__((aligned(x)))
int compare(int mat1[SIZE][SIZE],int mat2[SIZE][SIZE]){
	for(int r =0;r < SIZE;r++){
		for(int c = 0;c < SIZE;c++){
			if(mat1[r][c] != mat2[r][c]) return 0;
		}
	}
	return 1;
}
void swapScalar(int input[SIZE][SIZE]){
	int half = SIZE / 2;
	for(int r = 0;r < SIZE;r++){
		for(int c = 0;c < half;c++){
			int temp = input[r][half+c];
			input[r][half+c] = input[r][c];
			input[r][c] = temp;
		}
	}
}

void print_ae_int32x2(ae_int32x2 reg){
    int32_t temp[2] __attribute__((aligned(8)));
    AE_S32X2_I(reg, (ae_int32x2 *) temp, 0);
    printf("%d %d", temp[0], temp[1]);
    printf("\n");
}



void swapSIMDLU2(int input[SIZE][SIZE]){
	int half = SIZE / 2;
	ae_int32x2 temp,temp2;
	for(int r = 0;r < SIZE;r++){
		ae_int32x2 *firstHalfPtr = (ae_int32x2 *)&input[r][0];
		ae_int32x2 *secondHalfPtr = (ae_int32x2 *)&input[r][half];
		ae_int32x2 *thirdHalfPtr = (ae_int32x2 *)&input[r][0];
		ae_int32x2 *fourthHalfPtr = (ae_int32x2 *)&input[r][half];
		ae_valign align,align2;
		align = AE_LA64_PP(firstHalfPtr);
		align2 = AE_LA64_PP(secondHalfPtr);
		for(int c = 0; c < half-1; c += 2){
			AE_LA32X2_IP(temp, align,firstHalfPtr);
			AE_LA32X2_IP(temp2,align2, secondHalfPtr);
			AE_SA32X2_IP(temp2, align,thirdHalfPtr);
			AE_SA32X2_IP(temp,align2, fourthHalfPtr);
		}

	}
}
void print2D(int input[SIZE][SIZE]){
	for(int r = 0;r < SIZE;r++){
		for(int c = 0;c < SIZE;c++){
			printf("%d ",input[r][c]);
		}
		printf("\n");
	}
}

void swapSIMD_simple(int input[SIZE][SIZE]) {
    int half = SIZE / 2;
    ae_int32x2 tempL, tempR;
    for (int r = 0; r < SIZE; r++) {
        ae_int32x2 *left = (ae_int32x2 *)&input[r][0];
        ae_int32x2 *right = (ae_int32x2 *)&input[r][half];
        for (int c = 0; c < half / 2; c++) {
            tempL = *left;
            tempR = *right;
            *left++ = tempR;
            *right++ = tempL;
        }
    }
}

int main(){
	int input[SIZE][SIZE],input2[SIZE][SIZE];
	for(int r = 0;r < SIZE;r++){
		for(int c = 0;c < SIZE;c++){
			input[r][c] = (r * SIZE) + c + 1;
			input2[r][c] = (r * SIZE) + c + 1;
		}
	}
	swapScalar(input);
	swapSIMD_simple(input2);
	if(compare(input,input2)){
		printf("Output are Same\n");
	}
	else{
		printf("Output are not same\n");
	}
}
```