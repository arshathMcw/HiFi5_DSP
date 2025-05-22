* Typecasted the multiplied output from 32x4 into 16x4
```c
#include <stdio.h>
#include <stdint.h>
#include "xtensa/tie/xt_hifi5.h"

void print_ae_int16x4(ae_int16x4 reg) {
    int16_t temp[4];
    AE_S16X4_I(reg, (ae_int16x4 *) temp, 0);
    printf("ae_int16x4: [%d, %d, %d, %d]\n", temp[0], temp[1], temp[2], temp[3]);
}
void multiply_intrinsic(int16_t *arr, int16_t *arr2, int16_t *result, int size) {
	int16_t temp[4];
    ae_int16x4 *vec1_ptr = (ae_int16x4 *) arr;
    ae_int16x4 *vec2_ptr = (ae_int16x4 *) arr2;
    ae_int16x4 *res_ptr  = (ae_int16x4 *) result;
    ae_int16x4 *temp_ptr  = (ae_int16x4 *) temp;
    ae_int16x4 vec1, vec2, zero , casted_prod;
    ae_int32x4 prod;
    xtbool4 mask;
    for (int times = 0; times < size / 4; times++) {
        // AE_SEXT32X2D16_10
        // AE_SEXT32X2D16_32 
        AE_L16X4_IP(vec1, vec1_ptr, 8);
        AE_L16X4_IP(vec2, vec2_ptr, 8);
        prod = AE_INT16X4_MUL_INT16X4(vec1,vec2);
        casted_prod = (ae_int16x4) prod;
        zero = (ae_int16x4)(0);
        mask = AE_INT16X4_EQ_INT16X4(vec2, zero);
        AE_MOVT16X4(casted_prod,vec1,mask);
        AE_S16X4_IP(casted_prod, res_ptr, 8);
    }
}
void multiply_scalar(int16_t *arr, int16_t *arr2, int16_t *result, int size) {
	for(int idx = 0;idx < size;idx++){
		result[idx] = arr2[idx] != 0 ? arr[idx] * arr2[idx] : arr[idx];
	}
}
void compare(int16_t *result, int16_t *result2,int size){
	for(int idx = 0;idx < size;idx++){
		if(result[idx] != result2[idx]){
			printf("Value are Not Equal at Index : %d\n",idx);
			return;
		}
	}
	printf("Both are Equal\n");
}
int main() {
    int size = 8;
    int16_t arr1[8] = {1,2,3,4,5,6,7,8};
	int16_t arr2[8] = {1,2,3,4,0,6,0,8};
    int16_t result_intrinsic[size],result_normal[size];
    multiply_intrinsic(arr1, arr2, result_intrinsic, size);
    multiply_scalar(arr1, arr2, result_normal, size);
    compare(result_normal,result_intrinsic,size);
    return 0;
}
```

* Need to complete
```c
#include <stdio.h>
#include <stdint.h>
#include "xtensa/tie/xt_hifi5.h"

void print_ae_int16x4(ae_int16x4 reg) {
    int16_t temp[4];
    AE_S16X4_I(reg, (ae_int16x4 *) temp, 0);
    printf("ae_int16x4: [%d, %d, %d, %d]\n", temp[0], temp[1], temp[2], temp[3]);
}
void print_ae_int32x2(ae_int32x2 reg) {
    int32_t temp[2];
    AE_S32X2_I(reg, (ae_int32x2 *) temp, 0);

    printf("ae_int16x4: [%d, %d]\n", temp[0], temp[1]);
}
void multiply_intrinsic(int16_t *arr, int16_t *arr2, int32_t *result, int size) {
	int16_t temp[4];
    ae_int16x4 *vec1_ptr = (ae_int16x4 *) arr;
    ae_int16x4 *vec2_ptr = (ae_int16x4 *) arr2;
    ae_int32x2 *res_ptr  = (ae_int32x2 *) result;
    ae_int16x4 *temp_ptr  = (ae_int16x4 *) temp;
    ae_int16x4 vec1, vec2 , casted_prod;
    ae_int32x4 prod;
    ae_int32x2 zero;
    xtbool2 mask1,mask2;
    for (int times = 0; times < size / 4; times++) {
        AE_L16X4_IP(vec1, vec1_ptr, 8);
        AE_L16X4_IP(vec2, vec2_ptr, 8);
        ae_int32x2 vec1_low = AE_SEXT32X2D16_32(vec1);
        ae_int32x2 vec1_high = AE_SEXT32X2D16_10(vec1);
        ae_int32x2 vec2_low = AE_SEXT32X2D16_32(vec2);
        ae_int32x2 vec2_high = AE_SEXT32X2D16_10(vec2);
        prod = AE_INT16X4_MUL_INT16X4(vec1,vec2);
        ae_int32x2 prod_low = AE_SEXT32X2D16_32(prod);
        ae_int32x2 prod_high = AE_SEXT32X2D16_10(prod);
        zero = (ae_int32x2)(0);
        mask1 = AE_INT32X2_EQ_INT32X2(vec2_low, zero);
        mask2 = AE_INT32X2_EQ_INT32X2(vec2_high, zero);
        AE_MOVT32X2(prod_low,vec1_low,mask2);
        AE_S32X2_IP(prod_low, res_ptr, 8);
        AE_MOVT32X2(prod_high,vec1_high,mask1);
        AE_S32X2_IP(prod_high, res_ptr, 8);
    }
}
void multiply_scalar(int16_t *arr, int16_t *arr2, int32_t *result, int size) {
	for(int idx = 0;idx < size;idx++){
		result[idx] = arr2[idx] != 0 ? arr[idx] * arr2[idx] : arr[idx];
	}
}
void compare(int32_t *result, int32_t *result2,int size){
	for(int idx = 0;idx < size;idx++){
		if(result[idx] != result2[idx]){
			printf("Value are Not Equal at Index : %d\n",idx);
			return;
		}
	}
	printf("Both are Equal\n");
}
int main() {
    int size = 8;
    int16_t arr1[8] = {1,2,3,4,5,6,7,8};
	int16_t arr2[8] = {1,2,3,4,0,6,0,8};
    int32_t result_intrinsic[size],result_normal[size];
    multiply_intrinsic(arr1, arr2, result_intrinsic, size);
//    for(int idx = 0;idx < 8;idx++){
//    	printf("%d ",result_intrinsic[idx]);
//    }
    multiply_scalar(arr1, arr2, result_normal, size);
    compare(result_normal,result_intrinsic,size);
    return 0;
}
```