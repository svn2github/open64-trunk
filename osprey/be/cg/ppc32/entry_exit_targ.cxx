/*

  Copyright (C) 2006-2009 Tsinghua University.  All Rights Reserved.
  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


#include "defs.h"
#include "tn.h"
#include "op.h"
#include "bb.h"
#include "wn.h"
#include "symtab.h"
#include "errors.h"
#include "targ_sim.h"
#include "ttype.h"
#include "topcode.h"
#include "register.h"
#include "entry_exit_targ.h"
#include "calls.h"
#include "be_util.h"
#include "data_layout.h"
#include "stblock.h"
#include "cgtarget.h"
#include "whirl2ops.h"

void
EETARG_Save_Pfs (TN *saved_pfs, OPS *ops)
{
}


void
EETARG_Restore_Pfs (TN *saved_pfs, OPS *ops)
{
}


void
EETARG_Adjust_SP_For_Entry( TN *incr, OPS *ops )
{
  FmtAssert(FALSE, ("NYI: EETARG_Adjust_SP_For_Entry"));
}

void
EETARG_Adjust_SP_For_Exit( TN *incr, OPS *ops )
{
  FmtAssert(FALSE, ("NYI: EETARG_Adjust_SP_For_Exit"));
}

extern void 
Exp_Immediate (TN *dest, TN *src, BOOL is_signed, OPS *ops);

void
EETARG_Fixup_Entry_Code(BB *bb)
{
#if 0
  extern INT64 PU_Frame_Size;
  OP * op;
  FOR_ALL_OPS_OPs(&bb->ops, op) {
    if (OP_LR_spill(op)) {			
      if (CGTARG_Can_Fit_Immediate_In_Add_Instruction (PU_Frame_Size + 4)) {
        FmtAssert(OP_code(op) == TOP_stw, ("Store lr opcode error"));        
        Set_OP_opnd(op, 2, Gen_Literal_TN(PU_Frame_Size + 4, 4));
      }
      else {
        FmtAssert(OP_code(op) == TOP_stwx, ("Store lr opcode error"));
        OPS ops;
        OPS_Init(&ops);

        // hack
        OP * prev = OP_prev(op);
        TN * im   = OP_opnd(prev, 0);
        TN * len  = OP_result(prev, 0);
        FmtAssert(TN_has_value(im), ("Not a value tn"));
        FmtAssert(TN_is_register(len), ("Not a register tn"));

        /*
        Set_TN_value(im, PU_Frame_Size + 4);
        Exp_Immediate (len, im, FALSE, &ops); */
        INT64 val = PU_Frame_Size + 4;
        short sv = (short)((val >> 16)&0xffff);
        // Set_TN_value(im, sv);
        Build_OP (TOP_lis, len, Gen_Literal_TN(sv, 4), &ops);    
	      Build_OP (TOP_ori, len, len, Gen_Literal_TN(val & 0xffff, 4), &ops); 
        
        BB_Insert_Ops_Before(bb, op, &ops);
        BB_Remove_Op(bb, prev);
      }
      Reset_OP_LR_spill(op);
      break;
    }
  }
#endif  
}

void
EETARG_Fixup_Exit_Code (BB *bb)
{
#if 0
  extern INT64 PU_Frame_Size;
  OP * op;
  FOR_ALL_OPS_OPs_REV(&bb->ops, op) {
    if (OP_LR_spill(op)) {			
      if (CGTARG_Can_Fit_Immediate_In_Add_Instruction (PU_Frame_Size + 4)) {
        FmtAssert(OP_code(op) == TOP_lwz, ("load lr opcode error"));        
        Set_OP_opnd(op, 1, Gen_Literal_TN(PU_Frame_Size + 4, 4));
      }
      else {
        FmtAssert(OP_code(op) == TOP_lwzx, ("load lr opcode error"));
        OPS ops;
        OPS_Init(&ops);

        // hack
        OP * prev = OP_prev(op);
        TN * im   = OP_opnd(prev, 0);
        TN * len  = OP_result(prev, 0);
        FmtAssert(TN_has_value(im), ("Not a value tn"));
        FmtAssert(TN_is_register(len), ("Not a register tn"));

        INT64 val= PU_Frame_Size + 4;
        short sv = (short)((val >> 16)&0xffff);
        // Set_TN_value(im, sv);
        Build_OP (TOP_lis, len, Gen_Literal_TN(sv, 4), &ops);    
	      Build_OP (TOP_ori, len, len, Gen_Literal_TN(val & 0xffff, 4), &ops); 
        
        BB_Insert_Ops_Before(bb, op, &ops);
        BB_Remove_Op(bb, prev);
      }
      Reset_OP_LR_spill(op);
      break;
    }
  }
#endif  
}  

void
EETARG_Init_Entry_Exit_Code (WN *pu_wn, BOOL need_frame_pointer)
{
}

void EETARG_Save_Extra_Callee_Tns (OPS *ops)
{
}

void EETARG_Restore_Extra_Callee_Tns (OPS *ops)
{
}


OP *
EETARG_Build_Jump_Instead_Of_Call (OP *call_op)
{
  OP *jump_op;
  TOP jump_top;
  TOP call_top = OP_code(call_op);
  switch (call_top) {
  case TOP_blrl:
    jump_top = TOP_blr;
    break;
  case TOP_bl:
    jump_top = TOP_b;
    break;
  case TOP_bctrl:
    jump_top = TOP_bctr;
    break;
  default:
    FmtAssert(FALSE, ("don't know how to generate tail call for %s",
		     TOP_Name(call_top)));
    /*NOTREACHED*/
  }

  // Is_True(OP_opnds(call_op) == 1, ("unexpected number of call opnds"));
  if (jump_top == TOP_b) {
    jump_op = Mk_OP(jump_top, OP_opnd(call_op, 0));
  }
  else {
    jump_op = Mk_OP(jump_top);
  }
  // Is_True(OP_opnds(jump_op) == 1, ("unexpected number of jump opnds"));
  return jump_op;
}
