/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*             Xavier Leroy, projet Cristal, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1996 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#ifndef OCAML_C_CHECKER_FAIL_H
#define OCAML_C_CHECKER_FAIL_H

#include <caml/misc.h>
#include <caml/mlvalues.h>


CAMLnoreturn_start
CAMLextern void caml_raise (value bucket)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_constant (value tag)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_with_arg (value tag, value arg)
CAMLnoreturn_end;

// TODO: not supported yet
// CAMLnoreturn_start
// CAMLextern void caml_raise_with_args (value tag, int nargs, value arg[])
// CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_with_string (value tag, char const * msg)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_failwith (char const *msg)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_failwith_value (value msg)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_invalid_argument (char const *msg)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_invalid_argument_value (value msg)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_out_of_memory (void)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_stack_overflow (void)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_sys_error (value)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_end_of_file (void)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_zero_divide (void)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_not_found (void)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_array_bound_error (void)
CAMLnoreturn_end;

CAMLnoreturn_start
CAMLextern void caml_raise_sys_blocked_io (void)
CAMLnoreturn_end;


#endif // OCAML_C_CHECKER_FAIL_H
