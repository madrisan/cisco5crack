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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
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
#ifdef EGD_SUPPORT
# if ! defined(HAVE_STRUCT_SOCKADDR_UN_SUN_PATH)
# error Non standard socketaddr_un structure.  Please disable EGD support.
# endif
# if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif
# if HAVE_SYS_UN_H
#  include <sys/un.h>
# endif
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "entropy.h"
#include "main.h"
#include "mem.h"
#if defined (EGD_SUPPORT) || defined (DEV_RANDOM_SUPPORT)
# include "params.h"
#endif
#include "tty.h"
#ifdef EGD_SUPPORT
# ifndef offset_of
#  define offset_of(type, member) ((size_t) &((type *)0)->member)
# endif
static int rndegd_connect_socket(char *egd_path);
static int rndegd_write(int fd, u_char * buf, size_t nbytes);
static int rndegd_read(int fd, u_char * buf, size_t nbytes);
#endif

extern t_options usr_opt;

int
get_sys_entropy(u_char * buf, size_t nbytes)
{
     u_int i;

     if (usr_opt.verbose > 1)
          tty_message(
               "using as random source the ANSI rand() function...\n");
     srand((u_int) time(NULL));

     /* to use higher-order bits of pseudo-random number */
     for (i = 0; i < nbytes; i++)
          buf[i] = (u_char) ((1. * rand() * 255) / RAND_MAX);

     return nbytes;
}

#ifdef DEV_RANDOM_SUPPORT
/* if your Linux system does not have `/dev/random' created already,
 * it can be created with the following commands:
 * mknod -m 644 /dev/random c 1 8
 * chown root:root /dev/random
 */
int
get_dev_entropy(u_char * buf, size_t nbytes)
{
     int random_dev, n, nread = 0;

     if (usr_opt.verbose > 1)
          tty_message(
               "using as random source the device %s...\n", RND_DEV);

     random_dev = open(RND_DEV, O_RDONLY | O_NONBLOCK);
     if (random_dev != -1) {
          while (nbytes) {
               do
                    n = read(random_dev, (u_char *) buf + nread, nbytes);
               while (n == -1 && errno == EAGAIN);
               if (n <= 0) {
                    close(random_dev);
                    /* `rand()' function call is not a very good source
                     * of randomness
                     */
                    tty_warning("cannot use %s", RND_DEV);
                    return nread;
               }
               nread += n;
               nbytes -= n;
          }
          close(random_dev);
     }

     return nread;
}
#endif

#ifdef EGD_SUPPORT
/* you can download the EGD software (perl script) and documentation at
 * <http://sourceforge.net/projects/egd/>
 */
static int egd_socket = -1;

static int
rndegd_connect_socket(char *egd_path)
{
     int addr_len, fd;
     struct sockaddr_un addr;

     assert(strlen(EGD_SOCKET_PATH) < sizeof addr.sun_path);

     if (egd_socket != -1) {
          close(egd_socket);
          egd_socket = -1;
     }

     memset(&addr, 0, sizeof addr);
     addr.sun_family = AF_UNIX;

     if (egd_path && strlen(egd_path) >= sizeof addr.sun_path)
          tty_error(ERR_USAGE, "EGD socketname is too long");

     strcpy(addr.sun_path, egd_path ? egd_path : EGD_SOCKET_PATH);
     addr_len =
          offset_of(struct sockaddr_un, sun_path) + strlen(addr.sun_path);

     fd = socket(AF_UNIX, SOCK_STREAM, 0);
     if (fd == -1) {
          tty_warning("cannot create unix domain socket: %s",
                      strerror(errno));
     } else if (connect(fd, (struct sockaddr *) &addr, addr_len) == -1) {
          tty_warning("cannot connect to `%s': %s",
                      addr.sun_path, strerror(errno));
          close(fd);
          fd = -1;
     }

     return egd_socket = fd;
}

static int
rndegd_write(int fd, u_char * buf, size_t nbytes)
{
     size_t nleft = nbytes;
     int nwritten;

     while (nleft > 0) {
          nwritten = write(fd, buf, nleft);
          if (nwritten < 0) {
               if (errno == EINTR)
                    continue;
               return -1;
          }
          nleft -= nwritten;
          buf += nwritten;
     }
     return 0;
}

static int
rndegd_read(int fd, u_char * buf, size_t nbytes)
{
     int n, nread = 0;

     while (nbytes) {
          do {
               n = read(fd, buf + nread, nbytes);
          } while (n == -1 && errno == EINTR);
          if (n == -1)
               return nread ? nread : -1;
          else if (!n) {
               /* EGD probably died. */
               errno = ECONNRESET;
               return -1;
          }
          nread += n;
          nbytes -= n;
     }
     return nread;
}

int
get_egd_entropy(u_char * buf, size_t nbytes)
{
     int fd = egd_socket, nread, do_restart = 0;
     u_char *egd_buf;
     size_t length;

     if (!nbytes)               /* should never happen */
          return 0;

     if (usr_opt.verbose > 1)
          tty_message(("using as random source the EGD socket...\n"));

     length = nbytes < 255 ? nbytes : 255;
     egd_buf = zmem_alloc(length + 2);   /* FIXME: is `+2' mandatory ? */

   restart:
     if (fd == -1 || do_restart)
          fd = rndegd_connect_socket(usr_opt.cryptopt.egd_path);
     if (fd == -1)
          return 0;

     do_restart = 0;

     /* first time we do it with a non blocking request */
     egd_buf[0] = 1;            /* non blocking */
     egd_buf[1] = length;

     if (rndegd_write(fd, egd_buf, 2) == -1) {
          tty_warning("cannot write to the EGD: %s", strerror(errno));
          return 0;
     }

     nread = rndegd_read(fd, egd_buf, 1);
     if (nread == -1) {
          tty_warning("read error on EGD: %s", strerror(errno));
          do_restart = 1;
          goto restart;
     }

     if ((nread = egd_buf[0])) {
          nread = rndegd_read(fd, egd_buf, nread);
          if (nread == -1) {
               tty_warning("read error on EGD: %s", strerror(errno));
               do_restart = 1;
               goto restart;
          }
          /* ... */
          length -= nread;
     }
     if (length)
          tty_warning("not enought entropy gathered using EGD, "
                      "please do some work...");

     while (length) {
          egd_buf[0] = 2;       /* blocking */
          egd_buf[1] = nbytes;
          if (rndegd_write(fd, egd_buf, 2) == -1) {
               tty_warning("cannot write to the EGD: %s", strerror(errno));
               return 0;
          }

          nread = rndegd_read(fd, egd_buf, 1);
          if (nread == -1) {
               tty_warning("read error on EGD: %s", strerror(errno));
               do_restart = 1;
               goto restart;
          }
          /* ... */
          length -= nread;
     }

     memcpy(buf, egd_buf, nbytes);

  /* memset(egd_buf, 0, sizeof(egd_buf)); */
     mem_free(egd_buf);
     close(egd_socket);
     egd_socket = -1;

     return nbytes;
}
#endif
