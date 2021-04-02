/* ----------------------------------------------------------------------------
  Copyright (c) 2021, Microsoft Research, Daan Leijen
  This is free software; you can redistribute it and/or modify it 
  under the terms of the MIT License. A copy of the License can be 
  found in the LICENSE file at the root of this distribution.

  ARM64 (aarch64) calling convention. See:
  - <https://en.wikipedia.org/wiki/Calling_convention#ARM_.28A64.29>
  - <http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055c/IHI0055C_beta_aapcs64.pdf>

  NOTE: currently untested code!!!

  Primitives to switch stacks:
 
    typedef uint8_t mp_jmp_buf_t[MP_JMPBUF_SIZE];  // machine word aligned
  
    bool     mp_setjmp ( mp_jmp_buf_t jmp );
    void     mp_longjmp( mp_jmp_buf_t jmp );
    void* mp_stack_enter(void* stack_base, void* stack_commit_limit, void* stack_limit, mp_jmpbuf_t** return_jmp, 
                         void (*fun)(void* arg, void* trapframe), void* arg);
    
  `mp_stack_enter` enters a fresh stack and runs `fun(arg)`; it also receives 
  a (pointer to a pointer to a) return jmpbuf to which it longjmp's on return.
-----------------------------------------------------------------------------*/


/*
notes: 
- According to the ARM ABI specification, only the bottom 64 bits of the floating 
  point registers need to be preserved (sec. 5.1.2 of aapcs64).
  Todo: do we need to save the full 128 bits on Windows?
- The x18 register is the "platform register" and may be temporary or not. For safety
  we always save it.

jmpbuf layout:
   0: x18  
   8: x19
  16: x20
  24: x21
  32: x22
  40: x23
  48: x24
  56: x25
  64: x26
  72: x27
  80: x28
  88: fp   = x29
  96: lr   = x30
 104: sp   = x31
 112: fpcr
 120: fpsr
 128: d8  (64 bits)
 136: d9
 ...
 184: d15
 192: sizeof jmpbuf
*/

.global mp_setjmp
.global mp_longjmp
.global mp_switch_to_stack

.type mp_setjmp,%function
.type mp_longjmp,%function
.type mp_stack_enter,%function
.type abort,%function

/* called with x0: &jmp_buf */
mp_setjmp:                 
  stp   x18, x19, [x0], #16
  stp   x20, x21, [x0], #16
  stp   x22, x23, [x0], #16
  stp   x24, x25, [x0], #16
  stp   x26, x27, [x0], #16
  stp   x28, x29, [x0], #16   /* x28 and fp */
  mov   x10, sp               /* sp to x10 */
  stp   x30, x10, [x0], #16   /* lr and sp */
  /* store fp control and status */
  mrs   x10, fpcr
  mrs   x11, fpsr
  stp   x10, x11, [x0], #16    
  /* store float registers */
  stp   d8,  d9,  [x0], #16
  stp   d10, d11, [x0], #16
  stp   d12, d13, [x0], #16
  stp   d14, d15, [x0], #16
  /* always return zero */
  mov   x0, #0
  ret                         /* jump to lr */


/* called with x0: &jmp_buf */
mp_longjmp:                  
  ldp   x18, x19, [x0], #16
  ldp   x20, x21, [x0], #16
  ldp   x22, x23, [x0], #16
  ldp   x24, x25, [x0], #16
  ldp   x26, x27, [x0], #16
  ldp   x28, x29, [x0], #16   /* x28 and fp */
  ldp   x30, x10, [x0], #16   /* lr and sp */
  mov   sp,  x10
  /* load fp control and status */
  ldp   x10, x11, [x0], #16
  msr   fpcr, x10
  msr   fpsr, x11
  /* load float registers */
  ldp   d8,  d9,  [x0], #16
  ldp   d10, d11, [x0], #16
  ldp   d12, d13, [x0], #16
  ldp   d14, d15, [x0], #16
  /* always return 1 */
  mov   x0, #1
  ret                         /* jump to lr */



/* switch stack 
   x0: stack pointer, 
   x1: stack commit limit    (ignored on unix)
   x2: stack limit           (ignored on unix)
   x3: jmpbuf_t** return point
   x4: function to run
   x5: argument to pass to the function 
*/
mp_stack_enter:
  and     sp, x0, #~31          /* switch to the new stack (aligned down to 32 bytes) */
  sub     sp, sp, #32           /* sp = sp - 32 */
  stp     x29, x30, [sp, #16]   /* mimic call on new stack: sp[24] = x30, sp[16] = lr */
  str     x3, [sp, #8]          /* save jmpbuf_t** on the stack for (future) unwinding: sp[8] = x3 */
  add     x29, sp, #16          /* set our frame pointer to sp[16] */
  
  mov     x0, x5                /* argument to x0 */
  mov     x1, #0                /* no trap frame */
  bl      x4                    /* and call the function */

  /* should never get here */
  bl      abort

  ldr     x0, [sp, #8]          /* load jmpbuf_t* and longjmp back */
  ldr     x0, [x0]
  b       mp_longjmp