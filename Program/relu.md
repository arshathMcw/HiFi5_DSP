```c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <xtensa/config/core-isa.h>
#include "xtensa/tie/xt_hifi5.h"
#define N 223
int isSame(int16_t arr1[N],int16_t arr2[N]){
	for(int idx =0;idx < N;idx++){
		if(arr1[idx] != arr2[idx]) return 0;
	}
	return 1;
}
void relu_simd(int16_t arr[N],int16_t output[N]){
	ae_int16x4 zero = AE_ZERO16();
	xtbool4 mask;
	for(int idx = 0;idx < N;idx+=4){
		ae_int16x4 *ptr = (ae_int16x4 *)&arr[idx];
		ae_int16x4 *out_ptr = (ae_int16x4*)&output[idx];
		ae_int16x4 val = AE_L16X4_I(ptr, 0);
		mask = AE_INT16X4_LT_INT16X4(val,zero);
		AE_MOVT16X4(val, zero, mask);
		AE_S16X4_I(val, out_ptr, 0);
	}
}

void relu_scalar(int16_t arr[N],int16_t output[N]){
	for(int idx = 0;idx < N;idx++){
		output[idx] = arr[idx] < 0 ? 0 : arr[idx];
	}
}

int main(){
	srand(time(NULL));
	int16_t arr[N],output_normal[N],output_intrinsics[N];
	for(int idx = 0;idx < N;idx++){
		arr[idx] = (rand() % 21) - 10;
	}
	relu_scalar(arr,output_normal);
	relu_simd(arr,output_intrinsics);
	if(isSame(output_normal,output_intrinsics)){
		printf("It is Same :)\n");
	}
	else{
		printf("It is Not Same :(\n");
	}
}
```