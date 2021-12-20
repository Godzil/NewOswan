/*
 * NewOswan
 * nec_debugger.c:
 * Created by Manoël Trapier on 14/04/2021.
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <log.h>
#include <nec_debugger.h>
#include <necintrf.h>
#include <memory.h>

/***
 * Note: the while code to decode instruction is not meant to be optimised, but to be easy to maintain.
 * It probably could be more concise, but this is code for the debugger, not runtime, so optimisation does
 * not really matter here.
 */

typedef enum operandParameterTypes
{
    PR_RM_R8,       /**< reg8/mem8, reg8            */
    PR_RM_R16,      /**< reg16/mem16, reg16         */
    PR_AL_IM8,      /**< AL, immed8                 */
    PR_AX_IM16,     /**< AX, immed16                */
    PR_ES,          /**< ES                         */
    PR_R_RM8,       /**< reg8, reg8/mem8            */
    PR_R_RM16,      /**< reg16, ret16/mem16         */
    PR_CS,          /**< CS                         */
    PR_SS,          /**< SS                         */
    PR_DS,          /**< DS                         */
    PR_NONE,        /**< No parameter or Prefix     */
    PR_AX,          /**< AX                         */
    PR_CX,          /**< CX                         */
    PR_DX,          /**< DX                         */
    PR_BX,          /**< BX                         */
    PR_SP,          /**< SP                         */
    PR_BP,          /**< BP                         */
    PR_SI,          /**< SI                         */
    PR_DI,          /**< DI                         */
    PR_IM16,        /**< immediate 16               */
    PR_REL8,        /**< short label                */
    PR_RM_IM8,      /**< reg8/mem8, immediate8      */
    PR_RM_IM16,     /**< reg16/mem16, immed16       */
    PR_RM16_SEG,    /**< reg16/mem16, SegReg        */
    PR_SEG_RM16,    /**< SegReg, reg16/mem16        */
    PR_AX_CX,       /**< AX, CX                     */
    PR_AX_DX,       /**< AX, DX                     */
    PR_AX_BX,       /**< AX, BX                     */
    PR_AX_SP,       /**< AX, SP                     */
    PR_AX_BP,       /**< AX, BP                     */
    PR_AX_SI,       /**< AX, SI                     */
    PR_AX_DI,       /**< AX, DI                     */
    PR_ABS32,       /**< far proc                   */
    PR_AL_M8,       /**< AL, mem8                   */
    PR_AX_M16,      /**< AX, mem16                  */
    PR_M8_AL,       /**< mem8, AL                   */
    PR_M16_AX,      /**< mem16, AX                  */
    PR_CL_IM8,      /**< CL, immed8                 */
    PR_DL_IM8,      /**< DL, immed8                 */
    PR_BL_IM8,      /**< BL, immed8                 */
    PR_AH_IM8,      /**< AH, immed8                 */
    PR_CH_IM8,      /**< CH, immed8                 */
    PR_DH_IM8,      /**< DH, immed8                 */
    PR_BH_IM8,      /**< BH, immed8                 */
    PR_CX_IM16,     /**< CX, immed16                */
    PR_DX_IM16,     /**< DX, immed16                */
    PR_BX_IM16,     /**< BX, immed16                */
    PR_SP_IM16,     /**< SP, immed16                */
    PR_BP_IM16,     /**< BP, immed16                */
    PR_SI_IM16,     /**< SI, immed16                */
    PR_DI_IM16,     /**< DI, immed16                */
    PR_RM16_IM8,    /**< reg16/mem16, immed8        */
    PR_IM16_IM8,    /**< immed16, immed8            */
    PR_IM8,         /**< immed8                     */
    PR_RM8_1,       /**< reg8/mem8, 1               */
    PR_RM16_1,      /**< reg16/mem16, 1             */
    PR_RM8_CL,      /**< reg8/mem8, CL              */
    PR_RM16_CL,     /**< reg16/mem16, CL            */
    PR_AX_IM8,      /**< AX, immed8                 */
    PR_AL_DX,       /**< AL, DX                     */
    PR_NONE8,       /**< No param, work in 8 bit    */
    PR_NONE16,      /**< No param, work in 16 bits  */
    PR_PREFIX,      /**< No param, but is a prefix  */
    PR_REL16,       /**< near label                 */
    PR_IM8_AL,      /**< immed8, AL                 */
    PR_IM8_AX,      /**< immed8, AX                 */
    PR_DX_AL,       /**< DX, AL                     */
    PR_DX_AX,       /**< DX, AX                     */
    PR_RM8,         /**< reg8/mem8                  */
    PR_RM16,        /**< reg16/mem16                */
    PR_NONEFAR,     /**< no opcode but far related  */
} operandParameterTypes;

typedef enum operandTypes
{
    OP_ILEG = 0,
    OP_ADD, OP_PUSH, OP_POP, OP_OR, OP_ADC, OP_SBB, OP_AND, OP_DAA, OP_SUB, OP_DAS, OP_XOR, OP_AAA, OP_CMP,
    OP_AAS, OP_INC, OP_DEC, OP_PUSHA, OP_POPA, OP_IMUL, OP_INS, OP_OUTS, OP_JO, OP_JNO, OP_JB, OP_JNB,
    OP_JE, OP_JNE, OP_JBE, OP_JNBE, OP_JS, OP_JNS, OP_JP, OP_JNP, OP_JL, OP_JNL, OP_JLE, OP_JNLE, OP_TEST,
    OP_XCHG, OP_MOV, OP_LEA, OP_NOP, OP_WAIT, OP_PUSHF, OP_POPF, OP_SAHF, OP_LAHF, OP_MOVS, OP_CMPS,
    OP_STOS, OP_LODS, OP_SCAS, OP_LDS, OP_LES, OP_ENTER, OP_LEAVE, OP_RET, OP_INT3, OP_INT, OP_INTO,
    OP_IRET, OP_XLAT, OP_LOOPNZ, OP_LOOPE, OP_LOOP, OP_JCXZ, OP_IN, OP_OUT, OP_CALL, OP_JMP, OP_HLT,
    OP_CLC, OP_STC, OP_CLI, OP_STI, OP_CLS, OP_STD, OP_IMMED, OP_SHIFT, OP_GRP1, OP_GRP2, OP_BOUND,
    OP_CBW, OP_CWD, OP_AAM, OP_AAD, OP_LOCK, OP_REP, OP_REPNZ, OP_CMC, OP_CS, OP_DS, OP_ES, OP_SS, OP_MOVG
} operandTypes;

const char *operandTypeNameTable[] =
{
    [OP_ILEG] = "Illegal",
    [OP_ADD] = "add",
    [OP_PUSH] = "push",
    [OP_POP] = "pop",
    [OP_OR] = "or",
    [OP_ADC] = "adc",
    [OP_SBB] = "sbb",
    [OP_AND] = "and",
    [OP_DAA] = "daa",
    [OP_SUB] = "sub",
    [OP_DAS] = "das",
    [OP_XOR] = "xor",
    [OP_AAA] = "aaa",
    [OP_CMP] = "cmp",
    [OP_AAS] = "aas",
    [OP_INC] = "inc",
    [OP_DEC] = "dec",
    [OP_PUSHA] = "pusha",
    [OP_POPA] = "popa",
    [OP_IMUL] = "imul",
    [OP_INS] = "ins",
    [OP_OUTS] = "outs",
    [OP_JO] = "jo",
    [OP_JNO] = "jno",
    [OP_JB] = "jb",
    [OP_JNB] = "inb",
    [OP_JE] = "je",
    [OP_JNE] = "jne",
    [OP_JBE] = "jbe",
    [OP_JNBE] = "jnbe",
    [OP_JS] = "js",
    [OP_JNS] = "jns",
    [OP_JP] = "jo",
    [OP_JNP] = "jno",
    [OP_JL] = "jl",
    [OP_JNL] = "jnl",
    [OP_JLE] = "jle",
    [OP_JNLE] = "jnle",
    [OP_TEST] = "test",
    [OP_XCHG] = "xchg",
    [OP_MOV] = "mov",
    [OP_LEA] = "lea",
    [OP_NOP] = "nop",
    [OP_WAIT] = "wait",
    [OP_PUSHF] = "pushf",
    [OP_POPF] = "popf",
    [OP_SAHF] = "sahf",
    [OP_LAHF] = "lahf",
    [OP_MOVS] = "movs",
    [OP_CMPS] = "cmps",
    [OP_STOS] = "stos",
    [OP_LODS] = "lods",
    [OP_SCAS] = "scas",
    [OP_LDS] = "lds",
    [OP_LES] = "led",
    [OP_ENTER] = "enter",
    [OP_LEAVE] = "leave",
    [OP_RET] = "ret",
    [OP_INT3] = "int3",
    [OP_INT] = "int",
    [OP_INTO] = "into",
    [OP_IRET] = "iret",
    [OP_XLAT] = "xlat",
    [OP_LOOPNZ] = "loopnz",
    [OP_LOOPE] = "loope",
    [OP_LOOP] = "loop",
    [OP_JCXZ] = "jcxz",
    [OP_IN] = "in",
    [OP_OUT] = "out",
    [OP_CALL] = "call",
    [OP_JMP] = "jmp",
    [OP_HLT] = "hlt",
    [OP_CLC] = "clc",
    [OP_STC] = "stc",
    [OP_CLI] = "cli",
    [OP_STI] = "sti",
    [OP_CLS] = "cls",
    [OP_STD] = "std",
    [OP_IMMED] = "",
    [OP_SHIFT] = "",
    [OP_GRP1] = "",
    [OP_GRP2] = "",
    [OP_CS] = "cs:",
    [OP_ES] = "es:",
    [OP_DS] = "ds:",
    [OP_SS] = "ss:",
    [OP_BOUND] = "bound",
    [OP_CBW] = "cbw",
    [OP_CWD] = "cwd",
    [OP_AAM] = "aam",
    [OP_AAD] = "aad",
    [OP_LOCK] = "lock",
    [OP_REP] = "rep",
    [OP_REPNZ] = "repnz",
    [OP_CMC] = "cmc",
    [OP_MOVG] = "",
};

operandTypes opcodeTypeTable[256] =
{
 /* 0          1         2         3         4         5        6        7        8         9         A        B        C         D        E        F                */
    OP_ADD,    OP_ADD,   OP_ADD,   OP_ADD,   OP_ADD,   OP_ADD,  OP_PUSH, OP_POP,  OP_OR,    OP_OR,    OP_OR,   OP_OR,   OP_OR,    OP_OR,   OP_PUSH, OP_ILEG,    /* 0 */
    OP_ADC,    OP_ADC,   OP_ADC,   OP_ADC,   OP_ADC,   OP_ADC,  OP_PUSH, OP_POP,  OP_SBB,   OP_SBB,   OP_SBB,  OP_SBB,  OP_SBB,   OP_SBB,  OP_PUSH, OP_POP,     /* 1 */
    OP_AND,    OP_AND,   OP_AND,   OP_AND,   OP_AND,   OP_AND,  OP_ES,   OP_DAA,  OP_SUB,   OP_SUB,   OP_SUB,  OP_SUB,  OP_SUB,   OP_SUB,  OP_CS,   OP_DAS,     /* 2 */
    OP_XOR,    OP_XOR,   OP_XOR,   OP_XOR,   OP_XOR,   OP_XOR,  OP_SS,   OP_AAA,  OP_CMP,   OP_CMP,   OP_CMP,  OP_CMP,  OP_CMP,   OP_CMP,  OP_DS,   OP_AAS,     /* 3 */
    OP_INC,    OP_INC,   OP_INC,   OP_INC,   OP_INC,   OP_INC,  OP_INC,  OP_INC,  OP_DEC,   OP_DEC,   OP_DEC,  OP_DEC,  OP_DEC,   OP_DEC,  OP_DEC,  OP_DEC,     /* 4 */
    OP_PUSH,   OP_PUSH,  OP_PUSH,  OP_PUSH,  OP_PUSH,  OP_PUSH, OP_PUSH, OP_PUSH, OP_POP,   OP_POP,   OP_POP,  OP_POP,  OP_POP,   OP_POP,  OP_POP,  OP_POP,     /* 5 */
    OP_PUSHA,  OP_POPA,  OP_BOUND, OP_ILEG,  OP_ILEG,  OP_ILEG, OP_ILEG, OP_ILEG, OP_PUSH,  OP_IMUL,  OP_PUSH, OP_IMUL, OP_INS,   OP_INS,  OP_OUTS, OP_OUTS,    /* 6 */
    OP_JO,     OP_JNO,   OP_JB,    OP_JNB,   OP_JE,    OP_JNE,  OP_JBE,  OP_JNBE, OP_JS,    OP_JNS,   OP_JP,   OP_JNP,  OP_JL,    OP_JNL,  OP_JLE,  OP_JNLE,    /* 7 */
    OP_IMMED,  OP_IMMED, OP_IMMED, OP_IMMED, OP_TEST,  OP_TEST, OP_XCHG, OP_XCHG, OP_MOV,   OP_MOV,   OP_MOV,  OP_MOV,  OP_MOV,   OP_LEA,  OP_MOV,  OP_POP,     /* 8 */
    OP_NOP,    OP_XCHG,  OP_XCHG,  OP_XCHG,  OP_XCHG,  OP_XCHG, OP_XCHG, OP_XCHG, OP_CBW,   OP_CWD,   OP_CALL, OP_WAIT, OP_PUSHF, OP_POPF, OP_SAHF, OP_LAHF,    /* 9 */
    OP_MOV,    OP_MOV,   OP_MOV,   OP_MOV,   OP_MOVS,  OP_MOVS, OP_CMPS, OP_CMPS, OP_TEST,  OP_TEST,  OP_STOS, OP_STOS, OP_LODS,  OP_LODS, OP_SCAS, OP_SCAS,    /* A */
    OP_MOV,    OP_MOV,   OP_MOV,   OP_MOV,   OP_MOV,   OP_MOV,  OP_MOV,  OP_MOV,  OP_MOV,   OP_MOV,   OP_MOV,  OP_MOV,  OP_MOV,   OP_MOV,  OP_MOV,  OP_MOV,     /* B */
    OP_SHIFT,  OP_SHIFT, OP_RET,   OP_RET,   OP_LES,   OP_LDS,  OP_MOVG, OP_MOVG, OP_ENTER, OP_LEAVE, OP_RET,  OP_RET,  OP_INT3,  OP_INT,  OP_INTO, OP_IRET,    /* C */
    OP_SHIFT,  OP_SHIFT, OP_SHIFT, OP_SHIFT, OP_AAM,   OP_AAD,  OP_ILEG, OP_XLAT, OP_ILEG,  OP_ILEG,  OP_ILEG, OP_ILEG, OP_ILEG,  OP_ILEG, OP_ILEG, OP_ILEG,    /* D */
    OP_LOOPNZ, OP_LOOPE, OP_LOOP,  OP_JCXZ,  OP_IN,    OP_IN,   OP_OUT,  OP_OUT,  OP_CALL,  OP_JMP,   OP_JMP,  OP_JMP,  OP_IN,    OP_IN,   OP_OUT,  OP_OUT,     /* E */
    OP_LOCK,   OP_ILEG,  OP_REPNZ, OP_REP,   OP_HLT,   OP_CMC,  OP_GRP1, OP_GRP1, OP_CLC,   OP_STC,   OP_CLI,  OP_STI,  OP_CLS,   OP_STD,  OP_GRP2, OP_GRP2,    /* F */
 /* 0          1         2         3         4         5        6        7        8         9         A        B        C         D        E        F                */
};

operandParameterTypes opcodeParamTypeTable[256] =
{
 /* 0         1            2           3            4          5           6          7           8            9           A           B            C            D           E            F                   */
    PR_RM_R8,  PR_RM_R16,   PR_R_RM8,  PR_R_RM16,   PR_AL_IM8, PR_AX_IM16, PR_ES,     PR_ES,      PR_RM_R8,    PR_RM_R16,  PR_R_RM8,   PR_R_RM16,   PR_AL_IM8,   PR_AX_IM16, PR_CS,       PR_NONE,       /* 0 */
    PR_RM_R8,  PR_RM_R16,   PR_R_RM8,  PR_R_RM16,   PR_AL_IM8, PR_AX_IM16, PR_SS,     PR_SS,      PR_RM_R8,    PR_RM_R16,  PR_R_RM8,   PR_R_RM16,   PR_AL_IM8,   PR_AX_IM16, PR_DS,       PR_DS,         /* 1 */
    PR_RM_R8,  PR_RM_R16,   PR_R_RM8,  PR_R_RM16,   PR_AL_IM8, PR_AX_IM16, PR_PREFIX, PR_NONE,    PR_RM_R8,    PR_RM_R16,  PR_R_RM8,   PR_R_RM16,   PR_AL_IM8,   PR_AX_IM16, PR_PREFIX,   PR_NONE,       /* 2 */
    PR_RM_R8,  PR_RM_R16,   PR_R_RM8,  PR_R_RM16,   PR_AL_IM8, PR_AX_IM16, PR_PREFIX, PR_NONE,    PR_RM_R8,    PR_RM_R16,  PR_R_RM8,   PR_R_RM16,   PR_AL_IM8,   PR_AX_IM16, PR_PREFIX,   PR_NONE,       /* 3 */
    PR_AX,     PR_CX,       PR_DX,     PR_BX,       PR_SP,     PR_BP,      PR_SI,     PR_DI,      PR_AX,       PR_CX,      PR_DX,      PR_BX,       PR_SP,       PR_BP,      PR_SI,       PR_DI,         /* 4 */
    PR_AX,     PR_CX,       PR_DX,     PR_BX,       PR_SP,     PR_BP,      PR_SI,     PR_DI,      PR_AX,       PR_CX,      PR_DX,      PR_BX,       PR_SP,       PR_BP,      PR_SI,       PR_DI,         /* 5 */
    PR_NONE,   PR_NONE,     PR_R_RM16, PR_NONE,     PR_NONE,   PR_NONE,    PR_NONE,   PR_NONE,    PR_IM16,     PR_RM_IM16, PR_IM8,     PR_RM16_IM8, PR_NONE8,    PR_NONE16,  PR_NONE8,    PR_NONE16,     /* 6 */
    PR_REL8,   PR_REL8,     PR_REL8,   PR_REL8,     PR_REL8,   PR_REL8,    PR_REL8,   PR_REL8,    PR_REL8,     PR_REL8,    PR_REL8,    PR_REL8,     PR_REL8,     PR_REL8,    PR_REL8,     PR_REL8,       /* 7 */
    PR_RM_IM8, PR_RM_IM16,  PR_RM_IM8, PR_RM16_IM8, PR_RM_R8,  PR_RM_R16,  PR_R_RM8,  PR_R_RM16,  PR_RM_R8,    PR_RM_R16,  PR_R_RM8,   PR_R_RM16,   PR_RM16_SEG, PR_R_RM16,  PR_SEG_RM16, PR_NONE,       /* 8 */
    PR_NONE,   PR_AX_CX,    PR_AX_DX,  PR_AX_BX,    PR_AX_SP,  PR_AX_BP,   PR_AX_SI,  PR_AX_DI,   PR_NONE,     PR_NONE,    PR_ABS32,   PR_NONE,     PR_NONE,     PR_NONE,    PR_NONE,     PR_NONE,       /* 9 */
    PR_AL_M8,  PR_AX_M16,   PR_M8_AL,  PR_M16_AX,   PR_NONE8,  PR_NONE16,  PR_NONE8,  PR_NONE16,  PR_AL_IM8,   PR_AX_IM16, PR_NONE8,   PR_NONE16,   PR_NONE8,    PR_NONE16,  PR_NONE8,    PR_NONE16,     /* A */
    PR_AL_IM8, PR_CL_IM8,   PR_DL_IM8, PR_BL_IM8,   PR_AH_IM8, PR_CH_IM8,  PR_DH_IM8, PR_BH_IM8,  PR_AX_IM16,  PR_CX_IM16, PR_DX_IM16, PR_BX_IM16,  PR_SP_IM16,  PR_BP_IM16, PR_SI_IM16,  PR_DI_IM16,    /* B */
    PR_RM_IM8, PR_RM16_IM8, PR_IM16,   PR_NONE,     PR_R_RM16, PR_R_RM16,  PR_RM_IM8, PR_RM_IM16, PR_IM16_IM8, PR_NONE,    PR_IM16,    PR_NONEFAR,  PR_NONE,     PR_IM8,     PR_NONE,     PR_NONE,       /* C */
    PR_RM8_1,  PR_RM16_1,   PR_RM8_CL, PR_RM16_CL,  PR_NONE,   PR_NONE,    PR_NONE,   PR_NONE,    PR_NONE,     PR_NONE,    PR_NONE,    PR_NONE,     PR_NONE,     PR_NONE,    PR_NONE,     PR_NONE,       /* D */
    PR_REL8,   PR_REL8,     PR_REL8,   PR_REL8,     PR_AL_IM8, PR_AX_IM8,  PR_IM8_AL, PR_IM8_AX,  PR_REL16,    PR_REL16,   PR_ABS32,   PR_REL8,     PR_AL_DX,    PR_AX_DX,   PR_DX_AL,    PR_DX_AX,      /* E */
    PR_PREFIX, PR_NONE,     PR_PREFIX, PR_PREFIX,   PR_NONE,   PR_NONE,    PR_RM8,    PR_RM16,    PR_NONE,     PR_NONE,    PR_NONE,    PR_NONE,     PR_NONE,     PR_NONE,    PR_RM8,      PR_RM16,       /* F */
 /* 0         1            2           3            4          5           6          7           8            9           A           B            C            D           E            F                   */
};

enum extendedOpcodes
{
    OP_IMD_ADD = 0,
    OP_IMD_OR = 1,
    OP_IMD_ADC = 2,
    OP_IMD_SBB = 3,
    OP_IMD_AND = 4,
    OP_IMD_SUB = 5,
    OP_IMD_XOR = 6,
    OP_IMD_CMP = 7,

    OP_SFT_ROL = 0,
    OP_SFT_ROR = 1,
    OP_SFT_RCL = 2,
    OP_SFT_RCR = 3,
    OP_SFT_SHL = 4,
    OP_SFT_SHR = 5,
    OP_SFT_SAR = 7,

    OP_GP1_TEST = 0,
    OP_GP1_NOT = 2,
    OP_GP1_NEG = 3,
    OP_GP1_MUL = 4,
    OP_GP1_IMUL = 5,
    OP_GP1_DIV = 6,
    OP_GP1_IDIV = 7,

    OP_GP2_INC = 0,
    OP_GP2_DEC = 1,
    OP_GP2_CALL = 2,
    OP_GP2_CALLF = 3,
    OP_GP2_JMP = 4,
    OP_GP2_JMPF = 5,
    OP_GP2_PUSH = 6,
};

typedef enum modrmValues
{
    MR_BX_SI = 0,       /**< [BX + SI]            */
    MR_BX_DI,           /**< [BX + DI]            */
    MR_BP_SI,           /**< [BP + SI]            */
    MR_BP_DI,           /**< [BP + DI]            */
    MR_SI,              /**< [SI]                 */
    MR_DI,              /**< [DI]                 */
    MR_DISP16,          /**< disp16               */
    MR_BX,              /**< [BX]                 */
    MR_BX_SI_D8,        /**< [BX + SI] + disp8    */
    MR_BX_DI_D8,        /**< [BX + DI] + disp8    */
    MR_BP_SI_D8,        /**< [BP + SI] + disp8    */
    MR_BP_DI_D8,        /**< [BP + DI] + disp8    */
    MR_SI_D8,           /**< [SI] + disp8         */
    MR_DI_D8,           /**< [DI] + disp8         */
    MR_BP_D8,           /**< [BX] + disp8         */
    MR_BX_D8,           /**< [BP] + disp8         */
    MR_BX_SI_D16,       /**< [BX + SI] + disp16   */
    MR_BX_DI_D16,       /**< [BX + DI] + disp16   */
    MR_BP_SI_D16,       /**< [BP + SI] + disp16   */
    MR_BP_DI_D16,       /**< [BP + DI] + disp16   */
    MR_SI_D16,          /**< [SI] + disp16        */
    MR_DI_D16,          /**< [DI] + disp16        */
    MR_BP_D16,          /**< [BX] + disp16        */
    MR_BX_D16,          /**< [Bp] + disp16        */
    MR_AX_AL,           /**< AX or AL             */
    MR_CX_CL,           /**< CX or CL             */
    MR_DX_DL,           /**< DX or DL             */
    MR_BX_BL,           /**< BC or BL             */
    MR_SP_AH,           /**< SP or AH             */
    MR_BP_CH,           /**< BP or CH             */
    MR_SI_DH,           /**< SI or DH             */
    MR_DI_BH,           /**< DI or BH             */
} modrmValues;

const char *modrmReg8List[8] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
const char *modrmReg16List[8] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
const char *segmentRegList[8] = { "es", "cs", "ss", "ds", "ILLEGAL", "ILLEGAL", "ILLEGAL", "ILLEGAL" };
static inline void get_mod_reg_rm(uint8_t value, uint8_t *mod, uint8_t *reg, uint8_t *rm, uint8_t *modrm)
{
    if (mod)
    {
        *mod = (value >> 6) & 0x3;
    }
    if (reg)
    {
        *reg = (value >> 3) & 0x7;
    }
    if (rm)
    {
        *rm = (value) & 0x7;
    }
    if (modrm)
    {
        *modrm = (value) & 0x7;
        *modrm = ((value & 0xC0) >> 3) | (*modrm);
    }
}

#define MAKE_LINEAR(_seg, _off) ( ((_seg) << 4) + (_off) )

static int decode_modrm(uint16_t segment, uint16_t offset, modrmValues modrm, bool is8bit, char *buffer, uint32_t bufferLength)
{
    uint8_t disp8 = mem_readmem20(MAKE_LINEAR(segment, offset));
    uint16_t disp16 = (mem_readmem20(MAKE_LINEAR(segment, offset + 1)) << 8) | disp8;
    char buf[63];
    int offsetReturn = 0;

    switch (modrm)
    {
    case MR_BX_SI:
        snprintf(buf, 63, " %s [bx + si]", is8bit?"byte":"word");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_BX_DI:
        snprintf(buf, 63, " %s [bx + di]", is8bit?"byte":"word");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_BP_SI:
        snprintf(buf, 63, " %s [bp + si]", is8bit?"byte":"word");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_BP_DI:
        snprintf(buf, 63, " %s [bx + di]", is8bit?"byte":"word");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_SI:
        snprintf(buf, 63, " %s [si]", is8bit?"byte":"word");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_DI:
        snprintf(buf, 63, " %s [di]", is8bit?"byte":"word");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_DISP16:
        snprintf(buf, 63, " %s [0%Xh]", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_BX:
        snprintf(buf, 63, " %s [bx]", is8bit?"byte":"word");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_BX_SI_D8:
        snprintf(buf, 63, " %s [bx + si] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_BX_DI_D8:
        snprintf(buf, 63, " %s [bx + di] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_BP_SI_D8:
        snprintf(buf, 63, " %s [bp + si] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_BP_DI_D8:
        snprintf(buf, 63, " %s [bp + di] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_SI_D8:
        snprintf(buf, 63, " %s [si] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_DI_D8:
        snprintf(buf, 63, " %s [di] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_BP_D8:
        snprintf(buf, 63, " %s [bp] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_BX_D8:
        snprintf(buf, 63, " %s [bx] + 0%Xh", is8bit?"byte":"word", disp8);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 1;
        break;

    case MR_BX_SI_D16:
        snprintf(buf, 63, " %s [bx + si] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_BX_DI_D16:
        snprintf(buf, 63, " %s [bx + di] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_BP_SI_D16:
        snprintf(buf, 63, " %s [bp + si] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_BP_DI_D16:
        snprintf(buf, 63, " %s [bp + di] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_SI_D16:
        snprintf(buf, 63, " %s [si] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_DI_D16:
        snprintf(buf, 63, " %s [di] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_BP_D16:
        snprintf(buf, 63, " %s [bp] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_BX_D16:
        snprintf(buf, 63, " %s [bx] + 0%Xh", is8bit?"byte":"word", disp16);
        strncat(buffer, buf, bufferLength);
        offsetReturn = 2;
        break;

    case MR_AX_AL:
        snprintf(buf, 63, " %s", is8bit?"al":"ax");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_CX_CL:
        snprintf(buf, 63, " %s", is8bit?"cl":"cx");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_DX_DL:
        snprintf(buf, 63, " %s", is8bit?"dl":"dx");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_BX_BL:
        snprintf(buf, 63, " %s", is8bit?"bl":"bx");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_SP_AH:
        snprintf(buf, 63, " %s", is8bit?"ah":"sp");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_BP_CH:
        snprintf(buf, 63, " %s", is8bit?"ch":"bp");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_SI_DH:
        snprintf(buf, 63, " %s", is8bit?"dh":"si");
        strncat(buffer, buf, bufferLength);
        break;

    case MR_DI_BH:
        snprintf(buf, 63, " %s", is8bit?"bh":"di");
        strncat(buffer, buf, bufferLength);
        break;
    }

    return offsetReturn;
}


int nec_decode_instruction(uint16_t segment, uint16_t offset, char *buffer, unsigned int bufferSize)
{
    uint8_t opcode = mem_readmem20(MAKE_LINEAR(segment, offset));
    uint16_t currentOffset = offset;
    int16_t param1, param2;
    uint8_t modrm, reg;

    char buf[64];

    bool isPrefix = false;
    operandTypes opcodeType = opcodeTypeTable[opcode];
    const char *opcodeName = operandTypeNameTable[opcodeType];
    operandParameterTypes opcodeParams = opcodeParamTypeTable[opcode];

    currentOffset++;

    switch(opcodeType)
    {
    /* Need to handle opcode group */
    case OP_IMMED:
        param1 = mem_readmem20(MAKE_LINEAR(segment, offset + 1));
        get_mod_reg_rm(param1, NULL, &reg, NULL, NULL);
        switch (reg)
        {
        case OP_IMD_ADD:
            strncat(buffer, "add", bufferSize);
            break;

        case OP_IMD_OR:
            /* This is not available in B2 and B3 */
            if (opcode > 0xB1)
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
                currentOffset++;
            }
            else
            {
                strncat(buffer, "or", bufferSize);
            }
            break;

        case OP_IMD_ADC:
            strncat(buffer, "adc", bufferSize);
            break;

        case OP_IMD_SBB:
            strncat(buffer, "sbb", bufferSize);
            break;

        case OP_IMD_AND:
            /* This is not available in B2 and B3 */
            if (opcode > 0xB1)
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
                currentOffset++;
            }
            else
            {
                strncat(buffer, "and", bufferSize);
            }
            break;

        case OP_IMD_SUB:
            strncat(buffer, "sub", bufferSize);
            break;

        case OP_IMD_XOR:
            /* This is not available in B2 and B3 */
            if (opcode > 0xB1)
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
                currentOffset++;
            }
            else
            {
                strncat(buffer, "xor", bufferSize);
            }
            break;

        case OP_IMD_CMP:
            strncat(buffer, "cmp", bufferSize);
            break;
        }
        break;

    case OP_SHIFT:
        param1 = mem_readmem20(MAKE_LINEAR(segment, offset + 1));
        get_mod_reg_rm(param1, NULL, &reg, NULL, NULL);
        switch (reg)
        {
        case OP_SFT_ROL:
            strncat(buffer, "rol", bufferSize);
            break;
        case OP_SFT_ROR:
            strncat(buffer, "ror", bufferSize);
            break;
        case OP_SFT_RCL:
            strncat(buffer, "rcl", bufferSize);
            break;
        case OP_SFT_RCR:
            strncat(buffer, "rcr", bufferSize);
            break;
        case OP_SFT_SHL:
            strncat(buffer, "shl", bufferSize);
            break;
        case OP_SFT_SHR:
            strncat(buffer, "shr", bufferSize);
            break;
        case OP_SFT_SAR:
            strncat(buffer, "sar", bufferSize);
            break;

        default:
            strncat(buffer, "illegal", bufferSize);
            opcodeParams = PR_NONE;
            currentOffset++;
            break;
        }
        break;

    case OP_GRP1:
        param1 = mem_readmem20(MAKE_LINEAR(segment, offset + 1));
        get_mod_reg_rm(param1, NULL, &reg, NULL, NULL);
        switch (reg)
        {
        case OP_GP1_TEST:
            strncat(buffer, "test", bufferSize);
            /* Special case, test is IM_I not just IM */
            if (opcodeParams == PR_RM8)
            {
                opcodeParams = PR_RM_IM8;
            }
            else
            {
                opcodeParams = PR_RM_IM16;
            }
            break;

        case OP_GP1_NOT:
            strncat(buffer, "not", bufferSize);
            break;
        case OP_GP1_NEG:
            strncat(buffer, "neg", bufferSize);
            break;
        case OP_GP1_MUL:
            strncat(buffer, "mul", bufferSize);
            break;
        case OP_GP1_IMUL:
            strncat(buffer, "imul", bufferSize);
            break;
        case OP_GP1_DIV:
            strncat(buffer, "div", bufferSize);
            break;
        case OP_GP1_IDIV:
            strncat(buffer, "idiv", bufferSize);
            break;

        default:
            strncat(buffer, "illegal", bufferSize);
            opcodeParams = PR_NONE;
            currentOffset++;
            break;
        }
        break;

    case OP_GRP2:
        param1 = mem_readmem20(MAKE_LINEAR(segment, offset + 1));
        get_mod_reg_rm(param1, NULL, &reg, NULL, NULL);
        switch (reg)
        {
        case OP_GP2_INC:
            strncat(buffer, "inc", bufferSize);
            break;
        case OP_GP2_DEC:
            strncat(buffer, "dec", bufferSize);
            break;
        case OP_GP2_CALL:
            if (opcode == 0xFF)
            {
                strncat(buffer, "call", bufferSize);
            }
            else
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
            }
            break;
        case OP_GP2_CALLF:
            if (opcode == 0xFF)
            {
                strncat(buffer, "call far", bufferSize);
            }
            else
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
            }
            break;
        case OP_GP2_JMP:
            if (opcode == 0xFF)
            {
                strncat(buffer, "jmp", bufferSize);
            }
            else
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
            }
            break;
        case OP_GP2_JMPF:
            if (opcode == 0xFF)
            {
                strncat(buffer, "jmp far", bufferSize);
            }
            else
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
            }
            break;
        case OP_GP2_PUSH:
            if (opcode == 0xFF)
            {
                strncat(buffer, "push", bufferSize);
            }
            else
            {
                strncat(buffer, "illegal", bufferSize);
                opcodeParams = PR_NONE;
            }
            break;

        default:
            strncat(buffer, "illegal", bufferSize);
            opcodeParams = PR_NONE;
            currentOffset++;
            break;
        }
        break;

    case OP_MOVG:
        /* Special case for C6 and C7, they are valid ONLY if reg == 0 */
        param1 = mem_readmem20(MAKE_LINEAR(segment, offset + 1));
        get_mod_reg_rm(param1, NULL, &reg, NULL, NULL);
        if (reg > 0)
        {
            strncat(buffer, "illegal", bufferSize);
            opcodeParams = PR_NONE;
            currentOffset++;
        }
        else
        {
            strncat(buffer, "mov", bufferSize);
        }

    default:
        strncat(buffer, opcodeName, bufferSize);
        break;
    }

    switch(opcodeParams)
    {
    /*******************************************************************************************************************
     *********************************************** Trivials parameters ***********************************************
     ******************************************************************************************************************/

    /*----------------------------- Prefix have nothing to display, but need to be known -----------------------------*/
    case PR_PREFIX: isPrefix = true; break;

    /*---------------------------------------------- Nothing to display ----------------------------------------------*/
    case PR_NONE: break;

    /*------------------------------------------------ Only registers ------------------------------------------------*/
    case PR_AX: strncat(buffer, " ax", bufferSize); break;
    case PR_BX: strncat(buffer, " bx", bufferSize); break;
    case PR_CX: strncat(buffer, " cx", bufferSize); break;
    case PR_DX: strncat(buffer, " dx", bufferSize); break;
    case PR_SI: strncat(buffer, " si", bufferSize); break;
    case PR_DI: strncat(buffer, " di", bufferSize); break;
    case PR_BP: strncat(buffer, " bp", bufferSize); break;
    case PR_SP: strncat(buffer, " sp", bufferSize); break;
    case PR_SS: strncat(buffer, " ss", bufferSize); break;
    case PR_DS: strncat(buffer, " ds", bufferSize); break;
    case PR_ES: strncat(buffer, " es", bufferSize); break;
    case PR_CS: strncat(buffer, " cs", bufferSize); break;
    case PR_AX_CX: strncat(buffer, " ax, cx", bufferSize); break;
    case PR_AX_DX: strncat(buffer, " ax, dx", bufferSize); break;
    case PR_AX_BX: strncat(buffer, " ax, bx", bufferSize); break;
    case PR_AX_BP: strncat(buffer, " ax, bp", bufferSize); break;
    case PR_AX_SI: strncat(buffer, " ax, si", bufferSize); break;
    case PR_AX_DI: strncat(buffer, " ax, di", bufferSize); break;
    case PR_AX_SP: strncat(buffer, " ax, sp", bufferSize); break;
    case PR_AL_DX: strncat(buffer, " al, dx", bufferSize); break;
    case PR_DX_AL: strncat(buffer, " dx, al", bufferSize); break;
    case PR_DX_AX: strncat(buffer, " dx, ax", bufferSize); break;

    /*------------------------------- opcode name alteration to match their specifics --------------------------------*/
    case PR_NONE8: strncat(buffer, "b", bufferSize); break;     /* working on byte      */
    case PR_NONE16: strncat(buffer, "w", bufferSize); break;    /* working on word      */
    case PR_NONEFAR: strncat(buffer, "f", bufferSize); break;   /* far pointer related  */

    /*******************************************************************************************************************
     ********************************************** Somewhat simple cases **********************************************
     ******************************************************************************************************************/

    /******* Immediate values *******/
    case PR_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    /******* Register / Immediate *******/
    case PR_AL_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " al, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_AH_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " ah, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_AX_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " ax, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_AX_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " ax, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_IM8_AL:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " 0%Xh, al", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_IM8_AX:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " 0%Xh, ax", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_BL_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " bl, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_BH_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " bh, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_BX_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " bx, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_CL_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " cl, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_CH_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " ch, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_CX_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " cx, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_DL_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " dl, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_DH_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " dh, 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        currentOffset++;
        break;

    case PR_DX_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " dx, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_SP_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " sp, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_BP_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " bp, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_DI_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " di, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_SI_IM16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        snprintf(buf, 63, " si, 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    /******* Register / Memory *******/
    case PR_M8_AL:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        /* TODO: If having a list of known label, try to match and display label instead of value */
        snprintf(buf, 63, " byte [0%Xh], al", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_M16_AX:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        /* TODO: If having a list of known label, try to match and display label instead of value */
        snprintf(buf, 63, " word [0%Xh], ax", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_AL_M8:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        /* TODO: If having a list of known label, try to match and display label instead of value */
        snprintf(buf, 63, " al, byte [0%Xh]", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    case PR_AX_M16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        /* TODO: If having a list of known label, try to match and display label instead of value */
        snprintf(buf, 63, " ax, word [0%Xh]", param1);
        strncat(buffer, buf, bufferSize);
        currentOffset += 2;
        break;

    /******* Address calculation *******/
    case PR_REL8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        param1 = currentOffset + (int8_t)param1;
        /* TODO: If having a list of known label, try to match and display label instead of value */
        snprintf(buf, 63, " 0%Xh", param1);
        strncat(buffer, buf, bufferSize);

        break;

    case PR_REL16:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset += 2;
        param1 = currentOffset + (int16_t) param1;

        /* TODO: If having a list of known label, try to match and display label instead of value */
        snprintf(buf, 63, " 0%Xh", param1);
        strncat(buffer, buf, bufferSize);

        break;

    case PR_ABS32:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        param2 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 3)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset + 2));
        /* TODO: If having a list of known label, try to match and display label instead of value */
        snprintf(buf, 63, " 0%Xh:0%Xh", param2 & 0xFFFF, param1 & 0xFFFF);
        strncat(buffer, buf, bufferSize);
        currentOffset += 4;
        break;

    /******* Other cases *******/
    case PR_IM16_IM8:
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                 mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        param2 = mem_readmem20(MAKE_LINEAR(segment, currentOffset + 2));
        snprintf(buf, 63, " 0%Xh:0%Xh", param1, param2);
        strncat(buffer, buf, bufferSize);
        currentOffset += 3;
        break;

    /*******************************************************************************************************************
     ************************************************ Complicated cases ************************************************
     ******************************************************************************************************************/
    case PR_RM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, true, buffer, bufferSize);
        break;

    case PR_RM16:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        break;

    case PR_RM_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, true, buffer, bufferSize);
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        snprintf(buf, 63, ", 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        break;

    case PR_RM16_IM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset ++;
        snprintf(buf, 63, ", 0%Xh", param1 & 0xFF);
        strncat(buffer, buf, bufferSize);
        break;

    case PR_RM_IM16:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                  mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset += 2;
        snprintf(buf, 63, ", 0%Xh", param1);
        strncat(buffer, buf, bufferSize);
        break;

    case PR_RM8_1:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, true, buffer, bufferSize);
        strncat(buffer, ", 1", bufferSize);
        break;

    case PR_RM16_1:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        strncat(buffer, ", 1", bufferSize);
        break;

    case PR_RM8_CL:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, true, buffer, bufferSize);
        strncat(buffer, ", cl", bufferSize);
        break;

    case PR_RM16_CL:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, NULL, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        strncat(buffer, ", cl", bufferSize);
        break;

    case PR_RM_R8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, &reg, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, true, buffer, bufferSize);
        snprintf(buf, 63, ", %s", modrmReg8List[reg]);
        strncat(buffer, buf, bufferSize);
        break;

    case PR_RM_R16:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, &reg, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        snprintf(buf, 63, ", %s", modrmReg16List[reg]);
        strncat(buffer, buf, bufferSize);
        break;

    case PR_R_RM8:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, &reg, NULL, &modrm);
        snprintf(buf, 63, " %s,", modrmReg8List[reg]);
        strncat(buffer, buf, bufferSize);
        currentOffset += decode_modrm(segment, currentOffset, modrm, true, buffer, bufferSize);
        break;

    case PR_R_RM16:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, &reg, NULL, &modrm);
        snprintf(buf, 63, " %s,", modrmReg16List[reg]);
        strncat(buffer, buf, bufferSize);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        break;

    case PR_RM16_SEG:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, &reg, NULL, &modrm);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                 mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset += 2;
        snprintf(buf, 63, ", %s", segmentRegList[reg]);
        strncat(buffer, buf, bufferSize);
        break;

    case PR_SEG_RM16:
        param1 = mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset++;
        get_mod_reg_rm(param1, NULL, &reg, NULL, &modrm);
        snprintf(buf, 63, ", %s", segmentRegList[reg]);
        currentOffset += decode_modrm(segment, currentOffset, modrm, false, buffer, bufferSize);
        param1 = (mem_readmem20(MAKE_LINEAR(segment, currentOffset + 1)) << 8) |
                 mem_readmem20(MAKE_LINEAR(segment, currentOffset));
        currentOffset += 2;
        strncat(buffer, buf, bufferSize);
        break;

    default:
        Log(TLOG_ERROR, "debugger", "Unsupported opcode param type: %d for opcode %02X (%s)", opcodeParams, opcode, opcodeName);
        break;
    }

    if (isPrefix)
    {
        /* call this function from the next byte */
        strncat(buffer, " ", bufferSize);
        nec_decode_instruction(segment, currentOffset, buffer, bufferSize);
    }

    return currentOffset - offset;
}

#if 0
if ((I.sregs[CS] == 0x0600) /* && (I.ip > 0x0050) */)
{
int tmp;
char buffer[256];
uint8_t op = mem_readmem20((I.sregs[CS] << 4) + I.ip);
//Log(TLOG_NORMAL, "NEC v30", "[%04x:%04xh] %02xh '%s' - I=%d\n", I.sregs[CS], I.ip, op, instructionsName[op], I.IF);
printf("AX: %04X, BX: %04X, CX: %04X, DX: %04X, SI: %04X, DI: %04X, SP: %04X\n",
I.regs.w[AW],  I.regs.w[BW],  I.regs.w[CW],  I.regs.w[DW],
I.regs.w[IX],  I.regs.w[IY],  I.regs.w[SP]);
printf("CS: %04X, DS: %04X, SS: %04X, ES: %04X\n",
I.sregs[CS], I.sregs[DS], I.sregs[SS], I.sregs[ES]);
memset(buffer, 0, 256);
nec_decode_instruction(I.sregs[CS], I.ip, buffer, 255);
printf("[%04x:%04xh] %02xh '%s' - I=%d\n", I.sregs[CS], I.ip, op, buffer, I.IF);
tmp = getchar();
}
#endif