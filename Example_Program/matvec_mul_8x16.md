* Column 1 is multiplied by 16
* Actual Code
```c
#include <xtensa/config/core-isa.h>

#if (1== XCHAL_HAVE_HIFI5 )

#include <xtensa/tie/xt_hifi5.h>
#include "matvec_mul_8x16.h"

/***************************************************************************
 * Matrix vector multiplication of 8 bit matrix and 16 bit vectors
 * with 64 bit output.
 * Function computes multiplication of an 8 bit integer input matrix (MxN)
 * with P 16 bit integer vectors (Nx1) and outputs P 64 bit integer
 * vectors (MX1)
 *
 * Arguments:
 * p_out        : Pointer to output matrix
 * p_mat        : Pointer to input matrix, must be 8-byte aligned
 * p_vec        : Pointer to first input vector, must be 16-byte aligned
 * rows         : Number of rows in input matrix, must be multiple of 16
 * cols         : Number of columns in input matrix, must be multiple of 32
 * row_offset   : Offset to next row of input matrix, must be multiple of 16
 * vec_count    : Number of input vectors
 * vec_offset   : Offset to next input vector, must be multiple of 32
 * out_offset   : Offset to next row of output
 **************************************************************************/
void matvec_mul_8x16_64 (
        long long * __restrict__ p_out,
        const signed char * __restrict__ p_mat,
        const short * __restrict__ p_vec,
        int rows,
        int cols,
        int row_offset,
        int vec_count,
        int vec_offset,
        int out_offset
        )
#if XCHAL_HAVE_HIFI5_NN_MAC
{
    int row = 0, col = 0, vec = 0;

    for(vec = 0; vec < vec_count; vec++)
    {
        ae_int16x4 vec00, vec01;
        ae_int16x4 vec10, vec11;
        ae_int16x4 vec20, vec21;
        ae_int16x4 vec30, vec31;

        ae_int64 *p_dst0 = (ae_int64 *)&p_out[vec];
        row = 0;
        if(rows >= 16)
        {
            for (row = 0; row < ( rows & ~(15)) ; row += 16)
            {
                ae_int16x8 *p_vec0 = (ae_int16x8 *)&p_vec[vec *vec_offset];
                ae_int16x8 *p_vec1 = (ae_int16x8 *)&p_vec[vec *vec_offset + 8];

                ae_int64 accu00, accu10, accu20, accu30;
                ae_int64 accu01, accu11, accu21, accu31;
                ae_int64 accu02, accu12, accu22, accu32;
                ae_int64 accu03, accu13, accu23, accu33;
                ae_int8x8 mat00_0, mat10_0, mat20_0, mat30_0;
                ae_int8x8 mat01_0, mat11_0, mat21_0, mat31_0;
                ae_int8x8 mat00_1, mat10_1, mat20_1, mat30_1;
                ae_int8x8 mat01_1, mat11_1, mat21_1, mat31_1;
                ae_int8x8 mat00_2, mat10_2, mat20_2, mat30_2;
                ae_int8x8 mat01_2, mat11_2, mat21_2, mat31_2;
                ae_int8x8 mat00_3, mat10_3, mat20_3, mat30_3;
                ae_int8x8 mat01_3, mat11_3, mat21_3, mat31_3;

                ae_int8x16 *p_mat0 = (ae_int8x16 *)&p_mat[(row+4*0+0) * row_offset];
                ae_int8x16 *p_mat1 = (ae_int8x16 *)&p_mat[(row+4*1+0) * row_offset];
                ae_int8x16 *p_mat2 = (ae_int8x16 *)&p_mat[(row+4*2+0) * row_offset];
                ae_int8x16 *p_mat3 = (ae_int8x16 *)&p_mat[(row+4*3+0) * row_offset];

                accu00 = AE_ZERO64(); accu10 = AE_ZERO64();    accu20 = AE_ZERO64(); accu30 = AE_ZERO64();
                accu01 = AE_ZERO64(); accu11 = AE_ZERO64();    accu21 = AE_ZERO64(); accu31 = AE_ZERO64();
                accu02 = AE_ZERO64(); accu12 = AE_ZERO64();    accu22 = AE_ZERO64(); accu32 = AE_ZERO64();
                accu03 = AE_ZERO64(); accu13 = AE_ZERO64();    accu23 = AE_ZERO64(); accu33 = AE_ZERO64();

                AE_L16X4X2_IP(vec00, vec01, p_vec0, 32);
                AE_L16X4X2_IP(vec10, vec11, p_vec1, 32);
                AE_L16X4X2_IP(vec20, vec21, p_vec0, 32);
                AE_L16X4X2_IP(vec30, vec31, p_vec1, 32);

                /* This kernel implements a 16x32 by 32x1 matrix multiplication */
                for (col = 1; col < (cols>>5); col += 1)
                {
                    AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                    AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                    AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                    AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec00, vec01);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec10, vec11);

                    AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                    AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                    AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                    AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec20, vec21);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec30, vec31);

                    AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                    AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                    AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                    AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec00, vec01);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec10, vec11);

                    AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                    AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                    AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                    AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec20, vec21);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec30, vec31);

                    AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                    AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                    AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                    AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec00, vec01);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec10, vec11);

                    AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                    AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                    AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                    AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec20, vec21);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec30, vec31);

                    AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                    AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                    AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                    AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec00, vec01);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec10, vec11);

                    AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                    AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                    AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                    AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec20, vec21);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec30, vec31);

                    AE_L16X4X2_IP(vec00, vec01, p_vec0, 32);
                    AE_L16X4X2_IP(vec10, vec11, p_vec1, 32);
                    AE_L16X4X2_IP(vec20, vec21, p_vec0, 32);
                    AE_L16X4X2_IP(vec30, vec31, p_vec1, 32);
                }

                AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec00, vec01);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec10, vec11);

                AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec20, vec21);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec30, vec31);


                AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec00, vec01);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec10, vec11);

                AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec20, vec21);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec30, vec31);


                AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec00, vec01);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec10, vec11);

                AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec20, vec21);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec30, vec31);


                AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec00, vec01);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec10, vec11);

                AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec20, vec21);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec30, vec31);

                p_dst0[(row+4*0+0) * out_offset] = accu00;
                p_dst0[(row+4*0+1) * out_offset] = accu10;
                p_dst0[(row+4*0+2) * out_offset] = accu20;
                p_dst0[(row+4*0+3) * out_offset] = accu30;

                p_dst0[(row+4*1+0) * out_offset] = accu01;
                p_dst0[(row+4*1+1) * out_offset] = accu11;
                p_dst0[(row+4*1+2) * out_offset] = accu21;
                p_dst0[(row+4*1+3) * out_offset] = accu31;

                p_dst0[(row+4*2+0) * out_offset] = accu02;
                p_dst0[(row+4*2+1) * out_offset] = accu12;
                p_dst0[(row+4*2+2) * out_offset] = accu22;
                p_dst0[(row+4*2+3) * out_offset] = accu32;

                p_dst0[(row+4*3+0) * out_offset] = accu03;
                p_dst0[(row+4*3+1) * out_offset] = accu13;
                p_dst0[(row+4*3+2) * out_offset] = accu23;
                p_dst0[(row+4*3+3) * out_offset] = accu33;

            }
        }
    }
}
#else
{
    int row = 0, col = 0, vec = 0;

    for(vec = 0; vec < vec_count; vec++)
    {
        ae_int16x4 vec0, vec1;
        ae_int8x8 mat00, mat01, mat02, mat03, mat04, mat05, mat06, mat07;
        ae_int8x8 mat10, mat11, mat12, mat13, mat14, mat15, mat16, mat17;

        ae_int64 *p_dst0 = (ae_int64 *)&p_out[vec];
        for (row = 0; row < rows; row += 8)
        {
            ae_int8x16 *p_mat0 = (ae_int8x16 *)&p_mat[row * row_offset];
            ae_int8x16 *p_mat1 = (ae_int8x16 *)&p_mat[(row + 1) * row_offset];
            ae_int8x16 *p_mat2 = (ae_int8x16 *)&p_mat[(row + 2) * row_offset];
            ae_int8x16 *p_mat3 = (ae_int8x16 *)&p_mat[(row + 3) * row_offset];
            ae_int8x16 *p_mat4 = (ae_int8x16 *)&p_mat[(row + 4) * row_offset];
            ae_int8x16 *p_mat5 = (ae_int8x16 *)&p_mat[(row + 5) * row_offset];
            ae_int8x16 *p_mat6 = (ae_int8x16 *)&p_mat[(row + 6) * row_offset];
            ae_int8x16 *p_mat7 = (ae_int8x16 *)&p_mat[(row + 7) * row_offset];

            ae_int16x8 *p_vec1 = (ae_int16x8 *)&p_vec[vec * vec_offset];
            ae_int64 accu00 = AE_ZERO64(), accu10 = AE_ZERO64();
            ae_int64 accu01 = AE_ZERO64(), accu11 = AE_ZERO64();
            ae_int64 accu02 = AE_ZERO64(), accu12 = AE_ZERO64();
            ae_int64 accu03 = AE_ZERO64(), accu13 = AE_ZERO64();
            ae_int64 accu04 = AE_ZERO64(), accu14 = AE_ZERO64();
            ae_int64 accu05 = AE_ZERO64(), accu15 = AE_ZERO64();
            ae_int64 accu06 = AE_ZERO64(), accu16 = AE_ZERO64();
            ae_int64 accu07 = AE_ZERO64(), accu17 = AE_ZERO64();

            /* This kernel implements a 8x16 by 16x1 matrix multiplication */
            for (col = 0; col < cols>>4; col++)
            {
                AE_L8X8X2_IP(mat00, mat10, p_mat0, 16);
                AE_L8X8X2_IP(mat01, mat11, p_mat1, 16);
                AE_L8X8X2_IP(mat02, mat12, p_mat2, 16);
                AE_L8X8X2_IP(mat03, mat13, p_mat3, 16);
                AE_L8X8X2_IP(mat04, mat14, p_mat4, 16);
                AE_L8X8X2_IP(mat05, mat15, p_mat5, 16);
                AE_L8X8X2_IP(mat06, mat16, p_mat6, 16);
                AE_L8X8X2_IP(mat07, mat17, p_mat7, 16);

                AE_L16X4X2_IP(vec0, vec1, p_vec1, 16);

                AE_MULAAAA2Q16X8(accu00, accu10, vec0, vec1, mat00);
                AE_MULAAAA2Q16X8(accu01, accu11, vec0, vec1, mat01);
                AE_MULAAAA2Q16X8(accu02, accu12, vec0, vec1, mat02);
                AE_MULAAAA2Q16X8(accu03, accu13, vec0, vec1, mat03);
                AE_MULAAAA2Q16X8(accu04, accu14, vec0, vec1, mat04);
                AE_MULAAAA2Q16X8(accu05, accu15, vec0, vec1, mat05);
                AE_MULAAAA2Q16X8(accu06, accu16, vec0, vec1, mat06);
                AE_MULAAAA2Q16X8(accu07, accu17, vec0, vec1, mat07);

                AE_L16X4X2_IP(vec0, vec1, p_vec1, 16);

                AE_MULAAAA2Q16X8(accu00, accu10, vec0, vec1, mat10);
                AE_MULAAAA2Q16X8(accu01, accu11, vec0, vec1, mat11);
                AE_MULAAAA2Q16X8(accu02, accu12, vec0, vec1, mat12);
                AE_MULAAAA2Q16X8(accu03, accu13, vec0, vec1, mat13);
                AE_MULAAAA2Q16X8(accu04, accu14, vec0, vec1, mat14);
                AE_MULAAAA2Q16X8(accu05, accu15, vec0, vec1, mat15);
                AE_MULAAAA2Q16X8(accu06, accu16, vec0, vec1, mat16);
                AE_MULAAAA2Q16X8(accu07, accu17, vec0, vec1, mat17);
            }

            accu00 = accu00 + accu10;
            accu01 = accu01 + accu11;
            accu02 = accu02 + accu12;
            accu03 = accu03 + accu13;
            accu04 = accu04 + accu14;
            accu05 = accu05 + accu15;
            accu06 = accu06 + accu16;
            accu07 = accu07 + accu17;

            p_dst0[row * out_offset] = accu00;
            p_dst0[(row + 1) * out_offset] = accu01;
            p_dst0[(row + 2) * out_offset] = accu02;
            p_dst0[(row + 3) * out_offset] = accu03;
            p_dst0[(row + 4) * out_offset] = accu04;
            p_dst0[(row + 5) * out_offset] = accu05;
            p_dst0[(row + 6) * out_offset] = accu06;
            p_dst0[(row + 7) * out_offset] = accu07;
        }
    }

}
#endif

/***************************************************************************
 * Matrix - Matrix multiplication of 8 bit matrix0 and 16 bit matrix1
 * with 64 bit output.
 * Function computes multiplication of an 8 bit integer input matrix0 (MxN)
 * with 16 bit integer matrix1 (NxP) and outputs a 64 bit integer output
 * matrix with dimension (MxP).
 *
 * Arguments:
 * p_out                        : Pointer to output matrix
 * p_mtx0                        : Pointer to input matrix0, must be 8-byte aligned
 * p_mtx1                        : Pointer to input matrix1, must be 16-byte aligned
 * mtx0_rows        : Number of rows in input matrix0, must be multiple of 4
 * mtx0_cols        : Number of columns in input matrix0, must be multiple of 16
 * mtx0_row_offset  : Offset to next row of input matrix0, must be multiple of 16
 * mtx1_cols                : Number of columns in input matrix1
 * mtx1_col_offset  : Offset to next column of input matrix1, must be multiple of 16
 * out_offset                : Offset to next row of output
 **************************************************************************/
void mat_mul_8x16_64(
        long long * __restrict__ p_out,
        const signed char * __restrict__ p_mtx0,
        const short * __restrict__ p_mtx1,
        int mtx0_rows,
        int mtx0_cols,
        int mtx0_row_offset,
        int mtx1_cols,
        int mtx1_col_offset,
        int out_offset)
#if XCHAL_HAVE_HIFI5_NN_MAC
{
    int row0 = 0, col0 = 0, col1 = 0;

    if(mtx1_cols >= 4)
    {
        for(col1 = 0; col1 < (mtx1_cols & (~0x3)); col1 += 4)
        {
            for (row0 = 0; row0 < mtx0_rows; row0 += 4)
            {
                ae_int16x8 *p_mat10;
                ae_int16x8 *p_mat11;
                ae_int16x8 *p_mat12;
                ae_int16x8 *p_mat13;
                ae_valignx2 align0;
                ae_valignx2 align1;
                ae_valignx2 align2;
                ae_valignx2 align3;

                ae_int64 *p_out0 = (ae_int64 *)(&p_out[col1 + (row0+0) * out_offset]);
                ae_int64 *p_out1 = (ae_int64 *)(&p_out[col1 + (row0+1) * out_offset]);
                ae_int64 *p_out2 = (ae_int64 *)(&p_out[col1 + (row0+2) * out_offset]);
                ae_int64 *p_out3 = (ae_int64 *)(&p_out[col1 + (row0+3) * out_offset]);

                p_mat10 = (ae_int16x8 *)&p_mtx1[(col1 + 0) * mtx1_col_offset];
                p_mat11 = (ae_int16x8 *)&p_mtx1[(col1 + 1) * mtx1_col_offset];
                p_mat12 = (ae_int16x8 *)&p_mtx1[(col1 + 2) * mtx1_col_offset];
                p_mat13 = (ae_int16x8 *)&p_mtx1[(col1 + 3) * mtx1_col_offset];
                align0 = AE_LA128_PP(p_mat10);
                align1 = AE_LA128_PP(p_mat11);
                align2 = AE_LA128_PP(p_mat12);
                align3 = AE_LA128_PP(p_mat13);

                ae_int8x16 *p_mat00 = (ae_int8x16*)&p_mtx0[(row0 + 0) * mtx0_row_offset];
                ae_int8x16 *p_mat01 = (ae_int8x16*)&p_mtx0[(row0 + 1) * mtx0_row_offset];
                ae_int8x16 *p_mat02 = (ae_int8x16*)&p_mtx0[(row0 + 2) * mtx0_row_offset];
                ae_int8x16 *p_mat03 = (ae_int8x16*)&p_mtx0[(row0 + 3) * mtx0_row_offset];

                ae_int64 accu00 = AE_ZERO64(), accu01 = AE_ZERO64(), accu02 = AE_ZERO64(), accu03 = AE_ZERO64();
                ae_int64 accu10 = AE_ZERO64(), accu11 = AE_ZERO64(), accu12 = AE_ZERO64(), accu13 = AE_ZERO64();
                ae_int64 accu20 = AE_ZERO64(), accu21 = AE_ZERO64(), accu22 = AE_ZERO64(), accu23 = AE_ZERO64();
                ae_int64 accu30 = AE_ZERO64(), accu31 = AE_ZERO64(), accu32 = AE_ZERO64(), accu33 = AE_ZERO64();

#pragma ymemory(p_mat00)
#pragma ymemory(p_mat10)
#pragma ymemory(p_mat11)
#pragma ymemory(p_mat12)
#pragma ymemory(p_mat13)
                /* This kernel implements a 4x16 by 16x4 matrix multiplication */
                for (col0 = 0; col0 < mtx0_cols>>4; col0++)
                {
                    ae_int16x4 vec00, vec10;
                    ae_int16x4 vec01, vec11;
                    ae_int16x4 vec02, vec12;
                    ae_int16x4 vec03, vec13;
                    ae_int8x8 mat00, mat01, mat02, mat03;
                    ae_int8x8 mat10, mat11, mat12, mat13;

                    AE_L8X8X2_IP(mat00, mat10, p_mat00, 16);
                    AE_L8X8X2_IP(mat01, mat11, p_mat01, 16);
                    AE_L8X8X2_IP(mat02, mat12, p_mat02, 16);
                    AE_L8X8X2_IP(mat03, mat13, p_mat03, 16);

                    AE_LA16X4X2_IP(vec00, vec10, align0, p_mat10);
                    AE_LA16X4X2_IP(vec01, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec02, vec12, align2, p_mat12);
                    AE_LA16X4X2_IP(vec03, vec13, align3, p_mat13);
                    AE_MULA8QW8X16(accu00, accu01, accu02, accu03, mat00, mat01, mat02, mat03, vec00, vec10);
                    AE_MULA8QW8X16(accu10, accu11, accu12, accu13, mat00, mat01, mat02, mat03, vec01, vec11);
                    AE_MULA8QW8X16(accu20, accu21, accu22, accu23, mat00, mat01, mat02, mat03, vec02, vec12);
                    AE_MULA8QW8X16(accu30, accu31, accu32, accu33, mat00, mat01, mat02, mat03, vec03, vec13);

                    AE_LA16X4X2_IP(vec00, vec10, align0, p_mat10);
                    AE_LA16X4X2_IP(vec01, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec02, vec12, align2, p_mat12);
                    AE_LA16X4X2_IP(vec03, vec13, align3, p_mat13);
                    AE_MULA8QW8X16(accu00, accu01, accu02, accu03, mat10, mat11, mat12, mat13, vec00, vec10);
                    AE_MULA8QW8X16(accu10, accu11, accu12, accu13, mat10, mat11, mat12, mat13, vec01, vec11);
                    AE_MULA8QW8X16(accu20, accu21, accu22, accu23, mat10, mat11, mat12, mat13, vec02, vec12);
                    AE_MULA8QW8X16(accu30, accu31, accu32, accu33, mat10, mat11, mat12, mat13, vec03, vec13);
                }

                *p_out0++ = accu00; *p_out0++ = accu10; *p_out0++ = accu20; *p_out0++ = accu30;
                *p_out1++ = accu01; *p_out1++ = accu11; *p_out1++ = accu21; *p_out1++ = accu31;
                *p_out2++ = accu02; *p_out2++ = accu12; *p_out2++ = accu22; *p_out2++ = accu32;
                *p_out3++ = accu03; *p_out3++ = accu13; *p_out3++ = accu23; *p_out3++ = accu33;
            }
        }
    }

    if(mtx1_cols & 0x3)
    {
        matvec_mul_8x16_64(
                &p_out[col1],
                p_mtx0,
                &p_mtx1[col1 * mtx1_col_offset],
                mtx0_rows,
                mtx0_cols,
                mtx0_row_offset,
                mtx1_cols & 0x3,
                mtx1_col_offset,
                out_offset);
    }
}
#else
{
    int row0 = 0, col0 = 0, col1 = 0;

    if(mtx1_cols >= 4)
    {
        for(col1 = 0; col1 < (mtx1_cols & (~0x3)); col1 += 4)
        {
            for (row0 = 0; row0 < mtx0_rows; row0 += 2)
            {
                ae_int16x8 *p_mat10;
                ae_int16x8 *p_mat11;
                ae_int16x8 *p_mat12;
                ae_int16x8 *p_mat13;
                ae_valignx2 align0;
                ae_valignx2 align1;
                ae_valignx2 align2;
                ae_valignx2 align3;

                ae_int64 *p_out0 = (ae_int64 *)(&p_out[col1 + (row0+0) * out_offset]);
                ae_int64 *p_out1 = (ae_int64 *)(&p_out[col1 + (row0+1) * out_offset]);

                p_mat10 = (ae_int16x8 *)&p_mtx1[(col1 + 0) * mtx1_col_offset];
                p_mat11 = (ae_int16x8 *)&p_mtx1[(col1 + 1) * mtx1_col_offset];
                p_mat12 = (ae_int16x8 *)&p_mtx1[(col1 + 2) * mtx1_col_offset];
                p_mat13 = (ae_int16x8 *)&p_mtx1[(col1 + 3) * mtx1_col_offset];
                align0 = AE_LA128_PP(p_mat10);
                align1 = AE_LA128_PP(p_mat11);
                align2 = AE_LA128_PP(p_mat12);
                align3 = AE_LA128_PP(p_mat13);

                ae_int8x16 *p_mat00 = (ae_int8x16*)&p_mtx0[(row0 + 0) * mtx0_row_offset];
                ae_int8x16 *p_mat01 = (ae_int8x16*)&p_mtx0[(row0 + 1) * mtx0_row_offset];

                ae_int64 accu00_0 = AE_ZERO64(), accu00_1 = AE_ZERO64(), accu10_0 = AE_ZERO64(), accu10_1 = AE_ZERO64();
                ae_int64 accu01_0 = AE_ZERO64(), accu01_1 = AE_ZERO64(), accu11_0 = AE_ZERO64(), accu11_1 = AE_ZERO64();
                ae_int64 accu02_0 = AE_ZERO64(), accu02_1 = AE_ZERO64(), accu12_0 = AE_ZERO64(), accu12_1 = AE_ZERO64();
                ae_int64 accu03_0 = AE_ZERO64(), accu03_1 = AE_ZERO64(), accu13_0 = AE_ZERO64(), accu13_1 = AE_ZERO64();

#pragma ymemory(p_mat10)
#pragma ymemory(p_mat11)
                /*#pragma ymemory(p_mat12)
#pragma ymemory(p_mat13)*/
                /* This kernel implements a 2x16 by 16x4 matrix multiplication */
                for (col0 = 0; col0 < mtx0_cols>>4; col0++)
                {
                    ae_int16x4 vec00, vec01;
                    ae_int16x4 vec10, vec11;
                    ae_int16x4 vec20, vec21;
                    ae_int16x4 vec30, vec31;
                    ae_int8x8 mat00, mat01;
                    ae_int8x8 mat10, mat11;

                    AE_L8X8X2_IP(mat00, mat01, p_mat00, 16);
                    AE_L8X8X2_IP(mat10, mat11, p_mat01, 16);

                    AE_LA16X4X2_IP(vec00, vec01, align0, p_mat10);
                    AE_LA16X4X2_IP(vec10, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec20, vec21, align2, p_mat12);
                    AE_LA16X4X2_IP(vec30, vec31, align3, p_mat13);
                    AE_MULAAAA2Q16X8(accu00_0, accu00_1, vec00, vec01, mat00);
                    AE_MULAAAA2Q16X8(accu01_0, accu01_1, vec10, vec11, mat00);
                    AE_MULAAAA2Q16X8(accu02_0, accu02_1, vec20, vec21, mat00);
                    AE_MULAAAA2Q16X8(accu03_0, accu03_1, vec30, vec31, mat00);
                    AE_MULAAAA2Q16X8(accu10_0, accu10_1, vec00, vec01, mat10);
                    AE_MULAAAA2Q16X8(accu11_0, accu11_1, vec10, vec11, mat10);
                    AE_MULAAAA2Q16X8(accu12_0, accu12_1, vec20, vec21, mat10);
                    AE_MULAAAA2Q16X8(accu13_0, accu13_1, vec30, vec31, mat10);

                    AE_LA16X4X2_IP(vec00, vec01, align0, p_mat10);
                    AE_LA16X4X2_IP(vec10, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec20, vec21, align2, p_mat12);
                    AE_LA16X4X2_IP(vec30, vec31, align3, p_mat13);
                    AE_MULAAAA2Q16X8(accu00_0, accu00_1, vec00, vec01, mat01);
                    AE_MULAAAA2Q16X8(accu01_0, accu01_1, vec10, vec11, mat01);
                    AE_MULAAAA2Q16X8(accu02_0, accu02_1, vec20, vec21, mat01);
                    AE_MULAAAA2Q16X8(accu03_0, accu03_1, vec30, vec31, mat01);
                    AE_MULAAAA2Q16X8(accu10_0, accu10_1, vec00, vec01, mat11);
                    AE_MULAAAA2Q16X8(accu11_0, accu11_1, vec10, vec11, mat11);
                    AE_MULAAAA2Q16X8(accu12_0, accu12_1, vec20, vec21, mat11);
                    AE_MULAAAA2Q16X8(accu13_0, accu13_1, vec30, vec31, mat11);
                }

                accu00_0 += accu00_1;
                accu01_0 += accu01_1;
                accu02_0 += accu02_1;
                accu03_0 += accu03_1;
                accu10_0 += accu10_1;
                accu11_0 += accu11_1;
                accu12_0 += accu12_1;
                accu13_0 += accu13_1;

                *p_out0++ = accu00_0; *p_out0++ = accu01_0; *p_out0++ = accu02_0; *p_out0++ = accu03_0;
                *p_out1++ = accu10_0; *p_out1++ = accu11_0; *p_out1++ = accu12_0; *p_out1++ = accu13_0;
            }
        }
    }

    if(mtx1_cols & 0x3)
    {
        matvec_mul_8x16_64(
                &p_out[col1],
                p_mtx0,
                &p_mtx1[col1 * mtx1_col_offset],
                mtx0_rows,
                mtx0_cols,
                mtx0_row_offset,
                mtx1_cols & 0x3,
                mtx1_col_offset,
                out_offset);
    }
}
#endif

#else
#error "Works only with HiFi5"
#endif
```
## My Edited Code
```c
#include <xtensa/config/core-isa.h>

#if (1== XCHAL_HAVE_HIFI5 )

#include <xtensa/tie/xt_hifi5.h>
#include "matvec_mul_8x16.h"
#include <stdio.h>
/***************************************************************************
 * Matrix vector multiplication of 8 bit matrix and 16 bit vectors
 * with 64 bit output.
 * Function computes multiplication of an 8 bit integer input matrix (MxN)
 * with P 16 bit integer vectors (Nx1) and outputs P 64 bit integer
 * vectors (MX1)
 *
 * Arguments:
 * p_out        : Pointer to output matrix
 * p_mat        : Pointer to input matrix, must be 8-byte aligned
 * p_vec        : Pointer to first input vector, must be 16-byte aligned
 * rows         : Number of rows in input matrix, must be multiple of 16
 * cols         : Number of columns in input matrix, must be multiple of 32
 * row_offset   : Offset to next row of input matrix, must be multiple of 16
 * vec_count    : Number of input vectors
 * vec_offset   : Offset to next input vector, must be multiple of 32
 * out_offset   : Offset to next row of output
 **************************************************************************/

void matvec_mul_8x16_64 (
        long long * __restrict__ p_out,
        const signed char * __restrict__ p_mat,
        const short * __restrict__ p_vec,
        int rows,
        int cols,
        int row_offset,
        int vec_count,
        int vec_offset,
        int out_offset
        )
#if XCHAL_HAVE_HIFI5_NN_MAC
{
    int row = 0, col = 0, vec = 0;

    for(vec = 0; vec < vec_count; vec++)
    {
        ae_int16x4 vec00, vec01;
        ae_int16x4 vec10, vec11;
        ae_int16x4 vec20, vec21;
        ae_int16x4 vec30, vec31;

        ae_int64 *p_dst0 = (ae_int64 *)&p_out[vec];
        row = 0;
        if(rows >= 16)
        {
            for (row = 0; row < ( rows & ~(15)) ; row += 16)
            {
                ae_int16x8 *p_vec0 = (ae_int16x8 *)&p_vec[vec *vec_offset];
                ae_int16x8 *p_vec1 = (ae_int16x8 *)&p_vec[vec *vec_offset + 8];

                ae_int64 accu00, accu10, accu20, accu30;
                ae_int64 accu01, accu11, accu21, accu31;
                ae_int64 accu02, accu12, accu22, accu32;
                ae_int64 accu03, accu13, accu23, accu33;
                ae_int8x8 mat00_0, mat10_0, mat20_0, mat30_0;
                ae_int8x8 mat01_0, mat11_0, mat21_0, mat31_0;
                ae_int8x8 mat00_1, mat10_1, mat20_1, mat30_1;
                ae_int8x8 mat01_1, mat11_1, mat21_1, mat31_1;
                ae_int8x8 mat00_2, mat10_2, mat20_2, mat30_2;
                ae_int8x8 mat01_2, mat11_2, mat21_2, mat31_2;
                ae_int8x8 mat00_3, mat10_3, mat20_3, mat30_3;
                ae_int8x8 mat01_3, mat11_3, mat21_3, mat31_3;

                ae_int8x16 *p_mat0 = (ae_int8x16 *)&p_mat[(row+4*0+0) * row_offset];
                ae_int8x16 *p_mat1 = (ae_int8x16 *)&p_mat[(row+4*1+0) * row_offset];
                ae_int8x16 *p_mat2 = (ae_int8x16 *)&p_mat[(row+4*2+0) * row_offset];
                ae_int8x16 *p_mat3 = (ae_int8x16 *)&p_mat[(row+4*3+0) * row_offset];

                accu00 = AE_ZERO64(); accu10 = AE_ZERO64();    accu20 = AE_ZERO64(); accu30 = AE_ZERO64();
                accu01 = AE_ZERO64(); accu11 = AE_ZERO64();    accu21 = AE_ZERO64(); accu31 = AE_ZERO64();
                accu02 = AE_ZERO64(); accu12 = AE_ZERO64();    accu22 = AE_ZERO64(); accu32 = AE_ZERO64();
                accu03 = AE_ZERO64(); accu13 = AE_ZERO64();    accu23 = AE_ZERO64(); accu33 = AE_ZERO64();

                AE_L16X4X2_IP(vec00, vec01, p_vec0, 32);
                AE_L16X4X2_IP(vec10, vec11, p_vec1, 32);
                AE_L16X4X2_IP(vec20, vec21, p_vec0, 32);
                AE_L16X4X2_IP(vec30, vec31, p_vec1, 32);

                /* This kernel implements a 16x32 by 32x1 matrix multiplication */
                for (col = 1; col < (cols>>5); col += 1)
                {
                    AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                    AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                    AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                    AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec00, vec01);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec10, vec11);

                    AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                    AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                    AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                    AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec20, vec21);
                    AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec30, vec31);

                    AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                    AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                    AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                    AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec00, vec01);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec10, vec11);

                    AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                    AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                    AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                    AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec20, vec21);
                    AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec30, vec31);

                    AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                    AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                    AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                    AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec00, vec01);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec10, vec11);

                    AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                    AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                    AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                    AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec20, vec21);
                    AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec30, vec31);

                    AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                    AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                    AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                    AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec00, vec01);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec10, vec11);

                    AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                    AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                    AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                    AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec20, vec21);
                    AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec30, vec31);

                    AE_L16X4X2_IP(vec00, vec01, p_vec0, 32);
                    AE_L16X4X2_IP(vec10, vec11, p_vec1, 32);
                    AE_L16X4X2_IP(vec20, vec21, p_vec0, 32);
                    AE_L16X4X2_IP(vec30, vec31, p_vec1, 32);
                }

                AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec00, vec01);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec10, vec11);

                AE_L8X8X2_X(mat10_0, mat11_0, p_mat0, 1*row_offset);
                AE_L8X8X2_X(mat20_0, mat21_0, p_mat0, 2*row_offset);
                AE_L8X8X2_X(mat30_0, mat31_0, p_mat0, 3*row_offset);
                AE_L8X8X2_IP(mat00_0, mat01_0, p_mat0, 16);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat00_0, mat10_0, mat20_0, mat30_0, vec20, vec21);
                AE_MULA8QW8X16(accu00, accu10, accu20, accu30, mat01_0, mat11_0, mat21_0, mat31_0, vec30, vec31);


                AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec00, vec01);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec10, vec11);

                AE_L8X8X2_X(mat10_1, mat11_1, p_mat1, 1*row_offset);
                AE_L8X8X2_X(mat20_1, mat21_1, p_mat1, 2*row_offset);
                AE_L8X8X2_X(mat30_1, mat31_1, p_mat1, 3*row_offset);
                AE_L8X8X2_IP(mat00_1, mat01_1, p_mat1, 16);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat00_1, mat10_1, mat20_1, mat30_1, vec20, vec21);
                AE_MULA8QW8X16(accu01, accu11, accu21, accu31, mat01_1, mat11_1, mat21_1, mat31_1, vec30, vec31);


                AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec00, vec01);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec10, vec11);

                AE_L8X8X2_X(mat10_2, mat11_2, p_mat2, 1*row_offset);
                AE_L8X8X2_X(mat20_2, mat21_2, p_mat2, 2*row_offset);
                AE_L8X8X2_X(mat30_2, mat31_2, p_mat2, 3*row_offset);
                AE_L8X8X2_IP(mat00_2, mat01_2, p_mat2, 16);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat00_2, mat10_2, mat20_2, mat30_2, vec20, vec21);
                AE_MULA8QW8X16(accu02, accu12, accu22, accu32, mat01_2, mat11_2, mat21_2, mat31_2, vec30, vec31);


                AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec00, vec01);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec10, vec11);

                AE_L8X8X2_X(mat10_3, mat11_3, p_mat3, 1*row_offset);
                AE_L8X8X2_X(mat20_3, mat21_3, p_mat3, 2*row_offset);
                AE_L8X8X2_X(mat30_3, mat31_3, p_mat3, 3*row_offset);
                AE_L8X8X2_IP(mat00_3, mat01_3, p_mat3, 16);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat00_3, mat10_3, mat20_3, mat30_3, vec20, vec21);
                AE_MULA8QW8X16(accu03, accu13, accu23, accu33, mat01_3, mat11_3, mat21_3, mat31_3, vec30, vec31);

                p_dst0[(row+4*0+0) * out_offset] = accu00;
                p_dst0[(row+4*0+1) * out_offset] = accu10;
                p_dst0[(row+4*0+2) * out_offset] = accu20;
                p_dst0[(row+4*0+3) * out_offset] = accu30;

                p_dst0[(row+4*1+0) * out_offset] = accu01;
                p_dst0[(row+4*1+1) * out_offset] = accu11;
                p_dst0[(row+4*1+2) * out_offset] = accu21;
                p_dst0[(row+4*1+3) * out_offset] = accu31;

                p_dst0[(row+4*2+0) * out_offset] = accu02;
                p_dst0[(row+4*2+1) * out_offset] = accu12;
                p_dst0[(row+4*2+2) * out_offset] = accu22;
                p_dst0[(row+4*2+3) * out_offset] = accu32;

                p_dst0[(row+4*3+0) * out_offset] = accu03;
                p_dst0[(row+4*3+1) * out_offset] = accu13;
                p_dst0[(row+4*3+2) * out_offset] = accu23;
                p_dst0[(row+4*3+3) * out_offset] = accu33;

            }
        }
    }
}
#else
{
    int row = 0, col = 0, vec = 0;

    for(vec = 0; vec < vec_count; vec++)
    {
        ae_int16x4 vec0, vec1;
        ae_int8x8 mat00, mat01, mat02, mat03, mat04, mat05, mat06, mat07;
        ae_int8x8 mat10, mat11, mat12, mat13, mat14, mat15, mat16, mat17;

        ae_int64 *p_dst0 = (ae_int64 *)&p_out[vec];
        for (row = 0; row < rows; row += 8)
        {
            ae_int8x16 *p_mat0 = (ae_int8x16 *)&p_mat[row * row_offset];
            ae_int8x16 *p_mat1 = (ae_int8x16 *)&p_mat[(row + 1) * row_offset];
            ae_int8x16 *p_mat2 = (ae_int8x16 *)&p_mat[(row + 2) * row_offset];
            ae_int8x16 *p_mat3 = (ae_int8x16 *)&p_mat[(row + 3) * row_offset];
            ae_int8x16 *p_mat4 = (ae_int8x16 *)&p_mat[(row + 4) * row_offset];
            ae_int8x16 *p_mat5 = (ae_int8x16 *)&p_mat[(row + 5) * row_offset];
            ae_int8x16 *p_mat6 = (ae_int8x16 *)&p_mat[(row + 6) * row_offset];
            ae_int8x16 *p_mat7 = (ae_int8x16 *)&p_mat[(row + 7) * row_offset];

            ae_int16x8 *p_vec1 = (ae_int16x8 *)&p_vec[vec * vec_offset];
            ae_int64 accu00 = AE_ZERO64(), accu10 = AE_ZERO64();
            ae_int64 accu01 = AE_ZERO64(), accu11 = AE_ZERO64();
            ae_int64 accu02 = AE_ZERO64(), accu12 = AE_ZERO64();
            ae_int64 accu03 = AE_ZERO64(), accu13 = AE_ZERO64();
            ae_int64 accu04 = AE_ZERO64(), accu14 = AE_ZERO64();
            ae_int64 accu05 = AE_ZERO64(), accu15 = AE_ZERO64();
            ae_int64 accu06 = AE_ZERO64(), accu16 = AE_ZERO64();
            ae_int64 accu07 = AE_ZERO64(), accu17 = AE_ZERO64();

            /* This kernel implements a 8x16 by 16x1 matrix multiplication */
            for (col = 0; col < cols>>4; col++)
            {
                AE_L8X8X2_IP(mat00, mat10, p_mat0, 16);
                AE_L8X8X2_IP(mat01, mat11, p_mat1, 16);
                AE_L8X8X2_IP(mat02, mat12, p_mat2, 16);
                AE_L8X8X2_IP(mat03, mat13, p_mat3, 16);
                AE_L8X8X2_IP(mat04, mat14, p_mat4, 16);
                AE_L8X8X2_IP(mat05, mat15, p_mat5, 16);
                AE_L8X8X2_IP(mat06, mat16, p_mat6, 16);
                AE_L8X8X2_IP(mat07, mat17, p_mat7, 16);

                AE_L16X4X2_IP(vec0, vec1, p_vec1, 16);

                AE_MULAAAA2Q16X8(accu00, accu10, vec0, vec1, mat00);
                AE_MULAAAA2Q16X8(accu01, accu11, vec0, vec1, mat01);
                AE_MULAAAA2Q16X8(accu02, accu12, vec0, vec1, mat02);
                AE_MULAAAA2Q16X8(accu03, accu13, vec0, vec1, mat03);
                AE_MULAAAA2Q16X8(accu04, accu14, vec0, vec1, mat04);
                AE_MULAAAA2Q16X8(accu05, accu15, vec0, vec1, mat05);
                AE_MULAAAA2Q16X8(accu06, accu16, vec0, vec1, mat06);
                AE_MULAAAA2Q16X8(accu07, accu17, vec0, vec1, mat07);

                AE_L16X4X2_IP(vec0, vec1, p_vec1, 16);

                AE_MULAAAA2Q16X8(accu00, accu10, vec0, vec1, mat10);
                AE_MULAAAA2Q16X8(accu01, accu11, vec0, vec1, mat11);
                AE_MULAAAA2Q16X8(accu02, accu12, vec0, vec1, mat12);
                AE_MULAAAA2Q16X8(accu03, accu13, vec0, vec1, mat13);
                AE_MULAAAA2Q16X8(accu04, accu14, vec0, vec1, mat14);
                AE_MULAAAA2Q16X8(accu05, accu15, vec0, vec1, mat15);
                AE_MULAAAA2Q16X8(accu06, accu16, vec0, vec1, mat16);
                AE_MULAAAA2Q16X8(accu07, accu17, vec0, vec1, mat17);
            }

            accu00 = accu00 + accu10;
            accu01 = accu01 + accu11;
            accu02 = accu02 + accu12;
            accu03 = accu03 + accu13;

            accu04 = accu04 + accu14;
            accu05 = accu05 + accu15;
            accu06 = accu06 + accu16;
            accu07 = accu07 + accu17;

            p_dst0[row * out_offset] = accu00;
            p_dst0[(row + 1) * out_offset] = accu01;
            p_dst0[(row + 2) * out_offset] = accu02;
            p_dst0[(row + 3) * out_offset] = accu03;
            p_dst0[(row + 4) * out_offset] = accu04;
            p_dst0[(row + 5) * out_offset] = accu05;
            p_dst0[(row + 6) * out_offset] = accu06;
            p_dst0[(row + 7) * out_offset] = accu07;
        }
    }

}
#endif

/***************************************************************************
 * Matrix - Matrix multiplication of 8 bit matrix0 and 16 bit matrix1
 * with 64 bit output.
 * Function computes multiplication of an 8 bit integer input matrix0 (MxN)
 * with 16 bit integer matrix1 (NxP) and outputs a 64 bit integer output
 * matrix with dimension (MxP).
 *
 * Arguments:
 * p_out                        : Pointer to output matrix
 * p_mtx0                        : Pointer to input matrix0, must be 8-byte aligned
 * p_mtx1                        : Pointer to input matrix1, must be 16-byte aligned
 * mtx0_rows        : Number of rows in input matrix0, must be multiple of 4
 * mtx0_cols        : Number of columns in input matrix0, must be multiple of 16
 * mtx0_row_offset  : Offset to next row of input matrix0, must be multiple of 16
 * mtx1_cols                : Number of columns in input matrix1
 * mtx1_col_offset  : Offset to next column of input matrix1, must be multiple of 16
 * out_offset                : Offset to next row of output
 **************************************************************************/
// Own functions start
void print_ae_int8x8(ae_int8x8 reg) {
    int8_t temp[8];
    AE_S8X8_I(reg, (ae_int8x8 *) temp, 0);
    for (int i = 0; i < 8; i++) {
		printf("%d ", temp[i]);
		if (i != 7) printf(",");
	}
	printf("\n");
}

void print_ae_int32x2(ae_int32x2 reg){
	int32_t temp[2];
	AE_S32X2_I(reg, (ae_int32x2 *) temp, 0);
	printf("%d %d",temp[0],temp[1]);
	printf("\n");
}
void print_ae_int16x4(ae_int16x4 reg) {
    int16_t temp[4];
    AE_S16X4_I(reg, (ae_int16x4 *) temp, 0);
    for (int i = 0; i < 4; i++) {
		printf("%d ", temp[i]);
		if (i != 3) printf(",");
	}
	printf("\n");
}

void print_ae_int4x16(ae_int4x16 reg) {
    uint8_t temp[16];
    ae_int4x16_storei(reg, (ae_int4x16 *) temp, 0);

    for (int i = 0; i < 8; i++) {
        uint8_t byte = temp[i];
        int8_t high = (byte >> 4) & 0x0F;
        int8_t low = byte & 0x0F;
        if (high > 7) high -= 16;
        if (low > 7)  low -= 16;
        printf("%d", high);
        printf(":");
        printf("%d", low);
        if (i != 7) printf(", ");
    }
    printf("\n");
}
void print_ae_int16x8(ae_int16x8 reg) {
	int16_t temp[8];
//
    ae_int16x8_storei(reg, (ae_int16x8 *) temp, 0);

    for (int i = 0; i < 8; i++) {
		printf("%d ", temp[i]);
		if (i != 7) printf(",");
	}
	printf("\n");
}
void print_ae_int8x16(ae_int8x16 reg) {
    int8_t temp[16];
    ae_int8x16_storei(reg, (ae_int8x16 *) temp, 0);
	for (int i = 0; i < 16; i++) {
		printf("%d ", temp[i]);
		if (i != 15) printf(",");
	}
	printf("\n");
}

void print_ae_int64(ae_int64 reg) {
    int64_t temp[1];
    ae_int64_storei(reg, (ae_int64*) temp, 0);

	printf("%lld\n",temp[0]);
}
//ae_int8x16_storei
// Own functions end
void mat_mul_8x16_64(
        long long * __restrict__ p_out,
        const signed char * __restrict__ p_mtx0,
        const short * __restrict__ p_mtx1,
        int mtx0_rows,
        int mtx0_cols,
        int mtx0_row_offset,
        int mtx1_cols,
        int mtx1_col_offset,
        int out_offset)
#if XCHAL_HAVE_HIFI5_NN_MAC
{
	printf("Matrix 0 row : %d,Matrix 0 Col : %d,Matrix 0 row offset : %d,Matrix 1 col : %d,Matrix 1 cols offset : %d,Output offset : %d\n",mtx0_rows,mtx0_cols,mtx0_row_offset,mtx1_cols,mtx1_col_offset,out_offset);
    int row0 = 0, col0 = 0, col1 = 0;

    if(mtx1_cols >= 4)
    {	printf("mtx1_cols : %d ,(~0x3) : %d , (mtx1_cols & (~0x3)) : %d\n",mtx1_cols,(~0x3),(mtx1_cols & (~0x3)));
        for(col1 = 0; col1 < (mtx1_cols & (~0x3)); col1 += 4)
        {
            for (row0 = 0; row0 < mtx0_rows; row0 += 4)
            {
                ae_int16x8 *p_mat10;
                ae_int16x8 *p_mat11;
                ae_int16x8 *p_mat12;
                ae_int16x8 *p_mat13;
                ae_valignx2 align0;
                ae_valignx2 align1;
                ae_valignx2 align2;
                ae_valignx2 align3;

                ae_int64 *p_out0 = (ae_int64 *)(&p_out[col1 + (row0+0) * out_offset]);
                ae_int64 *p_out1 = (ae_int64 *)(&p_out[col1 + (row0+1) * out_offset]);
                ae_int64 *p_out2 = (ae_int64 *)(&p_out[col1 + (row0+2) * out_offset]);
                ae_int64 *p_out3 = (ae_int64 *)(&p_out[col1 + (row0+3) * out_offset]);

                p_mat10 = (ae_int16x8 *)&p_mtx1[(col1 + 0) * mtx1_col_offset];
                p_mat11 = (ae_int16x8 *)&p_mtx1[(col1 + 1) * mtx1_col_offset];
                p_mat12 = (ae_int16x8 *)&p_mtx1[(col1 + 2) * mtx1_col_offset];
                p_mat13 = (ae_int16x8 *)&p_mtx1[(col1 + 3) * mtx1_col_offset];
                printf("p_mat10 : ");
                print_ae_int16x8(*p_mat10);
                printf("p_mat11 : ");
                print_ae_int16x8(*p_mat11);
                printf("p_mat12 : ");
				print_ae_int16x8(*p_mat12);
				printf("p_mat13 : ");
				print_ae_int16x8(*p_mat13);
                align0 = AE_LA128_PP(p_mat10);
                align1 = AE_LA128_PP(p_mat11);
                align2 = AE_LA128_PP(p_mat12);
                align3 = AE_LA128_PP(p_mat13);

                ae_int8x16 *p_mat00 = (ae_int8x16*)&p_mtx0[(row0 + 0) * mtx0_row_offset];
                ae_int8x16 *p_mat01 = (ae_int8x16*)&p_mtx0[(row0 + 1) * mtx0_row_offset];
                ae_int8x16 *p_mat02 = (ae_int8x16*)&p_mtx0[(row0 + 2) * mtx0_row_offset];
                ae_int8x16 *p_mat03 = (ae_int8x16*)&p_mtx0[(row0 + 3) * mtx0_row_offset];
                printf("p_mat00 : ");
				print_ae_int8x16(*p_mat00);
				printf("p_mat01 : ");
				print_ae_int8x16(*p_mat01);
				printf("p_mat02 : ");
				print_ae_int8x16(*p_mat02);
				printf("p_mat03 : ");
				print_ae_int8x16(*p_mat03);
                ae_int64 accu00 = AE_ZERO64(), accu01 = AE_ZERO64(), accu02 = AE_ZERO64(), accu03 = AE_ZERO64();
                ae_int64 accu10 = AE_ZERO64(), accu11 = AE_ZERO64(), accu12 = AE_ZERO64(), accu13 = AE_ZERO64();
                ae_int64 accu20 = AE_ZERO64(), accu21 = AE_ZERO64(), accu22 = AE_ZERO64(), accu23 = AE_ZERO64();
                ae_int64 accu30 = AE_ZERO64(), accu31 = AE_ZERO64(), accu32 = AE_ZERO64(), accu33 = AE_ZERO64();

#pragma ymemory(p_mat00)
#pragma ymemory(p_mat10)
#pragma ymemory(p_mat11)
#pragma ymemory(p_mat12)
#pragma ymemory(p_mat13)
                /* This kernel implements a 4x16 by 16x4 matrix multiplication */
                printf("mtx0_cols>>4 : %d\n",mtx0_cols>>4);
                for (col0 = 0; col0 < mtx0_cols>>4; col0++)
                {
//					printf("col1 : %d row0 : %d col0 : %d\n",col1,row0,col0);
                    ae_int16x4 vec00, vec10;
                    ae_int16x4 vec01, vec11;
                    ae_int16x4 vec02, vec12;
                    ae_int16x4 vec03, vec13;
                    ae_int8x8 mat00, mat01, mat02, mat03;
                    ae_int8x8 mat10, mat11, mat12, mat13;
                    AE_L8X8X2_IP(mat00, mat10, p_mat00, 16);
                    AE_L8X8X2_IP(mat01, mat11, p_mat01, 16);
                    AE_L8X8X2_IP(mat02, mat12, p_mat02, 16);
                    AE_L8X8X2_IP(mat03, mat13, p_mat03, 16);
                    printf("mat00 : ");
					print_ae_int8x8(mat00);
					printf("mat01 : ");
					print_ae_int8x8(mat01);
					printf("mat02 : ");
					print_ae_int8x8(mat02);
					printf("mat03 : ");
					print_ae_int8x8(mat03);
					printf("mat10 : ");
					print_ae_int8x8(mat10);
					printf("mat11 : ");
					print_ae_int8x8(mat11);
					printf("mat12 : ");
					print_ae_int8x8(mat12);

                    AE_LA16X4X2_IP(vec00, vec10, align0, p_mat10);
                    AE_LA16X4X2_IP(vec01, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec02, vec12, align2, p_mat12);
                    AE_LA16X4X2_IP(vec03, vec13, align3, p_mat13);

                    printf("vec00 : ");
                    print_ae_int16x4(vec00);
                    printf("vec10 : ");
                    print_ae_int16x4(vec10);
                    printf("vec01 : ");
                    print_ae_int16x4(vec01);
                    printf("vec11 : ");
                    print_ae_int16x4(vec11);
                    printf("vec02 : ");
					print_ae_int16x4(vec02);
					printf("vec12 : ");
					print_ae_int16x4(vec12);
					printf("vec03 : ");
					print_ae_int16x4(vec03);
					printf("vec13 : ");
					print_ae_int16x4(vec13);

                    AE_MULA8QW8X16(accu00, accu01, accu02, accu03, mat00, mat01, mat02, mat03, vec00, vec10);
                    AE_MULA8QW8X16(accu10, accu11, accu12, accu13, mat00, mat01, mat02, mat03, vec01, vec11);
                    AE_MULA8QW8X16(accu20, accu21, accu22, accu23, mat00, mat01, mat02, mat03, vec02, vec12);
                    AE_MULA8QW8X16(accu30, accu31, accu32, accu33, mat00, mat01, mat02, mat03, vec03, vec13);
                    printf("accu00 : ");
                    print_ae_int64(accu00);
                    printf("accu01 : ");
				    print_ae_int64(accu01);
				    printf("accu02 : ");
				    print_ae_int64(accu02);
					printf("accu03 : ");
					print_ae_int64(accu03);
					printf("accu10 : ");
					print_ae_int64(accu10);
					printf("accu11 : ");
					print_ae_int64(accu11);
					printf("accu12 : ");
					print_ae_int64(accu12);
					printf("accu13 : ");
					print_ae_int64(accu13);
					printf("accu20 : ");
					print_ae_int64(accu20);
					printf("accu21 : ");
					print_ae_int64(accu21);
					printf("accu22 : ");
					print_ae_int64(accu22);
					printf("accu23 : ");
					print_ae_int64(accu23);
					printf("accu30 : ");
					print_ae_int64(accu30);
					printf("accu31 : ");
					print_ae_int64(accu31);
					printf("accu32 : ");
					print_ae_int64(accu32);
					printf("accu33 : ");
					print_ae_int64(accu33);
					printf("Next Half\n");
                    AE_LA16X4X2_IP(vec00, vec10, align0, p_mat10);
                    AE_LA16X4X2_IP(vec01, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec02, vec12, align2, p_mat12);
                    AE_LA16X4X2_IP(vec03, vec13, align3, p_mat13);
                    printf("vec00 : ");
					print_ae_int16x4(vec00);
					printf("vec10 : ");
					print_ae_int16x4(vec10);
					printf("vec01 : ");
					print_ae_int16x4(vec01);
					printf("vec11 : ");
					print_ae_int16x4(vec11);
					printf("vec02 : ");
					print_ae_int16x4(vec02);
					printf("vec12 : ");
					print_ae_int16x4(vec12);
					printf("vec03 : ");
					print_ae_int16x4(vec03);
					printf("vec13 : ");
					print_ae_int16x4(vec13);
                    AE_MULA8QW8X16(accu00, accu01, accu02, accu03, mat10, mat11, mat12, mat13, vec00, vec10);
                    AE_MULA8QW8X16(accu10, accu11, accu12, accu13, mat10, mat11, mat12, mat13, vec01, vec11);
                    AE_MULA8QW8X16(accu20, accu21, accu22, accu23, mat10, mat11, mat12, mat13, vec02, vec12);
                    AE_MULA8QW8X16(accu30, accu31, accu32, accu33, mat10, mat11, mat12, mat13, vec03, vec13);
                    printf("accu00 : ");
					print_ae_int64(accu00);
					printf("accu01 : ");
					print_ae_int64(accu01);
					printf("accu02 : ");
					print_ae_int64(accu02);
					printf("accu03 : ");
					print_ae_int64(accu03);
					printf("accu10 : ");
					print_ae_int64(accu10);
					printf("accu11 : ");
					print_ae_int64(accu11);
					printf("accu12 : ");
					print_ae_int64(accu12);
					printf("accu13 : ");
					print_ae_int64(accu13);
					printf("accu20 : ");
					print_ae_int64(accu20);
					printf("accu21 : ");
					print_ae_int64(accu21);
					printf("accu22 : ");
					print_ae_int64(accu22);
					printf("accu23 : ");
					print_ae_int64(accu23);
					printf("accu30 : ");
					print_ae_int64(accu30);
					printf("accu31 : ");
					print_ae_int64(accu31);
					printf("accu32 : ");
					print_ae_int64(accu32);
					printf("accu33 : ");
					print_ae_int64(accu33);

                }

                *p_out0++ = accu00; *p_out0++ = accu10; *p_out0++ = accu20; *p_out0++ = accu30;
                *p_out1++ = accu01; *p_out1++ = accu11; *p_out1++ = accu21; *p_out1++ = accu31;
                *p_out2++ = accu02; *p_out2++ = accu12; *p_out2++ = accu22; *p_out2++ = accu32;
                *p_out3++ = accu03; *p_out3++ = accu13; *p_out3++ = accu23; *p_out3++ = accu33;
            }
        }
    }

    if(mtx1_cols & 0x3)
    {
        matvec_mul_8x16_64(
                &p_out[col1],
                p_mtx0,
                &p_mtx1[col1 * mtx1_col_offset],
                mtx0_rows,
                mtx0_cols,
                mtx0_row_offset,
                mtx1_cols & 0x3,
                mtx1_col_offset,
                out_offset);
    }
}
#else
{
//	printf("Yes YEs\n");
    int row0 = 0, col0 = 0, col1 = 0;

    if(mtx1_cols >= 4)
    {
        for(col1 = 0; col1 < (mtx1_cols & (~0x3)); col1 += 4)
        {
            for (row0 = 0; row0 < mtx0_rows; row0 += 2)
            {
                ae_int16x8 *p_mat10;
                ae_int16x8 *p_mat11;
                ae_int16x8 *p_mat12;
                ae_int16x8 *p_mat13;
                ae_valignx2 align0;
                ae_valignx2 align1;
                ae_valignx2 align2;
                ae_valignx2 align3;

                ae_int64 *p_out0 = (ae_int64 *)(&p_out[col1 + (row0+0) * out_offset]);
                ae_int64 *p_out1 = (ae_int64 *)(&p_out[col1 + (row0+1) * out_offset]);

                p_mat10 = (ae_int16x8 *)&p_mtx1[(col1 + 0) * mtx1_col_offset];
                p_mat11 = (ae_int16x8 *)&p_mtx1[(col1 + 1) * mtx1_col_offset];
                p_mat12 = (ae_int16x8 *)&p_mtx1[(col1 + 2) * mtx1_col_offset];
                p_mat13 = (ae_int16x8 *)&p_mtx1[(col1 + 3) * mtx1_col_offset];
                align0 = AE_LA128_PP(p_mat10);
                align1 = AE_LA128_PP(p_mat11);
                align2 = AE_LA128_PP(p_mat12);
                align3 = AE_LA128_PP(p_mat13);

                ae_int8x16 *p_mat00 = (ae_int8x16*)&p_mtx0[(row0 + 0) * mtx0_row_offset];
                ae_int8x16 *p_mat01 = (ae_int8x16*)&p_mtx0[(row0 + 1) * mtx0_row_offset];

                ae_int64 accu00_0 = AE_ZERO64(), accu00_1 = AE_ZERO64(), accu10_0 = AE_ZERO64(), accu10_1 = AE_ZERO64();
                ae_int64 accu01_0 = AE_ZERO64(), accu01_1 = AE_ZERO64(), accu11_0 = AE_ZERO64(), accu11_1 = AE_ZERO64();
                ae_int64 accu02_0 = AE_ZERO64(), accu02_1 = AE_ZERO64(), accu12_0 = AE_ZERO64(), accu12_1 = AE_ZERO64();
                ae_int64 accu03_0 = AE_ZERO64(), accu03_1 = AE_ZERO64(), accu13_0 = AE_ZERO64(), accu13_1 = AE_ZERO64();

#pragma ymemory(p_mat10)
#pragma ymemory(p_mat11)
                /*#pragma ymemory(p_mat12)
#pragma ymemory(p_mat13)*/
                /* This kernel implements a 2x16 by 16x4 matrix multiplication */
                for (col0 = 0; col0 < mtx0_cols>>4; col0++)
                {
                    ae_int16x4 vec00, vec01;
                    ae_int16x4 vec10, vec11;
                    ae_int16x4 vec20, vec21;
                    ae_int16x4 vec30, vec31;
                    ae_int8x8 mat00, mat01;
                    ae_int8x8 mat10, mat11;

                    AE_L8X8X2_IP(mat00, mat01, p_mat00, 16);
                    AE_L8X8X2_IP(mat10, mat11, p_mat01, 16);

                    AE_LA16X4X2_IP(vec00, vec01, align0, p_mat10);
                    AE_LA16X4X2_IP(vec10, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec20, vec21, align2, p_mat12);
                    AE_LA16X4X2_IP(vec30, vec31, align3, p_mat13);
                    AE_MULAAAA2Q16X8(accu00_0, accu00_1, vec00, vec01, mat00);
                    AE_MULAAAA2Q16X8(accu01_0, accu01_1, vec10, vec11, mat00);
                    AE_MULAAAA2Q16X8(accu02_0, accu02_1, vec20, vec21, mat00);
                    AE_MULAAAA2Q16X8(accu03_0, accu03_1, vec30, vec31, mat00);
                    AE_MULAAAA2Q16X8(accu10_0, accu10_1, vec00, vec01, mat10);
                    AE_MULAAAA2Q16X8(accu11_0, accu11_1, vec10, vec11, mat10);
                    AE_MULAAAA2Q16X8(accu12_0, accu12_1, vec20, vec21, mat10);
                    AE_MULAAAA2Q16X8(accu13_0, accu13_1, vec30, vec31, mat10);

                    AE_LA16X4X2_IP(vec00, vec01, align0, p_mat10);
                    AE_LA16X4X2_IP(vec10, vec11, align1, p_mat11);
                    AE_LA16X4X2_IP(vec20, vec21, align2, p_mat12);
                    AE_LA16X4X2_IP(vec30, vec31, align3, p_mat13);
                    AE_MULAAAA2Q16X8(accu00_0, accu00_1, vec00, vec01, mat01);
                    AE_MULAAAA2Q16X8(accu01_0, accu01_1, vec10, vec11, mat01);
                    AE_MULAAAA2Q16X8(accu02_0, accu02_1, vec20, vec21, mat01);
                    AE_MULAAAA2Q16X8(accu03_0, accu03_1, vec30, vec31, mat01);
                    AE_MULAAAA2Q16X8(accu10_0, accu10_1, vec00, vec01, mat11);
                    AE_MULAAAA2Q16X8(accu11_0, accu11_1, vec10, vec11, mat11);
                    AE_MULAAAA2Q16X8(accu12_0, accu12_1, vec20, vec21, mat11);
                    AE_MULAAAA2Q16X8(accu13_0, accu13_1, vec30, vec31, mat11);
                }

                accu00_0 += accu00_1;
                accu01_0 += accu01_1;
                accu02_0 += accu02_1;
                accu03_0 += accu03_1;
                accu10_0 += accu10_1;
                accu11_0 += accu11_1;
                accu12_0 += accu12_1;
                accu13_0 += accu13_1;

                *p_out0++ = accu00_0; *p_out0++ = accu01_0; *p_out0++ = accu02_0; *p_out0++ = accu03_0;
                *p_out1++ = accu10_0; *p_out1++ = accu11_0; *p_out1++ = accu12_0; *p_out1++ = accu13_0;
            }
        }
    }

    if(mtx1_cols & 0x3)
    {
        matvec_mul_8x16_64(
                &p_out[col1],
                p_mtx0,
                &p_mtx1[col1 * mtx1_col_offset],
                mtx0_rows,
                mtx0_cols,
                mtx0_row_offset,
                mtx1_cols & 0x3,
                mtx1_col_offset,
                out_offset);
    }
}
#endif

#else
#error "Works only with HiFi5"
#endif
```