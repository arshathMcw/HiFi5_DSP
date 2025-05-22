## Notes
* The name conveys that We do multply and accumulation for 4bit input and 8bit vector and store the result in 32bit integer
* char : 
    * Size - One byte (8 bits)
    * Between -128 and 127 (if signed)
    * between 0 and 255 (if unsigned)
* 4-bit signed range: −8 to +7
    * So we do: value & 0x0F, then convert to 4-bit signed:
    * If result ≥ 8, subtract 16 (sign-extension logic)


## Helper Functions
## Resources : 
* Hex to integer - [link](https://www.rapidtables.com/convert/number/hex-to-decimal.html?x=118C)
*  Big Endian and Little Endian : [link](https://www.youtube.com/watch?v=WBA6svOyWb8&t=70s)


![alt text](image.png)

```
m_reg00 : 33 ,-95 ,-9 ,54 ,-7 ,-32 ,-89 ,70 ,-112 ,-3 ,-1 ,99 ,0 ,0 ,0 ,0 
m_reg01 : -40 ,-13 ,120 ,-79 ,95 ,90 ,103 ,-81 ,-112 ,-3 ,-1 ,99 ,0 ,0 ,0 ,0 
dvec00 : 38 ,63 ,126 ,-4 
dvec01 : 9 ,-12 ,-11 ,-81 
Q0 : -1233 -1399
Q1 : -623 898
```

## Understand Code
```c
#include "matvec_mul_4x8.h"
#include <stdio.h>
#define ZERO64 AE_MOVINT64_FROMINT32X2(AE_MOVDA32(0))
#define ZERO32 AE_MOVDA32(0)

#if !(XCHAL_HAVE_HIFI5_NN_MAC)
#error "This example works only with HiFi 5 + NN MaC"
#else


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
    printf("Row Offset Byte : %d\n",row_offset_byte);
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
            printf("m_reg00 : ");
            print_ae_int8x8(m_reg00);
            printf("m_reg01 : ");
            print_ae_int8x8(m_reg01);
			printf("m_reg10 : ");
			print_ae_int8x8(m_reg10);
			printf("m_reg11 : ");
			print_ae_int8x8(m_reg11);
			printf("m_reg20 : ");
			print_ae_int8x8(m_reg20);
			printf("m_reg21 : ");
			print_ae_int8x8(m_reg21);
			printf("m_reg30 : ");
			print_ae_int8x8(m_reg30);
			printf("m_reg31 : ");
			print_ae_int8x8(m_reg31);
            d3 = AE_L8X8_X(p_src1, 3*vec_offset_by4);
            d2 = AE_L8X8_X(p_src1, 2*vec_offset_by4);
            d1 = AE_L8X8_X(p_src1, 1*vec_offset_by4);
            AE_L8X8_IP(d0, p_src1, 8);
            printf("D0 : ");
			print_ae_int8x8(d0);
			printf("D1 : ");
			print_ae_int8x8(d1);
			printf("D2 : ");
			print_ae_int8x8(d2);
			printf("D3 : ");
			print_ae_int8x8(d3);


            AE_SUBW8(dvec00, dvec01, d0, zero8);
            AE_SUBW8(dvec10, dvec11, d1, zero8);
            AE_SUBW8(dvec20, dvec21, d2, zero8);
            AE_SUBW8(dvec30, dvec31, d3, zero8);
            printf("dvec00 : ");
            print_ae_int16x4(dvec00);
            printf("dvec01 : ");
            print_ae_int16x4(dvec01);
            printf("dvec10 : ");
			print_ae_int16x4(dvec10);
			printf("dvec11 : ");
			print_ae_int16x4(dvec11);
			printf("dvec20 : ");
			print_ae_int16x4(dvec20);
			printf("dvec21 : ");
			print_ae_int16x4(dvec21);
			printf("dvec30 : ");
			print_ae_int16x4(dvec30);
			printf("dvec31 : ");
			print_ae_int16x4(dvec31);
			printf("mmreg00 4x16 : ");
			ae_int4x16 temp = AE_MOVINT4X16_FROMINT8X8(m_reg00);
			print_ae_int4x16(temp);
			printf("mmreg01 4x16 : ");
			temp = AE_MOVINT4X16_FROMINT8X8(m_reg01);
			print_ae_int4x16(temp);
			printf("mmreg10 4x16 : ");
			temp = AE_MOVINT4X16_FROMINT8X8(m_reg10);
			print_ae_int4x16(temp);
			printf("mmreg11 4x16 : ");
			temp = AE_MOVINT4X16_FROMINT8X8(m_reg11);
			print_ae_int4x16(temp);
            AE_MULA8Q4X16(q0, q1, AE_MOVINT4X16_FROMINT8X8(m_reg00), AE_MOVINT4X16_FROMINT8X8(m_reg01), dvec00, dvec01);
            AE_MULA8Q4X16(q2, q3, AE_MOVINT4X16_FROMINT8X8(m_reg10), AE_MOVINT4X16_FROMINT8X8(m_reg11), dvec10, dvec11);
            printf("Q0 : ");
			print_ae_int32x2(q0);
			printf("Q1 : ");
			print_ae_int32x2(q1);
			printf("Q2 : ");
			print_ae_int32x2(q2);
			printf("Q3 : ");
			print_ae_int32x2(q3);
            AE_MULA8Q4X16(q0, q1, AE_MOVINT4X16_FROMINT8X8(m_reg20), AE_MOVINT4X16_FROMINT8X8(m_reg21), dvec20, dvec21);
            AE_MULA8Q4X16(q2, q3, AE_MOVINT4X16_FROMINT8X8(m_reg30), AE_MOVINT4X16_FROMINT8X8(m_reg31), dvec30, dvec31);
//            printf("mmreg30 4x16 : ");
//            ae_int4x16 temp = AE_MOVINT4X16_FROMINT8X8(m_reg30);
//			print_ae_int4x16(temp);
//			printf("mmreg31 4x16 : ");
//			temp = AE_MOVINT4X16_FROMINT8X8(m_reg31);
//			print_ae_int4x16(temp);
            printf("Q0 : ");
			print_ae_int32x2(q0);
			printf("Q1 : ");
			print_ae_int32x2(q1);
			printf("Q2 : ");
			print_ae_int32x2(q2);
			printf("Q3 : ");
			print_ae_int32x2(q3);
            AE_L8X8X2_X(m_reg70, m_reg71, p_mat4_0, 3*row_offset_byte);
            AE_L8X8X2_X(m_reg60, m_reg61, p_mat4_0, 2*row_offset_byte);
            AE_L8X8X2_X(m_reg50, m_reg51, p_mat4_0, row_offset_byte);
            AE_L8X8X2_IP(m_reg40, m_reg41, p_mat4_0, 16);
            printf("m_reg40 : ");
            print_ae_int8x8(m_reg40);
            printf("m_reg41 : ");
            print_ae_int8x8(m_reg41);
			printf("m_reg50 : ");
			print_ae_int8x8(m_reg50);
			printf("m_reg51 : ");
			print_ae_int8x8(m_reg51);
			printf("m_reg60 : ");
			print_ae_int8x8(m_reg60);
			printf("m_reg61 : ");
			print_ae_int8x8(m_reg61);
			printf("m_reg70 : ");
			print_ae_int8x8(m_reg70);
			printf("m_reg71 : ");
			print_ae_int8x8(m_reg71);
            AE_MULA8Q4X16(q4, q5, AE_MOVINT4X16_FROMINT8X8(m_reg40), AE_MOVINT4X16_FROMINT8X8(m_reg41), dvec00, dvec01);
            AE_MULA8Q4X16(q6, q7, AE_MOVINT4X16_FROMINT8X8(m_reg50), AE_MOVINT4X16_FROMINT8X8(m_reg51), dvec10, dvec11);
            printf("Q4 : ");
			print_ae_int32x2(q4);
			printf("Q5 : ");
			print_ae_int32x2(q5);
			printf("Q6 : ");
			print_ae_int32x2(q6);
			printf("Q7 : ");
			print_ae_int32x2(q7);
            AE_MULA8Q4X16(q4, q5, AE_MOVINT4X16_FROMINT8X8(m_reg60), AE_MOVINT4X16_FROMINT8X8(m_reg61), dvec20, dvec21);
            AE_MULA8Q4X16(q6, q7, AE_MOVINT4X16_FROMINT8X8(m_reg70), AE_MOVINT4X16_FROMINT8X8(m_reg71), dvec30, dvec31);
            printf("Q4 : ");
			print_ae_int32x2(q4);
			printf("Q5 : ");
			print_ae_int32x2(q5);
			printf("Q6 : ");
			print_ae_int32x2(q6);
			printf("Q7 : ");
			print_ae_int32x2(q7);
          }


//          //STORE;
          q0 += q2;
          q1 += q3;
          q4 += q6;
          q5 += q7;
//          Scale
      printf("After Added : \n");
      q0 = AE_SLAA32S(q0, lsh);
      q1 = AE_SLAA32S(q1, lsh);
      q4 = AE_SLAA32S(q4, lsh);
      q5 = AE_SLAA32S(q5, lsh);
      printf("Q0 = Q0+Q2: ");
      print_ae_int32x2(q0);
      printf("Q1 = Q1+Q3: ");
      print_ae_int32x2(q1);
      printf("Q4 = Q4+Q6: ");
      print_ae_int32x2(q4);
      printf("Q5 = Q5 +Q7: ");
      print_ae_int32x2(q5);
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
```