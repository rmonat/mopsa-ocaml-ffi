/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*          Manuel Serrano and Xavier Leroy, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 2000 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/


#ifndef OCAML_C_CHECKER_CUSTOM_H
#define OCAML_C_CHECKER_CUSTOM_H

#include <caml/mlvalues.h>



struct custom_fixed_length {
  intnat bsize_32;
  intnat bsize_64;
};

struct custom_operations {
  char const *identifier;
  void (*finalize)(value v);
  int (*compare)(value v1, value v2);
  intnat (*hash)(value v);
  void (*serialize)(value v,
                    /*out*/ uintnat * bsize_32 /*size in bytes*/,
                    /*out*/ uintnat * bsize_64 /*size in bytes*/);
  uintnat (*deserialize)(void * dst);
  int (*compare_ext)(value v1, value v2);
  const struct custom_fixed_length* fixed_length;
};

#define custom_finalize_default NULL
#define custom_compare_default NULL
#define custom_hash_default NULL
#define custom_serialize_default NULL
#define custom_deserialize_default NULL
#define custom_compare_ext_default NULL
#define custom_fixed_length_default NULL


struct custom_operations **val_as_custom_operations(value v);

#define Custom_ops_val(v) (*((struct custom_operations **) (val_as_custom_operations(v))))


CAMLextern value caml_alloc_custom(struct custom_operations * ops,
                                   uintnat size, /*size in bytes*/
                                   mlsize_t mem, /*resources consumed*/
                                   mlsize_t max  /*max resources*/);


CAMLextern value caml_alloc_custom_mem(struct custom_operations * ops,
                                       uintnat size, /*size in bytes*/
                                       mlsize_t mem  /*memory consumed*/);


CAMLextern void caml_register_custom_operations(struct custom_operations * ops);


/* Global variable moved to Caml_state in 4.10 */
#define caml_compare_unordered (*_ffi_unimplemented_macro())


#endif // OCAML_C_CHECKER_CUSTOM_H
