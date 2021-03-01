/***************************************************************/
/*                                                             */
/*   ARMv4-32 Instruction Level Simulator                      */
/*                                                             */
/*   ECEN 4243                                                 */
/*   Oklahoma State University                                 */
/*                                                             */
/***************************************************************/

#ifndef _SIM_ISA_H_
#define _SIM_ISA_H_
#define N_CUR ( (CURRENT_STATE.CPSR>>31) & 0x00000001 )
#define Z_CUR ( (CURRENT_STATE.CPSR>>30) & 0x00000001 )
#define C_CUR ( (CURRENT_STATE.CPSR>>29) & 0x00000001 )
#define V_CUR ( (CURRENT_STATE.CPSR>>28) & 0x00000001 )
#define N_NXT ( (NEXT_STATE.CPSR>>31) & 0x00000001 )
#define Z_NXT ( (NEXT_STATE.CPSR>>30) & 0x00000001 )
#define C_NXT ( (NEXT_STATE.CPSR>>29) & 0x00000001 )
#define V_NXT ( (NEXT_STATE.CPSR>>28) & 0x00000001 )

#define N_N 0x80000000
#define Z_N 0x40000000
#define C_N 0x20000000
#define V_N 0x10000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"


/*
  For these commands LSL and LSR are primary for internal
  use within the isa ONLY.

  for ROR and ASR: 
  if I==1 you are working with an IMMEDIATE
  if I==0 you are working with a REGISTER
*/
int LSL (int num, int shift)
{
  return (num << shift);
}

int LSR (int num, int shift)
{
  return (num >> shift);
}

int ROR (int Rd, int SBZ, int Operand2, int I, int S, int CC)
{
  int cur = 0;
  int temp = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;

    /*
      temp will gather the bits that are going to "fall off"
      during the rotation. After the rotation those bits
      will be appended back onto the final value.
    */
    temp = CURRENT_STATE.REGS[Rm] & shamt5;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rm] << shamt5; 
          break;
        case 1: cur = CURRENT_STATE.REGS[Rm] >> shamt5;
          break;
        case 2: cur = CURRENT_STATE.REGS[Rm] >> shamt5;
          break;
        case 3: cur = ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur =  CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs];
          break;
        case 1: cur = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
          break;
        case 2: cur = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
          break;
        case 3: cur = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = Imm>>2*rotate|(Imm<<(32-2*rotate));
  }
  cur = cur | temp; // add the bits that fell off
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }
  return 0;	
}

int ASR (int Rd, int SBZ, int Operand2, int I, int S, int CC)
{
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    /*
      Given that the value stored within each register 
      is a 24-bit number thus it has to be shifted left then
      an arithmetic right shift must be done giving is a 32
      bit number that can be shifted.
    */
    CURRENT_STATE.REGS[Rm] = CURRENT_STATE.REGS[Rm] << 8;
    CURRENT_STATE.REGS[Rm] = CURRENT_STATE.REGS[Rm] & 0xFFFFFFFF;
    CURRENT_STATE.REGS[Rm] = (signed int)CURRENT_STATE.REGS[Rm] >> 8; // now this should be 32-bits
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = (signed int)CURRENT_STATE.REGS[Rm] << shamt5;
          break;
        case 1: cur = (signed int)CURRENT_STATE.REGS[Rm] >> shamt5;
          break;
        case 2: cur = (signed int)CURRENT_STATE.REGS[Rm] >> shamt5;
          break;
        case 3: cur = ((signed int)(CURRENT_STATE.REGS[Rm] >> shamt5) |
                (signed int)(CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur =  (signed int)CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs];
          break;
        case 1: cur = (signed int)CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
          break;
        case 2: cur =(signed int)CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
          break;
        case 3: cur = ((signed int)CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                ((signed int)CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = (signed int)Imm>>2*rotate|((signed int)Imm<<(32-2*rotate));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }
  return 0;	
}

int ADD (int Rd, int Rn, int Operand2, int I, int S, int CC) {

  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] + ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] + ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;

}

int ADC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] + ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] + ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur + C_CUR;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;

}

int AND (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] & ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] & ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
    }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;
}

int B (int offset2, int CC){
  
  // sign extention of the offset
  int off = offset2 & 0x00111111;
  off = off << 6;
  off = (signed int) off >> 6;
  off = off << 2;
  
  switch(CC){
    case 0: // beq
      if (Z_CUR == 0){
        CURRENT_STATE.REGS[15] = (CURRENT_STATE.REGS[15] + 8) + off;
      }
      else return 0;
      break;
    case 1: // bne
      if (Z_CUR == 1){
        CURRENT_STATE.REGS[15] = (CURRENT_STATE.REGS[15] + 8) + off;
      }
      else return 0;
      break;
  }
  return 0;
}

int BIC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] & ~(CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] & ~(CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] & ~(CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] & ~((CURRENT_STATE.REGS[Rm] >> shamt5) |
                ~(CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] & ~(CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	        break;
        case 1: cur = CURRENT_STATE.REGS[Rn] & ~(CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] & ~(CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	        break;
        case 3: cur = CURRENT_STATE.REGS[Rn] & ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                ~(CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	        break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & ~(Imm>>2*rotate|(Imm<<(32-2*rotate)));

    if(S == 1){
      if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
      if (cur == 0)
        NEXT_STATE.CPSR |= Z_N;
      if (cur > 0xFFFFFFFF)
        NEXT_STATE.CPSR |= C_N;
      if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
        NEXT_STATE.CPSR |= V_N;
    }
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;
}

int BL (int offset2, int CC){

  // sign extention of the offset
  int off = offset2 & 0x00111111;
  off = off << 6;
  off = (signed int) off >> 6;
  off = off << 2;

  switch(CC){
    case 0: // beq
      if (Z_CUR == 0){
        CURRENT_STATE.REGS[14] = (CURRENT_STATE.REGS[15] + 8) - 4;
        CURRENT_STATE.REGS[15] = (CURRENT_STATE.REGS[15] + 8) + off;
      }
      else return 0;
      break;
    case 1: // bne
      if (Z_CUR == 1){
        CURRENT_STATE.REGS[14] = (CURRENT_STATE.REGS[15] + 8) - 4;
        CURRENT_STATE.REGS[15] = (CURRENT_STATE.REGS[15] + 8) + off;
      }
      else return 0;
      break;
  }
  return 0;
}

int CMN (int SBZ, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) {
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] << shamt5);
	        break;
        case 1: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> shamt5);
	        break;
        case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> shamt5);
    	    break;
        case 3: cur = CURRENT_STATE.REGS[Rn] + ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	        break;
      }  
    }   
    else{
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	        break;
        case 1: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	        break;
        case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	        break;
        case 3: cur = CURRENT_STATE.REGS[Rn] + ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	        break;
      } 
    }     
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
}

int CMP (int SBZ, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;
      case 1: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> shamt5);
    	  break;
      case 3: cur = CURRENT_STATE.REGS[Rn] - ((CURRENT_STATE.REGS[Rm] >> shamt5) |
              (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] - ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	        break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }
  return 0;	
}

int EOR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] ^ ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] ^ ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] ^ (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;
}

int LDR (int Rd, int Rn, int I, int P, int U, int W, int src2, int CC){
  //if(Rd == 15)
  int data = 0;
  int Rm = src2 & 0x00000000F;
  //Immediate values
  if(I == 0){
    if( P == 1 & W == 0){
      if( U == 1) data = CURRENT_STATE.REGS[Rn] + src2;
      else data = CURRENT_STATE.REGS[Rn] - src2; 
      return 0;
    }
    else if(P == 1 & W == 1){
      if( U == 1) data = CURRENT_STATE.REGS[Rn] + src2;
      else data = CURRENT_STATE.REGS[Rn] - src2; 
      //TODO: CONDITION TO BE MET
      //Always condition??
      return 0;
    }
    else if(P == 0 & W == 0){
      if(Rn == Rd){
        if( U == 1) data = CURRENT_STATE.REGS[Rn] + src2;
        else data = CURRENT_STATE.REGS[Rn] - src2; 
      }
      return 0;
    }
  }
  //Register values
  //Non-shifted registered treated as shifted register
  //with shift of 0
  else{
    int shamt_5 = (src2 >> 7) & 0x0000001F;
    int sh = (src2 >> 5) & 0x00000003;
    int Rm = src2 & 0x0000000F;
    int index = 0;
    int off = 0;

    if( P == 1 & W == 0){
      switch(sh){
        case 0:
          index = CURRENT_STATE.REGS[Rm] << shamt_5;
          break;
        case 1:
          index = CURRENT_STATE.REGS[Rm] >> shamt_5;
          break;
        case 2:
          off = CURRENT_STATE.REGS[Rm];
          off = (signed int) off >> shamt_5;
          index = off;
          break;
        case 3:
          index = ((CURRENT_STATE.REGS[Rm] >> shamt_5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt_5)));
          break;
      }
      if(U == 1) data = CURRENT_STATE.REGS[Rn] + index;
      else data = CURRENT_STATE.REGS[Rn] - index;
      
      return 0;  
    }
    else if( P == 1 & W == 1){
      int shamt_5 = (src2 >> 7) & 0x0000001F;
      int sh = (src2 >> 5) & 0x00000003;
      int Rm = src2 & 0x0000000F;
      int index = 0;
      int off;

      switch(sh){
        case 0:
          index = CURRENT_STATE.REGS[Rm] << shamt_5;
          break;
        case 1:
          index = CURRENT_STATE.REGS[Rm] >> shamt_5;
          break;
        case 2:
          off = CURRENT_STATE.REGS[Rm];
          off = (signed int) off >> shamt_5;
          index = off;
          break;
        case 3:
          index = ((CURRENT_STATE.REGS[Rm] >> shamt_5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt_5)));
          break;
      }
          
      if(U == 1) data = CURRENT_STATE.REGS[Rn] + index;
      else data = CURRENT_STATE.REGS[Rn] - index;
      CURRENT_STATE.REGS[Rn] = data;

      return 0;
    }
    else if( P == 0 & W == 0){
      int shamt_5 = (src2 >> 7) & 0x0000001F;
      int sh = (src2 >> 5) & 0x00000003;
      int Rm = src2 & 0x0000000F;
      int index = 0;
      int off = 0;

      
      switch(sh){
        case 0:
          index = CURRENT_STATE.REGS[Rm] << shamt_5;
          break;
        case 1:
          index = CURRENT_STATE.REGS[Rm] >> shamt_5;
          break;
        case 2:
          off = CURRENT_STATE.REGS[Rm];
          off = (signed int) off >> shamt_5;
          index = off;
          break;
        case 3:
          index = ((CURRENT_STATE.REGS[Rm] >> shamt_5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt_5)));
          break;
      }
      if(U == 1) data = CURRENT_STATE.REGS[Rn] + index;
      else data = CURRENT_STATE.REGS[Rn] - index;
      CURRENT_STATE.REGS[Rn] = data;

      return 0;
    }
  }
}

int LDRB (int Rd, int Rn, int I, int P, int U, int W, int src2, int CC){
  //if(Rd == 15)
  int data = 0;
  int Rm = src2 & 0x00000000F;
  //Immediate values
  if(I == 0){
    if( P == 1 & W == 0){
      if( U == 1) data = CURRENT_STATE.REGS[Rn] + src2;
      else data = CURRENT_STATE.REGS[Rn] - src2; 
      return 0;
    }
    else if(P == 1 & W == 1){
      if( U == 1) data = CURRENT_STATE.REGS[Rn] + src2;
      else data = CURRENT_STATE.REGS[Rn] - src2; 
      //TODO: CONDITION TO BE MET
      //Always condition??
      return 0;
    }
    else if(P == 0 & W == 0){
      if(Rn == Rd){
        if( U == 1) data = CURRENT_STATE.REGS[Rn] + src2;
        else data = CURRENT_STATE.REGS[Rn] - src2; 
      }
      return 0;
    }
  }
  //Register values
  //Non-shifted registered treated as shifted register
  //with shift of 0
  else{
    int shamt_5 = (src2 >> 7) & 0x0000001F;
    int sh = (src2 >> 5) & 0x00000003;
    int Rm = src2 & 0x0000000F;
    int index = 0;
    int off = 0;

    if( P == 1 & W == 0){
      switch(sh){
        case 0:
          index = CURRENT_STATE.REGS[Rm] << shamt_5;
          break;
        case 1:
          index = CURRENT_STATE.REGS[Rm] >> shamt_5;
          break;
        case 2:
          off = CURRENT_STATE.REGS[Rm];
          off = (signed int) off >> shamt_5;
          index = off;
          break;
        case 3:
          index = ((CURRENT_STATE.REGS[Rm] >> shamt_5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt_5)));
          break;
      }
      if(U == 1) data = CURRENT_STATE.REGS[Rn] + index;
      else data = CURRENT_STATE.REGS[Rn] - index;
      
      return 0;  
    }
    else if( P == 1 & W == 1){
      int shamt_5 = (src2 >> 7) & 0x0000001F;
      int sh = (src2 >> 5) & 0x00000003;
      int Rm = src2 & 0x0000000F;
      int index = 0;
      int off;

      switch(sh){
        case 0:
          index = CURRENT_STATE.REGS[Rm] << shamt_5;
          break;
        case 1:
          index = CURRENT_STATE.REGS[Rm] >> shamt_5;
          break;
        case 2:
          off = CURRENT_STATE.REGS[Rm];
          off = (signed int) off >> shamt_5;
          index = off;
          break;
        case 3:
          index = ((CURRENT_STATE.REGS[Rm] >> shamt_5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt_5)));
          break;
      }
          
      if(U == 1) data = CURRENT_STATE.REGS[Rn] + index;
      else data = CURRENT_STATE.REGS[Rn] - index;
      CURRENT_STATE.REGS[Rn] = data;

      return 0;
    }
    else if( P == 0 & W == 0){
      int shamt_5 = (src2 >> 7) & 0x0000001F;
      int sh = (src2 >> 5) & 0x00000003;
      int Rm = src2 & 0x0000000F;
      int index = 0;
      int off = 0;

      
      switch(sh){
        case 0:
          index = CURRENT_STATE.REGS[Rm] << shamt_5;
          break;
        case 1:
          index = CURRENT_STATE.REGS[Rm] >> shamt_5;
          break;
        case 2:
          off = CURRENT_STATE.REGS[Rm];
          off = (signed int) off >> shamt_5;
          index = off;
          break;
        case 3:
          index = ((CURRENT_STATE.REGS[Rm] >> shamt_5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt_5)));
          break;
      }
      if(U == 1) data = CURRENT_STATE.REGS[Rn] + index;
      else data = CURRENT_STATE.REGS[Rn] - index;
      CURRENT_STATE.REGS[Rn] = data;

      return 0;
    }
  }
}


int MOV (int Rd, int SBZ, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rm] << shamt5;
          break;
        case 1: cur = CURRENT_STATE.REGS[Rm] >> shamt5;
          break;
        case 2: cur = CURRENT_STATE.REGS[Rm] >> shamt5;
          break;
        case 3: cur = ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur =  CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs];
          break;
        case 1: cur = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
          break;
        case 2: cur = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
          break;
        case 3: cur = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = Imm>>2*rotate|(Imm<<(32-2*rotate));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;
}

int MVN (int Rd, int SBZ, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = ~(CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = ~(CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = ~(CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = ~(((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5))));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = ~(CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = ~(CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = ~(CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = ~((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = ~(Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;
}

int ORR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] | (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] | (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] | (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] | ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] | (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] | (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] | (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] | ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] | (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;
}

int SBC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] - ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;
}

// int STR (int Rd, int Rn, int I, int P, int U, int W, int src2, int CC);
// int STRB (int Rd, int Rn, int I, int P, int U, int W, int src2, int CC);

int SUB (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] - ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] - ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;
}

int TEQ (int SBZ, int Rn, int Operand2, int I, int one, int CC)
{
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] ^ ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] ^ (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] ^ ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] ^ (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;

  return 0;
}

int TST (int SBZ, int Rn, int Operand2, int I, int one, int CC)
{
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] & ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }     
    else
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: cur = CURRENT_STATE.REGS[Rn] & ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (cur > 0xFFFFFFFF)
      NEXT_STATE.CPSR |= C_N;
    if ((signed int) cur > 0x7FFFFFFF || (signed int) cur < 0xFFFFFFFF)
      NEXT_STATE.CPSR |= V_N;
  
  return 0;
}

int SWI (char* i_){return 0;}

#endif