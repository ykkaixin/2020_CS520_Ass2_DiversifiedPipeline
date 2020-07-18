//
// Created by kai on 4/10/20.
//

#ifndef PIPELINE_PIPELINE_H
#define PIPELINE_PIPELINE_H

#include "../instruction.h"

// WB forward
typedef struct
{
    char *control;
    int regData1;
    int regData2;
    int destReg;
    CPU_Instruction *ins;
    int forward;
} reg;
reg *ifId1, *ifId2;

// Latch between ID & EX stages
typedef struct
{
    char *control;
    int regData1;
    int regData2;
    int destReg;
    int memRd;
    int memWr;
    int regWr;
    int forward; /* 1 forward 0 not forward */
    CPU_Instruction *ins;
} reg1;
reg1 *idEx,*idEx2;

// Latch between EX & MEM stages
typedef struct reg2
{
    int data;
    int destReg;
    int memRd;
    int memWr;
    int regWr;
    int forWardValue;
    CPU_Instruction *ins;
} reg2;
reg2 *exaR1,*exaR2, *memReg1, *memReg2, *memReg3, *memReg4,*eXB1, *eXB2, *conR;

// latch between MEM & WB stages
typedef struct {
    int data;
    int destReg;
    int regWr;
    int forward;
    CPU_Instruction *ins;
} reg3;
reg3 *memWb, exaWb, exa2Wb, exbWb;

//store wb last value
CPU_Instruction *wbIns;
CPU_Instruction *halt;
void init();
void fetch();
void decode();
void con(reg *ins);
void exa1(reg *ins); /*Execution stage*/
void exa2(reg *ins);
void exb(reg *ins); /*multiplication and division*/
void exb2(reg *instruction);
void mem();
void mem2();
void mem3();
void mem4();
void wb(int i);
void printLog();

//util function
void dispatchToEx();
int decodeInstruction(reg *ins);
void decodeIns1(CPU_Instruction *ins);
void decodeIns2(CPU_Instruction *ins);
int hazardDectionUnit(reg *ins1, reg *ins2); /**@return 1 ins1 have hazard
                                                @return 2 ins2 have hazard
                                                @return 0 no hazard * **/
int threeOpcodeHazardCheck(reg *ins1, reg *ins2);
int oneOpcodeHazardCheck(reg *ins1, reg *ins2);
int hazardResolved(reg *ins, reg *ins2, int needToCheck);
void dispatchToSpecificUnit(reg *ins);
#endif //PIPELINE_PIPELINE_H
