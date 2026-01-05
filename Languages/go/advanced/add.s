// add.s

// 1.17之前
// #include "textflag.h"

// add(x, y int) int
//TEXT ·add(SB), NOSPLIT, $8-24
//	MOVQ $0, z-0x8(SP)
//	MOVQ x+0x0(FP), AX
//	MOVQ y+0x8(FP), BX
//	ADDQ AX, BX
//	MOVQ BX, z-0x8(SP)
//	MOVQ BX, ret+0x10(FP)
//	RET


// 1.17之后 amd64汇编
//#include "textflag.h"

// func add(x, y int64) int64
//TEXT ·add(SB), NOSPLIT, $0-24
//    MOVQ x+0(FP), AX    // AX = x（汇编器会优化为寄存器）
//    MOVQ y+8(FP), BX    // BX = y
//    ADDQ BX, AX         // AX += BX
//    MOVQ AX, ret+16(FP) // 注意：这行在寄存器 ABI 下可选（汇编器自动处理返回值寄存器）
//    RET

#include "textflag.h"

TEXT ·add(SB), NOSPLIT, $8-24
    ADD y+8(FP), R0      // 假设 x 已预置在 R0，直接加 y 到 R0
    RET