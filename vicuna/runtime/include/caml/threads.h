/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*          Xavier Leroy and Damien Doligez, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1995 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

// Origin: otherlibs/systhreads/threads.h

#ifndef OCAML_C_CHECKER_THREADS_H
#define OCAML_C_CHECKER_THREADS_H

#include <caml/misc.h>

CAMLextern void caml_enter_blocking_section (void);
CAMLextern void caml_leave_blocking_section (void);
#define caml_acquire_runtime_system caml_leave_blocking_section
#define caml_release_runtime_system caml_enter_blocking_section


#ifndef CAMLextern_libthreads
#define CAMLextern_libthreads CAMLextern
#endif
CAMLextern_libthreads int caml_c_thread_register(void);
CAMLextern_libthreads int caml_c_thread_unregister(void);



enum caml_thread_type { Thread_type_caml, Thread_type_c_registered };
struct caml_locking_scheme {
  void* context;
  void (*lock)(void*);
  void (*unlock)(void*);

  /* If non-NULL, these functions are called when threads start and stop.
     For threads created by OCaml, that's at creation and termination.
     For threads created by C, that's at caml_c_thread_register/unregister.
     The lock is not held when these functions are called. */
  void (*thread_start)(void*, enum caml_thread_type);
  void (*thread_stop)(void*, enum caml_thread_type);

  /* Called after fork().
     The lock should be held after this function returns. */
  void (*reinitialize_after_fork)(void*);

  /* can_skip_yield and yield are both called with the lock held,
     and expect it held on return */
  int (*can_skip_yield)(void*);
  void (*yield)(void*);
};

extern struct caml_locking_scheme caml_default_locking_scheme;

/* Switch to a new runtime locking scheme.

   The old runtime lock must be held (i.e. not in a blocking section),
   and the new runtime lock must not be held. After this function
   returns, the old lock is released and the new one is held.

   There is a period during this function when neither lock is held,
   so context-switches may occur. */
CAMLextern_libthreads
void caml_switch_runtime_locking_scheme(struct caml_locking_scheme*);

CAMLextern_libthreads
void caml_thread_save_runtime_state(void);

CAMLextern_libthreads
void caml_thread_restore_runtime_state(void);



#endif
