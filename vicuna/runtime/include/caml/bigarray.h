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

#ifndef OCAML_C_CHECKER_BIG_ARRAY_H
#define OCAML_C_CHECKER_BIG_ARRAY_H

#include <caml/mlvalues.h>



typedef signed char caml_ba_int8;
typedef unsigned char caml_ba_uint8;


#define CAML_BA_MAX_NUM_DIMS 16

enum caml_ba_kind {
  CAML_BA_FLOAT32,             /* Single-precision floats */
  CAML_BA_FLOAT64,             /* Double-precision floats */
  CAML_BA_SINT8,               /* Signed 8-bit integers */
  CAML_BA_UINT8,               /* Unsigned 8-bit integers */
  CAML_BA_SINT16,              /* Signed 16-bit integers */
  CAML_BA_UINT16,              /* Unsigned 16-bit integers */
  CAML_BA_INT32,               /* Signed 32-bit integers */
  CAML_BA_INT64,               /* Signed 64-bit integers */
  CAML_BA_CAML_INT,            /* OCaml-style integers (signed 31 or 63 bits) */
  CAML_BA_NATIVE_INT,       /* Platform-native long integers (32 or 64 bits) */
  CAML_BA_COMPLEX32,           /* Single-precision complex */
  CAML_BA_COMPLEX64,           /* Double-precision complex */
  CAML_BA_CHAR,                /* Characters */
  CAML_BA_KIND_MASK = 0xFF     /* Mask for kind in flags field */
};

#define Caml_ba_kind_val(v) Int_val(v)

#define Val_caml_ba_kind(k) Val_int(k)

enum caml_ba_layout {
  CAML_BA_C_LAYOUT = 0,           /* Row major, indices start at 0 */
  CAML_BA_FORTRAN_LAYOUT = 0x100, /* Column major, indices start at 1 */
  CAML_BA_LAYOUT_MASK = 0x100,    /* Mask for layout in flags field */
  CAML_BA_LAYOUT_SHIFT = 8        /* Bit offset of layout flag */
};

#define Caml_ba_layout_val(v) (Int_val(v) << CAML_BA_LAYOUT_SHIFT)

#define Val_caml_ba_layout(l) Val_int(l >> CAML_BA_LAYOUT_SHIFT)

enum caml_ba_managed {
  CAML_BA_EXTERNAL = 0,        /* Data is not allocated by OCaml */
  CAML_BA_MANAGED = 0x200,     /* Data is allocated by OCaml */
  CAML_BA_MAPPED_FILE = 0x400, /* Data is a memory mapped file */
  CAML_BA_MANAGED_MASK = 0x600 /* Mask for "managed" bits in flags field */
};

struct caml_ba_proxy {
  intnat refcount;              /* Reference count */
  void * data;                  /* Pointer to base of actual data */
  uintnat size;                 /* Size of data in bytes (if mapped file) */
};

struct caml_ba_array {
  void * data;                /* Pointer to raw data */
  intnat num_dims;            /* Number of dimensions */
  intnat flags;  /* Kind of element array + memory layout + allocation status */
  struct caml_ba_proxy * proxy; /* The proxy for sub-arrays, or NULL */
  /* PR#5516: use C99's flexible array types if possible */
  intnat dim[]  /*[num_dims]*/; /* Size in each dimension */
};

/* Size of struct caml_ba_array, in bytes, without dummy first dimension */
#define SIZEOF_BA_ARRAY sizeof(struct caml_ba_array)




// ffi implemented primtives
struct caml_ba_array *val_as_big_array(value v);

CAMLextern value caml_ba_alloc(int flags, int num_dims, void * data, intnat * dim);
CAMLextern value caml_ba_alloc_dims(int flags, int num_dims, void * data,
                                 ... /*dimensions, with type intnat */);
CAMLextern uintnat caml_ba_byte_size(struct caml_ba_array * b);
CAMLextern uintnat caml_ba_num_elts(struct caml_ba_array * b);



#define Caml_ba_array_val(v) (val_as_big_array(v))

#define Caml_ba_data_val(v) (Caml_ba_array_val(v)->data)


CAMLextern int caml_ba_element_size[];
CAMLextern void caml_ba_finalize(value v);
CAMLextern int caml_ba_compare(value v1, value v2);
CAMLextern intnat caml_ba_hash(value v);
CAMLextern void caml_ba_serialize(value, uintnat *, uintnat *);
CAMLextern uintnat caml_ba_deserialize(void * dst);


#endif
