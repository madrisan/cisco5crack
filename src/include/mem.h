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

#ifndef _MEMMNG_H
#define _MEMMNG_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

/*
 * #define zmem_alloc(ptr, size) do { \
 *     ptr = mem_alloc(size);     \
 *     memset(ptr, 0, size);      \
 * } while ( 0 )
 */

void *mem_alloc(size_t size);
void *zmem_alloc(size_t size);
void *mem_realloc(void *ptr, size_t size);
void mem_free(void *ptr);

/* uses `mem_alloc()' to allocate the memory, and copies `src' in there
 */
void *mem_alloc_copy(size_t size, void *src);

/* similar to the above function, but for ASCIIZ strings
 */
char *str_alloc_copy(const char *src);

/* free all the heap memory dinamically allocated
 */
void mem_free_all(void);

#endif
