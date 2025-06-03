```c
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <xtensa/config/core-isa.h>
#include "xtensa/tie/xt_hifi5.h"
#define NORMAL 1
#define SCALAR 0
#define INNER_LOOP_VECTORIZATION 1
#define OUTER_LOOP_VECTORIZATION 0
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
#define ALIGN(x) __attribute__((aligned(x)))
#define TIME_DECL long long start_time, end_time;
#define TIME_START(){ start_time = GETCLOCK(); }
#define TIME_END()  { end_time   = GETCLOCK(); }
#define INPUT_LEN 200
#define FLT_LEN   63
#define BUFFER_LEN (FLT_LEN - 1 + INPUT_LEN)
int insamp[BUFFER_LEN];
TIME_DECL;

void firInt(int *coeffs, int *input, int *output, int length, int filterLength) {
    memcpy(&insamp[filterLength - 1], input, length * sizeof(int));
    for (int n = 0; n < length; n++) {
        int acc = 0;
        for (int k = 0; k < filterLength; k++) {
            acc += coeffs[k] * insamp[filterLength - 1 + n - k];
        }
        output[n] = acc;
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}
void print_ae_int32x2(ae_int32x2 reg){
	int32_t temp[2];
	AE_S32X2_I(reg, (ae_int32x2 *) temp, 0);
	printf("%d %d",temp[0],temp[1]);
	printf("\n");
}
int compare(int *arr1,int* arr2,int length){
	for(int idx = 0;idx < length;idx++){
		if(arr1[idx] != arr2[idx]) return 0;
	}
	return 1;
}
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
#if INNER_LOOP_VECTORIZATION

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
        for (k = 0; k+4 < filterLength; k+=4) {
        	AE_LA32X2_RIP(a,align,iptr);
        	AE_L32X2_IP(b,(const ae_int32x2*)cptr, 8);
            ae_int32x2 mul = AE_INT32X2_MUL_INT32X2(a, b);
            acc = AE_INT32X2_ADD_INT32X2(acc,mul);

            AE_LA32X2_RIP(a,align,iptr);
			AE_L32X2_IP(b,(const ae_int32x2*)cptr, 8);
			mul = AE_INT32X2_MUL_INT32X2(a, b);
			acc = AE_INT32X2_ADD_INT32X2(acc,mul);
        }
        if(filterLength % 4 == 3){
        	AE_LA32X2_RIP(a,align,iptr);
			AE_L32X2_IP(b,(const ae_int32x2*)cptr, 8);
			ae_int32x2 mul = AE_INT32X2_MUL_INT32X2(a, b);
			acc = AE_INT32X2_ADD_INT32X2(acc,mul);
		}
        if(filterLength % 2){
        	int lastCoeff = coeffs[filterLength - 1];
			int lastInput = insamp[filterLength - 1 + n - filterLength + 1];
			ae_int32x2 temp = AE_MOVDA32X2(lastCoeff * lastInput,0);
			acc = AE_INT32X2_ADD_INT32X2(acc, temp);
        }
        output[n] = AE_INT32X2_RADD(acc);
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}

#elif OUTER_LOOP_VECTORIZATION

void firIntSIMD(int *coeffs, int *input, int *output, int length, int filterLength) {
	printf("Outer Loop : \n");
    int n, k;
    ae_int32x2 a, b, c, d;
    ae_int32x2 acc1, acc2;
    memcpy(&insamp[filterLength - 1], input, length * sizeof(int));
    for (n = 0; n < length - 1; n += 2) {
        acc1 = (ae_int32x2)(0);
        acc2 = (ae_int32x2)(0);
        ae_int32x2 *iptr1 = (ae_int32x2 *)&insamp[filterLength - 1 + n];
        ae_int32x2 *iptr2 = (ae_int32x2 *)&insamp[filterLength - 1 + n + 1];
        ae_int32x2 *cptr = (ae_int32x2 *)coeffs;

        ae_valign align1, align2;
        align1 = AE_LA64_PP(iptr1);
        align2 = AE_LA64_PP(iptr2);
        for (k = 0; k + 1 < filterLength; k += 2) {
            AE_LA32X2_RIP(a, align1, iptr1);
            AE_LA32X2_RIP(c, align2, iptr2);
            AE_L32X2_IP(b, (const ae_int32x2*)cptr, 8);
            ae_int32x2 mul1 = AE_INT32X2_MUL_INT32X2(a, b);
            ae_int32x2 mul2 = AE_INT32X2_MUL_INT32X2(c, b);
            acc1 = AE_INT32X2_ADD_INT32X2(acc1, mul1);
            acc2 = AE_INT32X2_ADD_INT32X2(acc2, mul2);
        }
        if (filterLength % 2) {
            int lastCoeff = coeffs[filterLength - 1];
            int lastInput1 = insamp[filterLength - 1 + n - filterLength + 1];
            int lastInput2 = insamp[filterLength - 1 + n + 1 - filterLength + 1];
            ae_int32x2 temp1= AE_MOVDA32X2(lastCoeff * lastInput1, 0);
            ae_int32x2 temp2 = AE_MOVDA32X2(lastCoeff * lastInput2, 0);
            acc1 = AE_INT32X2_ADD_INT32X2(acc1, temp1);
            acc2 = AE_INT32X2_ADD_INT32X2(acc2, temp2);
        }
        output[n] = AE_INT32X2_RADD(acc1);
        output[n + 1] = AE_INT32X2_RADD(acc2);
    }
    if (length % 2) {
        ae_int32x2 acc = (ae_int32x2)(0);
        ae_int32x2 *iptr = (ae_int32x2 *)&insamp[filterLength - 1 + (length - 1)];
        ae_int32x2 *cptr = (ae_int32x2 *)coeffs;
        ae_valign align;
        align = AE_LA64_PP(iptr);
        for (k = 0; k + 1 < filterLength; k += 2) {
            AE_LA32X2_RIP(a, align, iptr);
            AE_L32X2_IP(b, (const ae_int32x2*)cptr, 8);
            ae_int32x2 mul = AE_INT32X2_MUL_INT32X2(a, b);
            acc = AE_INT32X2_ADD_INT32X2(acc, mul);
        }
        if (filterLength % 2) {
            int lastCoeff = coeffs[filterLength - 1];
            int lastInput = insamp[filterLength - 1 + (length - 1) - filterLength + 1];
            ae_int32x2 temp = AE_MOVDA32X2(lastCoeff * lastInput, 0);
            acc = AE_INT32X2_ADD_INT32X2(acc, temp);
        }
        output[length - 1] = AE_INT32X2_RADD(acc);
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}


void firIntSIMDFour(int *coeffs, int *input, int *output, int length, int filterLength) {
    int n, k;
    ae_int32x2 a1, a2, a3, a4, b;
    ae_int32x2 acc1, acc2, acc3, acc4;
    memcpy(&insamp[filterLength - 1], input, length * sizeof(int));
    for (n = 0; n < length - 3; n += 4) {
        acc1 = (ae_int32x2)(0);
        acc2 = (ae_int32x2)(0);
        acc3 = (ae_int32x2)(0);
        acc4 = (ae_int32x2)(0);
        ae_int32x2 *iptr1 = (ae_int32x2 *)&insamp[filterLength - 1 + n];
        ae_int32x2 *iptr2 = (ae_int32x2 *)&insamp[filterLength - 1 + n + 1];
        ae_int32x2 *iptr3 = (ae_int32x2 *)&insamp[filterLength - 1 + n + 2];
        ae_int32x2 *iptr4 = (ae_int32x2 *)&insamp[filterLength - 1 + n + 3];
        ae_int32x2 *cptr = (ae_int32x2 *)coeffs;
        ae_valign align1, align2, align3, align4;
        align1 = AE_LA64_PP(iptr1);
        align2 = AE_LA64_PP(iptr2);
        align3 = AE_LA64_PP(iptr3);
        align4 = AE_LA64_PP(iptr4);
        for (k = 0; k + 1 < filterLength; k += 2) {
            AE_LA32X2_RIP(a1, align1, iptr1);
            AE_LA32X2_RIP(a2, align2, iptr2);
            AE_LA32X2_RIP(a3, align3, iptr3);
            AE_LA32X2_RIP(a4, align4, iptr4);
            AE_L32X2_IP(b, (const ae_int32x2*)cptr, 8);
            acc1 = AE_INT32X2_ADD_INT32X2(acc1, AE_INT32X2_MUL_INT32X2(a1, b));
            acc2 = AE_INT32X2_ADD_INT32X2(acc2, AE_INT32X2_MUL_INT32X2(a2, b));
            acc3 = AE_INT32X2_ADD_INT32X2(acc3, AE_INT32X2_MUL_INT32X2(a3, b));
            acc4 = AE_INT32X2_ADD_INT32X2(acc4, AE_INT32X2_MUL_INT32X2(a4, b));
        }
        if (filterLength % 2) {
            int lastCoeff = coeffs[filterLength - 1];
            acc1 = AE_INT32X2_ADD_INT32X2(acc1, AE_MOVDA32X2(lastCoeff * insamp[n], 0));
            acc2 = AE_INT32X2_ADD_INT32X2(acc2, AE_MOVDA32X2(lastCoeff * insamp[n + 1], 0));
            acc3 = AE_INT32X2_ADD_INT32X2(acc3, AE_MOVDA32X2(lastCoeff * insamp[n + 2], 0));
            acc4 = AE_INT32X2_ADD_INT32X2(acc4, AE_MOVDA32X2(lastCoeff * insamp[n + 3], 0));
        }
        output[n] = AE_INT32X2_RADD(acc1);
        output[n + 1] = AE_INT32X2_RADD(acc2);
        output[n + 2] = AE_INT32X2_RADD(acc3);
        output[n + 3] = AE_INT32X2_RADD(acc4);
    }
    for (; n < length; n++) {
        ae_int32x2 acc = (ae_int32x2)(0);
        ae_int32x2 *iptr = (ae_int32x2 *)&insamp[filterLength - 1 + n];
        ae_int32x2 *cptr = (ae_int32x2 *)coeffs;
        ae_valign align;
        align = AE_LA64_PP(iptr);

        for (k = 0; k + 1 < filterLength; k += 2) {
            AE_LA32X2_RIP(a1, align, iptr);
            AE_L32X2_IP(b, (const ae_int32x2*)cptr, 8);
            acc = AE_INT32X2_ADD_INT32X2(acc, AE_INT32X2_MUL_INT32X2(a1, b));
        }

        if (filterLength % 2) {
            int lastCoeff = coeffs[filterLength - 1];
            int lastInput = insamp[filterLength - 1 + n - filterLength + 1];
            acc = AE_INT32X2_ADD_INT32X2(acc, AE_MOVDA32X2(lastCoeff * lastInput, 0));
        }

        output[n] = AE_INT32X2_RADD(acc);
    }
    memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int));
}
#endif

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

int main() {
	int input[INPUT_LEN],coeffs[FLT_LEN];
	for(int i = 0;i < INPUT_LEN;i++){
		input[i] = i+1;
	}
	for(int i = 0;i < FLT_LEN;i++){
		coeffs[i] = i+1;
	}
    int output[INPUT_LEN],output_reference[INPUT_LEN];
	TIME_START();
    firInt(coeffs, input, output_reference, INPUT_LEN, FLT_LEN);
////   To clear Buffer
//    memset(insamp, 0, sizeof(insamp));

//	firIntSIMD(coeffs, input, output, INPUT_LEN, FLT_LEN);
    TIME_END();
    printf("The example algorithm takes %d cycles, \n",(int)(end_time - start_time));
    if(compare(output,output_reference,INPUT_LEN)){
    	printf("Outputs are Same :)\n` ");
    }
    else{
    	printf("Outputs are not same :)\n");
    }
    return 0;
}
```