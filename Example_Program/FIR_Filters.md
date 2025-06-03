## Fixed and Floating point numbers
* Computer need to represent numbers that are not in integer
* 
![alt text](image-1.png)
* But the above does not represent the 2.8 maybe it make closer (2.8125)
* And also we allocate place for fraction we reduced our dynamic range
* So to avoid this we have floating point
* Fixed point have 2 places (fraction and decimal) but floating point have (Mantissa and Exponent)
![alt text](image-2.png)
* The advantage is that it will give more range than the fixed point  but the precision will be very less
* So the floating point is standartised by the IEEE 754 . Which provide the differenct format for floating numbers
* The comman formats in IEEE 754 are Single precision and Double precision
* The single precision uses 32 bit to represent numbers and double precison use 64 bit to represent numbers
* Single precison = 1 -> Sign Bit , 8 -> Exponent (-127 - 128) , 23 -> Mantissa (every mantissa starts from 1 , so that is removed . All for the fractional part and it can have 24bit precision point (1(for every vaue so removed here) + 23fractional))
* we kno exponent represent -127 to 128 , The IEE standards takes the special cases for 2 very end (-127,128 or 0 and 255) so normal exponent represents -126 -> 127
![alt text](image-3.png)
* More exponent - More range and more smaller value
* More mantessa means more precision
![alt text](image-4.png)

## FIR program
* To run a pcm file : ffplay -f s16le -ar 44100 -ac 1 sound.pcm
* To convert wav file into pcm : ffmpeg -i mono_output.wav -f s16le -acodec pcm_s16le sound.pcm
* To get information of the wav signal : ffmpeg -i mono_output.wav
* To Convert Stereo to Mono : ffmpeg -i stereo_input.wav -ac 1 mono_output.wav
    - -ac 1 = set number of audio channels to 1 (mono)
```c
#include <stdio.h>
#include <string.h>  

#define MAX_INPUT_LEN 10
#define MAX_FLT_LEN   3
#define BUFFER_LEN (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)

int insamp[BUFFER_LEN];

void firInt(int *coeffs, int *input, int *output, int length, int filterLength) {
    int acc;
    int *coeffp;
    int *inputp;
    int n, k;
    memcpy(&insamp[filterLength - 1], input, length * sizeof(int));
    for (n = 0; n < length; n++) {
        coeffp = coeffs;
        inputp = &insamp[filterLength - 1 + n];
        acc = 0;
        for (k = 0; k < filterLength; k++) {
            int a = (*coeffp++);
            int b = (*inputp--);
            printf("a = %d b = %d\n",a,b);
            acc +=  a*b; 
            printf("Acc : %d\n",acc);
        }
        printf("\n");
        output[n] = acc;
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}
int main() {
    int input[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int length = sizeof(input) / sizeof(int);
    int output[length];
    int coeffs[] = {1, 2, 3};  
    int filterLength = sizeof(coeffs) / sizeof(int);
    // firFloatInit();
    firInt(coeffs, input, output, length, filterLength);
    printf("Filtered Output:\n");
    for (int i = 0; i < length; i++) {
        printf("%d ", output[i]);
    }
    printf("\n");
    return 0;
}
```

## FIR ins SIMD
* To implement the FIR filter , first they invert the impulse response coefficient and name them as c
* They also padding it with the N-1 Zeros (N = filter length)
* Loop vectorization
    * Outer Loop Vectorization
    * Inner Loop Vectorization
    * Outer Inner Loop Vectorization
```c
/*
 * main.c
 *
 *  Created on: 23-May-2025
 *      Author: arshathahamed10
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <xtensa/config/core-isa.h>
#include "xtensa/tie/xt_hifi5.h"
#define NORMAL 1
#define SCALAR 0

#if NORMAL

#if SCALAR

#define MAX_INPUT_LEN 12
#define MAX_FLT_LEN   4
#define BUFFER_LEN (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)
int insamp[BUFFER_LEN];
void firInt(int *coeffs, int *input, int *output, int length, int filterLength) {
    int acc;
    int *coeffp;
    int *inputp;
    int n, k;
    memcpy(&insamp[filterLength - 1], input, length * sizeof(int));
    for (n = 0; n < length; n++) {
        coeffp = coeffs;
        inputp = &insamp[filterLength - 1 + n];
        acc = 0;
        for (k = 0; k < filterLength; k++) {
            int a = (*coeffp++);
            int b = (*inputp--);
            printf("a = %d b = %d\n",a,b);
            acc +=  a*b;
            printf("Acc : %d\n",acc);
        }
        printf("\n");
        output[n] = acc;
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}
int main() {
    int input[] = {1, 2, 3, 4, 5, 6, 7, 8,9 , 10,11 , 12};
    int length = sizeof(input) / sizeof(int);
    int output[length];
    int coeffs[] = {1, 2, 3 , 4};
    int filterLength = sizeof(coeffs) / sizeof(int);
    // firFloatInit();
    firInt(coeffs, input, output, length, filterLength);
    printf("Filtered Output:\n");
    for (int i = 0; i < length; i++) {
        printf("%d ", output[i]);
    }
    printf("\n");
    return 0;
}
#else  // Normal Operation's Intrinsics


#if XCHAL_HAVE_CCOUNT
#ifndef NO_XT_RSR_CCOUNT
#include <xtensa/tie/xt_timer.h>
#endif

#ifdef XT_RSR_CCOUNT
#define GETCLOCK() XT_RSR_CCOUNT()
#else
static unsigned long inline GETCLOCK(void)
{
      unsigned long r;
        __asm__ volatile ("rsr.ccount %0" : "=r" (r));
          return r;
}
#endif
#else
#define GETCLOCK() clock();
#endif
#define TIME_DECL long long start_time, end_time;
#define TIME_START(){ start_time = GETCLOCK(); }
#define TIME_END()  { end_time   = GETCLOCK(); }
#define MAX_INPUT_LEN 50
#define MAX_FLT_LEN   4
#define BUFFER_LEN (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)
int insamp[BUFFER_LEN];
TIME_DECL;

void print_ae_int32x2(ae_int32x2 reg){
	int32_t temp[2];
	AE_S32X2_I(reg, (ae_int32x2 *) temp, 0);
	printf("%d %d",temp[0],temp[1]);
	printf("\n");
}

void firInt(int *coeffs, int *input, int *output, int length, int filterLength) {
    int acc;
    int *coeffp;
    int *inputp;
    int n, k;
    memcpy(&insamp[filterLength - 1], input, length * sizeof(int));
    for (n = 0; n < length; n++) {
        coeffp = coeffs;
        inputp = &insamp[filterLength - 1 + n];
        acc = 0;
        for (k = 0; k < filterLength; k++) {
            int a = (*coeffp++);
            int b = (*inputp--);
            acc +=  a*b;
        }
        output[n] = acc;
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}
void firIntSIMD(int *coeffs, int *input, int *output, int length, int filterLength) {
    int n, k;
    ae_int32x2 a,b;
    memcpy(&insamp[filterLength - 1], input, length * sizeof(int));
    for (n = 0; n < length; n++) {
        ae_int32x2 acc = (ae_int32x2)(0);
        ae_int32x2 *iptr = (ae_int32x2 *)&insamp[filterLength - 1 + n];
        ae_int32x2 *cptr = (ae_int32x2 *)coeffs;
        ae_valign align;
        align = AE_LA64_PP(iptr);
        for (k = 0; k < filterLength; k+=2) {
        	AE_LA32X2_RIP(a,align,iptr);
        	AE_L32X2_IP(b,(const ae_int32x2*)cptr, 8);
            ae_int32x2 mul = AE_INT32X2_MUL_INT32X2(a, b);
            acc = AE_INT32X2_ADD_INT32X2(acc,mul);
        }
        for (; k < filterLength; k++) {
			AE_LA32X2_RIP(a,align,iptr);
			AE_L32X2_IP(b,(const ae_int32x2*)cptr, 8);
			ae_int32x2 mul = AE_INT32X2_MUL_INT32X2(a, b);
			acc = AE_INT32X2_ADD_INT32X2(acc,mul);
		}
        output[n] = AE_INT32X2_RADD(acc);
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}
int compare(int *arr1,int* arr2,int length){
	for(int idx = 0;idx < length;idx++){
		if(arr1[idx] != arr2[idx]) return 0;
	}
	return 1;
}
int main() {
	int length = 40;
	int input[length];
	for(int i = 0;i < length;i++){
		input[i] = i+1;
	}
//    int input[] = {1, 2, 3, 4, 5, 6, 7, 8,9 , 10,11 , 12, 13};
//    int length = sizeof(input) / sizeof(int);
    printf("Input : ");
    for(int idx = 0;idx < length;idx++){
    	printf("%d ",input[idx]);
    }
    printf("\n");
    int output[length],output_reference[length];
    int coeffs[] = {1, 2, 3 , 4, 5};
    int filterLength = sizeof(coeffs) / sizeof(int);
    printf("Coeffs : ");
	for(int idx = 0;idx < filterLength;idx++){
		printf("%d ",coeffs[idx]);
	}
	printf("\n");
	TIME_START();
//    firIntSIMD(coeffs, input, output, length, filterLength);
    firInt(coeffs, input, output_reference, length, filterLength);
    TIME_END();
    printf("The example algorithm takes %d cycles, \n",(int)(end_time - start_time));
//   To clear Buffer
    memset(insamp, 0, sizeof(insamp));
	printf("Output : \n");
	for(int idx = 0;idx < length;idx++){
		printf("%d ",output[idx]);
	}
	printf("\n");
	printf("Output Ref : \n");
	for(int idx = 0;idx < length;idx++){
		printf("%d ",output_reference[idx]);
	}
	printf("\n");
    if(compare(output,output_reference,length)){
    	printf("Outputs are Same :)\n");
    }
    else{
    	printf("Outputs are not same :)\n");
    }
    return 0;
}
#endif
#else 

typedef struct {
    const float* x;         // input signal with (N_h-1) zeros appended
    size_t inputLength;     // N_x
    const float* c;         // reversed filter coefficients
    size_t filterLength;    // N_h
    float* y;               // output (filtered) signal
    size_t outputLength;    // should be N_x
} FilterInput;

float* applyFirFilterSingle(FilterInput* input) {
    const float* x = input->x;
    const float* c = input->c;
    float* y = input->y;

    for (size_t i = 0; i < input->outputLength; ++i) {
        y[i] = 0.f;
        for (size_t j = 0; j < input->filterLength; ++j) {
            y[i] += x[i + j] * c[j];
        }
    }
    return y;
}

int main() {
    float inputSignal[] = {1, 2, 3, 4, 5, 6, 7, 8};
    float filterCoeffs[] = {3.0f, 2.0f, 1.0f};
    size_t N_x = sizeof(inputSignal) / sizeof(inputSignal[0]);
    size_t N_h = sizeof(filterCoeffs) / sizeof(filterCoeffs[0]);

    // Create padded input
    size_t paddedLength = N_x + N_h - 1;
    float* paddedInput = (float*)calloc(paddedLength, sizeof(float));
    for (size_t i = 0; i < N_x; ++i) {
        paddedInput[i] = inputSignal[i];
    }

    // Output array
    float* output = (float*)calloc(N_x, sizeof(float));

    FilterInput input = {
        .x = paddedInput,
        .inputLength = N_x,
        .c = filterCoeffs,
        .filterLength = N_h,
        .y = output,
        .outputLength = N_x
    };

    applyFirFilterSingle(&input);

    // Print results
    printf("Filtered output:\n");
    for (size_t i = 0; i < N_x; ++i) {
        printf("%f ", output[i]);
    }
    printf("\n");

    // Free allocated memory
    free(paddedInput);
    free(output);

    return 0;
}
#endif
```