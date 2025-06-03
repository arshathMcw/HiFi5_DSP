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
