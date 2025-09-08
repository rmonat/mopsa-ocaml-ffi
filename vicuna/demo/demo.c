/******************************************************************************/
/*                                  Vicuna                                    */
/*                  Simon Spies and Greta Yorsh, Jane Street                  */
/* -------------------------------------------------------------------------- */
/*                               MIT License                                  */
/*                                                                            */
/* Copyright (c) 2025 Jane Street Group LLC                                   */
/* opensource-contacts@janestreet.com                                         */
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included    */
/* in all copies or substantial portions of the Software.                     */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */
/******************************************************************************/

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <string.h>

/*
 * Motivating Example
 *
 * This example compares two implementations of [make_ref]. The first one is
 * correct and the second one forgets to root the value [v].
 */
value demo_make_ref(value v) {
  CAMLparam1(v);
  value r = caml_alloc(1, 0);
  Store_field(r, 0, v);
  CAMLreturn(r);
}

value demo_make_ref_broken(value v) {
  value r = caml_alloc(1, 0);
  Store_field(r, 0, v);
  return r;
}

/*
 * Arity Checks
 *
 * The following functions are declared with different signatures in OCaml
 * than the ones they have here in C. In particular, one must distinguish
 * between [unit] and [void].
 */

void demo_returns_void(value v) { return; }

value demo_takes_unit() { return Val_unit; }

/*
 * Type Checks
 *
 * The following functions mix up types.
 */

// this function uses [Long_val] on a string
value demo_takes_string(value v_str) {
  // fine with the arity one,
  // not okay with the types check
  Long_val(v_str);

  return v_str;
}

// this function uses [Long_val] on a string
value demo_takes_string_record(value v_rec) {
  value v_str = Field(v_rec, 6);

  // this access is not okay, because [v_str] is a string
  Long_val(v_str);
  return Val_unit;
}

value demo_nested_shapes(value v) {
  value w = Field(v, 0);
  value u = Field(v, 1);
  value a = Field(u, 0);
  value b = Field(u, 42); // unsoundness: the field 42 is not defined, but the
                          // analysis does not know that
  value x = Field(v, 2);

  Long_val(a);

  Long_val(x); // soundness: this access succeeds because it is "plausible" that
               // [x] may be an int

  // this access fails, because [x] is known to not be a string
  String_val(x);
  return Val_unit;
}

// freshly allocated blocks allow storing any value in them
value demo_assign_freshly_allocated(value v) {
  CAMLparam1(v);
  value x = caml_alloc(1, 0);
  Store_field(x, 0, Val_long(1));
  Store_field(x, 0, Field(v, 0));

  CAMLreturn(Val_unit);
}

/*
 * Runtime Lock Checks
 *
 * The following functions release the runtime lock and
 * then perform operations that require the runtime lock.
 */

// inside a blocking section, we may not use OCaml values that are not integers
value demo_release_lock(value v_str) {
  CAMLparam1(v_str);
  caml_enter_blocking_section();
  const char *ptr = String_val(v_str);
  caml_leave_blocking_section();
  CAMLreturn(v_str);
}

// raising an exception requires the runtime lock
value demo_release_lock_failwith(value v_unit) {
  caml_enter_blocking_section();
  caml_failwith("Error occurred.");
  caml_leave_blocking_section();
  return Val_unit;
}

/*
 * Garbage Collection Issues
 *
 * The following functions showcase interesting interactions with garbage
 * collection.
 */

// this function forgets to root the string [v_str]
value demo_forget_rooting(value v_str) {

  value x = caml_alloc(1, 1);
  const char *ptr = String_val(v_str);
  return v_str;
}

// this function forgets to root the string [v_str]
// then it passes it to an external function [strdup]
value demo_forget_rooting_advanced(value v_str) {

  const char *str = String_val(v_str);
  value x = caml_alloc(1, 1);
  strdup(str);

  return v_str;
}

// the following function works just fine
value demo_with_rooting(value v_str) {
  CAMLparam1(v_str);

  value x = caml_alloc(1, 1);
  strdup(String_val(v_str));

  CAMLreturn(v_str);
}

/*
 * Named Values
 *
 * The following functions showcase examples using OCaml's named value feature.
 */

value demo_caml_named_value(value v_unit) {
  const value *p = caml_named_value("asdf");
  value q = *p;
  printf("%s", String_val(q));

  caml_alloc_small(42, 0);

  value r = *p;
  printf("%s", String_val(r));

  return Val_unit;
}

value demo_caml_named_value_two(value v_unit) {
  const value *ptr = caml_named_value("my_value");
  caml_alloc(1, 0);
  String_val(*ptr);
  return Val_unit;
}

/*
 * Limitations
 *
 * The following functions showcase limitations of the analysis.
 */

// the following function demonstrates that the analysis loses track through
// dynamically allocated memory
value demo_dynamic_allocation_imprecision(value v_unit) {

  value *ptr = malloc(sizeof(value));
  *ptr = Val_long(42);
  Long_val(*ptr);

  return Val_unit;
}

// Vicuna does not track whether record fields are mutable or immutable.
// Hence, it allows the following example.
value demo_assign_shapes_unsound(value v) {
  Store_field(v, 4, Val_true); // a bool, but not assignable
  return Val_unit;
}

// the following code "works" in principle, but is not picked up by the analysis
// as such
value demo_manually_tagged(value v_unit) {

  char *ptr = malloc(42);
  ptr[0] = '0';
  return ((value)ptr) | 1;
}

// the following function uses a currently unsupported function
value demo_unimplemented_feature(value v_unit) { return caml_alloc_float_array(10); }

// this function illustrates a tradeoff between soundness and avoiding false
// positives
value demo_variant_example(value v) {

  if (Is_block(v)) {
    // issue: inside here we do not know that v is guaranteed to be a block
    // the analysis loses track of this information, which is why we opted for
    // making it less sound
    value ref = Field(v, 0);
    Store_field(ref, 0, Val_long(41));
  }

  return Val_unit;
}

// custom block example
typedef struct example_block {
  int a;
  char name[20];
  char *off_heap_ptr;
} example_block_t;

#define Example_block_val(v) (*((example_block_t **)(Data_custom_val(v))))

value demo_alloc_custom_block(value v_unit) {

  example_block_t *ptr = malloc(sizeof(example_block_t));
  ptr->a = 42;
  ptr->name[0] = 0;
  ptr->off_heap_ptr = NULL;

  value blk = caml_alloc_custom(NULL, sizeof(example_block_t *), 0, 0);
  Example_block_val(blk) = ptr;
  return blk;
}

value demo_alloc_custom_block_bad(value v_unit) {

  example_block_t *ptr = malloc(sizeof(example_block_t));
  ptr->a = 42;
  ptr->name[0] = 0;
  ptr->off_heap_ptr = NULL;

  value blk = caml_alloc_custom(NULL, sizeof(example_block_t *), 0, 0);
  example_block_t **rt_ptr = &Example_block_val(blk);
  caml_alloc(1, 0);
  *rt_ptr = ptr;
  return blk;
}

value demo_update_custom_block(value v) {
  example_block_t *ptr = Example_block_val(v);
  ptr->a = 43;
  ptr->name[0] = 0;
  strncpy(ptr->name, "Hello World!", 20);
  return Val_unit;
}

// allocation tag sensitivity
value demo_alloc_tag_any(value v_int) {
  tag_t tag = Val_long(v_int);
  value v = caml_alloc(2, tag);
  Store_field(v, 0, Val_long(42));
  Store_field(v, 1, Val_long(43));
  return v;
}

value demo_alloc_tag_int_pair(value unit) {
  // if we are supposed to allocate an int pair, we cannot allocate a double,
  // so this fails; it succeeds with tag 0
  value v = caml_alloc(2, Double_tag);
  Store_field(v, 0, Val_long(42));
  Store_field(v, 1, Val_long(43));
  return v;
}
