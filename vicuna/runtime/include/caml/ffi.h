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


#ifndef OCAML_C_CHECKER_FFI_H
#define OCAML_C_CHECKER_FFI_H

#include <caml/config.h>
#include <caml/mopsa_aux.h>

/*
 * This file contains primitives that are provided by the Vicuna abstract interpretation
 * domain. To understand their effect on the analysis, it helps to understand what
 * information is being tracked inside the domain:
 *
 * 1. The domain tracks a boolean runtime lock. When an external function is analyzed,
 *    the runtime lock is initially held. The primitives below allow (1) asserting that
 *    the runtime lock is held, (2) acquiring the lock, and (3) releasing the lock.
 *    A garbage collection can run when runtime lock is not held.
 *
 *    Tracking of the lock has good precision, because for most code it is easy to
 *    determine statically where the runtime lock is held and where it is released.
 *
 * 2. The domain tracks, per variable, whether the variable is registered as a root or not.
 *    The primitives below allow registering and unregistering of roots. For variables
 *    registered as roots, we make sure that any operation that would usually invoke
 *    the GC does not invalidate the contents of the root. (See the status below.)
 *
 *    The tracking whether a local variable is registered as a root or not has good
 *    precision, because for most code it is easy to determine statically whether a
 *    variable is registered or not.
 *
 *    Registering dynamically allocated roots (e.g., global roots) is not yet supported.
 *
 * 3. The domain tracks, per variable, the status of the contents of the variable. The
 *    status can be (a) [untracked], for ordinary C data that has nothing to do with
 *    OCaml, (b) [active], for OCaml values and pointers that have not been invalidated
 *    by garbage collection, (c) [stale], for OCaml values and pointers that have
 *    potentially been invalidated by garbage collection (e.g., a runtime block that is
 *    not rooted after [caml_alloc] or when a runtime lock is not held).
 *    The boundaries between the three states can be fuzzy at times. By ordinary C data,
 *    we mean integers, pointers to stack variables, pointers to static variables,
 *    pointers to dynamically allocated memory, ...
 *
 *    There is one caveat to the meaning of [stale]: Under certain circumstances, the
 *    anlysis marks immediates (e.g., int or bool) as [stale] while it is perfectly safe
 *    to continue to use them. Concretely, the analysis marks all active values that are
 *    not rooted as stale upon encountering a garbage collecting operation (see the
 *    primitives below). In particular, even if a value is an immediate at runtime, the
 *    anlysis marks it as stale when garbage collection is run and the value is not
 *    rooted. As a consequence, operations on immediate values (e.g.,
 *    [Long_val]) do not require the value to be active (to avoid false positives).
 *
 *    The primtives below can be used to assert that a value or pointer is active, and to
 *    mark them as active to indicate that the value/pointer is managed by the OCaml
 *    runtime. The precision of the activeness tracking is good in many cases. It quickly
 *    leads to false positives when (1) unknown functions are involved that operate
 *    on/return OCaml values, (2) the runtime macros are circumvented by manually creating
 *    an OCaml value (e.g., by tagging a pointer), and (3) dynamically allocated memory
 *    that stores OCaml values.
 *
 *    The domain abstracts away the exact runtime representation of immediates as tagged.
 *
 * 4. The domain tracks, per variable, the possible shapes of the contents of the variable. The
 *    possible shapes roughly correspond to the different runtime tags that blocks can have +
 *    special shapes for immediates and non-OCaml values. The primitives below can be used to
 *    assert that a value has a certain shape or to set the shape of a value.
 *
 *    The most important shape is [Any] (also written [*]), which indicates (1) that the
 *    value is an OCaml value, but (2) that the analysis does not know which kind of OCaml
 *    value. For example, it could be a string, an immediate, a block, etc. The idea is
 *    that [Any] allows the analysis to be unsound: any operation on OCaml values will
 *    accept [Any] as an allowed input. In particular, all transformers that assert shapes
 *    will allow [Any] instead of, for example, [string]. Any is different from Top.
 *
 *
 * It is important to note that the analysis knows very little about OCaml block headers
 * and dynamically allocated memory. For example, if you malloc memory and craft your own
 * header, the analysis will get confused and not understand that you now have a valid
 * OCaml value. Concretely, the analysis already stumbles over the call to malloc, because
 * it has very rough treatment of malloc (it returns [Top: void *]). After that, the
 * analysis will not "understand" anything that happens to the pointer, since it does not
 * track the pointer precisely.
 *
 * ABSTRACT VS. CONCRETE SEMANTICS
 *
 * The analysis is based on abstract interpretation. One could take the raw C semantics as
 * the concrete semantics for this analsysis. However, there is no OCaml-specific
 * information in a raw C semantics and it contains no model of garbage collection or what
 * the runtime does (except for the raw, concrete runtime implementation in C). Instead, a
 * more suitable way to think about the concrete semantics of this analysis is something
 * along the lines of [1], where the C memory and the "OCaml memory" are largely kept separate and
 * there are primitives for modifying that OCaml memory representation from C. Such a
 * concrete semantics would be aware of runtime blocks and track their precise sizes,
 * tags, contents, C representation, etc.
 *
 * Our abstract semantics is not as precise in many regards. For example, (1) it quickly
 * loses the relationship between different variables, (2) it tracks "multiple possible
 * shapes" for each value, not the precise shape that it has during an execution, and (3)
 * it tracks only the possible shapes of values, not their precise contents (e.g., it
 * tracks [string] instead of the data contained in the string).
 *
 * [1] "Melocoton: A Program Logic for Verified Interoperability Between
 * OCaml and C" OOPSLA'23
 *
 *
 * SOUNDNESS (OVER VS. UNDERAPPROXIMATION)
 *
 * Ideally, the analysis would be an overapproximation of the possible states that the
 * program can reach during an execution. If this was the case, we could be certain that if
 * the overapproximation does not lead to issues, then the program does not have any issues.
 * Sady, the tracking of the analysis in its current form quickly loses precision (e.g.,
 * dynamic memory allocation with malloc returns [Top]), which would lead to a tremendous
 * amount of false positives in its reporting. Instead, the analysis also introduces
 * underapproximate reasoning (e.g., two sets of shapes are "compatible" if their
 * intersection is non-empty, not if one is a subset of the other), meaning the state it
 * tracks is effectively its "best guess" for plausible states during the execution. This
 * combination of overapproximate and underapproximate reasoning sacrifices soundness
 * in exchange for a more useful tool for detecting bugs.
 *
 *
 *
 * DYNAMICALLY ALLOCATED MEMORY
 *
 * We are not really materializing/tracking the runtime memory in our analysis.
 * Instead, we only track (1) shapes of values stored in variables and
 * how they are propagated, (2) whether a variable is storing a value
 * that is active or not, and (3) whether variables are rooted. In particular, what we are
 * missing in the domain are runtime blocks and their pointer chains.
 *
 * To make the analysis a bit more precise in some common cases (for example, when
 * `Field(v, i)` is executed) we "allocate" a new abstract memory cell and give it a name
 * that we can track just like other variables.
 * We can then add our usual information to this variable (e.g., whether it is rooted or
 * not, what the shape of the value is that it stores, and whether its value is active
 * or not).
 * The
 * primary purpose of this is a hack to enable `Field(v, i)` to be both an l-value and
 * an r-value.
 * One could think about it as a dynamic way to allocate a variable that will
 * persist beyond the lifetime of the function creating it + the variable is a fresh
 * variable on every invocation of the function.
 * The variable is a trick to transport
 * the information from the place where `&Field(v, i)` is called to the place where the
 * pointer is actually used. For example, consider:
 *
 *   value *tmp = &Field(v, i);
 *   ...
 *   Long_val(*tmp);
 *
 * Here, the variable behind [*tmp] simply transports our data from the invokation of
 * Field to the use point.
 *
 * To understand why the pointer is not active by default, consider the following example,
 * which consists of three different parts, each using a value pointer:
 *
 *   // Local Variable
 *   value v = Val_long(42);
 *   value *ptr = &v;                    // at this point [ptr] is untracked; its contents [v] are an immediate
 *   caml_alloc(0, 1);                   // at this point [ptr] is untracked; its contents [v] are an immediate
 *   Long_val(*ptr);                     // at this point [ptr] is untracked; its contents [v] are an immediate
 *
 * Reading from [ptr] in this case is okay, because [v] lives on the stack and is not managed by the OCaml runtime.
 * (Moreover, the contents of [ptr] are an immediate.)
 *
 *   // Field
 *   value v = create_int_ref();
 *   value *ptr = &Field(v, 0);          // at this point [ptr] is active; its contents [Field(v, 0)] are an immediate
 *   caml_alloc(0, 1);                   // at this point [ptr] is stale; its contents [Field(v, 0)] are an immediate
 *   Long_val(*ptr);                     // at this point [ptr] is stale, any access to it is undefined behavior
 *
 * Reading from [ptr] is not allowed here, even though its contents are supposed to be an
 * immediate. The OCaml GC could have moved the block, making any read to the pointer UB.
 * [&Field(v, i)] is only a "temporary pointer" in the sense that it should not be kept
 * across calls to the GC.
 *
 *
 *   // Named Value
 *   // suppose [foo_bar] this is an integer
 *   value *ptr = caml_named_value("foo_bar"); // at this point [ptr] is untracked; its contents are an immediate;
 *                                             // (the variable behind [*ptr] is registered as a root such that it remains active)
 *   caml_alloc(0, 1);                         // at this point [ptr] is untracked; its contents are an immediate
 *   Long_val(*ptr);                           // at this point [ptr] is untracked; its contents are an immediate
 *
 * Reading from the pointer is okay in this last example. The OCaml runtime promises to not move the value
 * and hence the pointer itself can be cached (see 7.2 in the manual).
 *
 * See also comments about the implementation of [caml_field] in [ffi.c]
 */


// An OCaml value, as far as the analysis is concerned, is of type [ffi_value]
// The analysis assumes this to be a [long int], and gets confused otherwise.
typedef intnat ffi_value;

/*
 * [_ffi_garbage_collect] is a primitive for simulating garabge collection. It should be
 * invoked by any operation that may invoke the garbage collection. To be sound, it should
 * be invoked even if garbage collection is only triggered in some execution paths.
 */
void _ffi_garbage_collect();

/*
 * [_ffi_mark_active_contents] and [_ffi_mark_active_ptr] can be used to mark
 * values/pointers as active. The function [_ffi_mark_active_contents] should be invoked
 * with a pointer to a value. It only works correctly when the pointer is to a local
 * variable or to memory allocated with [_ffi_fresh_value_ptr] below. For example, it can
 * be used as [_ffi_mark_active_contents(&v)] when v is a local variable
 * of type [ffi_value].
 * The operation [_ffi_mark_active_ptr] can be
 * used to mark a pointer itself active. This operation is used, for example, to mark the
 * [char *] returned by [String_val] as active, because its contents live on the OCaml
 * heap.
 */
void _ffi_mark_active_contents(ffi_value *ptr);
void _ffi_mark_active_ptr(void *ptr);



/*
 * [_ffi_fresh_value_ptr] can be thought of as allocating a abstract cell that can store a
 * single OCaml value (e.g., similar to [malloc(sizeof(value))]).
*/
ffi_value *_ffi_fresh_value_ptr();


// [_ffi_malloc_ptr(p)] allocates a pointer of type [typeof(p)]
// [_ffi_malloc_type] provides a macro to obtain a pointer for a given type
#define _ffi_malloc_type(type) ((type *)_ffi_malloc_ptr((type *)NULL))


/*
 * [_ffi_fresh_active_value_ptr] additionally ensures that the pointer stores an active value.
*/
ffi_value *_ffi_fresh_active_value_ptr();



/*
 * [_ffi_assert_active] asserts that the status of a value is [active]. It can be used,
 * for example, in [String_val(v)] to assert that [v] is active. As explained at the top
 * of this file, it should not be used in primitives that expect an immediate (e.g.,
 * [Long_val])
 */
void _ffi_assert_active(ffi_value v);
void _ffi_assert_active_ptr(void *ptr);

/*
 * [_ffi_register_root] and [_ffi_unregister_root] can be used to register/unregister
 * variables as roots with the garbage collector. They can be used on local/global
 * variables with [_ffi_register_root(&v)] and on pointers obtained from
 * [_ffi_fresh_value_ptr]
 */
void _ffi_register_root(ffi_value *v);
void _ffi_unregister_root(ffi_value *v);

/*
 * [_ffi_assert_locked], [_ffi_acquire_lock], and [_ffi_release_lock] can be used to
 * interact with the runtime lock. [_ffi_assert_lock] asserts that the runtime lock is
 * currently held. [_ffi_acquire_lock] unconditionally sets the lock to true.
 * [_ffi_release_lock] unconditionally sets the lock to false.
 */
void _ffi_assert_locked();
void _ffi_acquire_lock();
void _ffi_release_lock();


/*
 * [_ffi_unimplemented] can be used to indicate that a certain runtime feature is not yet
 * implemented. It will be displayed differently than a standard error, because it is a
 * limitation of the analysis, not a problem with the code of the user.
 * It will allow the analysis to continue to other function.
 */
void *_ffi_unimplemented() __attribute__ ((noreturn));

/*
 * [_ffi_unimplemented_macro] can be used in macros that are not yet implemented. It behaves
 * the same as [_ffi_unimplemented], but it adjusts the callstack correctly when used in macros.
 */
void *_ffi_unimplemented_macro() __attribute__ ((noreturn));
/*
 * [_ffi_is_immediate] can be used to query a runtime value for whether it is an immediate
 * or not. If the anlysis is sure that the value is a block, then the return value is
 * 0. If it is sure that the value is an immediate, the return value is 1.
 * Otherwise, the return value is "0 or 1".
 */
int _ffi_is_immediate(ffi_value v);


/*
 * Shapes
 *
 * The possible shapes that the C code can assert/set are as follows:
 * 1. [FFI_IMMEDIATE] for immediate values (tagged),
 *     for example ocaml int and bool, constant constructors, None and empty list []
 * 2. [FFI_BLOCK] for a "standard" runtime structured block
 *    (i.e., with a tag between 0 and no-scan) that represents tuples, records, arrays,
 *    and variant constructors with data
 * 3. [FFI_DOUBLE] for runtime blocks with the double tag (boxed ocaml float)
 * 4. [FFI_INT64] for runtime blocks that store a 64-bit integer (boxed ocaml int64)
 * 5. [FFI_INT32] for runtime blocks that store a 32-bit integer (boxed ocaml int32
 * 6. [FFI_NATIVEINT] for runtime blocks that store a word-size (64-bit integer on amd64/arm64)
 * 7. [FFI_STRING] for runtime blocks that store an ocaml string/bytes
 * 8. [FFI_VARIANT] for FFI_BLOCK or FFI_IMMEDIATE
 * 7. [FFI_BIGARRAY] for runtime blocks that store big-arrays
 * 8. [FFI_ABSTRACT] for "abstract" runtime blocks (with abstract tag) that wraps raw C data (such as a pointer to C-allocated data)
 * 9. [FFI_ANY] for arbitrary runtime values, including all of the above
 */

enum ffi_shape {
    FFI_IMMEDIATE = 1,
    FFI_BLOCK = 2,
    FFI_DOUBLE = 3,
    FFI_INT64 = 4,
    FFI_INT32 = 5,
    FFI_NATIVEINT = 6,
    FFI_STRING = 7,
    FFI_VARIANT = 8,  // block or immediate
    FFI_BIGARRAY = 9,
    FFI_ABSTRACT = 10,
    FFI_CUSTOM_BLOCK = 11,
    FFI_ANY = 255
};

/*
 * [_ffi_assert_shape] and [_ffi_set_shape] can be used to assert/set the shape of a
 * value. It is important to note that [_ffi_assert_shape] always accepts [Any] as a
 * possible shape, regardless of which shape is provided as the second value.
 *
 * [_ffi_add_shape] can be used to add another shape as a possibility.
 */
void _ffi_assert_shape(ffi_value v, enum ffi_shape sh);
void _ffi_set_shape(ffi_value *v, enum ffi_shape sh);
void _ffi_add_shape(ffi_value *v, enum ffi_shape sh);


// [_ffi_pass_on_shape] passes on the shape of the [idx]th field of [parent] to the variable
// behind the pointer [child]
void _ffi_pass_on_shape(ffi_value parent, unsigned long idx, ffi_value *child);


// [_ffi_assert_shape_compat] asserts that the shape of v is compatible with the shape of w
// Concretely, this means the intersection of shape(v) and shape(w) is non-empty
// The sound condition would be shape(v) is a subset of shape(w).
// Note that the current interpretation is symmetric while the intended one is not.
void _ffi_assert_shape_compat(ffi_value v, ffi_value w);


/*
 * Derived Functions
 *
 * The implementation of these functions can be found in [ffi.c]
 */


// [_ffi_generate_value] is a handy function for creating an arbitrary
// runtime value. The value is active and its shape is [Any]
ffi_value _ffi_generate_value();


void _ffi_assert_immediate(ffi_value v);
void _ffi_assert_block(ffi_value v);
void _ffi_assert_double(ffi_value v);
void _ffi_assert_int64(ffi_value v);
void _ffi_assert_int32(ffi_value v);
void _ffi_assert_nativeint(ffi_value v);
void _ffi_assert_string(ffi_value v);
void _ffi_assert_variant(ffi_value v);
void _ffi_assert_bigarray(ffi_value v);
void _ffi_assert_abstract(ffi_value v);
void _ffi_assert_custom_block(ffi_value v);

void _ffi_set_shape_immediate(ffi_value *v);
void _ffi_set_shape_block(ffi_value *v);
void _ffi_set_shape_double(ffi_value *v);
void _ffi_set_shape_int64(ffi_value *v);
void _ffi_set_shape_int32(ffi_value *v);
void _ffi_set_shape_nativeint(ffi_value *v);
void _ffi_set_shape_string(ffi_value *v);
void _ffi_set_shape_variant(ffi_value *v);
void _ffi_set_shape_bigarray(ffi_value *v);
void _ffi_set_shape_abstract(ffi_value *v);
void _ffi_set_shape_custom_block(ffi_value *v);
void _ffi_set_shape_any(ffi_value *v);


#endif // OCAML_C_CHECKER_FFI_H
