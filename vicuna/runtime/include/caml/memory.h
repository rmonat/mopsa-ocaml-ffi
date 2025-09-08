/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*              Damien Doligez, projet Para, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1996 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#ifndef OCAML_C_CHECKER_MEMORY_H
#define OCAML_C_CHECKER_MEMORY_H

#include <caml/mlvalues.h>


// caml modify
void caml_modify(value *v_ptr, value w);

// static allocation
/* [caml_stat_*] functions below provide an interface to the static memory
   manager built into the runtime, which can be used for managing static
   (that is, non-moving) blocks of heap memory.

   Function arguments that have type [caml_stat_block] must always be pointers
   to blocks returned by the [caml_stat_*] functions below. Attempting to use
   these functions on memory blocks allocated by a different memory manager
   (e.g. the one from the C runtime) will cause undefined behaviour.
*/
typedef void* caml_stat_block;

/* [caml_stat_alloc(size)] allocates a memory block of the requested [size]
   (in bytes) and returns a pointer to it. It throws an OCaml exception in case
   the request fails, and so requires the runtime lock to be held.
*/
CAMLextern caml_stat_block caml_stat_alloc(asize_t);

/* [caml_stat_alloc_noexc(size)] allocates a memory block of the requested
   [size] (in bytes) and returns a pointer to it, or NULL in case the request
   fails.
*/
CAMLextern caml_stat_block caml_stat_alloc_noexc(asize_t);


/* [caml_stat_free(block)] deallocates the provided [block]. */
CAMLextern void caml_stat_free(caml_stat_block);

/* [caml_stat_resize(block, size)] changes the size of the provided [block] to
   [size] bytes. The function may move the memory block to a new location (whose
   address is returned by the function). The content of the [block] is preserved
   up to the smaller of the new and old sizes, even if the block is moved to a
   new location. If the new size is larger, the value of the newly allocated
   portion is indeterminate. The function throws an OCaml exception in case the
   request fails, and so requires the runtime lock to be held.
*/
CAMLextern caml_stat_block caml_stat_resize(caml_stat_block, asize_t);


/* [caml_stat_resize_noexc] is a variant of [caml_stat_resize] that returns NULL
   in case the request fails, and doesn't require the runtime lock.
*/
CAMLextern caml_stat_block caml_stat_resize_noexc(caml_stat_block, asize_t);


typedef char* caml_stat_string;
/* [caml_stat_strdup(s)] returns a pointer to a heap-allocated string which is a
   copy of the NULL-terminated string [s]. It throws an OCaml exception in case
   the request fails, and so requires the runtime lock to be held.
*/
CAMLextern caml_stat_string caml_stat_strdup(const char *s);





// the CAML macros
#define CAMLparam0()

#define CAMLparam1(x) _ffi_register_root(&x)

#define CAMLparam2(x, y)                                        \
 _ffi_register_root(&x);                                         \
  _ffi_register_root(&y)

#define CAMLparam3(x, y, z)                                                    \
                                                                \
  _ffi_register_root(&x);                                         \
  _ffi_register_root(&y);                                         \
  _ffi_register_root(&z)

#define CAMLparam4(x, y, z, a)                                                    \
  CAMLparam0();                                                                \
  _ffi_register_root(&x);                                         \
  _ffi_register_root(&y);                                         \
  _ffi_register_root(&z);                                         \
  _ffi_register_root(&a)

#define CAMLparam5(x, y, z, a, b)                                                    \
  CAMLparam0();                                                                \
  _ffi_register_root(&x);                                         \
  _ffi_register_root(&y);                                         \
  _ffi_register_root(&z);                                         \
  _ffi_register_root(&a);                                         \
  _ffi_register_root(&b)


#define CAMLlocal0()

#define CAMLlocal1(x)                                                          \
  value x = Val_unit;                                                          \
  _ffi_register_root(&x)


#define CAMLlocal2(x, y)                                                       \
  value x = Val_unit;                                                          \
  value y = Val_unit;                                                          \
  _ffi_register_root(&x);                   \
  _ffi_register_root(&y)


#define CAMLlocal3(x, y, z)                                                       \
  value x = Val_unit;                                                          \
  value y = Val_unit;                                                          \
  value z = Val_unit;                                                          \
  _ffi_register_root(&x);                   \
  _ffi_register_root(&y);  \
  _ffi_register_root(&z)

#define CAMLlocal4(x, y, z, a)                                                       \
  value x = Val_unit;                                                          \
  value y = Val_unit;                                                          \
  value z = Val_unit;                                                          \
  value a = Val_unit;         \
  _ffi_register_root(&x);                   \
  _ffi_register_root(&y);  \
  _ffi_register_root(&z);  \
  _ffi_register_root(&a)


#define CAMLlocal5(x, y, z, a, b)                                                       \
  value x = Val_unit;                                                          \
  value y = Val_unit;                                                          \
  value z = Val_unit;                                                          \
  value a = Val_unit;         \
  value b = Val_unit;         \
  _ffi_register_root(&x);                   \
  _ffi_register_root(&y);  \
  _ffi_register_root(&z);  \
  _ffi_register_root(&a); \
  _ffi_register_root(&b)

#define CAMLlocalN(x, size) \
  value x[(size)];  \
  (void) _ffi_unimplemented()


#define CAMLxparam1(x) \
  CAMLparam1(x)

#define CAMLxparam2(x, y) \
  CAMLparam2(x, y)

#define CAMLxparam3(x, y, z) \
  CAMLparam3(x, y, z)

#define CAMLxparam4(x, y, z, t) \
  CAMLparam4(x, y, z, t)

#define CAMLxparam5(x, y, z, t, u) \
  CAMLparam4(x, y, z, t, u)

#define CAMLxparamN(x, size) \
  (void) _ffi_unimplemented()






// FIXME: roots allocated like this should be removed again with End_roots
#define Begin_root Begin_roots1

#define Begin_roots1(x) { _ffi_register_root(&x)

#define Begin_roots2(x, y) {                                                      \
  _ffi_register_root(&x);                   \
  _ffi_register_root(&y)


#define Begin_roots3(x, y, z) {                                                       \
  _ffi_register_root(&x);                   \
  _ffi_register_root(&y);  \
  _ffi_register_root(&z)

#define End_roots() }


// FIXME: add check on return
#define CAMLreturn(v)                                                          \
  return v

#define CAMLreturn0                                                          \
  return


#define CAMLreturnT(type, result) do{ \
  type caml__temp_result = (result); \
  return caml__temp_result; \
}while(0)

#define CAMLnoreturn

#define CAMLdrop

// store field
#define Store_field(block, offset, val) do{ \
  mlsize_t caml__temp_offset = (offset); \
  value caml__temp_val = (val); \
  caml_set_field ((block), caml__temp_offset, caml__temp_val); \
} while(0)


struct caml__roots_block {
  struct caml__roots_block *next;
  intnat ntables;
  intnat nitems;
  value *tables [5];
};



value caml_alloc_shr(mlsize_t len, tag_t tag);
value caml_alloc_local(mlsize_t len, tag_t tag);


CAMLextern void caml_register_global_root (value *);

CAMLextern void caml_remove_global_root (value *);

CAMLextern void caml_register_generational_global_root (value *);

CAMLextern void caml_remove_generational_global_root (value *);

CAMLextern void caml_modify_generational_global_root(value *r, value newval);







#endif // OCAML_C_CHECKER_MEMORY_H
