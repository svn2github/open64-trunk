/*
 * Copyright 2004 PathScale, Inc.  All Rights Reserved.
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



#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifdef USE_PCH
#include "be_com_pch.h"
#endif /* USE_PCH */
#pragma hdrstop
#include <sys/types.h>
#include <elf.h>
#include <ctype.h>
#include "wn.h"
#include "wn_map.h"
#include "wn_util.h"
#include <stdio.h>
#include "opt_du.h"
#include "opt_alias_mgr.h"
#include "dep_graph.h"
#include "prompf.h"
#include "ir_reader.h"
#include "wb_util.h"
#include "wb_buffer.h"
#include "wb_carray.h"
#include "wb_browser.h"
#include "wb.h"
#include "wb_ipl.h"

WB_BROWSER wb_ipl; 

// Save Area 

static WB_PHASE WB_Save_Phase; 
WB_BROWSER wb_save; 

extern void WB_IPL_Save()
{
  WB_Save_Phase = WB_Phase(); 
  wb_save.Set_Global_Fd(wb_ipl.Global_Fd());
  wb_save.Set_Du(wb_ipl.Du());
  wb_save.Set_Alias_Manager(wb_ipl.Alias_Mgr());
}

extern void WB_IPL_Initialize(WN* wn_global,
			      DU_MANAGER* du, 
			      ALIAS_MANAGER* am)
{ 
  WB_Set_Phase(WBP_IPL); 
  WB_Initialize(&wb_ipl, wn_global, &Get_Current_PU(), du, am, -1); 
} 

extern void WB_IPL_Set_Access_Array_Map(WN_MAP access_array_map)
{
  wb_ipl.Set_Access_Array_Map(access_array_map);
} 

extern void WB_IPL_Set_Reduction_Map(WN_MAP reduction_map)
{
  wb_ipl.Set_Reduction_Map(reduction_map);
} 

extern void WB_IPL_Set_Scalar_Summary(SUMMARIZE<IPL>* scalar_summary)
{
  wb_ipl.Set_Scalar_Summary(scalar_summary);
} 

extern void WB_IPL_Set_Array_Summary(ARRAY_SUMMARY* array_summary)
{
  wb_ipl.Set_Array_Summary(array_summary);
} 

extern void WB_IPL_Terminate()
{ 
  WB_Set_Phase(WBP_NONE); 
  WB_Terminate(&wb_ipl); 
} 

extern void WB_IPL_Restore()
{
  wb_ipl.Set_Global_Fd(wb_save.Global_Fd());
  wb_ipl.Set_Du(wb_save.Du());
  wb_ipl.Set_Alias_Manager(wb_save.Alias_Mgr());
  WB_Set_Phase(WB_Save_Phase);
}

extern void s_ipl_debug(char init_buffer[])
{ 
  wb_ipl.Sdebug(init_buffer); 
} 
