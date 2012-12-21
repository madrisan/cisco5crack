/* This file is part of `C5C'.
 * Copyright (C) 2002-2005 by Davide Madrisan <davide.madrisan@gmail.com>

 * This program is free software; you can redistribute it and/or modify it under
 * the terms of version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY, to the extent permitted by law; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if !HAVE_MEMCPY
# define memcpy(d, s, n) bcopy ((s), (d), (n))
#endif
#if !HAVE_MEMSET
extern void *memset(void *s, int c, int n);
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include "mem.h"
#include "tty.h"

static void mem_error(void) {
     tty_error(ERR_OUTOFMEM, "the system is out of memory");
}

typedef struct s_mem_block {
     void *data;
     struct s_mem_block *next;
} mem_block;

static mem_block *mem_head = NULL;

void *
mem_alloc(size_t size)
{
     void *res;
     mem_block *mblock = NULL;

     if (!size)
          return NULL;

     if (!(res = malloc(size)))
          mem_error();
     if (!(mblock = malloc(sizeof(mem_block))))
          mem_error();
     mblock->data = res;
     if (mem_head)              /* non void list of memory blocks */
          mblock->next = mem_head;
     else
          mblock->next = NULL;
     mem_head = mblock;

     return res;
}

void *
zmem_alloc(size_t size)
{
     void *res;
     mem_block *mblock = NULL;

     if (!size)
          return NULL;
     /* optimisation note: standard unix mmap using /dev/zero clears memory so
      * calloc doesn't need to; GNU libc keeps track of this
      */
     if (!(res = calloc(size, sizeof(char))))
          mem_error();
     if (!(mblock = malloc(sizeof(mem_block))))
          mem_error();
     mblock->data = res;
     if (mem_head)              /* non void list of memory blocks */
          mblock->next = mem_head;
     else
          mblock->next = NULL;
     mem_head = mblock;

     return res;
}

void *
mem_realloc(void *ptr, size_t size)
{
     void *res;
     mem_block *mblock;

     if (!size)
          return NULL;

     if (!(res = realloc(ptr, size)))
          mem_error();

     /* the first time, the memory is cleared
      * (to prevent errors when `strcat' is used)
      */
     if (!ptr)
          memset(res, 0, size);

     if (mem_head) {
          for (mblock = mem_head; mblock && mblock->data != ptr;
               mblock = mblock->next);
          if (mblock)           /* `mblock' with `ptr' as `data' found */
               mblock->data = res;
     } else {
          /* memory block info not found */
          if (!(mblock = malloc(sizeof(mem_block))))
               mem_error();

          mblock->data = res;
          mblock->next = NULL;
          mem_head = mblock;
     }
     return res;
}

void
mem_free(void *ptr)
{
     mem_block *mblock, *prev_mblock;

     if (ptr) {
          mblock = prev_mblock = mem_head;
          while (mblock && mblock->data != ptr) {
               prev_mblock = mblock;
               mblock = mblock->next;
          }
          free(ptr);
          if (mblock) {         /* memory block info found */
               mblock->data = NULL;
               if (mblock == mem_head)
                    mem_head = mblock->next;
               else
                    prev_mblock->next = mblock->next;
               free(mblock);
          }
     }
     ptr = NULL;
}

void *
mem_alloc_copy(size_t size, void *src)
{
     return memcpy(mem_alloc(size), src, size);
}

char *
str_alloc_copy(const char *src)
{
     size_t size;

     if (!src || !*src)
          return zmem_alloc(1);
     size = strlen(src) + 1;
     return (char *) memcpy(mem_alloc(size), src, size);
}

void
mem_free_all(void)
{
     mem_block *mblock, *next_mblock;

     mblock = mem_head;
     while (mblock) {
          next_mblock = mblock->next;
          mem_free(mblock->data);
          mblock = next_mblock;
     }
}
