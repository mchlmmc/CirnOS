;@-------------------------------------------------------------------------
;@-------------------------------------------------------------------------

.section ".text.startup"
	
.equ CPSR_MODE_USER,				0x10
.equ CPSR_MODE_FIQ,					0x11
.equ CPSR_MODE_IRQ,					0x12
.equ CPSR_MODE_SVR,					0x13
.equ CPSR_MODE_ABORT,				0x17
.equ CPSR_MODE_UNDEFINED,			0x1B
.equ CPSR_MODE_SYSTEM,				0x1F

.equ CPSR_IRQ_INHIBIT,				0x80
.equ CPSR_FIQ_INHIBIT,				0x40
.equ CPSR_THUMB,					0x20

.equ SCTLR_ENABLE_DATA_CACHE,			0x4
.equ SCTLR_ENABLE_BRANCH_PREDICTION,	0x800
.equ SCTLR_ENABLE_INSTRUCTION_CACHE,	0x1000


.global _start	
_start:
	mov r0, #0x8000
	mov r1, #0x0000
	ldmia r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
	stmia r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
	ldmia r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
	stmia r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
	
	mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)
	msr cpsr_c, r0

	mov sp, #0x7000

	
	mov	r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)
	msr cpsr_c, r0
	
	mov sp, #0x8000 
	
	// Enable L1 Cache -------------------------------------------
	mrc p15,0,r0,c1,c0,0

	// Enable caches and branch prediction
	orr r0,#SCTLR_ENABLE_BRANCH_PREDICTION
	orr r0,#SCTLR_ENABLE_DATA_CACHE
	orr r0,#SCTLR_ENABLE_INSTRUCTION_CACHE

	// System Control Register = R0
	mcr p15,0,r0,c1,c0,0
	//Enable VFP ---------------------------------------------
	
	//ri = Access Control Register
	mrc p15, #0, r1, c1, c0, #2
	//enable full access for p10,11
	orr r1, r1, #(0xf << 20)
	//access Control Register = r1
	mcr p15, #0, r1, c1, c0, #2
	mov r1, #0
	//flush prefetch buffer because of FMXR below
	mcr p15, #0, r1, c7, c5, #4
	//and CP 10 & 11 were only just enabled
	//Enable VFP itself
	mov r0,#0x40000000
	//fpexc = r0
	FMXR FPEXC, r0	
	
	bl notmain
	
hang: b hang

.globl PUT32
PUT32:
    str r1,[r0]
    bx lr

.globl PUT16
PUT16:
    strh r1,[r0]
    bx lr

.globl PUT8
PUT8:
    strb r1,[r0]
    bx lr

.globl GET32
GET32:
    ldr r0,[r0]
    bx lr

.globl GETPC
GETPC:
    mov r0,lr
    bx lr

.globl dummy
dummy:
    bx lr

;@ Used for the MMIO interface
.globl memory_barrier
memory_barrier:
    mov	r0, #0
    mcr	p15, #0, r0, c7, c10, #5
    mov	pc, lr
	


;@-------------------------------------------------------------------------
;@-------------------------------------------------------------------------


;@-------------------------------------------------------------------------
;@
;@ Copyright (c) 2012 David Welch dwelch@dwelch.com
;@
;@ Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;@
;@ The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;@
;@ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;@
;@-------------------------------------------------------------------------
