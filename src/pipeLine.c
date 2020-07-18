#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "../instruction.h"
#include "queue.h"
#include "../fileprocessor/fileProcess.h"
#include "queue2.h"
#include "pipeLine.h"


int REG[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int regSignal[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
CPU_Instruction* instructionMemory[999]; /* 1 - 1000 instruction */
int memory[16134]; /** 1000 - 65535 64KB*/

int pc = 0; /* program Counter*/
int numOfIns = 0; /* number of instruction */
int cycle = 1;
int end = 0;

/**branch**/
int branchPending = 0; /*0 not pending 1 pending*/
int is1Branch = 0;
int is2branch = 0;

// count
int controlHazard = 0;
int dataHazard = 0;
int structHazard = 0;
double numberOfInstruction = 0.0;
/**forward*/
int forwardA = 0;
int forwardB = 0;
int hazard = 0;
int wbForward[2] = {-2,-2};
/**exa**/
bool isFull = false;
bool isExBFull = false;
Queue *issueQueue;
WBQueue *wbQueue;

/** end*/
int siganl = 11; // 0 means end;

int main(int argc, char *argv[]) {
    char *buffer = (char*)malloc(100);

    printf("only support diversified pipeline\n");
    if (argc != 3) {
        printf(" please input 2 parameters");
    } else {
        strcpy(buffer,argv[2]);
    }


    openFile(buffer);
    init();
    while (1) {
        printf("cycle = %d--------------------------\n", cycle);

        for(int i = 0; i < 2; i++) {
            wb(i);
            printf("\n");
        }

        dispatchToEx();
        decode();
        fetch();
        cycle++;
        for (int i = 0; i < 16 ; ++i) {
            printf("register reg[%d] = %d  state:%d\n",i,REG[i], regSignal[i]);
        }
        if (siganl == 1) break;
    }
    printLog();

}

void fetch() {
    if (end) {
        printf("F    (P1): %s               | F    (P2): %s\n", instructionMemory[pc]->trace,
               instructionMemory[pc + 1]->trace);
    } else if (pc == numOfIns - 1 && !end) {
        printf("F    (P1):                | F    (P2):\n ");
    } else if(branchPending == 1 || hazard== 1) {
        if (pc >= numOfIns) {
            printf("F    (P1):                | F    (P2):\n ");
        } else {
            printf("F    (P1): %s               | F    (P2): %s\n", instructionMemory[pc]->trace,
                   instructionMemory[pc + 1]->trace);
        }
        if (branchPending == 1) {
            controlHazard++;
        }

    } else {
        // p1
        ifId1->destReg = instructionMemory[pc]->rd;
        ifId1->regData1 = instructionMemory[pc]->rs1;
        ifId1->regData2 = instructionMemory[pc]->rs2;
        ifId1->ins = instructionMemory[pc];
        ifId1->control = instructionMemory[pc]->opcode;
        ifId1->forward = 0;
        numberOfInstruction++;
        // p2
        ifId2->destReg = instructionMemory[pc+1]->rd;
        ifId2->regData1 = instructionMemory[pc+1]->rs1;
        ifId2->regData2 = instructionMemory[pc+1]->rs2;
        ifId2->ins = instructionMemory[pc+1];
        ifId2->control = instructionMemory[pc+1]->opcode;
        ifId2->forward = 0;
        printf("F    (P1): %s               | F    (P2): %s \n", instructionMemory[pc]->trace,
                                                                     instructionMemory[pc + 1]->trace);
        numberOfInstruction++;
    }
}

void decode() {
    if (!strcmp(ifId1->control, "ret") || end) {

        enqueue(issueQueue, ifId1);
        printf("DE   (P1): %s   \t| DE   (P2): %s \n", ifId1->ins->trace, "");
        if (!end) {
            pc += 2;
        }
        end = 1;
        return;
    }

    // ins1 and ins2  is nop and ins1, ins2 is null
    if (!strcmp(ifId1->control, "nop") && !strcmp(ifId2->control,"nop")) {
        printf("DE   (P1):    \t| DE   (P2):  \n");
        // there are no hazard
        hazard = 0;
        return;
    } else if (!strcmp(ifId1->control, "")){
        hazard = 0;
        return;
    }
    printf("DE   (P1): %s   \t| DE   (P2): %s \n", ifId1->ins->trace, ifId2->ins->trace);

    int signalOfHazard = hazardDectionUnit(ifId1,ifId2);
    //decode the instruction
    is1Branch = decodeInstruction(ifId1);
    is2branch = decodeInstruction(ifId2);
    if (is1Branch == 0) {
        // if ins1 is not branch instruction

        if(is2branch == 1) {
            if (signalOfHazard == 2) {
                // ins2 is branch instruction and ins1 have hazard
                // branchPending
                branchPending = 1;

            } else if(signalOfHazard == 1) {
                // ins2 is branch instruction and ins2 have hazard
                // set the branchpending = 1
                // issue ins1


                // ins1 != nop, then issue
                if (strcmp(ifId1->control, "nop")) {
                    enqueue(issueQueue, ifId1);
                    ifId1->control = "nop";
                    ifId1->destReg = -2;
                    ifId1->ins = malloc(sizeof(struct CPU_Instruction));
                }
                if(!hazard && !branchPending) {
                    pc+=2;
                }
                // pc+2 when first meet hazard, stall keep pc
                branchPending = 1;
                hazard = 1;
            } else if (signalOfHazard == 0){
                // ins2 is branch instruction and there are no hazard
                // squash ifId1 and ifId
                branchPending = 1;
                if (strcmp(ifId1->control, "nop")) {
                    enqueue(issueQueue, ifId1);
                    ifId1->control = "nop";
                    ifId1->destReg = -2;
                    ifId1->ins = malloc(sizeof(struct CPU_Instruction));
                }
                if (strcmp(ifId2->control, "nop")) {
                    enqueue(issueQueue, ifId2);
                    ifId2->control = "nop";
                    ifId2->destReg = -2;
                    ifId2->ins = malloc(sizeof(struct CPU_Instruction));
                }
                hazard == 0;
            }
        } else {
            // ins2 and ins1 is not branch instruction
            if (signalOfHazard == 2) {
                // ins1 have hazard
                // stall, !pc++
//                enqueue(issueQueue, ifId1);
//                enqueue(issueQueue, ifId2);
//                pc++;
                hazard == 1;
            } else if(signalOfHazard == 1) {
                // ins2 have hazard
                // stall ins2
                // issue ins1
                hazard = 1;
                // ins1 != nop, then issue
                if (strcmp(ifId1->control, "nop")) {
                    enqueue(issueQueue, ifId1);
                    ifId1->control = "nop";
                    ifId1->destReg = -2;
                    ifId1->ins = malloc(sizeof(struct CPU_Instruction));
                }
                //enqueue(issueQueue, ifId2);

            } else if (signalOfHazard == 0){
                // there are no hazard
                // squash ifId1 and ifId
                // set hazard signal = 0;
                hazard = 0;
                if (strcmp(ifId1->control, "nop")) {
                    enqueue(issueQueue, ifId1);
                }
                if (strcmp(ifId2->control, "nop")) {
                    enqueue(issueQueue, ifId2);
                }
                pc+=2;
            }


        }
    } else {
        // ins1 is branch instruction
        // pc = pc + 1;
        // ins2 = nop
        if (signalOfHazard == 0 || signalOfHazard == 2) {
            enqueue(issueQueue, ifId1);
            enqueue(issueQueue, ifId2);
            ifId2->control = "nop";
            ifId2->destReg = -2;
            ifId1->control = "nop";
            ifId1->destReg = -2;
            if (!branchPending) {
                pc+=2;
            }
            branchPending = 1;
        } else if(signalOfHazard == 1) {
            branchPending = 1;
        }
    }

}

int decodeInstruction(reg *ins) {
    if (!strcmp(ins->control, "set")) {
        // set imm to rs2, rs2 to data
        ins->regData2 = ins->ins->imm;
        regSignal[ins->destReg] = 0;
    } else if(!strcmp(ins->control, "sub") ||!strcmp(ins->control, "add")
               || !strcmp(ins->control, "mul") || !strcmp(ins->control, "div")){
        if(ins->regData2 == -1) {
            ins->regData2 = ins->ins->imm;
        }
        regSignal[ins->destReg] = 0;
    } else if(!strcmp(ins->control, "ret")) {
        printf("ret");
        return -1;
    } else if(!strcmp(ins->control, "st")) {

    } else if(!strcmp(ins->control,"bgez") || !strcmp(ins->control, "blez")
        || !strcmp(ins->control, "bgtz") || !strcmp(ins->control, "bltz")) {
        return 1;
    }

    return 0;


}


/**
 * check hazard and forward
 * @param ins1  instruction one
 * @param ins2  instruction two
 * @return  "0" means no hazard
 *          "1" means ins1 have hazard
 *          "2" means ins2 hava hazard
 */
int hazardDectionUnit(reg *instruction1, reg *instruction2) {
    CPU_Instruction *ins1;
    ins1 = instruction1->ins;

    int hazardType;
    if(ins1->rd == -1) return 0;
    if (ins1->rs1 == -1 && ins1->rs2 == -1
        ||(ins1->rs1 == -1 || ins1->rs2== -1)) {
       hazardType = oneOpcodeHazardCheck(instruction1, instruction2);
    } else {
       hazardType = threeOpcodeHazardCheck(instruction1, instruction2);
    }

    return hazardResolved(instruction1, instruction2, hazardType);
}


void dispatchToEx() {
    reg *ins1 = malloc(sizeof(reg));
    ins1->ins = malloc(sizeof(struct CPU_Instruction));
    ins1->control = (char *) malloc(sizeof(char));

    for(int i = 0; i < 2; i++) {
        dispatchToSpecificUnit(ins1);
    }
}

void dispatchToSpecificUnit(reg *ins1) {
    if (!queueEmpty(issueQueue)) {
        dequeue(issueQueue, ins1);

        if (!strcmp(ins1->control, "set") || !strcmp(ins1->control, "sub") ||!strcmp(ins1->control, "add") ) {
            exa1(ins1);
        } else if(!strcmp(ins1->control, "bgez") || !strcmp(ins1->control, "blez")
                  || !strcmp(ins1->ins->opcode, "ret") || !strcmp(ins1->control, "bgtz") || !strcmp(ins1->control, "bltz")) {
            con(ins1);
        } else if(!strcmp(ins1->control, "mul") || !strcmp(ins1->control, "div")) {
            exb2(ins1);
        }
    } else {
       // exb2(ins1);
        isFull = 0;
    }
}

void exb2(reg *instruction) {

    if (!strcmp(eXB1->ins->opcode, "")) {
        printf("EXb 2(P2):                   \t ");
    } else {
        printf("    Exb 2(P2): %s \n", eXB1->ins->trace);
        // check forward
        reg2 *exb1Result = malloc(sizeof(reg2));
        exb1Result->ins = malloc(sizeof(sizeof(CPU_Instruction)));
        exb1Result = eXB1;
        if (eXB1->forWardValue - eXB1->destReg == 0) {
            REG[eXB1->destReg] = eXB1->data;
            eXB2->forWardValue = eXB1->forWardValue;
        }
        enqueue1(wbQueue, exb1Result);
    }
    exb(instruction);

}

void exb(reg *instruction) {
    isExBFull = !isExBFull;
    reg2 *exb1Result = malloc(sizeof(reg2));
    exb1Result->ins = malloc(sizeof(sizeof(CPU_Instruction)));
    eXB1 = exb1Result;

    printf("    Exb   1(P1): %s \n", instruction->ins->trace);

    // cacluate the instruction
    int result;
    if (!strcmp(instruction->ins->opcode, "mul")) {
        if (instruction->ins->is_imm) {
            exb1Result->data = REG[instruction->regData1] * instruction->regData2;
        } else {
            exb1Result->data = REG[instruction->regData1] * REG[instruction->regData2];
        }
        exb1Result->destReg = instruction->destReg;
        exb1Result->regWr = 1;
        exb1Result->memWr = 0;
        exb1Result->memRd = 0;
        exb1Result->ins = instruction->ins;
        // set forward result
        result = exb1Result->data;

    } else if (!strcmp(instruction->ins->opcode, "div")) {
        exb1Result->data = REG[instruction->regData1] / instruction->regData2;
        exb1Result->data = REG[instruction->regData1] / REG[instruction->regData2];
        exb1Result->regWr = 1;
        exb1Result->memRd = 0;
        exb1Result->memWr = 0;
        exb1Result->ins = instruction->ins;
        result = exb1Result->data;
    }

    //enqueue1(wbQueue, exb1Result);
    //check forward
    if (instruction->forward) {
        //REG[instruction->destReg] = result;
        exb1Result->forWardValue = exb1Result->destReg;
    }
}

void con(reg *ins) {
    reg2 *conResult = malloc(sizeof(reg2));
    conResult->ins = malloc(sizeof(sizeof(CPU_Instruction)));
    exaR1 = conResult;
    if (!strcmp(ins->ins->opcode, "bgez")) {
        if (REG[ins->ins->rd] >= 0) {
            pc = ins->ins->imm / 4;
        }
        branchPending = 0;
    } else if(!strcmp(ins->ins->opcode, "blez")) {
        if (REG[ins->ins->rd] <= 0) {
            pc = ins->ins->imm / 4;
        }
        branchPending = 0;
    } else if(!strcmp(ins->ins->opcode, "ret")) {
        // end signal
        ifId1->ins = malloc(sizeof(CPU_Instruction));
        end = 1;
    }

    conResult->ins = ins->ins;
    conResult->forWardValue = -2;
    conResult->destReg = -2;
    conResult->data = -2;
    conResult->memWr = 0;
    conResult->regWr = 0;
    conResult->memRd = 0;
    enqueue1(wbQueue, conResult);
    printf("    Cond    (P1): %s \n", ins->ins->trace);
}

void exa1(reg *instruction) {
    reg2 *exa1Result = malloc(sizeof(reg2));
    exa1Result->ins = malloc(sizeof(sizeof(CPU_Instruction)));
    exaR1 = exa1Result;

    //check exa1 is Full
    if(isFull) {
        exa2(instruction);
        //isFull = !isFull;
        return;
    }
    printf("    Exa    (P1): %s \n", instruction->ins->trace);

    isFull = !isFull;
    // cacluate the instruction
    int result;
    if (!strcmp(instruction->ins->opcode, "sub")) {
        if (instruction->ins->is_imm) {
            exa1Result->data = REG[instruction->regData1] - instruction->regData2;
        } else {
            exa1Result->data = REG[instruction->regData1] - REG[instruction->regData2];
        }
        exa1Result->destReg = instruction->destReg;
        exa1Result->regWr = 1;
        exa1Result->memWr = 0;
        exa1Result->memRd = 0;
        exa1Result->ins = instruction->ins;
        // set forward result
        result = exa1Result->data;

    }

    if (!strcmp(instruction->ins->opcode, "set")) {
        exa1Result->data = instruction->regData2;
        exa1Result->destReg = instruction->destReg;
        exa1Result->regWr = 1;
        exa1Result->memRd = 0;
        exa1Result->memWr = 0;
        exa1Result->ins = instruction->ins;
        result = exa1Result->data;
    }else if(!strcmp(instruction->ins->opcode, "add")) {
        if (instruction->ins->is_imm) {
            exa1Result->data = REG[instruction->regData1] + instruction->regData2;
        } else {
            exa1Result->data = REG[instruction->regData1] + REG[instruction->regData2];
        }


        exa1Result->destReg = instruction->destReg;
        exa1Result->regWr = 1;
        exa1Result->memRd = 0;
        exa1Result->memWr = 0;
        exa1Result->ins = instruction->ins;
        result = exa1Result->data;
    } else {
    }

    enqueue1(wbQueue, exa1Result);
    //check forward
    if (instruction->forward) {
        REG[instruction->destReg] = result;
    }
    exa1Result->forWardValue = instruction->destReg;
}

void exa2(reg *instruction) {
    isFull = !isFull;
    // add to wb queue
    reg2 *exa2Result = malloc(sizeof(reg2));
    exa2Result->ins = malloc(sizeof(sizeof(CPU_Instruction)));
    exaR2 = exa2Result;
    CPU_Instruction *ins;
    ins = instruction->ins;

    printf("    Exa    (p2): %s \n", ins->trace);

    // cacluate the instruction
    int result;
    if (!strcmp(instruction->ins->opcode, "sub")) {
        if (instruction->ins->is_imm) {
            exa2Result->data = REG[instruction->regData1] - instruction->regData2;
        } else {
            exa2Result->data = REG[instruction->regData1] - REG[instruction->regData2];
        }
        exa2Result->destReg = instruction->destReg;
        exa2Result->regWr = 1;
        exa2Result->memWr = 0;
        exa2Result->memRd = 0;
        exa2Result->ins = instruction->ins;
        // set forward result
        result = exa2Result->data;

    }

    if (!strcmp(instruction->ins->opcode, "set")) {
        exa2Result->data = instruction->regData2;
        exa2Result->destReg = instruction->destReg;
        exa2Result->regWr = 1;
        exa2Result->memRd = 0;
        exa2Result->memWr = 0;
        exa2Result->ins = instruction->ins;
        result = exa2Result->data;
    }else if(!strcmp(instruction->ins->opcode, "add")) {
        if (instruction->ins->is_imm) {
            exa2Result->data = REG[instruction->regData1] + instruction->regData2;
        } else {
            exa2Result->data = REG[instruction->regData1] + REG[instruction->regData2];
        }
        exa2Result->destReg = instruction->destReg;
        exa2Result->regWr = 1;
        exa2Result->memRd = 0;
        exa2Result->memWr = 0;
        exa2Result->ins = instruction->ins;
        result = exa2Result->data;
    } else {
    }

    enqueue1(wbQueue, exa2Result);
    //check forward
    if (instruction->forward) {
        REG[instruction->destReg] = result;
        //forwardA = instruction->destReg;
    }
    exaR2->forWardValue = instruction->destReg;

}

// get result from WbQueue
void wb(int i) {
    // wbQueue is not null
    if(queueEmpty1(wbQueue)) return;
    reg2 *r = malloc(sizeof(struct reg2));
    r->ins = malloc(sizeof(CPU_Instruction));
    dequeue1(wbQueue, r);
    printf("WB | %s             \t", r->ins->trace);
    if (!strcmp(r->ins->opcode, "ret")) {
        siganl = 1;
        return;
    }
    if (r->regWr) {
        REG[r->destReg] = r->data;
        regSignal[r->destReg] = 1;
        wbForward[i] = r->destReg;
    }
}
/* init the queue and the decode set*/
void init() {
    issueQueue = malloc(sizeof(Queue));
    queueInit(issueQueue, sizeof(reg));

    // store the instruction in execution stage
    wbQueue = malloc(sizeof(WBQueue));
    queueInit1(wbQueue, sizeof(struct reg2));

    // init idEx register
    idEx = malloc(sizeof(reg1));
    idEx->control = (char *) malloc(sizeof(char));
    idEx->ins = malloc((sizeof(struct CPU_Instruction)));
    idEx2 = malloc(sizeof(reg1));
    idEx2->control = (char *) malloc(sizeof(char));
    idEx2->ins = malloc((sizeof(struct CPU_Instruction)));

    // init ifId register
    ifId1 = malloc(sizeof(reg1));
    ifId1->control = (char *) malloc(sizeof(char));
    ifId1->ins = malloc((sizeof(struct CPU_Instruction)));
    ifId2 = malloc(sizeof(reg1));
    ifId2->control = (char *) malloc(sizeof(char));
    ifId2->ins = malloc((sizeof(struct CPU_Instruction)));

    // init register after exa exb mem con
    exaR1 = malloc(sizeof(reg2));
    exaR1->ins = malloc((sizeof(struct CPU_Instruction)));
    exaR2 = malloc(sizeof(reg2));
    exaR2->ins = malloc((sizeof(struct CPU_Instruction)));
    memReg1 = malloc(sizeof(reg2));
    memReg1->ins = malloc((sizeof(struct CPU_Instruction)));
    memReg2 = malloc(sizeof(reg2));
    memReg2->ins = malloc((sizeof(struct CPU_Instruction)));
    memReg3 = malloc(sizeof(reg2));
    memReg3->ins = malloc((sizeof(struct CPU_Instruction)));
    memReg4 = malloc(sizeof(reg2));
    memReg4->ins =  malloc((sizeof(struct CPU_Instruction)));
    eXB1 = malloc(sizeof(reg2));
    eXB1->ins = malloc((sizeof(struct CPU_Instruction)));
    eXB2 = malloc(sizeof(reg2));
    eXB2->ins = malloc((sizeof(struct CPU_Instruction)));
    conR = malloc(sizeof(reg2));
    conR->ins = malloc((sizeof(struct CPU_Instruction)));
    int i = 0;
    while (1) {
        char *buffer = (char *) malloc(10000);
        char *instruction = poll();
        //instruction = poll();
        if (instruction == NULL) break;
        strcpy(buffer, instruction);
        CPU_Instruction *ins = (CPU_Instruction *) malloc(sizeof(CPU_Instruction));
        create_CPU_instruction(ins, buffer);
        instructionMemory[i] = ins;
        i++;
    }
    numOfIns = i;

    close();
}

/*
 * return 1 means ins1 have hazard
 * return 0 means no hazard
 * return 2 means ins2 have hazard
 * return 3 means ins1 and ins2 have hazard
 */
int threeOpcodeHazardCheck(reg *ins1, reg *ins2) {
    int rs1,rs2,rd;
    rs1 = regSignal[ins2->ins->rs1];
    rs2 = regSignal[ins2->ins->rs2];
    rd = regSignal[ins2->ins->rd];

    if (ins2->ins->rs1 == -1) rs1 = 1;
    if (ins2->ins->rs2 == -1) rs2 = 1;
    if (!regSignal[ins1->ins->rd] || !regSignal[ins1->ins->rs1] || !regSignal[ins1->ins->rs2]) {
        //ins 1 have hazard
        if (!rd || !rs1 || !rs2
        || ins2->ins->rs1 == ins1->ins->rd || ins2->ins->rs2 == ins1->ins->rd || ins2->ins->rd == ins1->ins->rd) {
                //ins 2 and ins2 have hazard
                // return 3;
                return 3;
        } else {
            // ins 1 have hazard ins2 no hazard
            return 1;
        }
    } else {
        // ins1 no hazard
        // check ins2
        if (!rd || !rs1 || !rs2
        ||ins2->ins->rs1 == ins1->ins->rd || ins2->ins->rs2 == ins1->ins->rd || ins2->ins->rd == ins1->ins->rd) {
                //ins 2 and ins2 have hazard
                // return 2;
                return 2;
        } else {
            // ins1 and ins2 no hazard
            return 0;
        }

    }
}
int oneOpcodeHazardCheck(reg *ins1, reg *ins2) {
    int rs1,rs2,rd;
    rs1 = regSignal[ins2->ins->rs1];
    rs2 = regSignal[ins2->ins->rs2];
    rd = regSignal[ins2->ins->rd];
    if (ins2->ins->rs1 == -1) rs1 = 1;
    if (ins2->ins->rs2 == -1) rs2 = 1;
    if ((ins2->ins->rs1 == -1 && ins2->ins->rs2 == -1) || (ins1->ins->rs1 == -1 && ins1->ins->rs2 == -1)) {
        if (!regSignal[ins1->ins->rd]) {
            //ins 1 have hazard
            if (!rd || !rs1 || !rs2 || ins2->ins->rd == ins1->ins->rd
                || ins2->ins->rs1 == ins1->ins->rd || ins2->ins->rs2 == ins1->ins->rd) {
                //ins 1 and ins2 have hazard
                // return 3;
                return 3;
            } else {
                // ins 1 have hazard ins2 no hazard
                return 1;
            }
        } else {
            // ins1 no hazard
            // check ins2
            if (!rd || !rs1 || !rs2 || ins2->ins->rd == ins1->ins->rd
                || ins2->ins->rs1 == ins1->ins->rd || ins2->ins->rs2 == ins1->ins->rd) {
                //ins 2 and ins2 have hazard
                // return 2;
                return 2;
            } else {
                // ins1 and ins2 no hazard
                return 0;
            }
        }
    } else {
        if (!regSignal[ins1->ins->rd]) {
            //ins 1 have hazard
            if (!rd || !rs1 || !rs2
                ||ins2->ins->rd == ins1->ins->rd || ins2->ins->rs1 == ins1->ins->rd || ins2->ins->rs2 == ins1->ins->rd) {
                //ins 2 and ins2 have hazard
                // return 3;
                return 3;
            } else {
                // ins 1 have hazard ins2 no hazard
                return 1;
            }
        } else {
            // ins1 no hazard
            // check ins2
            if (!rd || !rs1 || !rs2
                || ins2->ins->rs1 == ins1->ins->rd || ins2->ins->rs2 == ins1->ins->rd
        ||ins2->ins->rd == ins1->ins->rd) {
                //ins 2 and ins2 have hazard
                // return 2;
                return 2;
            } else {
                // ins1 and ins2 no hazard
                return 0;
            }
        }
    }


}

/**
 *
 * @param ins instruction 1
 * @param ins2  ins2
 * @param needToCheck  which hazard need to solved
 * @return 0 2 ins hazard solved
 * @return 1 ins1 solved, ins2 not solved
 * @return 2 ins1 not solved
 *
 */
int hazardResolved(reg *ins, reg *ins2, int needToCheck) {
    // check forward for ins1
    if (needToCheck == 0) {
        return 0;
    } else if (needToCheck == 1) {
        // ins1 have hazard, ins2 no hazard
        if (exaR1->forWardValue == ins->ins->rd || exaR1->forWardValue == ins->ins->rs1
        || exaR1->forWardValue == ins->ins->rs2 || exaR2->forWardValue == ins->ins->rd
        || exaR2->forWardValue == ins->ins->rs1 || exaR2->forWardValue == ins->ins->rs2) {
            ins->forward = 0;
            exaR1->forWardValue = -2;
            exaR2->forWardValue = -2;
            return 0;
        } else {
            // in1 hazard not solved
            ins->forward = 1;
            return 2;
        }
    } else if (needToCheck == 2) {
        // ins1 no hazard, ins2 have hazard
        if (exaR1->forWardValue == ins2->ins->rd || exaR1->forWardValue == ins2->ins->rs1
            || exaR1->forWardValue == ins2->ins->rs2 || exaR2->forWardValue == ins2->ins->rd
            || exaR2->forWardValue == ins2->ins->rs1 || exaR2->forWardValue == ins2->ins->rs2) {
            ins2->forward = 0;
            exaR1->forWardValue = -2;
            exaR2->forWardValue = -2;
            return 0;
        } else {
            // in2 hazard not solved
            ins->forward = 1;
            return 1;
        }

    } else if (needToCheck == 3) {
        // ins1 and ins2 have hazard
        if (exaR1->forWardValue == ins->ins->rd || exaR1->forWardValue == ins->ins->rs1
            || exaR1->forWardValue == ins->ins->rs2 || exaR2->forWardValue == ins->ins->rd
            || exaR2->forWardValue == ins->ins->rs1 || exaR2->forWardValue == ins->ins->rs2) {
            // ins1 solved
            ins->forward = 0;
            if (exaR1->forWardValue == ins2->ins->rd || exaR1->forWardValue == ins2->ins->rs1
                || exaR1->forWardValue == ins2->ins->rs2 || exaR2->forWardValue == ins2->ins->rd
                || exaR2->forWardValue == ins2->ins->rs1 || exaR2->forWardValue == ins2->ins->rs2) {
                // ins1 and ins2 solved
                ins2->forward = 0;
                exaR1->forWardValue = -2;
                exaR2->forWardValue = -2;
                return 0;
            } else {
                // ins 2 not solved
                ins2->forward = 1;
                return 1;
            }
        } else {
            // ins1 not solved, ins2 hazard not solved
            ins->forward = 1;
            if (exaR1->forWardValue == ins2->ins->rd || exaR1->forWardValue == ins2->ins->rs1
                || exaR1->forWardValue == ins2->ins->rs2 || exaR2->forWardValue == ins2->ins->rd
                || exaR2->forWardValue == ins2->ins->rs1 || exaR2->forWardValue == ins2->ins->rs2) {
                //  ins1 not solved, ins2 solved
                ins2->forward = 0;
                exaR1->forWardValue = -2;
                exaR2->forWardValue = -2;
                return 2;
            } else {
                // ins1 ins2 not solved
                ins2->forward = 1;
                return 2;
            }
        }
    }
}

void  printLog() {
    printf("> Diversified Pipeline < ");
    for (int i = 0; i < 16 ; ++i) {
        printf("R%d: %d\n",i, REG[i]);
    }
    printf("CPU) >> Simulation Complete\n");
    printf( "(A)Pipeline stalls due to data hazard: %d\n", dataHazard);
    printf("(B)Wasted cycles due to control hazard: %d\n", controlHazard);
    printf("(C)Pipeline stalls due to structural hazard:  %d\n", structHazard);
    printf("(A + B + C) Total pipeline stalls:  %d\n", dataHazard+controlHazard+structHazard);

    printf("Total cycles: %d", cycle);
    printf("IPC: %f", numberOfInstruction / cycle);

}
