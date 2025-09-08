/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*             Xavier Leroy, projet Cristal, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 2001 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#ifndef OCAML_C_CHECKER_PRINTEXC_H
#define OCAML_C_CHECKER_PRINTEXC_H

#include <caml/misc.h>
#include <caml/mlvalues.h>

#ifdef __cplusplus
extern "C" {
#endif

CAMLextern char * caml_format_exception (value);
CAMLnoreturn_start void caml_fatal_uncaught_exception (value) CAMLnoreturn_end;
CAMLnoreturn_start void caml_fatal_uncaught_exception_with_message
  (value, const char *) CAMLnoreturn_end;

#ifdef __cplusplus
}
#endif

#endif /* OCAML_C_CHECKER_PRINTEXC_H */
