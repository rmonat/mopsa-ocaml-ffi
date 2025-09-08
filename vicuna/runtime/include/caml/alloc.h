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

#ifndef OCAML_C_CHECKER_ALLOC_H
#define OCAML_C_CHECKER_ALLOC_H

#include <caml/mlvalues.h>
#include <caml/misc.h>

// allocation operations
value caml_alloc(mlsize_t len, tag_t tag);
value caml_alloc_small(mlsize_t len, tag_t tag);
value caml_alloc_tuple (mlsize_t wosize);
value caml_alloc_float_array (mlsize_t len);
value caml_alloc_string(mlsize_t len);
value caml_alloc_initialized_string(mlsize_t len, const char * str);
value caml_alloc_local_string (mlsize_t len);
value caml_copy_int64(int64_t x);
value caml_copy_int32(int x);
value caml_copy_nativeint(intnat x);
value caml_copy_double(double x);
value caml_copy_string(const char *str);
value caml_copy_string_array (char const ** x);
value caml_alloc_sprintf(const char * format, ...);

value caml_alloc_some(value x);

typedef void (*final_fun)(value);
CAMLextern value caml_alloc_final (mlsize_t wosize,
                                   final_fun, /*finalization function*/
                                   mlsize_t, /*resources consumed*/
                                   mlsize_t  /*max resources*/);


int caml_convert_flag_list(value list, int *flags);

#endif // OCAML_C_CHECKER_ALLOC_H
