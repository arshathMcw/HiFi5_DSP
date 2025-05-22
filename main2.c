// #include<stdio.h>
// #define ALIGN(x) __attribute__((aligned(x)))
// #include <stdlib.h>
// #include <string.h>
// #define MTX0_ROWS 4
// #define MTX0_COLS 4
// #define MTX1_ROWS MTX0_COLS
// #define MTX1_COLS 4

// signed char input_matrix0[MTX0_ROWS * MTX0_COLS];
// short input_matrix1[MTX1_COLS * MTX1_ROWS];
// long long ref_output[MTX0_ROWS*MTX1_COLS];
// long long output[MTX0_ROWS*MTX1_COLS];


// void mat_mul_8x16_64_ref(
//     long long * p_out,
//     const signed char * p_mtx0,
//     const short * p_mtx1,
//     int mtx0_rows,
//     int mtx0_cols,
//     int mtx0_row_offset,
//     int mtx1_cols,
//     int mtx1_col_offset,
//     int out_offset)
// {
//     int row0 = 0, col0 = 0, col1 = 0;
//     for(col1 = 0; col1 < mtx1_cols; col1++)
//     {
//         long long *p_dst0 = &p_out[col1];
//         for (row0 = 0; row0 < mtx0_rows; row0++)
//         {
//             const signed char *p_mat0 = &p_mtx0[row0 * mtx0_row_offset];
//             short *p_mat1 = (short *)&p_mtx1[col1 * mtx1_col_offset];
//             long long accu1 = 0;
//             for (col0 = 0; col0 < mtx0_cols; col0++)
//             {
//                 accu1 += p_mat0[col0]*p_mat1[col0];
//             }
//             p_dst0[row0 * out_offset] = accu1;
//         }
//     }
//   }


// void mat_mul_8x16_64_ref(
// long long * p_out,
// const signed char * p_mtx0,
// const short * p_mtx1,
// int mtx0_rows,
//     int mtx0_cols,
//     int mtx0_row_offset,
// int mtx1_cols,
//     int mtx1_col_offset,
//     int out_offset);
// int main(int argc, char *argv[])
// {
//     for (int i = 0; i < MTX0_ROWS * MTX0_COLS; i++) {
//         input_matrix0[i] = (signed char)(i % 128); 
//     }
//     printf("Input Matrix : \n");
//     for(int r = 0;r < MTX0_ROWS;r++){
//         for(int c = 0;c < MTX0_COLS;c++){
//             printf("%d ",input_matrix0[(r*MTX0_COLS)+c]);
//         }
//         printf("\n");
//     }
//     for (int i = 0; i < MTX1_ROWS * MTX1_COLS; i++) {
//         input_matrix1[i] = (short)((i % 256)); 
//     }
//     printf("\nInput Matrix 1 : \n");
//     for(int r = 0;r < MTX1_ROWS;r++){
//         for(int c = 0;c < MTX1_COLS;c++){
//             printf("%d ",input_matrix1[(r*MTX1_COLS)+c]);
//         }
//         printf("\n");
//     }
//     printf("\n");
//     mat_mul_8x16_64_ref(
//         ref_output,
//         input_matrix0,
//         input_matrix1,
//         MTX0_ROWS,
//         MTX0_COLS,
//         MTX0_COLS,
//         MTX1_COLS,
//         MTX1_ROWS,
//         MTX1_COLS
//     );
//     printf("Result : \n");
//     for(int r = 0;r < MTX0_ROWS;r++){
//         for(int c = 0;c < MTX1_COLS;c++){
//             printf("%lld ",ref_output[(r*MTX1_COLS)+c]);
//         }
//         printf("\n");
//     }
//     return 0;
// }



// #include "matvec_mul_4x8.h"
#include <stdio.h>
#define ZERO64 AE_MOVINT64_FROMINT32X2(AE_MOVDA32(0))
#define ZERO32 AE_MOVDA32(0)


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


int xa_nn_matXvec_4x8_32(
  Int32 * __restrict__ p_out, /* output */
  Int4 * __restrict__ p_mat, /* matrix: rows x cols */
  Int8 * __restrict__ p_vec, /* vec: cols */
  int rows, int cols, int row_offset,
  int vec_count, int vec_offset, int out_offset,
  Bool b_accumulate, int lsh)
{
  int row, col, vec=0;


  if ((0 >= rows ) || (0 >= cols ) || (cols & 0xf) || (rows & 0x3)) //minimum columns needed: 16, minimum rows needed is in multiples of 4
  {
    return -2;
  }
  if(0 >= vec_count) return -3;

#define DATA_DENSITY    2
  for(vec = 0; vec < vec_count ; vec+=1)
  {
    ae_int32x2 *p_dst = (ae_int32x2 *)&p_out[vec     * (out_offset)];
    ae_int32x2 q0, q1, q2, q3, q4, q5, q6, q7;
    ae_int16x4 dvec00, dvec10, dvec20, dvec30;
    ae_int16x4 dvec01, dvec11, dvec21, dvec31;


    ae_int8x8 m_reg00, m_reg10, m_reg20, m_reg30, m_reg40, m_reg50, m_reg60, m_reg70;
    ae_int8x8 m_reg01, m_reg11, m_reg21, m_reg31, m_reg41, m_reg51, m_reg61, m_reg71;

    int row_offset_byte = row_offset/DATA_DENSITY;
    int vec_offset_by4 = cols >> 2;
    //int vec_offset_by4 = 4 ;

#define UNROLL  8 /// Optimal unroll

      row = 0;
      if(rows >= (UNROLL))
      {
        for (row = 0; row < ( rows & ~((UNROLL)-1)) ; row+=(UNROLL))
        {
          //SETUP;
          ae_int8x16 *p_mat0_0 = (ae_int8x16 *)&p_mat[(row)*row_offset_byte];
          ae_int8x16 *p_mat4_0 = (ae_int8x16 *)&p_mat[(row+4)*row_offset_byte];
          ae_int8x8 *p_src1 = (ae_int8x8 *)&p_vec[vec *vec_offset];
          q0 = q1 = q2 = q3 = q4 = q5 = q6 = q7 = ZERO32;
          const ae_int8x8 zero8 = AE_MOVINT8X8_FROMINT32X2(AE_MOVDA32X2(0x0, 0x0));

#pragma ymemory(p_mat0_0)
#pragma ymemory(p_mat4_0)
//

          for (col = 0; col < cols>>5; col++) {
            ae_int8x8 d0, d1, d2, d3;
            AE_L8X8X2_X(m_reg30, m_reg31, p_mat0_0, 3*row_offset_byte);
            AE_L8X8X2_X(m_reg20, m_reg21, p_mat0_0, 2*row_offset_byte);
            AE_L8X8X2_X(m_reg10, m_reg11, p_mat0_0, row_offset_byte);
            AE_L8X8X2_IP(m_reg00, m_reg01, p_mat0_0, 16);

            d3 = AE_L8X8_X(p_src1, 3*vec_offset_by4);
            d2 = AE_L8X8_X(p_src1, 2*vec_offset_by4);
            d1 = AE_L8X8_X(p_src1, 1*vec_offset_by4);

            AE_L8X8_IP(d0, p_src1, 8);
            AE_SUBW8(dvec00, dvec01, d0, zero8);
            AE_SUBW8(dvec10, dvec11, d1, zero8);
            AE_SUBW8(dvec20, dvec21, d2, zero8);
            AE_SUBW8(dvec30, dvec31, d3, zero8);

            AE_MULA8Q4X16(q0, q1, AE_MOVINT4X16_FROMINT8X8(m_reg00), AE_MOVINT4X16_FROMINT8X8(m_reg01), dvec00, dvec01);
            AE_MULA8Q4X16(q2, q3, AE_MOVINT4X16_FROMINT8X8(m_reg10), AE_MOVINT4X16_FROMINT8X8(m_reg11), dvec10, dvec11);
            AE_MULA8Q4X16(q0, q1, AE_MOVINT4X16_FROMINT8X8(m_reg20), AE_MOVINT4X16_FROMINT8X8(m_reg21), dvec20, dvec21);
            AE_MULA8Q4X16(q2, q3, AE_MOVINT4X16_FROMINT8X8(m_reg30), AE_MOVINT4X16_FROMINT8X8(m_reg31), dvec30, dvec31);

            AE_L8X8X2_X(m_reg70, m_reg71, p_mat4_0, 3*row_offset_byte);
            AE_L8X8X2_X(m_reg60, m_reg61, p_mat4_0, 2*row_offset_byte);
            AE_L8X8X2_X(m_reg50, m_reg51, p_mat4_0, row_offset_byte);
            AE_L8X8X2_IP(m_reg40, m_reg41, p_mat4_0, 16);

            AE_MULA8Q4X16(q4, q5, AE_MOVINT4X16_FROMINT8X8(m_reg40), AE_MOVINT4X16_FROMINT8X8(m_reg41), dvec00, dvec01);
            AE_MULA8Q4X16(q6, q7, AE_MOVINT4X16_FROMINT8X8(m_reg50), AE_MOVINT4X16_FROMINT8X8(m_reg51), dvec10, dvec11);
            AE_MULA8Q4X16(q4, q5, AE_MOVINT4X16_FROMINT8X8(m_reg60), AE_MOVINT4X16_FROMINT8X8(m_reg61), dvec20, dvec21);
            AE_MULA8Q4X16(q6, q7, AE_MOVINT4X16_FROMINT8X8(m_reg70), AE_MOVINT4X16_FROMINT8X8(m_reg71), dvec30, dvec31);
          }


//          //STORE;
          q0 += q2;
          q1 += q3;
          q4 += q6;
          q5 += q7;
//          Scale
      q0 = AE_SLAA32S(q0, lsh);
      q1 = AE_SLAA32S(q1, lsh);
      q4 = AE_SLAA32S(q4, lsh);
      q5 = AE_SLAA32S(q5, lsh);
      if(b_accumulate)
      {
        p_dst[(row>>1)] = p_dst[(row>>1)] + q0;
        p_dst[(row>>1)+1] = p_dst[(row>>1)+1] + q1;
        p_dst[(row>>1)+2] = p_dst[(row>>1)+2] + q4;
        p_dst[(row>>1)+3] = p_dst[(row>>1)+3] + q5;
      }
      else
      {
        p_dst[(row>>1)] = q0;
        p_dst[(row>>1)+1] = q1;
        p_dst[(row>>1)+2] = q4;
        p_dst[(row>>1)+3] = q5;
      }
        }
      }
#undef UNROLL
  }

  return 0;
}

#endif