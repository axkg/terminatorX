/*  3DNow! interface library
 *  Copyright (C) 1998 Robert Dale
 *  
 *  This depends on mmx.h which should have been included.
 *
 */ 

#include "mmx.h"

/* 3DNow! instructions */

#define femms() \
        __asm__ __volatile__ ("femms")

#define pf2id_m2r(var, reg)    mmx_m2r(pf2id, var, reg)
#define pf2id_r2r(regs, regd)  mmx_r2r(pf2id, regs, regd)
#define pf2id(vars, vard)      mmx_m2m(pf2id, vars, vard)

#define pfacc_m2r(var, reg)    mmx_m2r(pfacc, var, reg)
#define pfacc_r2r(regs, regd)  mmx_r2r(pfacc, regs, regd)
#define pfacc(vars, vard)      mmx_m2m(pfacc, vars, vard)

#define pfadd_i2r(imm, reg)    mmx_i2r(pfadd, imm, reg)
#define pfadd_m2r(var, reg)    mmx_m2r(pfadd, var, reg)
#define pfadd_r2r(regs, regd)  mmx_r2r(pfadd, regs, regd)
#define pfadd(vars, vard)      mmx_m2m(pfadd, vars, vard)

#define pfcmpeq_m2r(var, reg)  mmx_m2r(pfcmpeq, var, reg)
#define pfcmpeq_r2r(regs, regd) mmx_r2r(pfcmpeq, regs, regd)
#define pfcmpeq(vars, vard)     mmx_m2m(pfcmpeq, vars, vard)

#define pfcmpge_m2r(var, reg)   mmx_m2r(pfcmpge, var, reg)
#define pfcmpge_r2r(regs, regd) mmx_r2r(pfcmpge, regs, regd)
#define pfcmpge(vars, vard)     mmx_m2m(pfcmpge, vars, vard)

#define pfcmpgt_m2r(var, reg)   mmx_m2r(pfcmpgt, var, reg)
#define pfcmpgt_r2r(regs, regd) mmx_r2r(pfcmpgt, regs, regd)
#define pfcmpgt(vars, vard)     mmx_m2m(pfcmpgt, vars, vard)

#define pfmax_m2r(var, reg)     mmx_m2r(pfmax, var, reg)
#define pfmax_r2r(regs, regd)   mmx_r2r(pfmax, regs, regd)
#define pfmax(vars, vard)       mmx_m2m(pfmax, vars, vard)

#define pfmin_m2r(var, reg)     mmx_m2r(pfmin, var, reg)
#define pfmin_r2r(regs, regd)   mmx_r2r(pfmin, regs, regd)
#define pfmin(vars, vard)       mmx_m2m(pfmin, vars, vard)

#define pfmul_i2r(imm, reg)     mmx_i2r(pfmul, imm, reg)
#define pfmul_m2r(var, reg)     mmx_m2r(pfmul, var, reg)
#define pfmul_r2r(regs, regd)   mmx_r2r(pfmul, regs, regd)
#define pfmul(vars, vard)       mmx_m2m(pfmul, vars, vard)

#define pfrcp_m2r(var, reg)     mmx_m2r(pfrcp, var, reg)
#define pfrcp_r2r(regs, regd)   mmx_r2r(pfrcp, regs, regd)
#define pfrcp(vars, vard)       mmx_m2m(pfrcp, vars, vard)

#define pfrcpit1_m2r(var, reg)  mmx_m2r(pfrcpit1, var, reg)
#define pfrcpit1_r2r(regs, regd) mmx_r2r(pfrcpit1, regs, regd)
#define pfrcpit1(vars, vard)    mmx_m2m(pfrcpit1, vars, vard)

#define pfrcpit2_m2r(var, reg)  mmx_m2r(pfrcpit2, var, reg)
#define pfrcpit2_r2r(regs, regd) mmx_r2r(pfrcpit2, regs, regd)
#define pfrcpit2(vars, vard)    mmx_m2m(pfrcpit2, vars, vard)

#define pfrsqrt_m2r(var, reg)   mmx_m2r(pfrsqrt, var, reg)
#define pfrsqrt_r2r(regs, regd) mmx_r2r(pfrsqrt, regs, regd)
#define pfrsqrt(vars, vard)     mmx_m2m(pfrsqrt, vars, vard)

#define pfrsqit1_m2r(var, reg)   mmx_m2r(pfrsqit1, var, reg)
#define pfrsqit1_r2r(regs, regd) mmx_r2r(pfrsqit1, regs, regd)
#define pfrsqit1(vars, vard)     mmx_m2m(pfrsqit1, vars, vard)

#define pfsub_m2r(var, reg)     mmx_m2r(pfsub, var, reg)
#define pfsub_r2r(regs, regd)   mmx_r2r(pfsub, regs, regd)
#define pfsub(vars, vard)       mmx_m2m(pfsub, vars, vard)

#define pfsubr_m2r(var, reg)    mmx_m2r(pfsubr, var, reg)
#define pfsubr_r2r(regs, regd)  mmx_r2r(pfsubr, regs, regd)
#define pfsubr(vars, vard)      mmx_m2m(pfsubr, vars, vard)

#define pi2fd_m2r(var, reg)     mmx_m2r(pi2fd, var, reg)
#define pi2fd_r2r(regs, regd)   mmx_r2r(pi2fd, regs, regd)
#define pi2fd(vars, vard)       mmx_m2m(pi2fd, vars, vard)

#define pavgusb_m2r(var, reg)   mmx_m2r(pavgusb, var, reg)
#define pavgusb_r2r(regs, regd) mmx_r2r(pavgusb, regs, regd)
#define pavgusb(vars, vard)     mmx_m2m(pavgusb, vars, vard)

#define pmulhrw_m2r(var, reg)   mmx_m2r(pmulhrw, var, reg)
#define pmulhrw_r2r(regs, regd) mmx_r2r(pmulhrw, regs, regd)
#define pmulhrw(vars, vard)     mmx_m2m(pmulhrw, vars, vard)

#define prefetch()           __asm__ __volatile__ ("prefetch") 

#define prefetchw()          __asm__ __volatile__ ("prefetchw") 
