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

#ifndef OCAML_C_CHECKER_GC_H
#define OCAML_C_CHECKER_GC_H


#include <caml/mlvalues.h>
#define Caml_white (0 << 8)
#define Caml_gray  (1 << 8)
#define Caml_blue  (2 << 8)
#define Caml_black (3 << 8)

#define Color_hd(hd) ((color_t) ((hd) & Caml_black))
#define Color_hp(hp) (Color_hd (Hd_hp (hp)))
#define Color_val(val) (Color_hd (Hd_val (val)))

#define Is_white_hd(hd) (Color_hd (hd) == Caml_white)
#define Is_gray_hd(hd) (Color_hd (hd) == Caml_gray)
#define Is_blue_hd(hd) (Color_hd (hd) == Caml_blue)
#define Is_black_hd(hd) (Color_hd (hd) == Caml_black)

#define Whitehd_hd(hd) (((hd)  & ~Caml_black)/*| Caml_white*/)
#define Grayhd_hd(hd)  (((hd)  & ~Caml_black)  | Caml_gray)
#define Blackhd_hd(hd) (((hd)/*& ~Caml_black*/)| Caml_black)
#define Bluehd_hd(hd)  (((hd)  & ~Caml_black)  | Caml_blue)


/* This depends on the layout of the header.  See [mlvalues.h]. */
#define Make_header(wosize, tag, color)                                       \
      (/*CAMLassert ((wosize) <= Max_wosize),*/                               \
       ((header_t) (((header_t) (wosize) << 10)                               \
                    + (color)                                                 \
                    + (tag_t) (tag)))                                         \
      )



/* Colors for locally allocated values.
   (Only used during root-scanning, never visible to the rest of the GC) */
#define Local_marked Caml_black
#define Local_unmarked Caml_blue /* allocation color of local objects */
#define Local_scanned Caml_gray


#define Init_local_arena_bsize 4096

/* We allow the local stack to quadruple 19 times, which is virtually infinite.
   Hardware limit will probably hit first (either out of address space on 32bit
   systems, or out of physical memory on 64bit)

   19 is the biggest number without triggering some compiler errors about
   integer overflow during shifting; I don't know if overflow would actually
   happen if I make the number bigger, but 19 corresponds to 1024TB and should
   be sufficient for a very long time. */
#define Max_local_arenas 19

struct caml_local_arena {
  char* base;
  uintnat length;
  void* alloc_block;
};
typedef struct caml_local_arenas {
  int count;
  intnat saved_sp;
  intnat next_length;
  struct caml_local_arena arenas[Max_local_arenas];
} caml_local_arenas;

#endif // OCAML_C_CHECKER_GC_H
