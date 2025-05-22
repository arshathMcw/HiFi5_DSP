```
This script detects software pipelined loops in assembly
And prints a summary of each such loop
See Window -> Preferences -> Directories and Tools -> Binary View Tools -> Perl Script for Loop Summary
For the location of this script
NOTE: This is a sample script. You are encouraged to write your own script for additional information.

Analysis start ...............
#<swps>	loop at line 19 at file ElementAdd/main.c
#<swps>	3 cycles per pipeline stage in steady state with unroll=1
#<swps>	1 pipeline stages
#<swps>	4 real ops (excluding nop)
#<swps>	2 cycles lower bound required by resources
#<swps>	min 3 cycles lower bound required by recurrence
#<swps>	min 3 cycles lower bound required by resource/recurrence
#<swps>	min 2 cycles required for critical path
#<swps>	3 cycles non-loop schedule length
#<swps>	unknown trip count
#<swps>	register allocated
#<swps>	debug id = 1

-------------------Instructions packing listed slotwise ----------------------------- 
Inst_Num  FORMAT    SLOT0                              SLOT1                              SLOT2                              SLOT3                              SLOT4                              cycle_num      Bubbles_Found?
1         ae_format_3ae_l16x4.ip  aed0,a3,8             ae_l16x4.ip  aed1,a2,8                                                                                                                      cycle_1        No
2         ae_format_1ae_add16  aed0,aed1,aed0           nop                                                                                                                                         cycle_2        No
3         Xtensa    ae_s16x4.ip  aed0,a4,8                                                                                                                                                         cycle_3        No

-------------------SPILL INFORMATION ----------------------------- 
NO SPILLS FOUND

-------------------Operations listed slotwise ----------------------------- 
OPCODE                    : S0 Freq   S1 Freq   S2 Freq   S3 Freq   S4 Freq   Scalar    Opcode Freq
ae_l16x4.ip               : 1         1         0         0         0         0         2         
ae_add16                  : 1         0         0         0         0         0         1         
ae_s16x4.ip               : 0         0         0         0         0         1         1         
TOTAL                     : 2         1         0         0         0         1         4         
Total FLIX Instructions found = 2
Total Scalar Instructions found = 1
Total Bubbles cycles = 0


#<swps>	loop at line 33 at file ElementAdd/main.c
#<swps>	4 cycles per pipeline stage in steady state with unroll=2
#<swps>	1 pipeline stages
#<swps>	8 real ops (excluding nop)
#<swps>	4 cycles lower bound required by resources
#<swps>	min 2 cycles lower bound required by recurrence
#<swps>	min 4 cycles lower bound required by resource/recurrence
#<swps>	min 1 cycles required for critical path
#<swps>	5 cycles non-loop schedule length
#<swps>	trip count: 100
#<swps>	register allocated
#<swps>	debug id = 3

-------------------Instructions packing listed slotwise ----------------------------- 
Inst_Num  FORMAT    SLOT0                              SLOT1                              SLOT2                              SLOT3                              SLOT4                              cycle_num      Bubbles_Found?
                                                                                                                                                                               cycle_1        No
1         ae_format_1s16i  a4,a3,0                      addi  a5,a4,1                                                                                                                               cycle_1        No
2         ae_format_1s16i  a5,a3,2                      addi  a3,a3,4                                                                                                                               cycle_2        No
3         ae_format_1s16i  a4,a2,0                      addi  a4,a5,1                                                                                                                               cycle_3        No
4         ae_format_1s16i  a5,a2,2                      addi  a2,a2,4                                                                                                                               cycle_4        No

-------------------SPILL INFORMATION ----------------------------- 
NO SPILLS FOUND

-------------------Operations listed slotwise ----------------------------- 
OPCODE                    : S0 Freq   S1 Freq   S2 Freq   S3 Freq   S4 Freq   Scalar    Opcode Freq
s16i                      : 4         0         0         0         0         0         4         
addi                      : 0         4         0         0         0         0         4         
TOTAL                     : 4         4         0         0         0         0         8         
Total FLIX Instructions found = 4
Total Scalar Instructions found = 0
Total Bubbles cycles = 0


#<swps>	loop at line 19 at file ElementAdd/main.c
#<swps>	3 cycles per pipeline stage in steady state with unroll=2
#<swps>	2 pipeline stages
#<swps>	8 real ops (excluding nop)
#<swps>	3 cycles lower bound required by resources
#<swps>	min 1 cycles lower bound required by recurrence
#<swps>	min 3 cycles lower bound required by resource/recurrence
#<swps>	min 2 cycles required for critical path
#<swps>	4 cycles non-loop schedule length
#<swps>	trip count: 24
#<swps>	register allocated
#<swps>	debug id = 2

-------------------Instructions packing listed slotwise ----------------------------- 
Inst_Num  FORMAT    SLOT0                              SLOT1                              SLOT2                              SLOT3                              SLOT4                              cycle_num      Bubbles_Found?
                                                                                                                                                                               cycle_1        No
1         ae_format ae_l16x4.ip  aed0,a3,16            ae_l16x4.ip  aed1,a4,16            ae_add16  aed2,aed0,aed1           nop                                                                   cycle_1        No
2         ae_format ae_s16x4.i  aed2,a2,-8             ae_l16x4.i  aed0,a4,-8             ae_add16  aed1,aed1,aed0           nop                                                                   cycle_2        No
3         ae_format_1ae_s16x4.ip  aed1,a2,16            ae_l16x4.i  aed1,a3,-8                                                                                                                      cycle_3        No

-------------------SPILL INFORMATION ----------------------------- 
NO SPILLS FOUND

-------------------Operations listed slotwise ----------------------------- 
OPCODE                    : S0 Freq   S1 Freq   S2 Freq   S3 Freq   S4 Freq   Scalar    Opcode Freq
ae_l16x4.ip               : 1         1         0         0         0         0         2         
ae_add16                  : 0         0         2         0         0         0         2         
ae_s16x4.i                : 1         0         0         0         0         0         1         
ae_l16x4.i                : 0         2         0         0         0         0         2         
ae_s16x4.ip               : 1         0         0         0         0         0         1         
TOTAL                     : 3         3         2         0         0         0         8         
Total FLIX Instructions found = 3
Total Scalar Instructions found = 0
Total Bubbles cycles = 0



Analysis end ...............
```