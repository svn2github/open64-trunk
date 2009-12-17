/*
 * Copyright (C) 2009 Advanced Micro Devices, Inc.  All Rights Reserved.
 */

/*

  OpenMP runtime library to be used in conjunction with Open64 Compiler Suites.

  Copyright (C) 2003 - 2009 Tsinghua University.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
  
  Contact information: HPC Institute, Department of Computer Science and Technology,
  Tsinghua University, Beijing 100084, CHINA, or:

  http://hpc.cs.tsinghua.edu.cn
  
*/

/*
 * File: omp_util.c
 * Abstract: some utilities of the OpenMP Runtime Library
 * History: 04/23/2003, built by Jiang Hongshan, Tsinghua Univ.
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <ctype.h>
#include <unistd.h>
#include "omp_util.h"

void
Not_Valid (char *error_message)
{
  fprintf(stderr, error_message);
  fprintf(stderr, "\n");
  fflush(stderr);
  abort();
}

void
Warning (char *warning_message)
{
  fprintf(stderr, "Warning: %s\n", warning_message);
  fflush(stderr);
}

char *
Trim_Leading_Spaces (char *input)
{
  char *output = input;
  DevWarning( output != NULL, ("input string is NULL!"));

  while ((*output != '\0') && isspace((int)(*output))) {
    output++;
  }

  return output;
}

unsigned long int
Get_System_Stack_Limit (void)
{
  struct rlimit cur_rlimit;
  int return_value;
  return_value = getrlimit(RLIMIT_STACK, &cur_rlimit);
  Is_True(return_value == 0, ("user stack limit setting wrong"));
  return cur_rlimit.rlim_cur;
}

int
Get_SMP_CPU_num (void)
{
  /* Maybe we still want to check it's supported by underlying system? */
  return (int) sysconf(_SC_NPROCESSORS_ONLN);
}

int
Get_CPU_Cores(void)
{
  FILE * fp;
  char buf[256], *data;
  int cores;

  if ((fp = fopen ("/proc/cpuinfo", "r")) != NULL) 
  {
    while (fgets (buf, 256, fp))
    {
      if (!strncasecmp (buf, "cpu cores", 9)) {
        strtok (buf, ":");
        data = strtok (NULL, "\n");
        cores = atoi(data);
        fclose (fp);
        return cores;
      }
    }
    fclose(fp);
  }
  Warning("Could not get cpu cores from cpuinfo\n");
  return 0;
}

/*
 * Check if the user specifies an environment variable to map
 * the core to thread.
 * Note:
 * (1) core_id needs to be in the numactl list if numactl
 *     is used; otherwise, that id will be filtered out later.
 * (2) O64_OMP_SET_AFFINITY needs to be TRUE to make
 *     this mapping effective.
 * 
 * Return:
 *  the number of cores in the list.
 * 
 * !!
 * !! the client needs to free the memory used by list !!
 * !!
 */

#define MAX_LIST_SIZE 4096

int
Get_Affinity_Map(int **list, int total_cores)
{
  char *data, *str;
  int list_size;
  int buf[MAX_LIST_SIZE], *my_list;
  int cnt, i;

  if ( (data = getenv("O64_OMP_AFFINITY_MAP")) == NULL)
    return 0;

  cnt = 0;
  str = strtok(data,", ");
  while (str && cnt<MAX_LIST_SIZE)
  {
    int core_id = atoi(str);
    // some sanity checks on core_id. Note that atoi() can return 0 when failed.
    if (core_id == 0 && str[0] != '0')
      fprintf(stderr,"O64_OMP_AFFINITY_MAP: ignored invalid core_id=%s.\n", str);
    else if ( core_id < 0 || core_id >= total_cores )
      fprintf(stderr,"O64_OMP_AFFINITY_MAP: ignored invalid core_id=%d.\n", core_id);
    else
      buf[cnt++] = atoi(str);
    str = strtok(NULL,", \n");
  }
  
  if (cnt == 0)
    return 0;

  if (cnt == MAX_LIST_SIZE)
      fprintf(stderr,
           "O64_OMP_AFFINITY_MAP: map is too big, ignore items after %dth.\n", 
           MAX_LIST_SIZE);
  my_list = (int*) malloc(sizeof(int) * cnt);
  Is_True(my_list!= NULL, ("Can't allocate list in Get_Affinity_Map"));

  memcpy(my_list, buf, sizeof(int)*cnt);

  *list = my_list;

  return cnt;
}

#define SET_DEFAULT {  \
        for (i=0; i<total_cores; i++)\
           list[i] = i; \
        if (fp != NULL) fclose(fp); \
        return; }

void
Get_Ordered_Corelist(int *list, int total_cores)
{
  FILE * fp;
  char buf[256], *data;
  int proc_id, proc_done=0;
  int core_id = -1, socket_id = -1, cores = 0;   
  int i;

  // could not find /proc/cpuinfo  
  if ((fp = fopen ("/proc/cpuinfo", "r")) == NULL) SET_DEFAULT;
  while (fgets (buf, 256, fp))
  {
    if (!strncasecmp (buf, "cpu cores", 9)) {
      strtok (buf, ":");
      data = strtok (NULL, "\n");
      cores = atoi(data); 
      break;  
    }
  }
  
  // illegal cpu cores
  if (cores == 0 || cores > total_cores) SET_DEFAULT;

  rewind(fp);
  while (fgets (buf, 256, fp))
  {
    if (!strncasecmp (buf, "processor", 9)) 
    {
       strtok (buf, ":");
       data = strtok (NULL, "\n");
       proc_id = atoi(data);
       // illegal proc_id
       if (proc_id >= total_cores) SET_DEFAULT;

       while (fgets(buf, 256, fp))
       {
         if(!strncasecmp (buf, "physical id", 11)) 
         {
           strtok (buf, ":");
           data = strtok (NULL, "\n");
           socket_id = atoi(data);
         }
         if(!strncasecmp (buf, "core id", 7)) 
         {
           strtok (buf, ":");
           data = strtok (NULL, "\n");
           core_id = atoi(data);
         }

         if((socket_id >= 0) && (core_id >= 0)) 
         {
           // illegal socket_id or cores_id
           if (socket_id * cores + core_id >= total_cores)
             SET_DEFAULT;

           list[socket_id * cores + core_id] = proc_id;
           socket_id = -1;
           core_id = -1;
           proc_done ++;
           break;
         }
      }
    }
  } 
  
  if (proc_done != total_cores) SET_DEFAULT;

  fclose(fp);  
}

/*
 * This routine returns an piece of allocated memory with the alignment
 * specified by the caller.
 *
 * It uses system's malloc to allocate memory.  The return address will be
 * adjusted according to the alignment requirement. The adjusted size
 * is stored to the allocated memory, right before the return address.
 *
 * Argument "alignment" must be power of 2. This routine will
 * check this and will return NULL if otherwise.
 *
 * If argument "bytes" is 0, it will return NULL. The adjusted size
 * will not be stored.
 *
 * To free the memory, aligned_free() needs to be used. Using
 * free() leads to an unmatched deallocation.
 *
 */

void* aligned_malloc(size_t bytes, size_t alignment)
{
  size_t real_alignment;   // the real alignment used in this call
  size_t real_size;        // the real allocated memory size
  size_t alignment_mask;   // the mask bits to compute the alignment
  size_t adj_size;         // the adjusted size that stored to memory
  unsigned long p;         // address returned from malloc()
  unsigned long ret_p;     // address returned to caller
  size_t sizet;            // the size of type size_t

  if (bytes == 0)
    return NULL;

  /*
   *  First make sure the argument "alignment" is indeed power of 2.
   */
  if (alignment & (alignment-1))
    return NULL;

  /*
   * We store the adj_size to the allocated memory. We adjust the
   * alignment so that the access to adj_size is not an unaligned
   * access.
   */
  sizet = sizeof(size_t);
  real_alignment = alignment < sizet ? sizet : alignment;

  alignment_mask = ~(real_alignment - 1);

  /*
   * We can save some space if we know the minimal alignment of
   * system's malloc. Here we assume we don't know the minimal
   * alignment.
   */

  // sanity check to detect overflow.
  if (bytes > (size_t)-1 - sizet - real_alignment-1)
    return NULL;

  real_size = bytes + real_alignment-1 + sizet;

  // real allocation
  if ((p = (unsigned long) malloc(real_size)) == 0)
    return NULL;

  // adjust the address according to the alignment requirement
  ret_p = (p + sizet + real_alignment - 1) & alignment_mask;
  adj_size = ret_p - p;

  // record the adj_size right before the returned address.
  ((size_t*)ret_p)[-1] = adj_size;

  return (void*) ret_p;
}


/*
 * This routine free the memory allocated by aligned_malloc().
 *
 * Using this routine to a address directly from malloc() will cause
 * program errors.
 */

void aligned_free(void* p)
{
  size_t adj_size;         // the adjusted size that stored
  unsigned long real_p;    // the real address of the allocated memory

  if (p == NULL)
    return;

  adj_size = ((size_t*)p)[-1];
  real_p = (unsigned long) p - adj_size;

  free((void*) real_p);
}

void* aligned_realloc(void *p, size_t old_size, size_t new_size, size_t alignment)
{
    void * ret_p = NULL;
    
    if (new_size != 0)
    {
        ret_p = aligned_malloc(new_size, alignment);
        Is_True(ret_p != NULL, ("Can not aligned malloc"));
    }
    
    // copy the data to the new address 
    if (p != NULL)
    {
        memcpy(ret_p, p, (old_size >= new_size) ? new_size : old_size);
        aligned_free (p);
    }
    
    return ret_p;
          
}

void
__ompc_do_nothing (void)
{
}

