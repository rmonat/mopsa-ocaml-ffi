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

#ifndef OCAML_C_CHECKER_SIGNALS_H
#define OCAML_C_CHECKER_SIGNALS_H

#if defined(CAML_INTERNALS) && defined(POSIX_SIGNALS)
#include<signal.h>
#endif

#ifndef CAML_NAME_SPACE
#include "compatibility.h"
#endif
#include "misc.h"
#include "mlvalues.h"

#ifdef __cplusplus
extern "C" {
#endif

CAMLextern void caml_enter_blocking_section (void);
CAMLextern void caml_leave_blocking_section (void);

// TODO: Support this header.

#ifdef __cplusplus
}
#endif

#endif /* OCAML_C_CHECKER_SIGNALS_H */
