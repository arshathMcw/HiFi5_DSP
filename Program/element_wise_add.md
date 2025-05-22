## Question
```
Need to do ElementWise Addition for 2 Arrays and store it in Result Array
```
## Things I learned
* Handling short datatype ae_int16x4
* Load and Store withiout the align (AE_L16X4_IP,AE_S16X4_IP)
* Offset (1nt16_t is 2 bytes and ae_int16x4 holds 4 elemnts so 2x4 = 8 bytes)
* Tried Solution with Loop unrolling `(for(int times = 0;times+ k - 1  < size / 4;times+=k){..})`
## Scalar
* Time for Simulation = 35.78 seconds
* Cycles: total = 33766300
```c
#include <stdio.h>
#include <stdint.h>
#include "xtensa/tie/xt_hifi5.h"
int main(int argc, char **argv )
{
	int16_t size = 20000;
	int16_t arr1[size],arr2[size],result[size];

	for(int idx = 0;idx < size;idx++){
		arr1[idx] = idx;
		arr2[idx] = idx;
	}
	for(int idx = 0;idx < size;idx++){
		result[idx] = arr1[idx]+arr2[idx];
	}
	printf("Array 1 : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",arr1[idx]);
	}
	printf("\nArray 2 : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",arr2[idx]);
	}
	printf("\nResult : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",result[idx]);
	}
	return 0;
}
```
## Intrinsic
* Time for Simulation = 33.90 seconds
* Cycles: total = 33708104
```c
#include <stdio.h>
#include <stdint.h>
#include "xtensa/tie/xt_hifi5.h"
void add_elementwise(int16_t * arr,int16_t * arr2,int16_t *result,int16_t size){
	ae_int16x4 *vec1_ptr=(ae_int16x4 *) &arr[0];
	ae_int16x4 *vec2_ptr=(ae_int16x4 *) &arr2[0];
	ae_int16x4 *res_ptr=(ae_int16x4 *) &result[0];
	ae_int16x4 vec1,vec2;
	int cnt = 0;
	for(int times = 0;times  < size / 4;times++){
		AE_L16X4_IP(vec1,vec1_ptr,8);
		AE_L16X4_IP(vec2,vec2_ptr,8);
		ae_int16x4 result_vec = AE_ADD16(vec1 ,vec2);
		AE_S16X4_IP(result_vec, res_ptr, 8);
		cnt++;
	}
	printf("Count : %d\n",cnt);
}
int main(int argc, char **argv )
{
	int16_t size = 200;
	int16_t arr1[size],arr2[size],result[size];

	for(int idx = 0;idx < size;idx++){
		arr1[idx] = idx;
		arr2[idx] = idx;
	}
	add_elementwise(arr1,arr2,result,size);
	printf("Array 1 : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",arr1[idx]);
	}
	printf("\nArray 2 : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",arr2[idx]);
	}
	printf("\nResult : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",result[idx]);
	}
	return 0;
}
```
## Intrinsic - Unrolling
* Time for Simulation = 36.60 seconds
* Cycles: total = 33708366
```c
#include <stdio.h>
#include <stdint.h>
#include "xtensa/tie/xt_hifi5.h"
void add_elementwise(int16_t * arr,int16_t * arr2,int16_t *result,int16_t size){
	ae_int16x4 *vec1_ptr=(ae_int16x4 *) &arr[0];
	ae_int16x4 *vec2_ptr=(ae_int16x4 *) &arr2[0];
	ae_int16x4 *res_ptr=(ae_int16x4 *) &result[0];
	ae_int16x4 vec1,vec2;
	int k = 2;
	int cnt = 0;
	for(int times = 0;times+ k - 1  < size / 4;times+=k){
		AE_L16X4_IP(vec1,vec1_ptr,8);
		AE_L16X4_IP(vec2,vec2_ptr,8);
		ae_int16x4 result_vec = AE_ADD16(vec1 ,vec2);
		AE_S16X4_IP(result_vec, res_ptr, 8);

		AE_L16X4_IP(vec1,vec1_ptr,8);
		AE_L16X4_IP(vec2,vec2_ptr,8);
		result_vec = AE_ADD16(vec1 ,vec2);
		AE_S16X4_IP(result_vec, res_ptr, 8);
		cnt++;
	}
	printf("Count : %d\n",cnt);
}


int main(int argc, char **argv )
{
	int16_t size = 200;
	int16_t arr1[size],arr2[size],result[size];

	for(int idx = 0;idx < size;idx++){
		arr1[idx] = idx;
		arr2[idx] = idx;
	}
	add_elementwise(arr1,arr2,result,size);
	printf("Array 1 : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",arr1[idx]);
	}
	printf("\nArray 2 : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",arr2[idx]);
	}
	printf("\nResult : \n");
	for(int idx = 0;idx < size;idx++){
		printf("%d ",result[idx]);
	}
	return 0;
}
```