#define TARGET_CPU_DEFAULT ("7450")
#include "auto-host.h"
#ifdef IN_GCC
/* Provide three core typedefs used by everything, if we are compiling
   GCC.  These used to be found in rtl.h and tree.h, but this is no
   longer practical.  Providing these here rather that system.h allows
   the typedefs to be used everywhere within GCC. */
struct rtx_def;
typedef struct rtx_def *rtx;
struct rtvec_def;
typedef struct rtvec_def *rtvec;
union tree_node;
typedef union tree_node *tree;
#endif
#define GTY(x)
#ifdef IN_GCC
# include "ansidecl.h"
# include "rs6000/rs6000.h"
# include "dbxelf.h"
# include "elfos.h"
# include "svr4.h"
# include "freebsd-spec.h"
# include "rs6000/sysv4.h"
# include "rs6000/linux.h"
# include "rs6000/altivec-defs.h"
# include "defaults.h"
#endif
#ifndef POSIX
# define POSIX
#endif
#ifndef GENERATOR_FILE
# include "insn-constants.h"
#if (defined _LANGUAGE_C) && (defined _GNU_SOURCE)
# include "insn-flags.h"
#else 
# include "insn-flags-cxx.h" 
#endif
#endif
