/*
 * Copyright (C) 2009 Advanced Micro Devices, Inc.  All Rights Reserved.
 */

/*
 * Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

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


/* ====================================================================
 * ====================================================================
 *
 *  TN utility routines which include target dependencies.
 *
 *  THIS FILE IS ONLY TO BE INCLUDE BY ../tn.h!!!!
 *
 * ====================================================================
 * ====================================================================
 */

//
// Generate TN to contain a general predicate value
//

#include "targ_sim.h"

inline TN*
Gen_Predicate_TN()
{
  FmtAssert( false, ("No predicate TN for x86.") );
  return Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
}

//
// No fcc registers for this arch.
//
inline BOOL TN_is_fcc_register (const TN *tn)
{
  return FALSE;
}

inline TN* Rflags_TN()
{
  return Build_Dedicated_TN( ISA_REGISTER_CLASS_rflags, REGISTER_MIN, 0 );
}

inline TN* Rip_TN()
{
  return Build_Dedicated_TN( ISA_REGISTER_CLASS_rip, REGISTER_MIN, 0 );
}

inline TN* X87_cw_TN()
{
  return Build_Dedicated_TN( ISA_REGISTER_CLASS_x87_cw, REGISTER_MIN, 0 );
}

inline TN* Ebx_TN()
{
  return Build_Dedicated_TN( ISA_REGISTER_CLASS_integer, RBX, 4 );
}

// what mtype best represents the given tn
inline TYPE_ID
Mtype_Of_TN (TN *tn)
{
  ISA_REGISTER_CLASS rc = TN_register_class(tn);
  switch (rc) {
  case ISA_REGISTER_CLASS_integer: 
	// since regs are unsigned, use unsigned types
	switch (TN_size(tn)) {
           case 1: return MTYPE_U1;
           case 2: return MTYPE_U2;
           case 4: return MTYPE_U4;
           case 8: return MTYPE_U8;
        }
        break;
  case ISA_REGISTER_CLASS_float: return MTYPE_F4;
	switch (TN_size(tn)) {
           case 4: return MTYPE_F4;
           case 8: return MTYPE_F8;
           case 10: return MTYPE_F10;
           case 16: return MTYPE_FQ;
        }
  default: FmtAssert(FALSE, ("unexpected rclass"));
  	return MTYPE_UNKNOWN;
  }
}
