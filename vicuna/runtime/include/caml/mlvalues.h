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

#ifndef OCAML_C_CHECKER_VALUES_H
#define OCAML_C_CHECKER_VALUES_H

#include <caml/misc.h>
#include <caml/ffi.h>



// OCAML int definitions
typedef long int intnat;
typedef unsigned long int uintnat;
typedef long int int64_t;
typedef unsigned long int mlsize_t;
typedef unsigned int tag_t;


// OCaml values
typedef ffi_value value;

long int val_as_long(value v);
value val_of_long(long int x);
int64_t val_as_unboxed_int64(value v);
int val_as_unboxed_int32(value v);
double val_as_unboxed_double(value v);
intnat val_as_unboxed_nativeint(value v);

/* TODO: Functions defined here should start with a special prefix that is not
   "caml_" to distinuish them from functions defined in the compiler distribution. For
   example, consider using "ffi_field" instead of "caml_field" and "ffi_long_val" instead
   "val_as_long".  Note that these names can appear in user error messages from the
   checker, as part of the call stack, so we need a clear naming convention.
 */
value *caml_field(value v, mlsize_t i);
void caml_set_field(value v, mlsize_t i, value vi);
int caml_is_block(value v);
int caml_is_long(value v);
mlsize_t caml_block_size(value v);
unsigned char caml_block_tag(value v);


const char *val_as_string(value v);
unsigned char *val_as_bytes(value v);
CAMLextern mlsize_t caml_string_length (value);   /* size in bytes */
CAMLextern int caml_string_is_c_safe (value);

void caml_store_double_idx(value v, mlsize_t i, double d);
double *caml_get_double_idx(value v, mlsize_t i);


/* TODO: consider renaming val_of_long / val_as_long
   to something more standard*/
/* Conversion macro names are always of the form  "to_from". */
/* Example: Val_long as in "Val from long" or "Val of long". */
#define Val_long(x) (val_of_long(x))
#define Long_val(x) (val_as_long(x))
#define Max_long (((intnat)1 << (8 * sizeof(value) - 2)) - 1)
#define Min_long (-((intnat)1 << (8 * sizeof(value) - 2)))

#define Val_int(x) Val_long(x)
#define Int_val(x) ((int) Long_val(x))
#define Unsigned_long_val(x) ((uintnat)Long_val(x))
#define Unsigned_int_val(x)  ((int) Unsigned_long_val(x))


#define Val_bool(x) Val_long(x != 0)
#define Bool_val(x) Long_val(x)
#define Val_false Val_int(0)
#define Val_true Val_int(1)
#define Val_not(x) Val_long((Bool_val(x) == 0))
#define Val_unit Val_long(0)

// int64 and int32
#define Int64_val(x) (val_as_unboxed_int64(x))
#define Int32_val(x) (val_as_unboxed_int32(x))


// double
#define Double_tag 253
#define Double_wosize ((sizeof(double) / sizeof(value)))
#define Double_val(x) (val_as_unboxed_double(x))

// Additional tags. Currently [Lazy_tag], [Cont_tag], [Forcing_tag], and [Forward_tag]
// have no special support. They are only defined here.
/* This tag is used (with Forcing_tag & Forward_tag) to implement lazy values.
   See major_gc.c and stdlib/lazy.ml. */
#define Lazy_tag 246

/* Tag used for continuations (see fiber.c) */
#define Cont_tag 245

/* This tag is used (with Lazy_tag & Forward_tag) to implement lazy values.
 * See major_gc.c and stdlib/lazy.ml. */
#define Forcing_tag 244

/* Forward_tag: forwarding pointer that the GC may silently shortcut.
   See stdlib/lazy.ml. */
#define Forward_tag 250
#define Forward_val(v) Field(v, 0)

// strings
#define String_tag 252
#define String_val(x) (val_as_string(x))
#define Bytes_val(x)  (val_as_bytes(x))

#define Bp_val(v) ((char *) (val_as_bytes(v)))
#define Val_bp(p) ((value) (_ffi_unimplemented_macro()))
#define Byte(x, i) (((char *) (val_as_bytes(x))) [i])            /* Also an l-value. */
#define Byte_u(x, i) (((unsigned char *) (val_as_bytes(x))) [i]) /* Also an l-value. */


// records and pairs
#define Field(v, i) *caml_field(v, i)
#define Op_val(x) (caml_field(x, 0)) // FIXME: does this work?
#define Is_block(v) caml_is_block(v)
#define Is_long(v)  caml_is_long(v)
#define Wosize_val(v) caml_block_size(v)
// #define Tag_val(v) caml_block_tag(v) // FIXME:  also an l-value


/* List constructors */
#define Val_emptylist Val_int(0)
#define Tag_cons 0

/* Option constructors */
#define Val_none Val_int(0)
#define Some_val(v) Field(v, 0)
#define Tag_some 0
#define Is_none(v) ((v) == Val_none)
#define Is_some(v) Is_block(v)


// Float Arrays
#define Double_array_tag 254
#define Double_field(v,i) *caml_get_double_idx(v, i)
#define Store_double_field(v, i, d) caml_store_double_idx(v, i, d)

#define Double_flat_field(v,i) *caml_get_double_idx(v, i)
#define Store_double_flat_field(v,i,d) caml_store_double_idx(v, i, d)


/* header macros */
typedef uintnat header_t;
#define Tag_hd(hd) ((tag_t) ((hd) & 0xFF))

header_t *caml_header_pointer(value v);
value caml_value_of_header_pointer(header_t *ptr);


#define Hd_val(val) (*caml_header_pointer(val))        /* Also an l-value. */
#define Hd_op(op) (Hd_val (op))                        /* Also an l-value. */
#define Hd_bp(bp) (Hd_val (bp))                        /* Also an l-value. */
#define Hd_hp(hp) (* ((header_t *) (hp)))              /* Also an l-value. */
#define Hp_val(val) (caml_header_pointer(val))
#define Hp_op(op) (Hp_val (op))
#define Hp_bp(bp) (Hp_val (bp))
#define Val_op(op) ((value) (op))
#define Val_hp(hp) ((value) (caml_value_of_header_pointer(hp)))
#define Op_hp(hp) ((value *) Val_hp (hp))
#define Bp_hp(hp) ((char *) Val_hp (hp))


#ifdef ARCH_BIG_ENDIAN
#define Tag_hp(hp) (((unsigned char *) (hp)) [sizeof(value)-1]) /* Also an l-value. */
#define Tag_val(val) (Tag_hp(Hp_val(val)))
                                                 /* Also an l-value. */
#else
#define Tag_hp(hp) (((unsigned char *) (hp)) [0])
                                                 /* Also an l-value. */
#define Tag_val(val) (Tag_hp(Hp_val(val)))
                                                 /* Also an l-value. */
#endif

#define Nativeint_val(v) val_as_nativeint(v)

/* Custom blocks.  They contain a pointer to a "method suite"
   of functions (for finalization, comparison, hashing, etc)
   followed by raw data.  The contents of custom blocks is not traced by
   the GC; therefore, they must not contain any [value].
   See [custom.h] for operations on method suites. */

void *val_as_custom_block(value v);
void *val_as_abstract_block(value v);

#define Custom_tag 255
#define Data_custom_val(v) ((void *) val_as_custom_block(v))

#define Abstract_tag 251
#define Data_abstract_val(v) ((void*) val_as_abstract_block(v))


/* Encoded exceptional return values, when functions are suffixed with
   _exn. Encoded exceptions are invalid values and must not be seen
   by the garbage collector. */
#define Make_exception_result(v) ((value)((char *)_ffi_unimplemented_macro()))
#define Is_exception_result(v) ((int)((char *)_ffi_unimplemented_macro()))
#define Extract_exception(v) ((value)((char *)_ffi_unimplemented_macro()))


#define Atom(tag) ((value)((char *)_ffi_unimplemented_macro()))


/* Another special case: variants */
CAMLextern value caml_hash_variant(char const * tag);

#endif // OCAML_C_CHECKER_VALUES_H
