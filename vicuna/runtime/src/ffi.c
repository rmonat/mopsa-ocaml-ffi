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
#include <caml/socketaddr.h>
#include <caml/threads.h>
#include <caml/custom.h>
#include <caml/roots.h>
#include <caml/gc.h>
#include <unistd.h>
#include <caml/mopsa_aux.h>



/*
 * Derived Runtime Primitives
 */


value _ffi_generate_value() {
    value f = _mopsa_rand_s64();
    _ffi_mark_active_contents(&f);
    _ffi_set_shape_any(&f);
    return f;
}

// we generate a fresh pointer and make sure it stores a value
value *_ffi_fresh_active_value_ptr(){
    value *f = _ffi_fresh_value_ptr();
    *f = _ffi_generate_value();
    return f;
}

// we diverge to turn the domain into bottom
void _ffi_no_return(){ while (1); }

// We duplicate the function such that it can be used inside of macros
// in a way that still allows one to look at the last entry in the call stack.
// TODO: turn these kinds of things into OCaml for better callstacks in
// error messages.
void *_ffi_unimplemented_macro() {
    return _ffi_unimplemented();
}

void _ffi_assert_immediate(ffi_value v){
    _ffi_assert_shape(v, FFI_IMMEDIATE);
}
void _ffi_assert_block(ffi_value v){
    _ffi_assert_shape(v, FFI_BLOCK);
}
void _ffi_assert_double(ffi_value v){
    _ffi_assert_shape(v, FFI_DOUBLE);
}
void _ffi_assert_int64(ffi_value v){
    _ffi_assert_shape(v, FFI_INT64);
}
void _ffi_assert_int32(ffi_value v){
    _ffi_assert_shape(v, FFI_INT32);
}
void _ffi_assert_nativeint(ffi_value v){
    _ffi_assert_shape(v, FFI_NATIVEINT);
}
void _ffi_assert_string(ffi_value v){
    _ffi_assert_shape(v, FFI_STRING);
}
void _ffi_assert_variant(ffi_value v){
    _ffi_assert_shape(v, FFI_VARIANT);
}
void _ffi_assert_bigarray(ffi_value v){
    _ffi_assert_shape(v, FFI_BIGARRAY);
}
void _ffi_assert_abstract(ffi_value v){
    _ffi_assert_shape(v, FFI_ABSTRACT);
}
void _ffi_assert_custom_block(ffi_value v){
    _ffi_assert_shape(v, FFI_CUSTOM_BLOCK);
}


void _ffi_set_shape_immediate(ffi_value *v){
    _ffi_set_shape(v, FFI_IMMEDIATE);
}
void _ffi_set_shape_block(ffi_value *v){
    _ffi_set_shape(v, FFI_BLOCK);
}
void _ffi_set_shape_double(ffi_value *v){
    _ffi_set_shape(v, FFI_DOUBLE);
}
void _ffi_set_shape_int64(ffi_value *v){
    _ffi_set_shape(v, FFI_INT64);
}
void _ffi_set_shape_int32(ffi_value *v){
    _ffi_set_shape(v, FFI_INT32);
}
void _ffi_set_shape_nativeint(ffi_value *v){
    _ffi_set_shape(v, FFI_NATIVEINT);
}
void _ffi_set_shape_string(ffi_value *v){
    _ffi_set_shape(v, FFI_STRING);
}
void _ffi_set_shape_variant(ffi_value *v){
    _ffi_set_shape(v, FFI_VARIANT);
}
void _ffi_set_shape_bigarray(ffi_value *v){
    _ffi_set_shape(v, FFI_BIGARRAY);
}
void _ffi_set_shape_abstract(ffi_value *v){
    _ffi_set_shape(v, FFI_ABSTRACT);
}
void _ffi_set_shape_custom_block(ffi_value *v){
    _ffi_set_shape(v, FFI_CUSTOM_BLOCK);
}
void _ffi_set_shape_general_block(ffi_value *v, tag_t tag){
    switch (tag) {
      case Double_tag:
        _ffi_set_shape(v, FFI_DOUBLE);
        break;
      case String_tag:
        _ffi_set_shape(v, FFI_STRING);
        break;
      case Custom_tag:
        _ffi_set_shape(v, FFI_CUSTOM_BLOCK);
        _ffi_set_shape(v, FFI_BIGARRAY);
        break;
      case Abstract_tag:
        _ffi_set_shape(v, FFI_ABSTRACT);
        break;
      default:
        _ffi_set_shape(v, FFI_BLOCK);
        break;
    }
}
void _ffi_set_shape_any(ffi_value *v){
    _ffi_set_shape(v, FFI_ANY);
}


/*
 * ML Values
 */


value val_of_long(long int imm) {
    // The correlation between [imm] and [v] is lost.
    value v = _ffi_generate_value();
    _ffi_set_shape_immediate(&v);
    return v;
}

long int val_as_long(value v){
      // The correlation between [imm] and [v] is lost.
    _ffi_assert_immediate(v);
    // NOTE: We add no activeness check for longs.
    // To understand why, see the comment at the top of [ffi.h].
    return _mopsa_rand_s64();
}

int64_t val_as_unboxed_int64(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_int64(v);
    return _mopsa_rand_s64();
}

int val_as_unboxed_int32(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_int32(v);
    return _mopsa_rand_s32();
}

intnat val_as_unboxed_nativeint(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_nativeint(v);
    return _mopsa_rand_s32();
}

double val_as_unboxed_double(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_double(v);
    return _mopsa_rand_double();
}

/*
 * The primitive [caml_field] can be used to project out a field from a runtime block. The
 * implementation is somewhat unusual, because it allocates and returns a fresh pointer
 *  every time. The way it works is as follows:
 *
 *  1. The Vicuna analysis currently does not keep track of dynamically allocated
 *     memory for runtime blocks. The representation of a value for a runtime block is
 *     (1) [Top] in an integer domain of the analysis,
 *     (2) there are no corresponding pointers in the pointer domain of the analysis, and
 *     (3) there is additional runtime state  associated with the value
 *     (e.g., the shape is often [Block] or [Any]).
 *  2. Since the runtime block does not exist in the pointer domain, the [caml_field]
 *     primitive cannot simply return a pointer to the field of the block. Moreover, since
 *     it is possible to use the corresponding macro [Field] as an l-value, it is not enough
 *     to simply return a "fresh value" for the field (i.e., a pointer is required, just
 *     [value] of field cannot be used as l-values). Instead, we create a
 *     "fresh value" whenever [caml_field] is invoked, and then create a "fresh pointer"
 *     which stores the value. We then return this freshly allocated pointer. Importantly,
 *     the pointer is [active] and will be marked [stale] during the next garbage
 *     collection operation.
 *
 *
 *    This handling of Field with fresh allocations is probably a source of unsoundness:
 *    the analysis observes that two pointers can never point to the same object
 *    and based on it avoid going down a path to a bad state that is
 *    feasible in practice. For example:
 *
 *    value *p1 = &Field(v, 0);
 *    value *p2 = &Field(v, 0);
 *    if (p1 == p2) { UNSAFE STUFF }
 *
 *    The same Field is read twice and each time returns a fresh object
 *    (callsite abstraction with calling context of 1 is enough to distinguish them).
 *    This code pattern is not common.
 */
value *caml_field(value v, mlsize_t i){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_block(v);
    value *f = _ffi_fresh_value_ptr();
    _ffi_mark_active_ptr(f);
    *f = _ffi_generate_value();
    _ffi_pass_on_shape(v, i, f);
    return f;
}

/*
 * The primtive [caml_set_field] can be used to update a field. Compared to
 * [*caml_field(v, i) = w], this primitive performs additional checks: it ensures the
 * value [w] is active.
 */
void caml_set_field(value v, mlsize_t i, value w) {
    _ffi_assert_locked();
    _ffi_assert_active(v);
    // NOTE: The following check [_ffi_assert_active(w)] is overapproximate (i.e., sound
    // but can lead to false positives), it fails for integer values that persist beyond
    // an allocation and are then stored in the block
    _ffi_assert_active(w);
    _ffi_assert_block(v);

    value *f = caml_field(v, i);
    _ffi_assert_shape_compat(w, *f);
    return;
}

void caml_modify(value *v_ptr, value w){
    _ffi_assert_locked();
    _ffi_assert_active_ptr(v_ptr);
    _ffi_assert_active(*v_ptr);
    _ffi_assert_block(*v_ptr);
    _ffi_assert_shape_compat(w, *v_ptr);
    return;
}


int caml_is_block(value w){
    // NOTE: checking whether a value is a block does not require
    // it to be alive for now
    int imm = _ffi_is_immediate(w);
    return !imm;
}

int caml_is_long(value w){
    return _ffi_is_immediate(w);
}

mlsize_t caml_block_size(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_block(v);
    // FIXME: we could eventually do something more precise here using shape information
    return _mopsa_rand_u64();
}

unsigned char caml_block_tag(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_block(v);
    // FIXME: we could eventually do something more precise here using shape information
    return _mopsa_rand_u8();
}



const char *val_as_string(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_string(v);
    void *str = _mopsa_rand_void_pointer();
    // NOTE: it is important that this pointer is marked active, because the corresponding
    // string lives on the OCaml heap and can be moved by the GC.
    _ffi_mark_active_ptr(str);
    return (const char *) str;
}

unsigned char *val_as_bytes(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_string(v);
    void *str = _mopsa_rand_void_pointer();
    // NOTE: it is important that this pointer is marked active, because the corresponding
    // string lives on the OCaml heap and can be moved by the GC.
    _ffi_mark_active_ptr(str);
    return (unsigned char *) str;
}


void *val_as_abstract_block(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_abstract(v);
    void *str = _mopsa_rand_void_pointer();
    // NOTE: it is important that this pointer is marked active, because the corresponding
    // string lives on the OCaml heap and can be moved by the GC.
    _ffi_mark_active_ptr(str);
    return str;
}

void *val_as_custom_block(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_custom_block(v);
    void *str = _mopsa_rand_void_pointer();
    // NOTE: it is important that this pointer is marked active, because the corresponding
    // string lives on the OCaml heap and can be moved by the GC.
    _ffi_mark_active_ptr(str);
    return str;
}

struct custom_operations **val_as_custom_operations(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_custom_block(v);
    struct custom_operations **str = _ffi_malloc_type(struct custom_operations *);
    _ffi_mark_active_ptr(str);
    return str;
}




mlsize_t caml_string_length(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_string(v);
    return _mopsa_rand_u64();
}

int caml_string_is_c_safe (value v) {
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_string(v);
    return _mopsa_rand_s8() == 0;
}


void caml_store_double_idx(value v, mlsize_t i, double d) {
    _ffi_unimplemented();
}

double *caml_get_double_idx(value v, mlsize_t i) {
    _ffi_unimplemented();
    return NULL;
}


header_t *caml_header_pointer(value v){
    _ffi_assert_locked();
    _ffi_assert_active(v);
    /* TODO: add support for something like
       __ffi_assert_not_immediate(v)
       or perhaps
       __ffi_assert_has_header(v)

       so we can check here that v is not immediate or that
       the shape of v is something that has a header, i.e.,
       either a structured block, a string, a double, etc.
    */
    value *f = _ffi_fresh_value_ptr();
    _ffi_mark_active_ptr(f);
    return (header_t *)f;
}

value caml_value_of_header_pointer(header_t *p){
    _ffi_assert_locked();
    _ffi_assert_active_ptr(p);
    value v = _ffi_generate_value();
    _ffi_set_shape_block(&v);
    return v;
}


value caml_hash_variant(char const *tag) {
    return Val_long(_mopsa_rand_s64());
}

/*
 * Misc
 */

void caml_failed_assert (char * s1, char *s2, int x) {
    _ffi_assert_locked();
    _ffi_no_return();
    return;
}

/*
 * Allocation
 *
 * Note that none of the allocation functions actually allocate anything. This is because
 * the analysis currently does not keep track of the pointers that correspond to a runtime
 * block. For more information, see the explanation for the macros
 */
value caml_alloc_small(mlsize_t len, tag_t tag){
    // FIXME: After allocation with this function, the fields must be initialized.
    // We currently do not track this in the analysis.
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_general_block(&v, tag);
    return v;
}


value caml_alloc_shr(mlsize_t len, tag_t tag){
    // FIXME: After allocation with this function, the fields must be initialized.
    // We currently do not track this in the analysis.
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_general_block(&v, tag);
    return v;
}

value caml_alloc(mlsize_t len, tag_t tag) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_general_block(&v, tag);
    return v;
}
value caml_alloc_local(mlsize_t len, tag_t tag) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_general_block(&v, tag);
    return v;
}
value caml_alloc_string(mlsize_t len) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_string(&v);
    return v;
}
value caml_alloc_initialized_string(mlsize_t len, const char * str) {
    // FIXME: we could check here that str is untracked or alive This is not supported
    // with the current set of runtime primitives, but should not be hard to add
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_string(&v);
    return v;
}
value caml_copy_int64(int64_t x){
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_int64(&v);
    return v;
}
value caml_copy_int32(int x){
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_int32(&v);
    return v;
}
value caml_copy_nativeint(intnat x){
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_nativeint(&v);
    return v;
}
value caml_copy_double(double x){
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_double(&v);
    return v;
}
value caml_copy_string(const char *str){
    // FIXME: we could check here that str is untracked or alive
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_string(&v);
    return v;
}

value caml_copy_string_array (char const **arr) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_block(&v);
    return v;
}

value caml_alloc_tuple (mlsize_t wosize) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_block(&v);
    return v;
}

value caml_alloc_float_array(mlsize_t len) {
    _ffi_unimplemented();
}

value caml_alloc_sprintf(const char * format, ...) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_string(&v);
    return v;
}

value caml_alloc_some(value v)
{
  CAMLparam1(v);
  value some = caml_alloc_small(1, 0);
  Field(some, 0) = v;
  CAMLreturn(some);
}


// NOTE: [caml_alloc_final] allocates a custom block
value caml_alloc_final(mlsize_t wosize,
                                   final_fun, /*finalization function*/
                                   mlsize_t, /*resources consumed*/
                                   mlsize_t  /*max resources*/){
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_custom_block(&v);
    return v;
}

int caml_convert_flag_list(value list, int *flags) {
    _ffi_assert_locked();
    _ffi_assert_active(list);
    _ffi_assert_variant(list);
    return _mopsa_rand_s32();
}


value caml_alloc_local_string (mlsize_t len) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_string(&v);
    return v;
}


/*
 * Memory
 */

caml_stat_block caml_stat_alloc(asize_t size) {
    _ffi_assert_locked();
    // NOTE: [caml_stat_alloc] returns memory outside of the OCaml heap. In particular, it
    // is not tracked by the garbage collector. Hence, we simply return a [Top] pointer.
    return _mopsa_rand_void_pointer();
}

caml_stat_string caml_stat_strdup(const char *s) {
    _ffi_assert_locked();
    return _mopsa_rand_void_pointer();
}

caml_stat_block caml_stat_alloc_noexc(asize_t size) {
    // NOTE: [caml_stat_alloc_noexc] does not require the runtime lock.
    // NOTE: [caml_stat_alloc_noexc] returns memory outside of the OCaml heap. In particular, it
    // is not tracked by the garbage collector. Hence, we simply return a [Top] pointer.
    return _mopsa_rand_void_pointer();
}

void caml_stat_free(caml_stat_block blk) {
    // NOTE: we do not allocate, so there is nothing to free
    return;
}


caml_stat_block caml_stat_resize(caml_stat_block blk, asize_t sz){
    _ffi_unimplemented();
}


caml_stat_block caml_stat_resize_noexc(caml_stat_block blk, asize_t sz){
    _ffi_unimplemented();
}



void caml_register_global_root(value *ptr) {
    _ffi_unimplemented();
}

void caml_remove_global_root(value *ptr) {
    _ffi_unimplemented();
}

void caml_register_generational_global_root(value *ptr) {
    _ffi_unimplemented();
}

void caml_remove_generational_global_root(value *ptr) {
    _ffi_unimplemented();
}

void caml_modify_generational_global_root(value *r, value newval) {
    _ffi_unimplemented();
}



/*
 * Threads
 */

void caml_enter_blocking_section() {
    // FIXME: we are also not allowed to access roots (e.g., to read from them or to write to them),
    // this is not enough, but a good first step
    _ffi_assert_locked();
    _ffi_garbage_collect();
    _ffi_release_lock();
    return;
}

void caml_leave_blocking_section() {
    // FIXME: it would be nice to assert that we are unlocked
    _ffi_acquire_lock();
    return;
}

int caml_c_thread_register() {
    _ffi_unimplemented();
}

int caml_c_thread_unregister(){
    _ffi_unimplemented();
}


void caml_switch_runtime_locking_scheme(struct caml_locking_scheme* sh){
    _ffi_unimplemented();
}


void caml_thread_save_runtime_state(void){
    _ffi_unimplemented();
}


void caml_thread_restore_runtime_state(void){
    _ffi_unimplemented();
}

/*
 * Big Arrays
 *
 * The treatment of big arrays is interesting. The runtime representation of a big-array
 * is a block with its own tag. The block stores a structure, which itself lives on the
 * OCaml heap. The structure stores a data pointer which, however, points outside of the
 * OCaml heap. As a result, the analysis marks the pointer obtained from
 * [val_as_big_array] as active, does not do anything for the fields.
 */

struct caml_ba_array *val_as_big_array(value v) {
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_bigarray(v);
    void *ba = _mopsa_rand_void_pointer();
    _ffi_mark_active_ptr(ba);
    // NOTE: struct pointer is internal, struct contains external poiner
    return (struct caml_ba_array *) ba;
}

value caml_ba_alloc(int flags, int num_dims, void * data, intnat * dim){
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_bigarray(&v);
    return v;
}

value caml_ba_alloc_dims(int flags, int num_dims, void * data, ...) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_bigarray(&v);
    return v;
}

uintnat caml_ba_byte_size(struct caml_ba_array * b) {
    _ffi_assert_locked();
    // FIXME: Assert the big array is active here. Currently, the runtime primitives
    // do not offer an option to assert the activeness of pointers.
    return _mopsa_rand_u64();
}
uintnat caml_ba_num_elts(struct caml_ba_array * b) {
    _ffi_assert_locked();
    // FIXME: Assert the big array is active here. Currently, the runtime primitives
    // do not offer an option to assert the activeness of pointers.
    return _mopsa_rand_u64();
}


CAMLexport int caml_ba_element_size[] =
{ 4 /*FLOAT32*/, 8 /*FLOAT64*/,
  1 /*SINT8*/, 1 /*UINT8*/,
  2 /*SINT16*/, 2 /*UINT16*/,
  4 /*INT32*/, 8 /*INT64*/,
  sizeof(value) /*CAML_INT*/, sizeof(value) /*NATIVE_INT*/,
  8 /*COMPLEX32*/, 16 /*COMPLEX64*/,
  1 /*CHAR*/
};

CAMLextern void caml_ba_finalize(value v){
    _ffi_unimplemented();
}
CAMLextern int caml_ba_compare(value v1, value v2){
    _ffi_unimplemented();
}
CAMLextern intnat caml_ba_hash(value v){
    _ffi_unimplemented();
}
CAMLextern void caml_ba_serialize(value v, uintnat * p, uintnat *q){
    _ffi_unimplemented();
}
CAMLextern uintnat caml_ba_deserialize(void * dst){
    _ffi_unimplemented();
}

/*
 * Failure
 */

void caml_raise(value v)
{
    // FIXME: should this check liveness of v, should be false for integers and true for blocks
    // We would need something like [_ffi_assert_active_if_not_immediate].
    _ffi_assert_locked();
    // We currently perform no check on the value, since it lead to several false positives in the past
    _ffi_no_return();
}

void caml_raise_constant(value tag)
{
    caml_raise(tag);
}

void caml_raise_with_arg(value tag, value arg)
{
  _ffi_assert_locked();
  _ffi_assert_active(arg);
  // FIXME: we currently perform no check on the tag, since it lead to several false positives in the past
  _ffi_no_return();
}

void caml_raise_with_string(value tag, char const * msg)
{
  _ffi_assert_locked();
  // FIXME: we currently perform no check on the tag, since it lead to several false positives in the past
  _ffi_no_return();
}

void caml_failwith(const char *msg){
    _ffi_assert_locked();
      // FIXME: should we check the msg?
    _ffi_no_return();
}
void caml_failwith_value (value msg)
{
    _ffi_assert_locked();
    _ffi_assert_active(msg);
    _ffi_assert_string(msg);
    _ffi_no_return();
}

void caml_invalid_argument (char const *msg)
{
    _ffi_assert_locked();
  // FIXME: should we check the msg?
    _ffi_no_return();
}

void caml_invalid_argument_value (value msg)
{
    _ffi_assert_locked();
    _ffi_assert_active(msg);
    _ffi_assert_string(msg);
    _ffi_no_return();
}

void caml_raise_out_of_memory()
{
    _ffi_assert_locked();
    _ffi_no_return();
}

void caml_raise_stack_overflow()
{
    _ffi_assert_locked();
    _ffi_no_return();
}


void caml_raise_sys_error(value msg)
{
    _ffi_assert_locked();
    _ffi_assert_active(msg);
    _ffi_assert_string(msg);
    _ffi_no_return();
}

void caml_raise_end_of_file(){
    _ffi_assert_locked();
    _ffi_no_return();
}

void caml_raise_zero_divide()
{
    _ffi_assert_locked();
    _ffi_no_return();
}

void caml_raise_not_found()
{
    _ffi_assert_locked();
    _ffi_no_return();
}

void caml_array_bound_error()
{
    _ffi_assert_locked();
    _ffi_no_return();
}

void caml_raise_sys_blocked_io()
{
    _ffi_assert_locked();
    _ffi_no_return();
}



/*
 * Unix Support
 */

value caml_unix_error_of_code (int errcode) {
    _ffi_assert_locked();
    // NOTE: This function does not always allocate. We overapproximate its behavior by
    // always allocating.
    _ffi_garbage_collect();
    return _ffi_generate_value();
}
int caml_unix_code_of_unix_error (value error) {
    _ffi_assert_locked();
    _ffi_assert_active(error);
    _ffi_assert_variant(error);
    return _mopsa_rand_s32();
}

void caml_unix_error (int errcode, const char * cmdname, value arg){
    _ffi_assert_locked();
    _ffi_assert_active(arg);
    // FIXME: double check that this is a string
    _ffi_assert_string(arg);
    _ffi_no_return();
}

void caml_uerror (const char * cmdname, value arg) {
    _ffi_assert_locked();
    _ffi_assert_active(arg);
    // FIXME: double check that this is a string
    _ffi_assert_string(arg);
    _ffi_no_return();
}

// NOTE: external pointer
void **val_as_dir(value v) {
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_abstract(v);
    return _mopsa_rand_void_pointer();
}

void caml_unix_check_path(value path, const char * cmdname){
    _ffi_assert_locked();
    _ffi_assert_active(path);
    _ffi_assert_string(path);
    // in the error case, we abort, so the only passing case is the success case
    return;
}


// FIXME: These are defined via macros in the runtime.
char ** cstringvect(value arg, char * cmdname){
    _ffi_unimplemented();
}
void cstringvect_free(char **vec){
    _ffi_unimplemented();
}

int unix_cloexec_default = 0;
int unix_cloexec_p(value cloexec){
    _ffi_unimplemented();
}
void unix_set_cloexec(int fd, char * cmdname, value arg){
    _ffi_unimplemented();
}
void unix_clear_cloexec(int fd, char * cmdname, value arg){
    _ffi_unimplemented();
}



/*
 * Sockets
 */
enum option_type {
  TYPE_BOOL = 0,
  TYPE_INT = 1,
  TYPE_LINGER = 2,
  TYPE_TIMEVAL = 3,
  TYPE_UNIX_ERROR = 4
};


// NOTE: this should get the same treatment as abstract blocks
// active pointer, inactive contents
struct in_addr *val_get_in_addr(value v) {
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_abstract(v);
    void *addr = _mopsa_rand_void_pointer();
    _ffi_mark_active_ptr(addr);
    return (struct in_addr *) addr;
}


struct in6_addr *val_get_in6_addr(value v) {
    _ffi_assert_locked();
    _ffi_assert_active(v);
    _ffi_assert_abstract(v);
    void *addr = _mopsa_rand_void_pointer();
    _ffi_mark_active_ptr(addr);
    return (struct in6_addr *) addr;
}


void caml_unix_get_sockaddr (value mladdr,
                          union sock_addr_union * addr /*out*/,
                          socklen_param_type * addr_len /*out*/) {

    _ffi_assert_block(mladdr);
    _ffi_assert_active(mladdr);
    // FIXME: Shoud we do other checks here?
}

value caml_unix_alloc_sockaddr (union sock_addr_union * addr /*in*/,
                      socklen_param_type addr_len, int close_on_error) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_block(&v);
    return v;
}


value caml_unix_alloc_inet_addr (struct in_addr * inaddr) {
  _ffi_assert_locked();
  _ffi_garbage_collect();
  value v = _ffi_generate_value();
  _ffi_set_shape_string(&v);
  return v;
}

value caml_unix_alloc_inet6_addr (struct in6_addr * inaddr) {
  _ffi_assert_locked();
  _ffi_garbage_collect();
  value v = _ffi_generate_value();
  _ffi_set_shape_string(&v);
  return v;
}



/*
 * Callback
 */

const value *caml_named_value(const char *str) {
    value *ptr = _ffi_fresh_value_ptr();
    /*
        NOTE: [ptr] points to a block of memory allocated on the C
        heap by the OCaml runtime (essentially, a global variable)
        to hold the OCaml value
        registered on the OCaml side using [Callback.register].

        We register the block of memory as root to ensure
        that its contents will always stay active.

        We do not mark the content of [ptr] as active explicitly,
        because _ffi_generate_value() does it.

        [ptr] itself should not be marked as active: For example:

            value *ptr = caml_named_value("my_value");
            caml_alloc(1, 0);
            String_val(*ptr);

        If [ptr] was active, then the analysis would report an error,
        since [ptr] itself goes from [active] to [stale]. However,
        this code is perfectly fine. The pointer is still guaranteed to point to the
        correct OCaml value. It is important to distinguish here between a value pointer
        being active and the thing it points to being active. In this case, the pointer
        itself should not be active (akin to a pointer to a local variable such as
        &local_var) since it lives on the C heap, but the variable that stores the
        contents must be marked as active and rooted.
    */
    *ptr = _ffi_generate_value();
    _ffi_register_root(ptr);

    return ptr;
}

value caml_callback (value closure, value arg){
    _ffi_unimplemented();
}

value caml_callback2 (value closure, value arg1, value arg2){
    _ffi_unimplemented();
}

value caml_callback3 (value closure, value arg1, value arg2, value arg3){
    _ffi_unimplemented();
}

value caml_callbackN (value closure, int narg, value args[]){
    _ffi_unimplemented();
}


value caml_callback_exn (value closure, value arg){
    _ffi_unimplemented();
}

value caml_callback2_exn (value closure, value arg1, value arg2){
    _ffi_unimplemented();
}

value caml_callback3_exn (value closure, value arg1, value arg2, value arg3){
    _ffi_unimplemented();
}

value caml_callbackN_exn (value closure, int narg, value args[]){
    _ffi_unimplemented();
}




typedef void (*caml_named_action) (const value*, char *);
void caml_iterate_named_values(caml_named_action f){
    _ffi_unimplemented();
}

void caml_main (char_os ** argv){
    _ffi_unimplemented();
}
void caml_startup (char_os ** argv){
    _ffi_unimplemented();
}
value caml_startup_exn (char_os ** argv){
    _ffi_unimplemented();
}
void caml_startup_pooled (char_os ** argv){
    _ffi_unimplemented();
}
value caml_startup_pooled_exn (char_os ** argv){
    _ffi_unimplemented();
}
void caml_shutdown (void){
    _ffi_unimplemented();
}

int caml_callback_depth = 0;


/*
 * Custom Blocks
 */

CAMLextern value caml_alloc_custom(struct custom_operations * ops,
                                   uintnat size, /*size in bytes*/
                                   mlsize_t mem, /*resources consumed*/
                                   mlsize_t max  /*max resources*/) {
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_custom_block(&v);
    return v;

}

CAMLextern value caml_alloc_custom_mem(struct custom_operations * ops,
                                       uintnat size, /*size in bytes*/
                                       mlsize_t mem  /*memory consumed*/){
    _ffi_assert_locked();
    _ffi_garbage_collect();
    value v = _ffi_generate_value();
    _ffi_set_shape_custom_block(&v);
    return v;
}

CAMLextern void caml_register_custom_operations(struct custom_operations * ops){
    _ffi_unimplemented();
}




/*
 * Hash
 */


uint32_t caml_hash_mix_uint32(uint32_t h, uint32_t d){
    return _mopsa_rand_u32();
}
uint32_t caml_hash_mix_intnat(uint32_t h, intnat d){
    return _mopsa_rand_u32();
}
uint32_t caml_hash_mix_int64(uint32_t h, int64_t d){
    return _mopsa_rand_u32();
}
uint32_t caml_hash_mix_double(uint32_t h, double d){
    return _mopsa_rand_u32();
}
uint32_t caml_hash_mix_float(uint32_t h, float d){
    return _mopsa_rand_u32();
}
uint32_t caml_hash_mix_string(uint32_t h, value s){
    return _mopsa_rand_u32();
}

/*
 * PrintExc
 */

char * caml_format_exception (value v){
    _ffi_unimplemented();
}
void caml_fatal_uncaught_exception (value v) {
    _ffi_unimplemented();
}
void caml_fatal_uncaught_exception_with_message (value v, const char * c) {
    _ffi_unimplemented();
}

/*
 * GC Ctrl
 */

void caml_init_gc (uintnat minor_size, uintnat major_size, uintnat major_incr,
                   uintnat percent_fr, uintnat percent_m, uintnat window,
                   uintnat custom_maj, uintnat custom_min, uintnat custom_bsz,
                   uintnat policy){
    _ffi_unimplemented();
}

void caml_heap_check (void) {
    _ffi_unimplemented();
}



/*
 * Roots (OCaml Header)
 */

void caml_oldify_local_roots (void){
    _ffi_unimplemented();
}
void caml_darken_all_roots_start (void){
    _ffi_unimplemented();
}
intnat caml_darken_all_roots_slice (intnat b){
    _ffi_unimplemented();
}
void caml_do_roots (scanning_action a, int b){
    _ffi_unimplemented();
}

uintnat caml_incremental_roots_count = 0;

void caml_do_local_roots_byt (scanning_action a, value *b, value *c,
                                         struct caml__roots_block *d){
    _ffi_unimplemented();
}
void caml_do_local_roots_nat (
                     scanning_action maj, scanning_action min,
                     char * c_bottom_of_stack,
                     uintnat last_retaddr, value * v_gc_regs,
                     struct caml__roots_block * gc_local_roots,
                     struct caml_local_arenas* local_arenas){
    _ffi_unimplemented();
}

// Stack
uintnat (*caml_stack_usage_hook)(void) = NULL;


/*
 * Sys (extended with caml_sys_exit)
 */


char * caml_strerror(int errnum, char * buf, size_t buflen) {
    _ffi_unimplemented();
    return NULL;
}

void caml_sys_error (value v) {
    _ffi_unimplemented();
}

void caml_sys_io_error (value v) {
    _ffi_unimplemented();
}

double caml_sys_time_unboxed(value v) {
    _ffi_unimplemented();
    return 0.0;
}
void caml_sys_init (char_os * exe_name, char_os ** argv) {
    _ffi_unimplemented();
}

void caml_do_exit (int i) {
    _ffi_unimplemented();
}

value caml_sys_exit (value v) {
    _ffi_assert_immediate(v);
    _ffi_no_return();
    return Val_unit;
}

/*
 * Backtrace
 */

void caml_record_backtraces(int x){
    _ffi_unimplemented();
};


void caml_stash_backtrace(value exn, value * sp, int reraise){
    _ffi_unimplemented();
}

void caml_load_main_debug_info(void) {
    _ffi_unimplemented();
}

void caml_print_exception_backtrace(void) {
    _ffi_unimplemented();
}

void caml_init_backtrace(void){
    _ffi_unimplemented();
}

void caml_init_debug_info(void){
    _ffi_unimplemented();
}

/*
 * MD5
 */

void caml_md5_block(unsigned char digest[16], void * data, uintnat len) {
    _ffi_unimplemented();
}

value caml_md5_channel(struct channel *chan, intnat toread) {
    _ffi_unimplemented();
}

void caml_MD5Init (struct MD5Context *context){
    _ffi_unimplemented();
}
void caml_MD5Update (struct MD5Context *context, unsigned char *buf, uintnat len){
    _ffi_unimplemented();
}
void caml_MD5Final (unsigned char *digest, struct MD5Context *ctx) {
    _ffi_unimplemented();
}
void caml_MD5Transform (uint32_t *buf, uint32_t *in) {
    _ffi_unimplemented();
}


/*
 * IO
 */
struct channel * caml_open_descriptor_in (int x) {
    _ffi_unimplemented();
    return NULL;
}
struct channel * caml_open_descriptor_out (int x) {
    _ffi_unimplemented();
    return NULL;
}
void caml_close_channel (struct channel * c) {
    _ffi_unimplemented();
}
file_offset caml_channel_size (struct channel * c) {
    _ffi_unimplemented();
    return 0;
}
void caml_seek_in (struct channel *c, file_offset o) {
    _ffi_unimplemented();
}
void caml_seek_out (struct channel *c, file_offset o) {
    _ffi_unimplemented();
}
file_offset caml_pos_in (struct channel *c) {
    _ffi_unimplemented();
    return 0;
}
file_offset caml_pos_out (struct channel *c){
    _ffi_unimplemented();
    return 0;
}

value caml_alloc_channel(struct channel *chan) {
    _ffi_unimplemented();
    return Val_unit;
}
int caml_channel_binary_mode (struct channel *chan) {
    _ffi_unimplemented();
    return 0;
}

int caml_flush_partial (struct channel *c) {
    _ffi_unimplemented();
    return 0;
}
void caml_flush (struct channel *c){
    _ffi_unimplemented();
}
void caml_putch(struct channel *c, int x) {
    _ffi_unimplemented();
}
void caml_putword (struct channel *c, uint32_t u) {
    _ffi_unimplemented();
}
int caml_putblock (struct channel *c, char * str, intnat l){
    _ffi_unimplemented();
    return 0;
}
void caml_really_putblock (struct channel *c, char *str, intnat l) {
    _ffi_unimplemented();
}

unsigned char caml_refill (struct channel *chan) {
    _ffi_unimplemented();
    return 0;
}
unsigned char caml_getch(struct channel *c) {
    _ffi_unimplemented();
    return 0;
}
uint32_t caml_getword (struct channel *c) {
    _ffi_unimplemented();
    return 0;
}
int caml_getblock (struct channel *c, char *str, intnat l) {
    _ffi_unimplemented();
    return 0;
}
intnat caml_really_getblock (struct channel *chan, char *str, intnat l) {
    _ffi_unimplemented();
    return 0;
}

value caml_ml_open_descriptor_in(value fd) {
    _ffi_unimplemented();
    return Val_unit;
}
value caml_ml_open_descriptor_out(value fd) {
    _ffi_unimplemented();
    return Val_unit;
}


/*
 * Memprof
*/

void caml_memprof_set_suspended(int x){
    _ffi_unimplemented();
}

value caml_memprof_handle_postponed_exn(void) {
    _ffi_unimplemented();
    return Val_unit;
}

void caml_memprof_track_alloc_shr(value block) {
    _ffi_unimplemented();
}
void caml_memprof_track_custom(value block, mlsize_t bytes) {
    _ffi_unimplemented();
}
void caml_memprof_track_young(uintnat wosize, int from_caml,
                                     int nallocs, unsigned char* alloc_lens) {
    _ffi_unimplemented();
}
void caml_memprof_track_interned(header_t* block, header_t* blockend) {
    _ffi_unimplemented();
}

void caml_memprof_renew_minor_sample(void) {
    _ffi_unimplemented();
}

void caml_memprof_oldify_young_roots(void) {
    _ffi_unimplemented();
}
void caml_memprof_minor_update(void) {
    _ffi_unimplemented();
}
void caml_memprof_do_roots(scanning_action f) {
    _ffi_unimplemented();
}
void caml_memprof_update_clean_phase(void) {
    _ffi_unimplemented();
}
void caml_memprof_invert_tracked(void) {
    _ffi_unimplemented();
}

struct caml_memprof_th_ctx* caml_memprof_new_th_ctx(void) {
    _ffi_unimplemented();
}
void caml_memprof_leave_thread(void) {
    _ffi_unimplemented();
}
void caml_memprof_enter_thread(struct caml_memprof_th_ctx*) {
    _ffi_unimplemented();
}
void caml_memprof_delete_th_ctx(struct caml_memprof_th_ctx*) {
    _ffi_unimplemented();
}


/*
 * Address class
 */

int caml_page_table_lookup(void * addr) {
    _ffi_assert_locked();
    // since we do not track where the address is, we return a random value
    return _mopsa_rand_s8();
}

int caml_page_table_add(int kind, void * start, void * end) {
    _ffi_unimplemented();
    return 0;
}
int caml_page_table_remove(int kind, void * start, void * end) {
    _ffi_unimplemented();
    return 0;
}
int caml_page_table_initialize(mlsize_t bytesize){
    _ffi_unimplemented();
    return 0;
}
