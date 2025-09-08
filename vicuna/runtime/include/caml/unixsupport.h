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

#ifndef OCAML_C_CHECKER_UNIX_SUPPORT_H
#define OCAML_C_CHECKER_UNIX_SUPPORT_H

#include <caml/mlvalues.h>
#include <unistd.h>

#define Nothing ((value) _ffi_generate_value())


extern value caml_unix_error_of_code (int errcode);
extern int caml_unix_code_of_unix_error (value error);

CAMLnoreturn_start
extern void caml_unix_error (int errcode, const char * cmdname, value arg)
CAMLnoreturn_end;

CAMLnoreturn_start
extern void caml_uerror (const char * cmdname, value arg)
CAMLnoreturn_end;


#define unix_error_of_code caml_unix_error_of_code
#define code_of_unix_error caml_unix_code_of_unix_error

#define uerror caml_uerror
#define unix_error caml_unix_error


extern void caml_unix_check_path(value path, const char * cmdname);

#define UNIX_BUFFER_SIZE 65536

// FFI checker primitive
void **val_as_dir(value v);
#define DIR_Val(v) (*((DIR**)val_as_dir(v)))

extern char ** cstringvect(value arg, char * cmdname);
extern void cstringvect_free(char **);

extern int unix_cloexec_default;
extern int unix_cloexec_p(value cloexec);
extern void unix_set_cloexec(int fd, char * cmdname, value arg);
extern void unix_clear_cloexec(int fd, char * cmdname, value arg);




#endif
