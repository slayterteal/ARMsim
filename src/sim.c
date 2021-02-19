/***************************************************************/
/*                                                             */
/*   ARMv8-32 Instruction Level Simulator                      */
/*                                                             */
/*   ECEN 4243                                                 */
/*   Oklahoma State University                                 */
/*   Slayter Teal and Daniel Albrecht                          */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "isa.h"


char *byte_to_binary12 (int x) {

  static char b[13];
  b[0] = '\0';

  int z;
  for (z = 2048; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

char *byte_to_binary4 (int x) {

  static char b[5];
  b[0] = '\0';

  int z;
  for (z = 8; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

char *byte_to_binary32(int x) {

  static char b[33];
  b[0] = '\0';

  unsigned int z;
  for (z = 2147483648; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

int bchar_to_int(char* rsa) {

  int i = 0;
  int result = 0;
  int t = 0;
  while(rsa[i] != '\0')i++;
  while(i>0)
    {
      --i;
      // printf("%d\n", (rsa[i]-'0')<<t);
      result += (rsa[i] - '0')<<t;
      t++;
    }
  return result;
}

int SBZ = 0;

int data_process(char* i_) {

  /*
    This function further decode and execute subset of data processing 
    instructions of ARM ISA.

    0000 = AND - Rd:= Op1 AND Op2
    0001 = EOR - Rd:= Op1 EOR Op2
    0010 = SUB - Rd:= Op1 - Op2
    0011 = RSB - Rd:= Op2 - Op1
    0100 = ADD - Rd:= Op1 + Op2
    0101 = ADC - Rd:= Op1 + Op2 + C
    0110 = SBC - Rd:= Op1 - Op2 + C - 1
    0111 = RSC - Rd:= Op2 - Op1 + C - 1
    1000 = TST - set condition codes on Op1 AND Op2 
    1001 = TEQ - set condition codes on Op1 EOR Op2 
    1010 = CMP - set condition codes on Op1 - Op2 
    1011 = CMN - set condition codes on Op1 + Op2 
    1100 = ORR - Rd:= Op1 OR Op2
    1101 = MOV - Rd:= Op2
    1110 = BIC - Rd:= Op1 AND NOT Op2 
    1111 = MVN - Rd:= NOT Op2
  */

  char d_opcode[5]; // get the opcode of the instruction
  d_opcode[0] = i_[7]; 
  d_opcode[1] = i_[8]; 
  d_opcode[2] = i_[9]; 
  d_opcode[3] = i_[10]; 
  d_opcode[4] = '\0'; //ending the string array
  char d_cond[5]; // get the condition codes
  d_cond[0] = i_[0]; 
  d_cond[1] = i_[1]; 
  d_cond[2] = i_[2]; 
  d_cond[3] = i_[3]; 
  d_cond[4] = '\0';
  char rn[5]; rn[4] = '\0';
  char rd[5]; rd[4] = '\0';
  char operand2[13]; operand2[12] = '\0';
  for(int i = 0; i < 4; i++) {
    rn[i] = i_[12+i];
    rd[i] = i_[16+i];
  }
  for(int i = 0; i < 12; i++) {
    operand2[i] = i_[20+i];
  }

  /*
    Convert all of the data (gathered above) into a 
    workable format for the C code. 
  */
  int Rn = bchar_to_int(rn);
  int Rd = bchar_to_int(rd);
  int Operand2 = bchar_to_int(operand2);
  int I = i_[6]-'0';
  int S = i_[11]-'0';
  int CC = bchar_to_int(d_cond);
  printf("Opcode = %s\n Rn = %d\n Rd = %d\n Operand2 = %s\n I = %d\n S = %d\n COND = %s\n", d_opcode, Rn, Rd, byte_to_binary12(Operand2), I, S, byte_to_binary4(CC));
  printf("\n");

  /* Example - use and replicate */
  // ADD
  if(!strcmp(d_opcode,"0100")) {
    printf("--- This is an ADD instruction. \n");
    ADD(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }	
  /* Add other data instructions here */ 
  // AND
  if(!strcmp(d_opcode,"0000")) {
    printf("--- This is an AND instruction. \n");
    AND(Rd, Rn, Operand2, I, S);
    return 0;
  }	
  // ADC
  if(!strcmp(d_opcode,"0101")) {
    printf("--- This is an ADC instruction. \n");
    ADC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }	
  // BIC
  if(!strcmp(d_opcode,"1110")) {
    printf("--- This is an BIC instruction. \n");
    BIC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }	
  // CMN Compare Negative
  if(!strcmp(d_opcode,"1011")) {
    printf("--- This is an CMN (Compare Negative) instruction. \n");
    CMN(SBZ, Rn, Operand2, I, 1, CC);
    return 0;
  }	
  // CMP Compare Positive
  if(!strcmp(d_opcode,"1010")) {
    printf("--- This is an CMP (Compare Positive) instruction. \n");
    CMP(SBZ, Rn, Operand2, I, 1, CC);
    return 0;
  }	
  // EOR
  if(!strcmp(d_opcode,"0001")) {
    printf("--- This is an EOR instruction. \n");
    EOR(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }	
  // Shift Instructions, assuming Imm
  if(!strcmp(d_opcode,"1101")) {
    if(I==1)
    {
      printf("--- This is an MOV instruction. \n");
      MOV(Rd, SBZ, Operand2, I, S, CC);
    }

    int f = operand2[7] -'0';
    int s = operand2[6] -'0';
    int h = operand2[5] -'0';
    int se = operand2[4] -'0';
    // else if(I == 0 && !strcmp(sh, "00"))
    // {
    //   printf("--- This is a LSL instruction. \n");
    //   LSL(Rd, SBZ, Operand2, I, S, CC);
    // }
    // else if(I == 0 && !strcmp(sh, "01"))
    // {
    //   printf("--- This is a LSR instruction. \n");
    //   LSR(Rd, SBZ, Operand2, I, S, CC);
    // }
    if(I == 0 && h == 1 && s == 0)
    {
      printf("--- This is a ASR instruction. \n");
      if(f == 0) I == 1;
      ASR(Rd, SBZ, Operand2, I, S, CC);
    }
    else if(I == 0 && h == 1 && s == 1)
    {
      printf("--- This is a ROR instruction. \n");
      if(f == 0) I == 1;
      ROR(Rd, SBZ, Operand2, I, S, CC);
    }
    return 0;
  }	
  // MVN
  if(!strcmp(d_opcode,"1111")) {
    printf("--- This is an MVN (Move Not) instruction. \n");
    MVN(Rd, SBZ, Operand2, I, S, CC);
    return 0;
  }	
  // ORR
  if(!strcmp(d_opcode,"1100")) {
    printf("--- This is an ORR instruction. \n");
    ORR(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }	
  // RSB
  if(!strcmp(d_opcode,"0011")) {
    printf("--- This is an RSB instruction. \n");
    RSB(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }	
  // RSC
  if(!strcmp(d_opcode,"0111")) {
    printf("--- This is an RSC instruction. \n");
    RSC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }
  // SBC
  if(!strcmp(d_opcode,"0110")) {
    printf("--- This is an SBC (Subtract with Carry) instruction. \n");
    SBC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }
  // SUB
  if(!strcmp(d_opcode,"0010")) {
    printf("--- This is an SUB instruction. \n");
    SUB(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }
  // TEQ (Test Equivalence)
  if(!strcmp(d_opcode,"1001")) {
    printf("--- This is an TEQ (Test Equivalence) instruction. \n");
    TEQ(SBZ, Rn, Operand2, I, 1, CC);
    return 0;
  }	
  // TST
  if(!strcmp(d_opcode,"1000")) {
    printf("--- This is an TST (Test) instruction. \n");
    TST(SBZ, Rn, Operand2, I, 1, CC);
    return 0;
  }	

  return 1;	
}

int branch_process(char* i_) {
  /* This function execute branch instruction */

  char L[2];
  L[0] = i_[7];
  L[1] = '\0';
  
  // get the condition codes
  char d_cond[5]; 
  d_cond[0] = i_[0]; 
  d_cond[1] = i_[1]; 
  d_cond[2] = i_[2]; 
  d_cond[3] = i_[3]; 
  d_cond[4] = '\0';

  char offset[25]; offset[24] = '\0';
  for(int i = 0; i < 24; i++)
  {
    offset[i] = i_[8+i];
  }

  // conversion logic
  int L2 = bchar_to_int(L);
  int offset2 = bchar_to_int(offset);
  int CC = bchar_to_int(d_cond);
  print("L = " + L2 + "\n" + "Offset = " + offset2 + "\n" + "CC: " + byte_to_binary4(CC));
  
  /* Add branch instructions here */ 
  // B
  if(L2 == 0) {
    printf("--- This is an B (Branch) instruction. \n");
    B(offset2, CC);
    return 0;
  }	
  // BL 
  if(L2 == 1) {
    printf("--- This is an BL (Branch with Link Register) instruction. \n");
    BL(offset2, CC);
    return 0;
  }	
  return 1;
}

// not required
int mul_process(char* i_) {

  /* This function execute multiply instruction */

  /* Add multiply instructions here */ 

  return 1;
}

// load/store instructions
int transfer_process(char* i_) {
  /* This function execute memory instruction */
  // get the condition codes
  char d_cond[5]; 
  d_cond[0] = i_[0]; 
  d_cond[1] = i_[1]; 
  d_cond[2] = i_[2]; 
  d_cond[3] = i_[3]; 
  d_cond[4] = '\0';

  /* 
    Gets the I, P, U, B values.
    These 4 variables define the addressing modes.
    L, and B are used here.
    P, W are used in the isa.h.
    I don't know what the U is so we'll just ignore it...
  */
  char i[2]; i[0] = i_[6]; i[1] = '\0';
  char p[2]; p[0] = i_[7]; p[1] = '\0';
  char u[2]; u[0] = i_[8]; u[1] = '\0';
  char w[2]; w[0] = i_[10]; w[1] = '\0';

  // get the L and B ints
  char l[2]; l[0] = i_[11]; l[1] = '\0';
  char b[2]; b[0] = i_[9]; b[1] = '\0';

  // the meaning of this changes depending on the addressing mode
  // I will send this to the isa.h as a char array
  char src2[13]; src2[12] = '0'; 
  // rn and rd are our register files
  char rn[5]; rn[4] = '\0';
  char rd[5]; rd[4] = '\0';
  for(int i = 0; i < 4; i++) {
    rn[i] = i_[12+i];
    rd[i] = i_[16+i];
  }
  for(int i = 0; i < 12; i++) {
    src2[i] = i_[20+i];
  }

  int CC = bchar_to_int(d_cond);
  int L = bchar_to_int(l);
  int B = bchar_to_int(b);
  int P = bchar_to_int(p);
  int W = bchar_to_int(w);
  int Rn = bchar_to_int(rn);
  int Rd = bchar_to_int(rd);

  /* Add memory instructions here */ 
  // STR
  if(L == 0 && B == 0) {
    printf("--- This is an STR instruction. \n");
    STR(Rd, Rn, p, w, src2, CC);
    return 0;
  }
  //STRB
  if(L == 0 && B == 1) {
    printf("--- This is an STRB instruction. \n");
    STRB(Rd, Rn, p, w, src2, CC);
    return 0;
  }
  // LDR
  if(L == 1 && B == 0) {
    printf("--- This is an LDR instruction. \n");
    LDR(Rd, Rn, p, w, src2, CC);
    return 0;
  }
  // LDRB
  if(L == 1 && B == 1) {
    printf("--- This is an LDR instruction. \n");
    LDR(Rd, Rn, p, w, src2, CC);
    return 0;
  }

  /*
    As a general note, the 'type' of addressing mode
    (preindex, offset, or postindex) needs to be determined in the
    isa.h. The lec_isa_pt2.pptx should help.
  */
  return 1;
}

int interruption_process(char* i_) {
  SWI(i_);
  RUN_BIT = 0;
  return 0;
}

unsigned int COND(unsigned int i_word) {

  return (i_word>>28);

}

unsigned int OPCODE(unsigned int i_word) {

  return ((i_word<<7)>>27);

}


int decode_and_execute(char* i_) {

  /* 
     This function decode the instruction and update 
     CPU_State (NEXT_STATE)
  */

  if((i_[4] == '1') && (i_[5] == '0') && (i_[6] == '1')) {
    printf("- This is a Branch Instruction. \n");
    branch_process(i_);
  }
  if((i_[4] == '0') && (i_[5] == '0') && (i_[6] == '0') && (i_[7] == '0') && (i_[24] == '1') && (i_[25] == '0') && (i_[26] == '0') && (i_[27] == '1')) {
    printf("- This is a Multiply Instruction. \n");
    mul_process(i_);
  }
  else {
    printf("- This is a Data Processing Instruction. \n");
    data_process(i_);
  }
  if((i_[4] == '0') && (i_[5] == '1')) {
    printf("- This is a Single Data Transfer Instruction. \n");
    transfer_process(i_);
  }
  // Calls the SWI() function
  if((i_[4] == '1') && (i_[5] == '1') && (i_[6] == '1') && (i_[7] == '1')) {
    printf("- This is a Software Interruption Instruction. \n");
    interruption_process(i_);
  }
  return 0;

}

void process_instruction() {

  /* 
     execute one instruction here. You should use CURRENT_STATE and modify
     values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     access memory. 
  */   

  unsigned int inst_word = mem_read_32(CURRENT_STATE.PC);
  printf("The instruction is: %x \n", inst_word);
  printf("33222222222211111111110000000000\n");
  printf("10987654321098765432109876543210\n");
  printf("--------------------------------\n");
  printf("%s \n", byte_to_binary32(inst_word));
  printf("\n");
  decode_and_execute(byte_to_binary32(inst_word));

  NEXT_STATE.PC += 4;

}
