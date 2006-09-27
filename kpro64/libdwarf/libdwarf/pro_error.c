/*
 * Copyright 2005 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000,2002,2004 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/



#include "config.h"
#include "libdwarfdefs.h"
#ifdef HAVE_ELF_H
#include <elf.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include "pro_incl.h"

extern char *_dwarf_errmsgs[];

/* 
    This function performs error handling as described in the 
    libdwarf consumer document section 3.  Dbg is the Dwarf_P_debug
    structure being processed.  Error is a pointer to the pointer
    to the error descriptor that will be returned.  Errval is an
    error code listed in dwarf_error.h.
*/
void
_dwarf_p_error(Dwarf_P_Debug dbg,
	       Dwarf_Error * error, Dwarf_Word errval)
{
    Dwarf_Error errptr;

    /* Allow NULL dbg on entry, since sometimes that can happen and we
       want to report the upper-level error, not this one. */
    if ((Dwarf_Sword) errval < 0)
	printf("ERROR VALUE: %ld - %s\n",
	       (long) errval, _dwarf_errmsgs[-errval - 1]);
    if (error != NULL) {
	errptr = (Dwarf_Error)
	    _dwarf_p_get_alloc(dbg, sizeof(struct Dwarf_Error_s));
	if (errptr == NULL) {
	    fprintf(stderr,
		    "Could not allocate Dwarf_Error structure\n");
	    abort();
	}
	errptr->er_errval = (Dwarf_Sword) errval;
	*error = errptr;
	return;
    }

    if (dbg != NULL && dbg->de_errhand != NULL) {
	errptr = (Dwarf_Error)
	    _dwarf_p_get_alloc(dbg, sizeof(struct Dwarf_Error_s));
	if (errptr == NULL) {
	    fprintf(stderr,
		    "Could not allocate Dwarf_Error structure\n");
	    abort();
	}
	errptr->er_errval = (Dwarf_Sword) errval;
	dbg->de_errhand(errptr, dbg->de_errarg);
	return;
    }

    abort();
}
