type pair = int * string
type open_type = ..

type variant =
  | A (* Val_long(0) *)
  | D (* Val_long(1) *)
  | B of int ref (* Block with tag 0 *)

type record =
  { pair_field : pair (* field 0 *)
  ; array_field : int array (* field 1 *)
  ; variant_field : variant (* field 2 *)
  ; ref_field : int ref (* field 3 *)
  ; int_field : bool (* field 4 *)
  ; open_field : int * open_type (* field 5 *)
  ; string_field : string (* field 6 *)
  }

(* demo.ml *)
external make_ref : 'a -> 'a ref = "demo_make_ref"
external make_ref_broken : 'a -> 'a ref = "demo_make_ref_broken"

(* arity *)
external returns_void : unit -> unit = "demo_returns_void"
external takes_unit : unit -> unit = "demo_takes_unit"

(* typing *)
external takes_string : string -> string = "demo_takes_string"
external takes_string_record : record -> unit = "demo_takes_string_record"
external nested_shapes : record -> unit = "demo_nested_shapes"

external assign_shapes_freshly_allocated
  :  record
  -> unit
  = "demo_assign_freshly_allocated"

(* runtime lock*)
external release_lock : string -> string = "demo_release_lock"
external release_lock_failwith : string -> string = "demo_release_lock_failwith"

(* garbage collection *)
external forget_rooting : string -> string = "demo_forget_rooting"
external forget_rooting_advanced : string -> string = "demo_forget_rooting_advanced"
external with_rooting : string -> string = "demo_with_rooting"

(* named value examples *)
external caml_named_value_example : unit -> unit = "demo_caml_named_value"
external caml_named_value_example_two : unit -> unit = "demo_caml_named_value_two"

(* limitations *)
external variant_example : variant -> unit = "demo_variant_example"

external dynamic_allocation_imprecision
  :  unit
  -> unit
  = "demo_dynamic_allocation_imprecision"

external assign_shapes_unsound : record -> unit = "demo_assign_shapes_unsound"

type t

external manually_tagged : unit -> t = "demo_manually_tagged"
external unimplemented_feature : unit -> float array = "demo_unimplemented_feature"

type cb

external alloc_custom_block : unit -> cb = "demo_alloc_custom_block"
external alloc_custom_block_bad : unit -> cb = "demo_alloc_custom_block_bad"
external update_custom_block : cb -> unit = "demo_update_custom_block"

(* allocation of blocks with different tags *)
type t_abstract

external alloc_tag_any : int -> t_abstract = "demo_alloc_tag_any"
external alloc_tag_int_pair : unit -> int * int = "demo_alloc_tag_int_pair"
