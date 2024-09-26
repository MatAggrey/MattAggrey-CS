#include <stdio.h>
#include <stdlib.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define MOVL 2
#define MOVS 3
#define JE 4
#define CMP 5
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000
#define BTBSIZE 4 // Branch Target Buffer size
#define PHBSIZE 4 // Pattern History Buffer size

typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct {
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
    int fetched;
    int retired;
    int branches;
    int mispred;
    int phb[PHBSIZE]; // Pattern History Buffer for branch prediction
    int btbPC[BTBSIZE]; // Branch Target Buffer PC tags
    int btbTarget[BTBSIZE]; // Branch Target Buffer targets
    int btbValid[BTBSIZE]; // BTB Valid bits
    int btbIndex; // BTB replacement index (FIFO)
} stateType;

/* Function prototypes */
void printState(stateType *state);
int opcode(int instr);
void run(stateType *state);
int branchPrediction(stateType *state, int pc);
void updateBranchPrediction(stateType *state, int pc, int taken);

/* --------------------- Utility Functions --------------------- */

/* Extract opcode from instruction */
int opcode(int instr) {
    return (instr >> 22) & 0x7;
}

/* Utility function to print machine state */
void printState(stateType *state) {
    printf("\n@@@\nstate before cycle %d starts\n", state->cycles);
    printf("\tpc %d\n", state->pc);
    printf("\tdata memory:\n");
    for (int i = 0; i < state->numMemory; i++) {
        printf("\t\tdataMem[%d] %d\n", i, state->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (int i = 0; i < NUMREGS; i++) {
        printf("\t\treg[%d] %d\n", i, state->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction %d\n", state->IFID.instr);
    printf("\t\tpcPlus1 %d\n", state->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction %d\n", state->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", state->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", state->IDEX.readRegA);
    printf("\t\treadRegB %d\n", state->IDEX.readRegB);
    printf("\t\toffset %d\n", state->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction %d\n", state->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", state->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", state->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", state->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction %d\n", state->MEMWB.instr);
    printf("\t\twriteData %d\n", state->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction %d\n", state->WBEND.instr);
    printf("\t\twriteData %d\n", state->WBEND.writeData);
    printf("total of %d cycles executed\n", state->cycles);
}

/* Initialize the branch predictor (PHB and BTB) */
void initBranchPredictor(stateType *state) {
    for (int i = 0; i < PHBSIZE; i++) {
        state->phb[i] = 1; // Weakly not taken
    }
    for (int i = 0; i < BTBSIZE; i++) {
        state->btbPC[i] = -1; // Invalid PC
        state->btbValid[i] = 0;
    }
    state->btbIndex = 0; // Start BTB index at 0
}

/* --------------------- Branch Prediction Logic --------------------- */

int branchPrediction(stateType *state, int pc) {
    for (int i = 0; i < BTBSIZE; i++) {
        if (state->btbValid[i] && state->btbPC[i] == pc) {
            return state->btbTarget[i];
        }
    }
    return pc + 1; // Default prediction: next sequential instruction
}

void updateBranchPrediction(stateType *state, int pc, int taken) {
    // Update Pattern History Buffer (PHB) based on the outcome
    state->phb[pc % PHBSIZE] = taken;

    // Update BTB if branch is taken
    if (taken) {
        state->btbPC[state->btbIndex] = pc;
        state->btbTarget[state->btbIndex] = pc + 1;
        state->btbValid[state->btbIndex] = 1;
        state->btbIndex = (state->btbIndex + 1) % BTBSIZE; // FIFO replacement
    }
}

/* --------------------- Main Simulation Loop --------------------- */

void run(stateType *state) {
    while (1) {
        printState(state);

        /* check for halt */
        if (opcode(state->MEMWB.instr) == HALT) {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state->cycles);
            printf("FETCHED: %d\n", state->fetched);
            printf("RETIRED: %d\n", state->retired);
            printf("BRANCHES: %d\n", state->branches);
            printf("MISPRED: %d\n", state->mispred);
            exit(0);
        }

        stateType newState = *state;
        newState.cycles++;

        /* --------------------- IF stage --------------------- */
        int predictedPC = branchPrediction(state, state->pc);
        newState.IFID.instr = state->instrMem[state->pc];
        newState.IFID.pcPlus1 = predictedPC;
        newState.fetched++;

        /* --------------------- ID stage --------------------- */
        newState.IDEX.instr = state->IFID.instr;
        newState.IDEX.pcPlus1 = state->IFID.pcPlus1;

        // Decode registers
        newState.IDEX.readRegA = state->reg[(state->IFID.instr >> 19) & 0x7];
        newState.IDEX.readRegB = state->reg[(state->IFID.instr >> 16) & 0x7];
        newState.IDEX.offset = state->IFID.instr & 0xFFFF;

        /* --------------------- EX stage --------------------- */
        newState.EXMEM.instr = state->IDEX.instr;
        newState.EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
        int aluResult = 0;
        switch (opcode(state->IDEX.instr)) {
            case ADD:
                aluResult = state->IDEX.readRegA + state->IDEX.readRegB;
                break;
            case NAND:
                aluResult = ~(state->IDEX.readRegA & state->IDEX.readRegB);
                break;
            case CMP:
                // Compare and set cmp flag
                aluResult = (state->IDEX.readRegA == state->IDEX.readRegB);
                break;
            case MOVL:
            case MOVS:
                aluResult = state->IDEX.readRegA + state->IDEX.offset;
                break;
        }
        newState.EXMEM.aluResult = aluResult;

        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr = state->EXMEM.instr;
        if (opcode(state->EXMEM.instr) == MOVL) {
            newState.MEMWB.writeData = state->dataMem[state->EXMEM.aluResult];
        } else if (opcode(state->EXMEM.instr) == MOVS) {
            state->dataMem[state->EXMEM.aluResult] = state->EXMEM.readRegB;
        } else {
            newState.MEMWB.writeData = state->EXMEM.aluResult;
        }

        /* --------------------- WB stage --------------------- */
        newState.WBEND.instr = state->MEMWB.instr;
        newState.WBEND.writeData = state->MEMWB.writeData;
        if (opcode(state->MEMWB.instr) == ADD || opcode(state->MEMWB.instr) == NAND || opcode(state->MEMWB.instr) == CMP) {
            state->reg[(state->MEMWB.instr >> 16) & 0x7] = state->MEMWB.writeData;
        }

        *state = newState;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <machine-code-file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error: Can't open machine code file %s\n", argv[1]);
        return 1;
    }

    stateType state;
    state.pc = 0;
    state.cycles = 0;
    state.fetched = 0;
    state.retired = 0;
    state.branches = 0;
    state.mispred = 0;

    /* Initialize register and memory */
    for (int i = 0; i < NUMREGS; i++) {
        state.reg[i] = 0;
    }
    for (int i = 0; i < NUMMEMORY; i++) {
        state.instrMem[i] = 0;
        state.dataMem[i] = 0;
    }

    /* Load machine code */
    int i = 0;
    while (fscanf(fp, "%d", &state.instrMem[i]) != EOF) {
        i++;
    }
    state.numMemory = i;
    fclose(fp);

    /* Initialize branch predictor */
    initBranchPredictor(&state);

    run(&state);
    return 0;
}
