/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*          Xavier Leroy and Damien Doligez, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1996 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#ifndef OCAML_C_CHECKER_MISC_C
#define OCAML_C_CHECKER_MISC_C

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>


#define CAMLexport
#define CAMLprim
#define CAMLextern extern

#define CAMLweakdef __attribute__((weak))

#define CAMLnoreturn_start
#define CAMLnoreturn_end __attribute__ ((noreturn))
#define Noreturn __attribute__ ((noreturn))

#define caml_copy_string_of_os caml_copy_string

#define CAML_PREPROWARNING(msg)
#define CAML_DEPRECATED(name1,name2)

typedef size_t asize_t;
typedef char char_os;


#define CAMLassert(x) \
  ((x) ? (void) 0 : caml_failed_assert ( #x , __OSFILE__, __LINE__))

CAMLnoreturn_start
CAMLextern void caml_failed_assert (char *, char_os *, int)
CAMLnoreturn_end;

#endif // OCAML_C_CHECKER_MISC_C
