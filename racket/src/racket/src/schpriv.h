/*
  Racket
  Copyright (c) 2004-2017 PLT Design Inc.
  Copyright (c) 1995-2001 Matthew Flatt
  All rights reserved.

  Please see the full copyright in the documentation.

  libscheme
  Copyright (c) 1994 Brent Benson
  All rights reserved.
*/

/*
   Racket prototypes and declarations for internal consumption.
*/

#ifndef __mzscheme_private__
#define __mzscheme_private__

// #define MZ_GC_STRESS_TESTING 1

#include "scheme.h"
#include "longdouble/longdouble.h"

#ifdef CIL_ANALYSIS
#define ROSYM          __attribute__((__ROSYM__))
#define READ_ONLY      __attribute__((__READ_ONLY__))
#define SHARED_OK      __attribute__((__SHARED_OK__))
#define HOOK_SHARED_OK __attribute__((__HOOK_SHARED_OK__))
#else
#define ROSYM          /* EMPTY */
#define READ_ONLY      /* EMPTY */
#define SHARED_OK      /* EMPTY */
#define HOOK_SHARED_OK /* EMPTY */
#endif

#ifdef OS_X
# define MZ_CHECK_ASSERTS
#endif

#ifdef MZ_CHECK_ASSERTS
# include <assert.h>
# define MZ_ASSERT(x) assert(x)
#else
# define MZ_ASSERT(x) /* empty */
#endif

/*========================================================================*/
/*                        optimization flags                              */
/*========================================================================*/

/* Used with SCHEME_LOCAL_TYPE_MASK, LET_ONE_TYPE_MASK, etc.*/
#define SCHEME_LOCAL_TYPE_FLONUM    1
#define SCHEME_LOCAL_TYPE_FIXNUM    2
#define SCHEME_LOCAL_TYPE_EXTFLONUM 3

#define SCHEME_MAX_LOCAL_TYPE       3

#define SCHEME_MAX_LOCAL_TYPE_MASK  0x3
#define SCHEME_MAX_LOCAL_TYPE_BITS  2

/* Flonum unboxing is only useful if a value is going to flow to a
   function that wants it, otherwise we'll have to box the flonum anyway.
   Also, we can only leave flonums unboxed if they don't escape
   before a potential continuation capture.
   Fixnum unboxing is always fine, since it's easy to box and doesn't
   involve allocation. */
#define ALWAYS_PREFER_UNBOX_TYPE(ty) ((ty) == SCHEME_LOCAL_TYPE_FIXNUM)

#define IN_FIXNUM_RANGE_ON_ALL_PLATFORMS(v) (((v) >= -1073741824) && ((v) <= 1073741823))


/* We support 2^SCHEME_PRIM_OPT_INDEX_SIZE combinations of optimization flags: */

/* marks a primitive as JIT-inlined for 1 argument: */
#define SCHEME_PRIM_IS_UNARY_INLINED       (1 << 0)
/* marks a primitive as JIT-inlined for 2 arguments: */
#define SCHEME_PRIM_IS_BINARY_INLINED      (1 << 1)
/* marks a primitive as JIT-inlined for 0 or 3+ arguments: */
#define SCHEME_PRIM_IS_NARY_INLINED        (1 << 2)
/* indicates that a primitive call can be dropped if it's result is not used;
   although the function never raises an exception, it should not be reordered
   past a test that might be a guard or past an expression that might
   have a side effect: */
#define SCHEME_PRIM_IS_UNSAFE_OMITABLE     (1 << 3)
/* indicates that a primitive call can be dropped if it's result is not used,
   because it has no side-effect and never raises an exception: */
#define SCHEME_PRIM_IS_OMITABLE            (1 << 4)
/* indicates that a primitive call can be dropped, but it allocates,
   so it's not as reorderable as it might be otherwise: */
#define SCHEME_PRIM_IS_OMITABLE_ALLOCATION (1 << 5)
/* indicates that a primitive call will produce the same results for the same
   inputs; note that UNSAFE_FUNCTIONAL is stronger than UNSAFE_OMITABLE: */
#define SCHEME_PRIM_IS_UNSAFE_FUNCTIONAL   (1 << 6)
/* the SCHEME_PRIMT_WANTS_... flags indicate a primitive that
   expects certain kinds of arguments and can encourage unboxing: */
#define SCHEME_PRIM_WANTS_FLONUM_FIRST     (1 << 7)
#define SCHEME_PRIM_WANTS_FLONUM_SECOND    (1 << 8)
#define SCHEME_PRIM_WANTS_FLONUM_THIRD     (1 << 9)
#define SCHEME_PRIM_WANTS_EXTFLONUM_FIRST  (1 << 10)
#define SCHEME_PRIM_WANTS_EXTFLONUM_SECOND (1 << 11)
#define SCHEME_PRIM_WANTS_EXTFLONUM_THIRD  (1 << 12)
/* indicates an unsafe operation that does not allocate: */
#define SCHEME_PRIM_IS_UNSAFE_NONALLOCATE  (1 << 13)
/* indicates a primitive that always raises an exception or
   otherwise escapes from the current continuation: */
#define SCHEME_PRIM_ALWAYS_ESCAPES         (1 << 14)
/* indicates a primitive that is JIT-inlined on some platforms,
   but not the current one: */
#define SCHEME_PRIM_SOMETIMES_INLINED      (1 << 15)
/* indicates a primitive that produces a real or number (or
   errors): */
#define SCHEME_PRIM_PRODUCES_REAL          (1 << 16)
#define SCHEME_PRIM_PRODUCES_NUMBER        (1 << 17)
/* indicates a primitive that requires certain argument types (all the
   same type): */
#define SCHEME_PRIM_WANTS_REAL             (1 << 18)
#define SCHEME_PRIM_WANTS_NUMBER           (1 << 19)
/* indicates a primitive that always succeed when given
   arguments of the expected type: */
#define SCHEME_PRIM_OMITTABLE_ON_GOOD_ARGS (1 << 20)
/* indicates a primitive that produces a real number when
   given real-number arguments: */
#define SCHEME_PRIM_CLOSED_ON_REALS        (1 << 21)

#define SCHEME_PRIM_OPT_TYPE_SHIFT           22
#define SCHEME_PRIM_OPT_TYPE_MASK            (SCHEME_MAX_LOCAL_TYPE_MASK << SCHEME_PRIM_OPT_TYPE_SHIFT)
#define SCHEME_PRIM_OPT_TYPE(x) ((x & SCHEME_PRIM_OPT_TYPE_MASK) >> SCHEME_PRIM_OPT_TYPE_SHIFT)

#define SCHEME_PRIM_IS_OMITABLE_ANY (SCHEME_PRIM_IS_OMITABLE | SCHEME_PRIM_IS_OMITABLE_ALLOCATION | SCHEME_PRIM_IS_UNSAFE_OMITABLE)

#define SCHEME_PRIM_PRODUCES_FLONUM (SCHEME_LOCAL_TYPE_FLONUM << SCHEME_PRIM_OPT_TYPE_SHIFT)
#define SCHEME_PRIM_PRODUCES_FIXNUM (SCHEME_LOCAL_TYPE_FIXNUM << SCHEME_PRIM_OPT_TYPE_SHIFT)

#define SCHEME_PRIM_WANTS_FLONUM_BOTH (SCHEME_PRIM_WANTS_FLONUM_FIRST | SCHEME_PRIM_WANTS_FLONUM_SECOND)

#define SCHEME_PRIM_PRODUCES_EXTFLONUM (SCHEME_LOCAL_TYPE_EXTFLONUM << SCHEME_PRIM_OPT_TYPE_SHIFT)
#define SCHEME_PRIM_WANTS_EXTFLONUM_BOTH (SCHEME_PRIM_WANTS_EXTFLONUM_FIRST | SCHEME_PRIM_WANTS_EXTFLONUM_SECOND)

extern int scheme_prim_opt_flags[]; /* uses an index from SCHEME_PRIM_OPT_INDEX_MASK */
extern XFORM_NONGCING int scheme_intern_prim_opt_flags(int);

#define SCHEME_PRIM_PROC_OPT_FLAGS(proc) \
  scheme_prim_opt_flags[(SCHEME_PRIM_PROC_FLAGS(proc) >> SCHEME_PRIM_OPT_INDEX_SHIFT) \
                        & SCHEME_PRIM_OPT_INDEX_MASK]

/*========================================================================*/
/*                         allocation and GC                              */
/*========================================================================*/

#define MAKE_CLOSED_PRIM(f,v,n,mi,ma) \
  scheme_make_closed_prim_w_arity((Scheme_Closed_Prim *)f, (void *)v, n, mi, ma)

#define _MALLOC_N(x, n, malloc) ((x*)malloc(sizeof(x)*(n)))
#define MALLOC_ONE(x) _MALLOC_N(x, 1, scheme_malloc)
#define MALLOC_ONE_TAGGED(x) _MALLOC_N(x, 1, scheme_malloc_small_tagged)
#define MALLOC_N_TAGGED(x, n) _MALLOC_N(x, n, scheme_malloc_array_tagged)
#ifdef MZTAG_REQUIRED
# define scheme_malloc_rt(x) scheme_malloc_tagged(x)
# define MALLOC_ONE_RT(x) MALLOC_ONE_TAGGED(x)
# define MALLOC_ONE_WEAK(x) _MALLOC_N(x, 1, scheme_malloc)
# define MALLOC_N_WEAK(x,c) _MALLOC_N(x, c, scheme_malloc)
# define MALLOC_ONE_TAGGED_WEAK(x) _MALLOC_N(x, 1, scheme_malloc_tagged)
# define MALLOC_ONE_WEAK_RT(x) MALLOC_ONE_TAGGED_WEAK(x)
#else
# define scheme_malloc_rt(x) scheme_malloc(x)
# define MALLOC_ONE_RT(x) MALLOC_ONE(x)
# define MALLOC_ONE_WEAK(x) MALLOC_ONE_ATOMIC(x)
# define MALLOC_N_WEAK(x,c) MALLOC_N_ATOMIC(x,c)
# define MALLOC_ONE_WEAK_RT(x) MALLOC_ONE_WEAK(x)
# define MALLOC_ONE_TAGGED_WEAK(x) MALLOC_ONE_WEAK(x)
#endif
#define MALLOC_N(x, n) _MALLOC_N(x, n, scheme_malloc)
#define MALLOC_ONE_ATOMIC(x) _MALLOC_N(x, 1, scheme_malloc_atomic)
#define MALLOC_N_ATOMIC(x, n) _MALLOC_N(x, n, scheme_malloc_atomic)
#define MALLOC_SO_BOX() _MALLOC_ONE(Scheme_Object*, scheme_malloc)
#define MALLOC_N_STUBBORN(x, n) _MALLOC_N(x, n, scheme_malloc_stubborn)

#ifdef MZ_PRECISE_GC
# define WEAKIFY(x) scheme_make_weak_box(x)
# define WEAKIFIED(x) SCHEME_WEAK_BOX_VAL(x)
# define HT_EXTRACT_WEAK(x) SCHEME_WEAK_BOX_VAL(x)
#else
# define WEAKIFY(x) x
# define WEAKIFIED(x) x
# define HT_EXTRACT_WEAK(x) (*(char **)(x))
#endif

#ifndef MZ_XFORM
# define START_XFORM_SKIP /**/
# define END_XFORM_SKIP /**/
# define GC_CAN_IGNORE /**/
# define GC_MAYBE_IGNORE_INTERIOR /**/
# define XFORM_OK_PLUS +
# define XFORM_OK_MINUS -
#else
# ifdef GC_INTERIORABLES_NEVER_MOVE
#  define GC_MAYBE_IGNORE_INTERIOR GC_CAN_IGNORE
# else
#  define GC_MAYBE_IGNORE_INTERIOR /**/
# endif
#endif

#ifdef MZ_PRECISE_GC
intptr_t scheme_hash_key(Scheme_Object *o);
#else
# define scheme_hash_key(o) ((intptr_t)(o))
#endif
typedef int (*Compare_Proc)(void *v1, void *v2);

XFORM_NONGCING void scheme_install_symbol_hash_code(Scheme_Object *sym, uintptr_t h);

Scheme_Object *scheme_dump_gc_stats(int c, Scheme_Object *p[]);

#define REGISTER_SO(x) MZ_REGISTER_STATIC(x)

THREAD_LOCAL_DECL(extern struct rktio_t *scheme_rktio);
THREAD_LOCAL_DECL(extern int scheme_current_place_id);
THREAD_LOCAL_DECL(extern intptr_t scheme_total_gc_time);
THREAD_LOCAL_DECL(extern int scheme_cont_capture_count);
THREAD_LOCAL_DECL(extern int scheme_continuation_application_count);
THREAD_LOCAL_DECL(extern struct Scheme_Prefix *scheme_prefix_finalize);
THREAD_LOCAL_DECL(extern struct Scheme_Prefix *scheme_inc_prefix_finalize);

int scheme_num_types(void);

#ifdef MZTAG_REQUIRED
# define MZTAG_IF_REQUIRED  Scheme_Type type;
# define SET_REQUIRED_TAG(e) e
#else
# define MZTAG_IF_REQUIRED /* empty */
# define SET_REQUIRED_TAG(e) /* empty */
#endif

#if MZ_USE_NOINLINE
# define MZ_DO_NOT_INLINE(decl) decl __attribute__ ((noinline))
#elif _MSC_VER
# define MZ_DO_NOT_INLINE(decl) __declspec(noinline) decl
#else
# define MZ_DO_NOT_INLINE(decl) decl
#endif


#define GC_REG_TRAV(type, base) \
  GC_register_traversers2(type, base ## _SIZE, base ## _MARK, base ## _FIXUP, base ## _IS_CONST_SIZE, base ## _IS_ATOMIC)

void scheme_reset_finalizations(void);

uintptr_t scheme_get_primordial_thread_stack_base(void);
uintptr_t scheme_get_current_os_thread_stack_base(void);
void scheme_set_current_os_thread_stack_base(void *base);

#ifdef MZ_PRECISE_GC
uintptr_t scheme_get_current_thread_stack_start(void);
#endif

int scheme_propagate_ephemeron_marks(void);
void scheme_clear_ephemerons(void);

#ifndef MZ_XFORM
# define HIDE_FROM_XFORM(x) x
#endif

#define mzALIAS (void *)

#define BITS_PER_MZSHORT (8 * sizeof(mzshort))

#ifndef NO_INLINE_KEYWORD
# define MZ_INLINE M_MSC_IZE(inline)
#else
# define MZ_INLINE /* empty */
#endif

#if _MSC_VER
# define MZ_NO_INLINE _declspec(noinline)
#elif defined(__GNUC__)
# define MZ_NO_INLINE __attribute ((__noinline__))
#else
# define MZ_NO_INLINE /* empty */
#endif

#ifdef MZ_PRECISE_GC
# define CLEAR_KEY_FIELD(o) ((o)->keyex = 0)
#else
# define CLEAR_KEY_FIELD(o) /* empty */
#endif

#define SCHEME_PAIR_FLAGS(pr) MZ_OPT_HASH_KEY(&((Scheme_Simple_Object *)pr)->iso)
#define PAIR_IS_LIST 0x1
#define PAIR_IS_NON_LIST 0x2
#define PAIR_FLAG_MASK 0x3

#define SCHEME_PAIR_COPY_FLAGS(dest, src) (SCHEME_PAIR_FLAGS((dest)) |= (SCHEME_PAIR_FLAGS((src)) & PAIR_FLAG_MASK))


/*========================================================================*/
/*                             initialization                             */
/*========================================================================*/

THREAD_LOCAL_DECL(extern int scheme_starting_up);

void scheme_init_finalization(void);
void scheme_init_portable_case(void);
void scheme_init_stack_check(void);
void scheme_init_overflow(void);
#ifdef MZ_PRECISE_GC
void scheme_register_traversers(void);
void scheme_init_hash_key_procs(void);
#endif
Scheme_Thread *scheme_make_thread(void*);
void scheme_init_process_globals(void);
void scheme_init_true_false(void);
void scheme_init_symbol_table(void);
void scheme_init_symbol_type(Scheme_Env *env);
void scheme_init_type();
void scheme_init_custodian_extractors();
void scheme_init_bignum();
void scheme_init_compenv();
void scheme_init_letrec_check();
void scheme_init_optimize();
void scheme_init_resolve();
void scheme_init_sfs();
void scheme_init_validate();
void scheme_init_port_wait();
void scheme_init_logger_wait();
void scheme_init_struct_wait();
void scheme_init_list(Scheme_Env *env);
void scheme_init_unsafe_list(Scheme_Env *env);
void scheme_init_unsafe_hash(Scheme_Env *env);
void scheme_init_stx(Scheme_Env *env);
void scheme_init_module(Scheme_Env *env);
void scheme_init_module_path_table(void);
void scheme_init_port(Scheme_Env *env);
void scheme_init_port_fun(Scheme_Env *env);
void scheme_init_network(Scheme_Env *env);
void scheme_init_file(Scheme_Env *env);
void scheme_init_proc(Scheme_Env *env);
void scheme_init_vector(Scheme_Env *env);
void scheme_init_unsafe_vector(Scheme_Env *env);
void scheme_init_string(Scheme_Env *env);
void scheme_init_number(Scheme_Env *env);
void scheme_init_flfxnum_number(Scheme_Env *env);
void scheme_init_extfl_number(Scheme_Env *env);
void scheme_init_unsafe_number(Scheme_Env *env);
void scheme_init_extfl_unsafe_number(Scheme_Env *env);
void scheme_init_numarith(Scheme_Env *env);
void scheme_init_flfxnum_numarith(Scheme_Env *env);
void scheme_init_extfl_numarith(Scheme_Env *env);
void scheme_init_unsafe_numarith(Scheme_Env *env);
void scheme_init_extfl_unsafe_numarith(Scheme_Env *env);
void scheme_init_numcomp(Scheme_Env *env);
void scheme_init_flfxnum_numcomp(Scheme_Env *env);
void scheme_init_extfl_numcomp(Scheme_Env *env);
void scheme_init_unsafe_numcomp(Scheme_Env *env);
void scheme_init_extfl_unsafe_numcomp(Scheme_Env *env);
void scheme_init_numstr(Scheme_Env *env);
void scheme_init_extfl_numstr(Scheme_Env *env);
void scheme_init_eval(Scheme_Env *env);
void scheme_init_promise(Scheme_Env *env);
void scheme_init_struct(Scheme_Env *env);
void scheme_init_reduced_proc_struct(Scheme_Env *env);
void scheme_init_fun(Scheme_Env *env);
void scheme_init_unsafe_fun(Scheme_Env *env);
void scheme_init_compile(Scheme_Env *env);
void scheme_init_symbol(Scheme_Env *env);
void scheme_init_char_constants(void);
void scheme_init_char(Scheme_Env *env);
void scheme_init_bool(Scheme_Env *env);
void scheme_init_syntax(Scheme_Env *env);
void scheme_init_marshal(Scheme_Env *env);
void scheme_init_error(Scheme_Env *env);
#ifndef NO_SCHEME_EXNS
void scheme_init_exn(Scheme_Env *env);
#endif
void scheme_init_debug(Scheme_Env *env);
void scheme_init_thread(Scheme_Env *env);
void scheme_init_unsafe_thread(Scheme_Env *env);
void scheme_init_read(Scheme_Env *env);
void scheme_init_print(Scheme_Env *env);
#ifndef NO_SCHEME_THREADS
void scheme_init_sema(Scheme_Env *env);
#endif
void scheme_init_dynamic_extension(Scheme_Env *env);
#ifndef NO_REGEXP_UTILS
extern void scheme_regexp_initialize(Scheme_Env *env);
#endif
void scheme_init_paramz(Scheme_Env *env);
void scheme_init_parameterization();
void scheme_init_getenv(void);
void scheme_init_inspector(void);
void scheme_init_compenv_symbol(void);
void scheme_init_param_symbol(void);
void scheme_init_longdouble_fixup(void);

#ifndef DONT_USE_FOREIGN
void scheme_init_foreign_globals();
#endif
void scheme_init_foreign(Scheme_Env *env);
void scheme_init_place(Scheme_Env *env);
void scheme_init_places_once();
void scheme_init_futures(Scheme_Env *env);
void scheme_init_futures_once();
void scheme_init_futures_per_place();
void scheme_end_futures_per_place();
void scheme_init_linklet(Scheme_Env *env);
 
void scheme_init_print_buffers_places(void);
void scheme_init_string_places(void);
void scheme_init_thread_places(void);
void scheme_init_eval_places(void);
void scheme_init_compile_places(void);
void scheme_init_compenv_places(void);
void scheme_init_port_places(void);
void scheme_init_regexp_places(void);
void scheme_init_stx_places(int initial_main_os_thread);
void scheme_init_fun_places(void);
void scheme_init_sema_places(void);
void scheme_init_gmp_places(void);
void scheme_init_print_global_constants(void);
void scheme_init_variable_references_constants(void);
void scheme_init_logger(void);
void scheme_init_logging_once(void);
void scheme_init_file_places(void);
void scheme_init_foreign_places(void);
void scheme_init_place_local_symbol_table(void);

Scheme_Logger *scheme_get_main_logger(void);
Scheme_Logger *scheme_get_gc_logger(void);
Scheme_Logger *scheme_get_future_logger(void);
Scheme_Logger *scheme_get_place_logger(void);
void scheme_init_logger_config(void);

void scheme_register_network_evts();

void scheme_free_dynamic_extensions(void);
void scheme_free_all_code(void);

XFORM_NONGCING int scheme_is_multithreaded(int now);

/* Type readers & writers for compiled code data */
typedef Scheme_Object *(*Scheme_Type_Reader)(Scheme_Object *list);
typedef Scheme_Object *(*Scheme_Type_Writer)(Scheme_Object *obj);

extern Scheme_Type_Reader *scheme_type_readers;
extern Scheme_Type_Writer *scheme_type_writers;

extern Scheme_Equal_Proc *scheme_type_equals;
extern Scheme_Primary_Hash_Proc *scheme_type_hash1s;
extern Scheme_Secondary_Hash_Proc *scheme_type_hash2s;

void scheme_init_port_config(void);
void scheme_init_port_fun_config(void);
Scheme_Config *scheme_init_error_escape_proc(Scheme_Config *c);
void scheme_init_error_config(void);
#ifndef NO_SCHEME_EXNS
void scheme_init_exn_config(void);
#endif
#ifdef WINDOWS_PROCESSES
void scheme_init_thread_memory(void);
#endif
void scheme_init_module_resolver(void);

void scheme_finish_kernel(Scheme_Env *env);

void scheme_init_syntax_bindings(void);

Scheme_Object *scheme_make_initial_inspectors(void);
Scheme_Object *scheme_get_current_inspector(void);
XFORM_NONGCING Scheme_Object *scheme_get_initial_inspector(void);

Scheme_Object *scheme_get_local_inspector();

extern int scheme_builtin_ref_counter;

Scheme_Object **scheme_make_builtin_references_table(int *_unsafe_start);
Scheme_Object *scheme_make_local(Scheme_Type type, int pos, int flags);

void scheme_add_embedded_builtins(Scheme_Env *env);
void scheme_do_add_global_symbol(Scheme_Env *env, Scheme_Object *sym,
				 Scheme_Object *obj, int constant,
				 int primitive);

void *scheme_get_os_thread_like();
void scheme_init_os_thread_like(void *);
void scheme_done_os_thread();
int scheme_is_place_main_os_thread();

/*========================================================================*/
/*                                constants                               */
/*========================================================================*/

extern Scheme_Object *scheme_symbol_p_proc;
extern Scheme_Object *scheme_keyword_p_proc;
extern Scheme_Object *scheme_char_p_proc;
extern Scheme_Object *scheme_interned_char_p_proc;
extern Scheme_Object *scheme_fixnum_p_proc;
extern Scheme_Object *scheme_flonum_p_proc;
extern Scheme_Object *scheme_extflonum_p_proc;
extern Scheme_Object *scheme_real_p_proc;
extern Scheme_Object *scheme_number_p_proc;
extern Scheme_Object *scheme_apply_proc;
extern Scheme_Object *scheme_values_proc;
extern Scheme_Object *scheme_procedure_p_proc;
extern Scheme_Object *scheme_procedure_arity_includes_proc;
extern Scheme_Object *scheme_procedure_specialize_proc;
extern Scheme_Object *scheme_void_proc;
extern Scheme_Object *scheme_void_p_proc;
extern Scheme_Object *scheme_syntax_p_proc;
extern Scheme_Object *scheme_check_not_undefined_proc;
extern Scheme_Object *scheme_check_assign_not_undefined_proc;
extern Scheme_Object *scheme_null_p_proc;
extern Scheme_Object *scheme_pair_p_proc;
extern Scheme_Object *scheme_mpair_p_proc;
extern Scheme_Object *scheme_unsafe_cons_list_proc;
extern Scheme_Object *scheme_unsafe_car_proc;
extern Scheme_Object *scheme_unsafe_cdr_proc;
extern Scheme_Object *scheme_unsafe_mcar_proc;
extern Scheme_Object *scheme_unsafe_mcdr_proc;
extern Scheme_Object *scheme_unsafe_unbox_proc;
extern Scheme_Object *scheme_car_proc;
extern Scheme_Object *scheme_cdr_proc;
extern Scheme_Object *scheme_cons_proc;
extern Scheme_Object *scheme_mcons_proc;
extern Scheme_Object *scheme_list_p_proc;
extern Scheme_Object *scheme_list_proc;
extern Scheme_Object *scheme_list_star_proc;
extern Scheme_Object *scheme_list_pair_p_proc;
extern Scheme_Object *scheme_vector_proc;
extern Scheme_Object *scheme_vector_p_proc;
extern Scheme_Object *scheme_make_vector_proc;
extern Scheme_Object *scheme_vector_immutable_proc;
extern Scheme_Object *scheme_vector_ref_proc;
extern Scheme_Object *scheme_vector_set_proc;
extern Scheme_Object *scheme_list_to_vector_proc;
extern Scheme_Object *scheme_unsafe_vector_length_proc;
extern Scheme_Object *scheme_unsafe_struct_ref_proc;
extern Scheme_Object *scheme_unsafe_struct_star_ref_proc;
extern Scheme_Object *scheme_hash_ref_proc;
extern Scheme_Object *scheme_box_p_proc;
extern Scheme_Object *scheme_box_proc;
extern Scheme_Object *scheme_box_immutable_proc;
extern Scheme_Object *scheme_call_with_values_proc;
extern Scheme_Object *scheme_call_with_immed_mark_proc;
extern Scheme_Object *scheme_make_struct_type_proc;
extern Scheme_Object *scheme_make_struct_field_accessor_proc;
extern Scheme_Object *scheme_make_struct_field_mutator_proc;
extern Scheme_Object *scheme_make_struct_type_property_proc;
extern Scheme_Object *scheme_struct_to_vector_proc;
extern Scheme_Object *scheme_struct_type_p_proc;
extern Scheme_Object *scheme_current_inspector_proc;
extern Scheme_Object *scheme_make_inspector_proc;
extern Scheme_Object *scheme_varref_const_p_proc;
extern Scheme_Object *scheme_unsafe_fxnot_proc;
extern Scheme_Object *scheme_unsafe_fxand_proc;
extern Scheme_Object *scheme_unsafe_fxior_proc;
extern Scheme_Object *scheme_unsafe_fxxor_proc;
extern Scheme_Object *scheme_unsafe_fxrshift_proc;

extern Scheme_Object *scheme_string_p_proc;
extern Scheme_Object *scheme_unsafe_string_length_proc;
extern Scheme_Object *scheme_byte_string_p_proc;
extern Scheme_Object *scheme_unsafe_byte_string_length_proc;

extern Scheme_Object *scheme_unsafe_real_add1_proc;
extern Scheme_Object *scheme_unsafe_real_sub1_proc;
extern Scheme_Object *scheme_unsafe_real_abs_proc;
extern Scheme_Object *scheme_unsafe_real_plus_proc;
extern Scheme_Object *scheme_unsafe_real_minus_proc;
extern Scheme_Object *scheme_unsafe_real_times_proc;
extern Scheme_Object *scheme_unsafe_real_divide_proc;
extern Scheme_Object *scheme_unsafe_real_modulo_proc;
extern Scheme_Object *scheme_unsafe_real_quotient_proc;
extern Scheme_Object *scheme_unsafe_real_remainder_proc;

extern Scheme_Object *scheme_unsafe_real_eq_proc;
extern Scheme_Object *scheme_unsafe_real_lt_proc;
extern Scheme_Object *scheme_unsafe_real_gt_proc;
extern Scheme_Object *scheme_unsafe_real_lt_eq_proc;
extern Scheme_Object *scheme_unsafe_real_gt_eq_proc;
extern Scheme_Object *scheme_unsafe_real_min_proc;
extern Scheme_Object *scheme_unsafe_real_max_proc;

extern Scheme_Object *scheme_unsafe_fx_eq_proc;
extern Scheme_Object *scheme_unsafe_fx_lt_proc;
extern Scheme_Object *scheme_unsafe_fx_gt_proc;
extern Scheme_Object *scheme_unsafe_fx_lt_eq_proc;
extern Scheme_Object *scheme_unsafe_fx_gt_eq_proc;
extern Scheme_Object *scheme_unsafe_fx_min_proc;
extern Scheme_Object *scheme_unsafe_fx_max_proc;

extern Scheme_Object *scheme_define_values_syntax, *scheme_define_syntaxes_syntax;
extern Scheme_Object *scheme_lambda_syntax;
extern Scheme_Object *scheme_begin_syntax;

extern Scheme_Object *scheme_not_proc;
extern Scheme_Object *scheme_true_object_p_proc;
extern Scheme_Object *scheme_boolean_p_proc;
extern Scheme_Object *scheme_eq_proc;
extern Scheme_Object *scheme_eqv_proc;
extern Scheme_Object *scheme_equal_proc;

extern Scheme_Object *scheme_def_exit_proc;

extern Scheme_Object *scheme_unsafe_poller_proc;

THREAD_LOCAL_DECL(extern Scheme_Object *scheme_orig_stdout_port);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_orig_stdin_port);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_orig_stderr_port);

extern Scheme_Object *scheme_arity_at_least, *scheme_make_arity_at_least;

extern Scheme_Object *scheme_write_proc, *scheme_display_proc, *scheme_print_proc;

extern Scheme_Object *scheme_raise_arity_error_proc;

extern Scheme_Object *scheme_date;

extern Scheme_Object *scheme_liberal_def_ctx_type;

THREAD_LOCAL_DECL(extern Scheme_Object *scheme_module_stx);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_modulestar_stx);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_begin_stx);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_module_begin_stx);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_define_values_stx);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_define_syntaxes_stx);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_begin_for_syntax_stx);
THREAD_LOCAL_DECL(extern Scheme_Object *scheme_top_stx);

extern Scheme_Object *scheme_recur_symbol, *scheme_display_symbol, *scheme_write_special_symbol;

extern Scheme_Object *scheme_none_symbol, *scheme_line_symbol, *scheme_block_symbol;

extern Scheme_Object *scheme_paren_shape_symbol;
extern Scheme_Object *scheme_paren_shape_preserve_square;
extern Scheme_Object *scheme_paren_shape_preserve_curly;
extern Scheme_Hash_Tree *scheme_source_stx_props;

extern Scheme_Object *scheme_stack_dump_key;

extern Scheme_Object *scheme_default_prompt_tag;

THREAD_LOCAL_DECL(extern Scheme_Object *scheme_system_idle_channel);

extern Scheme_Object *scheme_input_port_property, *scheme_output_port_property;
extern Scheme_Object *scheme_cpointer_property;

extern Scheme_Object *scheme_equal_property;
extern Scheme_Object *scheme_object_name_property;
extern Scheme_Object *scheme_impersonator_of_property;

extern Scheme_Object *scheme_app_mark_impersonator_property;

extern Scheme_Object *scheme_no_arity_property;

extern Scheme_Object *scheme_authentic_property;

extern Scheme_Object *scheme_chaperone_undefined_property;

extern Scheme_Object *scheme_reduced_procedure_struct;

/* recycle some constants that can't appear in code: */
#define scheme_constant_key scheme_stack_dump_key
#define scheme_fixed_key    scheme_default_prompt_tag

extern Scheme_Object *scheme_double_ctype;
extern Scheme_Object *scheme_float_ctype;
extern Scheme_Object *scheme_pointer_ctype;
extern Scheme_Object *scheme_int8_ctype;
extern Scheme_Object *scheme_uint8_ctype;
extern Scheme_Object *scheme_int16_ctype;
extern Scheme_Object *scheme_uint16_ctype;
extern Scheme_Object *scheme_int32_ctype;
extern Scheme_Object *scheme_uint32_ctype;
extern Scheme_Object *scheme_int64_ctype;
extern Scheme_Object *scheme_uint64_ctype;

/*========================================================================*/
/*                    thread state and maintenance                        */
/*========================================================================*/

#define RUNSTACK_IS_GLOBAL

#ifdef RUNSTACK_IS_GLOBAL
THREAD_LOCAL_DECL(extern Scheme_Object **scheme_current_runstack);
THREAD_LOCAL_DECL(extern Scheme_Object **scheme_current_runstack_start);
THREAD_LOCAL_DECL(extern MZ_MARK_STACK_TYPE scheme_current_cont_mark_stack);
THREAD_LOCAL_DECL(extern MZ_MARK_POS_TYPE scheme_current_cont_mark_pos);
# define MZ_RUNSTACK scheme_current_runstack
# define MZ_RUNSTACK_START scheme_current_runstack_start
# define MZ_CONT_MARK_STACK scheme_current_cont_mark_stack
# define MZ_CONT_MARK_POS scheme_current_cont_mark_pos
#else
# define MZ_RUNSTACK (scheme_current_thread->runstack)
# define MZ_RUNSTACK_START (scheme_current_thread->runstack_start)
# define MZ_CONT_MARK_STACK (scheme_current_thread->cont_mark_stack)
# define MZ_CONT_MARK_POS (scheme_current_thread->cont_mark_pos)
#endif

THREAD_LOCAL_DECL(extern volatile int scheme_fuel_counter);

THREAD_LOCAL_DECL(extern Scheme_Thread *scheme_main_thread);

#if defined(MZ_USE_PLACES) || defined(MZ_USE_FUTURES) || defined(USE_PTHREAD_THREAD_TIMER) || defined(WINDOWS_FILE_HANDLES)
# define MZ_USE_MZRT
#endif

#ifdef MZ_USE_MZRT
# include "mzrt.h"
#endif

#ifdef MZ_USE_PLACES
extern mz_proc_thread *scheme_master_proc_thread;
THREAD_LOCAL_DECL(extern mz_proc_thread *proc_thread_self);
#endif

THREAD_LOCAL_DECL(extern int scheme_no_stack_overflow);

typedef struct Scheme_Thread_Set {
  Scheme_Object so;
  struct Scheme_Thread_Set *parent;
  Scheme_Object *first;
  Scheme_Object *next;
  Scheme_Object *prev;
  Scheme_Object *search_start;
  Scheme_Object *current;
} Scheme_Thread_Set;

THREAD_LOCAL_DECL(extern Scheme_Thread_Set *scheme_thread_set_top);

#define SCHEME_TAIL_COPY_THRESHOLD 5

/* Flags for Scheme_Thread's `running' field: */
#define MZTHREAD_RUNNING 0x1
#define MZTHREAD_SUSPENDED 0x2
#define MZTHREAD_KILLED 0x4
#define MZTHREAD_NEED_KILL_CLEANUP 0x8
#define MZTHREAD_USER_SUSPENDED 0x10
#define MZTHREAD_NEED_SUSPEND_CLEANUP 0x20
#define MZTHREAD_STILL_RUNNING(running) ((running) && !((running) & MZTHREAD_KILLED))

#ifdef WINDOWS_PROCESSES
MZ_EXTERN struct Scheme_Thread_Memory *scheme_remember_thread(void *, int);
void scheme_remember_subthread(struct Scheme_Thread_Memory *, void *);
MZ_EXTERN void scheme_forget_thread(struct Scheme_Thread_Memory *);
void scheme_forget_subthread(struct Scheme_Thread_Memory *);
void scheme_suspend_remembered_threads(void);
void scheme_resume_remembered_threads(void);
#endif

void scheme_thread_start_child(Scheme_Thread *child, Scheme_Object *child_thunk);
void scheme_do_thread_start_child(Scheme_Thread *child, Scheme_Object *child_thunk);

int scheme_wait_until_suspend_ok(void);

#ifdef MZ_USE_MZRT
extern void scheme_check_foreign_work(void);
#endif

#ifndef DONT_USE_FOREIGN
XFORM_NONGCING extern void *scheme_extract_pointer(Scheme_Object *v);
#endif

Scheme_Object *scheme_foreign_ptr_ref(int argc, Scheme_Object **argv);
void scheme_foreign_ptr_set(int argc, Scheme_Object **argv);

Scheme_Object *scheme_cpointer_tag(Scheme_Object *ptr);
void scheme_set_cpointer_tag(Scheme_Object *ptr, Scheme_Object *val);

void scheme_kickoff_green_thread_time_slice_timer(intptr_t usec);

void scheme_prepare_this_thread_for_GC(Scheme_Thread *t);

Scheme_Object **scheme_alloc_runstack(intptr_t len);
void scheme_set_runstack_limits(Scheme_Object **rs, intptr_t len, intptr_t start, intptr_t end);
void scheme_check_runstack_edge(Scheme_Object **rs);

void scheme_alloc_list_stack(Scheme_Thread *p);
void scheme_clean_list_stack(Scheme_Thread *p);

Scheme_Object *scheme_get_thread_dead(Scheme_Thread *p);
Scheme_Object *scheme_get_thread_suspend(Scheme_Thread *p);
Scheme_Object *scheme_get_thread_sync(Scheme_Thread *p);
void scheme_clear_thread_sync(Scheme_Thread *p);

void scheme_zero_unneeded_rands(Scheme_Thread *p);

void scheme_realloc_tail_buffer(Scheme_Thread *p);

int scheme_can_break(Scheme_Thread *p);
void scheme_thread_wait(Scheme_Object *thread);

# define DO_CHECK_FOR_BREAK(p, e) \
	if (DECREMENT_FUEL(scheme_fuel_counter, 1) <= 0) { \
	  e scheme_thread_block(0); \
          (p)->ran_some = 1; \
	}

THREAD_LOCAL_DECL(extern int scheme_overflow_count);

#define MZTHREADELEM(p, x) scheme_ ## x

struct Scheme_Custodian {
  Scheme_Object so;
  char shut_down, has_limit, recorded;
  int count, alloc, elems;
  Scheme_Object ***boxes;
  Scheme_Custodian_Reference **mrefs;
  Scheme_Close_Custodian_Client **closers;
  void **data;
  void ***data_ptr; /* points to `data`, registered as finalizer data for strong retention */

  /* weak indirections: */
  Scheme_Custodian_Reference *parent;
  Scheme_Custodian_Reference *sibling;
  Scheme_Custodian_Reference *children;

  Scheme_Custodian_Reference *global_next;
  Scheme_Custodian_Reference *global_prev;

#ifdef MZ_PRECISE_GC
  int gc_owner_set;
  Scheme_Object *cust_boxes;
  int num_cust_boxes, checked_cust_boxes;
  int really_doing_accounting;
#endif
};

typedef struct Scheme_Custodian_Box {
  Scheme_Object so;
  Scheme_Custodian *cust;
  Scheme_Object *v;
} Scheme_Custodian_Box;

Scheme_Thread *scheme_do_close_managed(Scheme_Custodian *m, Scheme_Exit_Closer_Func f);
Scheme_Custodian *scheme_get_current_custodian(void);
void scheme_run_atexit_closers_on_all(Scheme_Exit_Closer_Func alt);
void scheme_run_atexit_closers(Scheme_Object *o, Scheme_Close_Custodian_Client *f, void *data);

typedef struct Scheme_Security_Guard {
  Scheme_Object so;
  struct Scheme_Security_Guard *parent;
  Scheme_Object *file_proc;    /* who-symbol path mode-symbol -> void */
  Scheme_Object *network_proc; /* who-symbol host-string-or-'listen port-k -> void */
  Scheme_Object *link_proc;    /* who-symbol path path -> void */
} Scheme_Security_Guard;

/* Always allocated on the stack: */
typedef struct {
  Scheme_Thread *false_positive_ok;  /* non-zero => return 1 to swap in thread rather than running Racket code */
  int potentially_false_positive; /* => returning 1 to swap thread in, but truth may be 0 */
  Scheme_Object *current_syncing;
  double sleep_end;
  int w_i;
  char spin, is_poll, no_redirect;
  Scheme_Object *replace_chain; /* turns non-tail replace_evt recursion into a loop */
} Scheme_Schedule_Info;

typedef Scheme_Object *(*Scheme_Accept_Sync)(Scheme_Object *wrap);

void scheme_set_sync_target(Scheme_Schedule_Info *sinfo, Scheme_Object *target,
			    Scheme_Object *wrap, Scheme_Object *nack,
			    int repost, int retry, Scheme_Accept_Sync accept);
struct Syncing;
void scheme_accept_sync(struct Syncing *syncing, int i);

struct Syncing *scheme_make_syncing(int argc, Scheme_Object **argv);
int scheme_syncing_ready(struct Syncing *s, Scheme_Schedule_Info *sinfo, int can_suspend);
void scheme_syncing_needs_wakeup(struct Syncing *s, void *fds);
void scheme_escape_during_sync(struct Syncing *syncing);
Scheme_Object *scheme_syncing_result(struct Syncing *syncing, int tailok);

struct Syncing *scheme_replace_evt_get(Scheme_Object *active_replace);
struct Syncing *scheme_replace_evt_nack(Scheme_Object *active_replace);
struct Syncing *scheme_replace_evt_needs_wakeup(Scheme_Object *o);

typedef int (*Scheme_Ready_Fun_FPC)(Scheme_Object *o, Scheme_Schedule_Info *sinfo);
typedef int (*Scheme_Out_Ready_Fun_FPC)(Scheme_Output_Port *port, Scheme_Schedule_Info *sinfo);
typedef int (*Scheme_In_Ready_Fun_FPC)(Scheme_Input_Port *port, Scheme_Schedule_Info *sinfo);

void scheme_check_break_now(void);

THREAD_LOCAL_DECL(extern int scheme_main_was_once_suspended);

/* A "flattened" config. Maps parameters to thread cells. */
typedef struct {
  MZTAG_IF_REQUIRED
  Scheme_Bucket_Table *extensions;
  Scheme_Object *prims[mzFLEX_ARRAY_DECL];
} Scheme_Parameterization;

struct Scheme_Config {
  Scheme_Object so;
  Scheme_Hash_Tree *ht;
  Scheme_Parameterization *root;
};

extern Scheme_Object *scheme_parameterization_key;
extern Scheme_Object *scheme_exn_handler_key;
extern Scheme_Object *scheme_break_enabled_key;

Scheme_Object *scheme_extend_parameterization(int argc, Scheme_Object *args[]);
XFORM_NONGCING int scheme_is_parameter(Scheme_Object *o);

extern void scheme_flatten_config(Scheme_Config *c);

extern Scheme_Object *scheme_apply_thread_thunk(Scheme_Object *rator);

Scheme_Custodian* scheme_custodian_extract_reference(Scheme_Custodian_Reference *mr);

/*========================================================================*/
/*                       hash tables and globals                          */
/*========================================================================*/

/* a primitive constant: */
#define GLOB_IS_CONST 1
/* always defined as the same kind of value (e.g., proc with a particular arity): */
#define GLOB_IS_CONSISTENT 2
/* whether home_link is strong or weak: */
#define GLOB_STRONG_HOME_LINK 4
/* a kernel constant: */
#define GLOB_HAS_REF_ID 16
/* can cast to Scheme_Bucket_With_Home: */
#define GLOB_HAS_HOME_PTR 32
/* Racket-level constant (cannot be changed further): */
#define GLOB_IS_IMMUTATED 64
/* Linked from other (cannot be undefined): */
#define GLOB_IS_LINKED 128

typedef struct {
  Scheme_Bucket bucket;
  short flags, id;
} Scheme_Bucket_With_Flags;

typedef Scheme_Bucket_With_Flags Scheme_Bucket_With_Ref_Id;

typedef struct {
  Scheme_Bucket_With_Ref_Id bucket;
  Scheme_Object *home_link; /* weak to Scheme_Env *, except when GLOB_STRONG_HOME_LINK */
} Scheme_Bucket_With_Home;

Scheme_Env *scheme_get_bucket_home(Scheme_Bucket *b);
void scheme_set_bucket_home(Scheme_Bucket *b, Scheme_Env *e);
Scheme_Object *scheme_get_home_weak_link(Scheme_Env *e);

Scheme_Object *
scheme_get_primitive_global(Scheme_Object *var, Scheme_Env *env,
			    int bucket_ok, int can_opt, int signal);

void scheme_add_bucket_to_table(Scheme_Bucket_Table *table, Scheme_Bucket *b);
Scheme_Bucket *scheme_bucket_or_null_from_table(Scheme_Bucket_Table *table, const char *key, int add);

typedef unsigned int hash_tree_bitmap_t; /* must be unsigned int */
struct Scheme_Hash_Tree {
  Scheme_Inclhash_Object iso; /* 0 => keys only; 0x1 => keys and values; 0x3 => keys, values, and codes */
  hash_tree_bitmap_t bitmap;
  intptr_t count;
  Scheme_Object *els[mzFLEX_ARRAY_DECL]; /* keys, then vals (if any), then codes (if any) */
};

#define SCHEME_HASHTR_FLAGS(tr) MZ_OPT_HASH_KEY(&(tr)->iso)
#define SCHEME_HASHTR_KIND(tr) (SCHEME_HASHTR_FLAGS(tr) & 0x3)

#define SCHEME_HASHTR_TYPE(tr) (SAME_TYPE(SCHEME_TYPE(tr), scheme_hash_tree_indirection_type) \
                                ? SCHEME_TYPE(((Scheme_Hash_Tree *)tr)->els[0]) \
                                : SCHEME_TYPE(tr))

Scheme_Object *scheme_intern_literal_string(Scheme_Object *str);
Scheme_Object *scheme_intern_literal_number(Scheme_Object *num);

/*========================================================================*/
/*                    hash functions                                      */
/*========================================================================*/

Scheme_Object *scheme_make_immutable_hash(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_make_immutable_hasheq(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_make_immutable_hasheqv(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_eq_p(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_eqv_p(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_equal_p(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_table_put(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_table_iterate_start(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_table_iterate_next(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_table_iterate_value(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_hash_table_iterate_key(int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_hash_get_w_key_wraps(Scheme_Hash_Table *table, Scheme_Object *key,
                                           Scheme_Object *key_wraps);
void scheme_hash_set_w_key_wraps(Scheme_Hash_Table *table, Scheme_Object *key, Scheme_Object *val,
                                 Scheme_Object *key_wraps);
Scheme_Bucket *scheme_bucket_or_null_from_table_w_key_wraps(Scheme_Bucket_Table *table,
                                                            const char *key, int add,
                                                            Scheme_Object *key_wraps);
void scheme_add_to_table_w_key_wraps(Scheme_Bucket_Table *table, const char *key, void *val, 
                                     int constant, Scheme_Object *key_wraps);
void *scheme_lookup_in_table_w_key_wraps(Scheme_Bucket_Table *table, const char *key,
                                         Scheme_Object *key_wraps);
Scheme_Object *scheme_hash_tree_get_w_key_wraps(Scheme_Hash_Tree *tree, Scheme_Object *key,
                                                Scheme_Object *key_wraps);
Scheme_Hash_Tree *scheme_hash_tree_set_w_key_wraps(Scheme_Hash_Tree *tree, Scheme_Object *key, Scheme_Object *val,
                                                   Scheme_Object *key_wraps);

Scheme_Object *scheme_unsafe_hash_tree_start(Scheme_Hash_Tree *ht);
XFORM_NONGCING_NONALIASING void scheme_unsafe_hash_tree_subtree(Scheme_Object *obj, Scheme_Object *args,
                                                                Scheme_Hash_Tree **_subtree, int *_i);
XFORM_NONGCING Scheme_Object *scheme_unsafe_hash_tree_access(Scheme_Hash_Tree *subtree, int i);
Scheme_Object *scheme_unsafe_hash_tree_next(Scheme_Hash_Tree *ht, Scheme_Object *args);
Scheme_Object *scheme_hash_tree_next_pos(Scheme_Hash_Tree *tree, mzlonglong pos);
int scheme_hash_tree_equal(Scheme_Hash_Tree *t1, Scheme_Hash_Tree *t2);
int scheme_is_hash_tree_equal(Scheme_Object *o);
int scheme_is_hash_tree_eqv(Scheme_Object *o);

/*========================================================================*/
/*                              structs                                   */
/*========================================================================*/

typedef struct Scheme_Inspector {
  Scheme_Object so;
  int depth;
  struct Scheme_Inspector *superior;
} Scheme_Inspector;

typedef struct Scheme_Struct_Property {
  Scheme_Object so;
  char can_impersonate; /* 1 if impersonatable property, 0 otherwise */
  Scheme_Object *name; /* a symbol */
  Scheme_Object *guard; /* NULL, a procedure, or 'can-impersonate */
  Scheme_Object *supers; /* implied properties: listof (cons <prop> <proc>) */
} Scheme_Struct_Property;

int scheme_inspector_sees_part(Scheme_Object *s, Scheme_Object *insp, int pos);
int scheme_struct_is_transparent(Scheme_Object *s);

typedef struct Scheme_Struct_Type {
  Scheme_Inclhash_Object iso; /* scheme_struct_type_type */
  mzshort num_slots;   /* initialized + auto + parent-initialized + parent-auto */
  mzshort num_islots; /* initialized + parent-initialized */
  mzshort name_pos;
  char authentic; /* 1 => chaperones/impersonators disallowed */

  Scheme_Object *name;

  Scheme_Object *inspector;
  Scheme_Object *accessor, *mutator;
  Scheme_Object *prefab_key;

  Scheme_Object *uninit_val;

  Scheme_Object **props; /* normally an array of pair of (property, value) pairs */
  int num_props; /* < 0 => props is really a hash table */

  Scheme_Object *proc_attr; /* int (position) or proc, only for proc_struct */
  char *immutables; /* for immediate slots, only (not parent) */

  Scheme_Object *guard;

#if defined(MZ_GC_BACKTRACE) && defined(MZ_PRECISE_GC)
  intptr_t current_instance_count;
  intptr_t current_instance_sizes;
  intptr_t total_instance_count;
  intptr_t total_instance_sizes;
#endif

  struct Scheme_Struct_Type *parent_types[mzFLEX_ARRAY_DECL];
} Scheme_Struct_Type;

#define STRUCT_TYPE_ALL_IMMUTABLE 0x1
#define STRUCT_TYPE_CHECKED_PROC  0x2

typedef struct Scheme_Structure
{
  Scheme_Object so;
  Scheme_Struct_Type *stype;
  Scheme_Object *slots[mzFLEX_ARRAY_DECL];
} Scheme_Structure;

#ifdef MZ_USE_PLACES
typedef struct Scheme_Serialized_Structure
{
  Scheme_Object so;
  Scheme_Object *prefab_key;
  int num_slots;
  Scheme_Object *slots[mzFLEX_ARRAY_DECL];
} Scheme_Serialized_Structure;
#endif

#define SCHEME_STRUCT_TYPE(o) (((Scheme_Structure *)o)->stype)

#define SCHEME_STRUCT_NUM_SLOTS(o) (SCHEME_STRUCT_TYPE(o)->num_slots)
#define SCHEME_STRUCT_NAME_SYM(o) (SCHEME_STRUCT_TYPE(o)->name)

Scheme_Object **scheme_make_struct_names_from_array(const char *base,
						    int fcount,
						    const char **field_names,
						    int flags, int *count_out);
Scheme_Object *scheme_make_struct_type_from_string(const char *base,
						   Scheme_Object *parent,
						   int num_fields,
						   Scheme_Object *props,
						   Scheme_Object *guard,
						   int immutable);

Scheme_Object *scheme_struct_to_vector(Scheme_Object *_s, Scheme_Object *unknown_val, Scheme_Object *insp);

Scheme_Object *scheme_extract_struct_procedure(Scheme_Object *obj, int num_rands, Scheme_Object **rands, int *is_method);

Scheme_Object *scheme_proc_struct_name_source(Scheme_Object *a);
Scheme_Object *scheme_object_name(Scheme_Object *a);

int scheme_is_simple_struct_type(Scheme_Struct_Type *stype);

Scheme_Object *scheme_is_writable_struct(Scheme_Object *s);
Scheme_Object *scheme_print_attribute_ref(Scheme_Object *s);

#define SCHEME_STRUCT_INSPECTOR(obj) (((Scheme_Structure *)obj)->stype->inspector)

extern Scheme_Object *scheme_source_property;
extern Scheme_Object *scheme_module_path_property;

Scheme_Struct_Type *scheme_lookup_prefab_type(Scheme_Object *key, int field_count);
Scheme_Object *scheme_make_blank_prefab_struct_instance(Scheme_Struct_Type *stype);
Scheme_Object *scheme_make_prefab_struct_instance(Scheme_Struct_Type *stype,
                                                         Scheme_Object *vec);
Scheme_Object *scheme_clone_prefab_struct_instance(Scheme_Structure *s);
Scheme_Struct_Type *scheme_make_prefab_struct_type_in_master(Scheme_Object *base,
					Scheme_Object *parent,
					int num_slots,
					int num_islots,
					Scheme_Object *uninit_val,
					char *immutable_pos_list);
Scheme_Struct_Type *scheme_make_prefab_struct_type_raw(Scheme_Object *base,
					Scheme_Object *parent,
					int num_slots,
					int num_islots,
					Scheme_Object *uninit_val,
					char *immutable_pos_list);
Scheme_Object *scheme_prefab_struct_key(Scheme_Object *s);
#ifdef MZ_USE_PLACES
Scheme_Object *scheme_make_serialized_struct_instance(Scheme_Object *s, int num_slots);
#endif

Scheme_Object *scheme_struct_getter(int argc, Scheme_Object **args, Scheme_Object *prim);
Scheme_Object *scheme_struct_setter(int argc, Scheme_Object **args, Scheme_Object *prim);

void scheme_force_struct_type_info(Scheme_Struct_Type *stype);

Scheme_Object *scheme_extract_checked_procedure(int argc, Scheme_Object **argv);

Scheme_Object *scheme_rename_struct_proc(Scheme_Object *p, Scheme_Object *sym);

typedef struct Scheme_Chaperone {
  Scheme_Inclhash_Object iso; /* 0x1 => impersonator, rather than a checking chaperone */
  Scheme_Object *val;  /* root object */
  Scheme_Object *prev; /* immediately chaperoned object */
  Scheme_Hash_Tree *props;
  Scheme_Object *redirects; /* specific to the type of chaperone and root object */
} Scheme_Chaperone;

#define SCHEME_CHAPERONE_FLAGS(c) MZ_OPT_HASH_KEY(&(c)->iso)
#define SCHEME_CHAPERONE_IS_IMPERSONATOR 0x1
#define SCHEME_PROC_CHAPERONE_CALL_DIRECT 0x2
/*
We use the same bit to indicate either chaperone-vector* as well as
procedure chaperones which do not call interposition procedures.
This is ok because no value is simultaneously a vector and a procedure,
so we can safely reuse the bit.
 */
#define SCHEME_VEC_CHAPERONE_STAR 0x2

#define SCHEME_CHAPERONE_VAL(obj) (((Scheme_Chaperone *)obj)->val)

#define SCHEME_P_CHAPERONEP(obj) (SAME_TYPE(SCHEME_TYPE(obj), scheme_proc_chaperone_type))
#define SCHEME_NP_CHAPERONEP(obj) (SAME_TYPE(SCHEME_TYPE(obj), scheme_chaperone_type))

/* Does the shape of the redirects field match the pattern for particular chaperone types */
#define SCHEME_REDIRECTS_PROCEDUREP(red) (SCHEME_VECTORP(red) \
					  && (SCHEME_VEC_SIZE(red) & 1))
#define SCHEME_REDIRECTS_STRUCTP(red) (SCHEME_VECTORP(red)		\
				       && SCHEME_VEC_SIZE(red)		\
				       && !(SCHEME_VEC_SIZE(red) & 1))
#define SCHEME_REDIRECTS_PROP_ONLY_VECTORP(red) (SCHEME_VECTORP(red)	\
						 && !(SCHEME_VEC_SIZE(red)))

#define SCHEME_CHAPERONE_VECTORP(obj) (SCHEME_VECTORP(obj) \
                                   || (SCHEME_NP_CHAPERONEP(obj) && SCHEME_VECTORP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_BOXP(obj) (SCHEME_BOXP(obj) \
                                || (SCHEME_NP_CHAPERONEP(obj) && SCHEME_BOXP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_STRUCTP(obj) (SCHEME_STRUCTP(obj)              \
                                       || (SCHEME_CHAPERONEP(obj) && SCHEME_STRUCTP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_PROC_STRUCTP(obj) (SCHEME_PROC_STRUCTP(obj)              \
                                           || (SCHEME_P_CHAPERONEP(obj) && SCHEME_PROC_STRUCTP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_STRUCT_TYPEP(obj) (SCHEME_STRUCT_TYPEP(obj)              \
                                            || (SCHEME_NP_CHAPERONEP(obj) && SCHEME_STRUCT_TYPEP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_HASHTP(obj) (SCHEME_HASHTP(obj) \
                                      || (SCHEME_NP_CHAPERONEP(obj) && SCHEME_HASHTP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_HASHTRP(obj) (SCHEME_HASHTRP(obj) \
                                       || (SCHEME_NP_CHAPERONEP(obj) && SCHEME_HASHTRP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_BUCKTP(obj) (SCHEME_BUCKTP(obj) \
                                      || (SCHEME_NP_CHAPERONEP(obj) && SCHEME_BUCKTP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_PROMPT_TAGP(obj) (SCHEME_PROMPT_TAGP(obj) \
                                           || (SCHEME_NP_CHAPERONEP(obj) && SCHEME_PROMPT_TAGP(SCHEME_CHAPERONE_VAL(obj))))
#define SCHEME_CHAPERONE_CONTINUATION_MARK_KEYP(obj) (SCHEME_CONTINUATION_MARK_KEYP(obj) \
                                                      || (SCHEME_NP_CHAPERONEP(obj) \
                                                          && SCHEME_CONTINUATION_MARK_KEYP(SCHEME_CHAPERONE_VAL(obj))))

#define SCHEME_CHAPERONE_VEC_SIZE(obj) (SCHEME_NP_CHAPERONEP(obj) ? SCHEME_VEC_SIZE(SCHEME_CHAPERONE_VAL(obj)) : SCHEME_VEC_SIZE(obj))

Scheme_Object *scheme_chaperone_vector_ref(Scheme_Object *o, int i);
void scheme_chaperone_vector_set(Scheme_Object *o, int i, Scheme_Object *v);

Scheme_Object *scheme_apply_chaperone(Scheme_Object *o, int argc, Scheme_Object **argv, 
                                      Scheme_Object *auto_val, int checks);

Scheme_Hash_Tree *scheme_parse_chaperone_props(const char *who, int start_at, int argc, Scheme_Object **argv);

Scheme_Object *scheme_chaperone_hash_get(Scheme_Object *table, Scheme_Object *key);
Scheme_Object *scheme_chaperone_hash_traversal_get(Scheme_Object *table, Scheme_Object *key, Scheme_Object **alt_key);
void scheme_chaperone_hash_set(Scheme_Object *table, Scheme_Object *key, Scheme_Object *val);

Scheme_Object *scheme_chaperone_not_undefined(Scheme_Object *orig_val);

int scheme_is_noninterposing_chaperone(Scheme_Object *obj);

Scheme_Object *scheme_apply_impersonator_of(int for_chaperone, Scheme_Object *procs, Scheme_Object *obj);

/*========================================================================*/
/*                         syntax objects                                 */
/*========================================================================*/

#define MZ_LABEL_PHASE 30000

typedef struct Scheme_Stx_Srcloc {
  MZTAG_IF_REQUIRED
  intptr_t line, col, pos, span;
  Scheme_Object *src;
} Scheme_Stx_Srcloc;

#define STX_SUBSTX_FLAG   0x1
#define STX_ARMED_FLAG    0x2

typedef struct Scheme_Scope_Set Scheme_Scope_Set;

typedef struct Scheme_Scope_Table {
  Scheme_Object so; /* scheme_scope_table_type or scheme_propagate_table_type */
  Scheme_Scope_Set *simple_scopes; /* scopes that span all phases */
  Scheme_Object *multi_scopes; /* list of (cons multi-scope phase-shift) or fallback chain */
} Scheme_Scope_Table;

typedef struct Scheme_Stx {
  Scheme_Inclhash_Object iso; /* 0x1 and 0x2 of keyex used */
  Scheme_Object *val;
  Scheme_Stx_Srcloc *srcloc;
  Scheme_Scope_Table *scopes;
  union {
    Scheme_Scope_Table *to_propagate;
    Scheme_Object *cache;
  } u;
  Scheme_Object *shifts; /* <all-shifts> or (vector <all-shifts> <shifts-to-propagate> <base-shifts>) */
  Scheme_Object *taints; /* taint or taint-arming */
  Scheme_Hash_Tree *props;
} Scheme_Stx;

typedef struct Scheme_Stx_Offset {
  Scheme_Object so;
  intptr_t line, col, pos;
  Scheme_Object *src;
} Scheme_Stx_Offset;

struct Scheme_Marshal_Tables;
struct Scheme_Unmarshal_Tables;

Scheme_Object *scheme_make_stx(Scheme_Object *val,
			       Scheme_Stx_Srcloc *srcloc,
			       Scheme_Hash_Tree *props);
Scheme_Object *scheme_make_stx_w_offset(Scheme_Object *val,
					intptr_t line, intptr_t col, intptr_t pos, intptr_t span,
					Scheme_Object *src,
					Scheme_Hash_Tree *props);

Scheme_Object *scheme_datum_to_syntax(Scheme_Object *o, Scheme_Object *stx_src,
				      Scheme_Object *stx_wraps,
				      int cangraph, int copyprops);
Scheme_Object *scheme_syntax_to_datum(Scheme_Object *stx, int with_scopes,
				      struct Scheme_Marshal_Tables *mt);
Scheme_Object *scheme_unmarshal_datum_to_syntax(Scheme_Object *o,
                                                struct Scheme_Unmarshal_Tables *ut,
                                                int can_graph);

Scheme_Object *scheme_stx_track(Scheme_Object *naya,
				Scheme_Object *old,
				Scheme_Object *origin);

int scheme_stx_has_empty_wraps(Scheme_Object *stx, Scheme_Object *phase);

int scheme_syntax_is_original(Scheme_Object *_stx);
Scheme_Object *scheme_syntax_remove_original(Scheme_Object *_stx);

XFORM_NONGCING Scheme_Object *scheme_stx_root_scope();
Scheme_Object *scheme_new_scope(int kind);
Scheme_Object *scheme_scope_printed_form(Scheme_Object *m);
Scheme_Object *scheme_stx_scope(Scheme_Object *o, Scheme_Object *m, int mode);

#define SCHEME_STX_MODULE_SCOPE       0
#define SCHEME_STX_MODULE_MULTI_SCOPE 1
#define SCHEME_STX_MACRO_SCOPE        2
#define SCHEME_STX_LOCAL_BIND_SCOPE   3
#define SCHEME_STX_INTDEF_SCOPE       4
#define SCHEME_STX_USE_SITE_SCOPE     5

#define SCHEME_STX_SCOPE_KIND_SHIFT 3
#define SCHEME_STX_SCOPE_KIND_MASK  ((1 << SCHEME_STX_SCOPE_KIND_SHIFT) - 1)

#define SCHEME_STX_ADD      0
#define SCHEME_STX_REMOVE   1
#define SCHEME_STX_FLIP     2
#define SCHEME_STX_PUSH     4
#define SCHEME_STX_MUTATE   16 /* or'ed */
#define SCHEME_STX_PROPONLY 32 /* or'ed, internal */
Scheme_Object *scheme_stx_adjust_scope(Scheme_Object *o, Scheme_Object *m, Scheme_Object *phase, int mode);
Scheme_Object *scheme_stx_add_scope(Scheme_Object *o, Scheme_Object *m, Scheme_Object *phase);
Scheme_Object *scheme_stx_remove_scope(Scheme_Object *o, Scheme_Object *m, Scheme_Object *phase);
Scheme_Object *scheme_stx_flip_scope(Scheme_Object *o, Scheme_Object *m, Scheme_Object *phase);
Scheme_Object *scheme_stx_adjust_scopes(Scheme_Object *o, Scheme_Scope_Set *scopes, Scheme_Object *phase, int mode);

Scheme_Scope_Set *scheme_module_context_scopes(Scheme_Object *mc);
Scheme_Object *scheme_module_context_frame_scopes(Scheme_Object *mc, Scheme_Object *keep_intdef_scopes);
void scheme_module_context_add_use_site_scope(Scheme_Object *mc, Scheme_Object *use_site_scope);
Scheme_Object *scheme_stx_adjust_frame_scopes(Scheme_Object *o, Scheme_Object *scope, Scheme_Object *phase, int mode);
Scheme_Object *scheme_stx_adjust_frame_bind_scopes(Scheme_Object *o, Scheme_Object *scope, Scheme_Object *phase, int mode);
Scheme_Object *scheme_stx_adjust_frame_use_site_scopes(Scheme_Object *o, Scheme_Object *scope, Scheme_Object *phase, int mode);

Scheme_Object *scheme_make_frame_scopes(Scheme_Object *scope);
Scheme_Object *scheme_add_frame_use_site_scope(Scheme_Object *frame_scopes, Scheme_Object *use_site_scope);
Scheme_Object *scheme_add_frame_intdef_scope(Scheme_Object *frame_scopes, Scheme_Object *intdef_scope);

Scheme_Object *scheme_make_shift(Scheme_Object *phase_delta,
                                 Scheme_Object *old_midx, Scheme_Object *new_midx,
                                 Scheme_Hash_Table *export_registry,
                                 Scheme_Object *src_insp_desc, Scheme_Object *insp);
Scheme_Object *scheme_stx_add_shift(Scheme_Object *o, Scheme_Object *shift);
Scheme_Object *scheme_stx_add_shifts(Scheme_Object *o, Scheme_Object *shift);
Scheme_Object *scheme_stx_shift(Scheme_Object *stx,
                                Scheme_Object *phase_delta,
                                Scheme_Object *old_midx, Scheme_Object *new_midx,
                                Scheme_Hash_Table *export_registry,
                                Scheme_Object *src_insp_desc, Scheme_Object *insp);

Scheme_Object *scheme_syntax_make_transfer_intro(int argc, Scheme_Object **argv);
int scheme_get_introducer_mode(const char *who, int which, int argc, Scheme_Object **argv);

struct Scheme_Module_Phase_Exports; /* forward declaration */

Scheme_Object *scheme_make_module_context(Scheme_Object *insp,
                                          Scheme_Object *shift_or_shifts,
                                          Scheme_Object *name);
Scheme_Object *scheme_module_context_at_phase(Scheme_Object *mc, Scheme_Object *phase);

Scheme_Object *scheme_stx_add_module_context(Scheme_Object *stx, Scheme_Object *mc);
Scheme_Object *scheme_stx_remove_module_context(Scheme_Object *stx, Scheme_Object *mc);
Scheme_Object *scheme_stx_add_module_frame_context(Scheme_Object *stx, Scheme_Object *mc);
Scheme_Object *scheme_stx_adjust_module_use_site_context(Scheme_Object *stx, Scheme_Object *mc, int mode);
Scheme_Object *scheme_stx_introduce_to_module_context(Scheme_Object *stx, Scheme_Object *mc);
Scheme_Object *scheme_stx_unintroduce_from_module_context(Scheme_Object *stx, Scheme_Object *mc);
Scheme_Object *scheme_stx_push_module_context(Scheme_Object *stx, Scheme_Object *mc);
Scheme_Object *scheme_stx_push_introduce_module_context(Scheme_Object *stx, Scheme_Object *mc);

Scheme_Object *scheme_stx_from_module_context_to_generic(Scheme_Object *stx, Scheme_Object *mc);
Scheme_Object *scheme_stx_from_generic_to_module_context(Scheme_Object *stx, Scheme_Object *mc);

Scheme_Object *scheme_module_context_to_stx(Scheme_Object *mc, Scheme_Object *orig_src);
Scheme_Object *scheme_stx_to_module_context(Scheme_Object *stx);

Scheme_Object *scheme_module_context_use_site_frame_scopes(Scheme_Object *mc);
Scheme_Object *scheme_module_context_inspector(Scheme_Object *mc);

void scheme_module_context_add_mapped_symbols(Scheme_Object *mc, Scheme_Hash_Table *mapped);

XFORM_NONGCING void scheme_stx_set(Scheme_Object *q_stx, Scheme_Object *val, Scheme_Object *context);

void scheme_extend_module_context(Scheme_Object *mc, Scheme_Object *ctx, Scheme_Object *modidx,
                                  Scheme_Object *locname, Scheme_Object *exname,
                                  Scheme_Object *nominal_src, Scheme_Object *nominal_ex,
                                  intptr_t mod_phase, Scheme_Object *src_phase_index, 
                                  Scheme_Object *nom_export_phase);
void scheme_extend_module_context_with_shared(Scheme_Object *mc, Scheme_Object *modidx, 
                                              struct Scheme_Module_Phase_Exports *pt, 
                                              Scheme_Object *prefix,
                                              Scheme_Hash_Tree *excepts,
                                              Scheme_Object *src_phase_index, 
                                              Scheme_Object *context,
                                              Scheme_Object *replace_at);

void scheme_do_module_context_unmarshal(Scheme_Object *modidx, Scheme_Object *req_modidx,
                                        Scheme_Object *context,
                                        Scheme_Object *bind_phase, Scheme_Object *pt_phase, Scheme_Object *src_phase,
                                        Scheme_Object *prefix,
                                        Scheme_Hash_Tree *excepts,
                                        Scheme_Hash_Table *export_registry,
                                        Scheme_Object *insp, Scheme_Object *req_insp,
                                        Scheme_Object *replace_at);

int scheme_stx_equal_module_context(Scheme_Object *stx, Scheme_Object *mc_as_stx);

Scheme_Object *scheme_stx_content(Scheme_Object *o);
Scheme_Object *scheme_flatten_syntax_list(Scheme_Object *lst, int *islist);

int scheme_stx_could_bind(Scheme_Object *bind_id, Scheme_Object *ref_id, Scheme_Object *phase);

int scheme_stx_free_eq(Scheme_Object *a, Scheme_Object *b, intptr_t phase);
int scheme_stx_free_eq_x(Scheme_Object *a, Scheme_Object *b, intptr_t b_phase);
int scheme_stx_free_eq2(Scheme_Object *a, Scheme_Object *b, Scheme_Object *phase);
int scheme_stx_free_eq3(Scheme_Object *a, Scheme_Object *b, 
                        Scheme_Object *a_phase, Scheme_Object *b_phase);
Scheme_Object *scheme_stx_get_module_eq_sym(Scheme_Object *a, Scheme_Object *phase);

void scheme_add_local_binding(Scheme_Object *o, Scheme_Object *phase, Scheme_Object *binding_sym);
void scheme_add_module_binding(Scheme_Object *o, Scheme_Object *phase, 
                               Scheme_Object *modidx, Scheme_Object *inspector,
                               Scheme_Object *sym, Scheme_Object *defn_phase);
void scheme_add_module_binding_w_nominal(Scheme_Object *o, Scheme_Object *phase,
                                         Scheme_Object *modidx, Scheme_Object *defn_name, Scheme_Object *defn_phase,
                                         Scheme_Object *inspector,
                                         Scheme_Object *nominal_mod, Scheme_Object *nominal_name,
                                         Scheme_Object *nominal_import_phase, 
                                         Scheme_Object *nominal_export_phase,
                                         struct Scheme_Module_Phase_Exports *from_pt,
                                         Scheme_Hash_Table *collapse_table);
void scheme_add_binding_copy(Scheme_Object *o, Scheme_Object *from_o, Scheme_Object *phase);

Scheme_Object *scheme_stx_lookup(Scheme_Object *o, Scheme_Object *phase);
Scheme_Object *scheme_stx_lookup_stop_at_free_eq(Scheme_Object *o, Scheme_Object *phase, int *_exact_match);
Scheme_Object *scheme_stx_lookup_exact(Scheme_Object *o, Scheme_Object *phase);
Scheme_Object *scheme_stx_lookup_w_nominal(Scheme_Object *o, Scheme_Object *phase,
                                           int stop_at_free_eq,
                                           int *_exact_match, int *_ambiguous,
                                           Scheme_Scope_Set **_binding_scopes,
                                           Scheme_Object **insp,              /* access-granting inspector */
                                           Scheme_Object **nominal_modidx,    /* how it was imported */
                                           Scheme_Object **nominal_name,      /* imported as name */
                                           Scheme_Object **src_phase,         /* phase level of import from nominal modidx */ 
                                           Scheme_Object **nominal_src_phase); /* phase level of export from nominal modidx */

int scheme_stx_bound_eq(Scheme_Object *a, Scheme_Object *b, Scheme_Object *phase);
int scheme_stx_env_bound_eq(Scheme_Object *a, Scheme_Object *b, Scheme_Object *phase);
int scheme_stx_env_bound_eq2(Scheme_Object *a, Scheme_Object *b,
                             Scheme_Object *a_phase, Scheme_Object *b_phase);

Scheme_Object *scheme_stx_binding_union(Scheme_Object *o, Scheme_Object *b, Scheme_Object *phase);
Scheme_Object *scheme_stx_binding_subtract(Scheme_Object *o, Scheme_Object *b, Scheme_Object *phase);

Scheme_Object *scheme_stx_source_module(Scheme_Object *stx, int resolve, int source);

char *scheme_stx_describe_context(Scheme_Object *stx, Scheme_Object *phase, int always);

Scheme_Object *scheme_stx_property(Scheme_Object *_stx,
				   Scheme_Object *key,
				   Scheme_Object *val);
Scheme_Object *scheme_stx_property2(Scheme_Object *_stx,
                                    Scheme_Object *key,
                                    Scheme_Object *val,
                                    int preserve);

int scheme_stx_list_length(Scheme_Object *list);
int scheme_stx_proper_list_length(Scheme_Object *list);

Scheme_Object *scheme_resolve_placeholders(Scheme_Object *obj);

#define SCHEME_STX_VAL(s) ((Scheme_Stx *)s)->val

#define SCHEME_STX_PAIRP(o) (SCHEME_PAIRP(o) || (SCHEME_STXP(o) && SCHEME_PAIRP(SCHEME_STX_VAL(o))))
#define SCHEME_STX_SYMBOLP(o) (SCHEME_SYMBOLP(o) || (SCHEME_STXP(o) && SCHEME_SYMBOLP(SCHEME_STX_VAL(o))))
#define SCHEME_STX_NULLP(o) (SCHEME_NULLP(o) || (SCHEME_STXP(o) && SCHEME_NULLP(SCHEME_STX_VAL(o))))

#define SCHEME_STX_CAR(o) (SCHEME_PAIRP(o) ? SCHEME_CAR(o) : SCHEME_CAR(scheme_stx_content(o)))
#define SCHEME_STX_CDR(o) (SCHEME_PAIRP(o) ? SCHEME_CDR(o) : SCHEME_CDR(scheme_stx_content(o)))
#define SCHEME_STX_SYM(o) (SCHEME_STXP(o) ? SCHEME_STX_VAL(o) : o)

Scheme_Object *scheme_source_to_name(Scheme_Object *code);

#define STX_SRCTAG scheme_source_stx_props

Scheme_Object *scheme_stx_taint(Scheme_Object *o);
Scheme_Object *scheme_stx_taint_arm(Scheme_Object *o, Scheme_Object *insp);
Scheme_Object *scheme_stx_taint_rearm(Scheme_Object *o, Scheme_Object *arm_from);
int scheme_stx_is_tainted(Scheme_Object *id);
int scheme_stx_is_clean(Scheme_Object *id);
int scheme_module_protected_wrt(Scheme_Object *home_insp, Scheme_Object *insp);
Scheme_Object *scheme_stx_taint_disarm(Scheme_Object *o, Scheme_Object *insp);

/* variants that use 'taint-mode and look up inspector: */
Scheme_Object *scheme_syntax_taint_arm(Scheme_Object *stx, Scheme_Object *insp, int use_mode);
Scheme_Object *scheme_syntax_taint_rearm(Scheme_Object *o, Scheme_Object *arm_from);
Scheme_Object *scheme_syntax_taint_disarm(Scheme_Object *o, Scheme_Object *insp);

Scheme_Object *scheme_delayed_shift(Scheme_Object **o, intptr_t i);

struct Resolve_Prefix;
void scheme_load_delayed_syntax(struct Resolve_Prefix *rp, intptr_t i);

Scheme_Object *scheme_stx_force_delayed(Scheme_Object *stx);

Scheme_Object *scheme_explode_syntax(Scheme_Object *stx, Scheme_Hash_Table *ht);
void scheme_populate_pt_ht(struct Scheme_Module_Phase_Exports * pt);

Scheme_Object *scheme_transfer_srcloc(Scheme_Object *to, Scheme_Object *from);

int scheme_is_predefined_module_p(Scheme_Object *name);

Scheme_Object *scheme_get_kernel_modidx(void);

Scheme_Object *scheme_scope_marshal_content(Scheme_Object *m, struct Scheme_Marshal_Tables *mt);
void scheme_iterate_reachable_scopes(struct Scheme_Marshal_Tables *mt);

void scheme_stx_debug_print(Scheme_Object *stx, Scheme_Object *phase, int level);

Scheme_Object *scheme_revert_use_site_scopes(Scheme_Object *o, struct Scheme_Comp_Env *env);

Scheme_Object *scheme_top_introduce(Scheme_Object *form, Scheme_Env *genv);

/*========================================================================*/
/*                   syntax run-time structures                           */
/*========================================================================*/

/* A Scheme_IR_Local record represents a local variable, where
   both the binding and references to that same binding are
   represented by the same allocated object. When inlining
   or other transformations duplicate a variable, a new instance
   is allocated to represent a separate variable. Different passes
   in the comiler store different information about the variable. */
typedef struct Scheme_IR_Local
{
  Scheme_Object so;

  /* The `mode` value is one of `SCHEME_VAR_MODE_NONE`, etc.,
     and it determines which of the union cases below (if any)
     is active, corresponding to information for a particular
     pass: */
  unsigned int mode : 3;
  /* Number of time the variable was referenced as counted by
     the initial compile phase; a `SCHEME_USE_COUNT_INF`
     value corresponds to "more than we counted": */
  unsigned int use_count : 3;
  /* Subset of `use_count` references that are in non-rator
     positions: */
  unsigned int non_app_count : 3;
  /* Records whether the variable is mutated; set in several
     phases, and currently never unset: */
  unsigned int mutated : 1;
  /* Records whether the optimizer discovered any uses;
     if true, then `use_count` must be non-zero, but the
     optimizer eliminate references and produce 0 here even
     if `use_count` is non-zero: */
  unsigned int optimize_used : 1;
  /* Set while compiling the right-hand side of a letrec
     to indicate that current and later left-hand sides
     are not yet initialized: */
  unsigned int optimize_unready : 1;
  /* After optimizing a `let[rec]` form, we might still go into
     the body (e.g., for funciton inlining), but mark the variable
     as having a binding set up: */
  unsigned int optimize_outside_binding : 1;
  /* Records an anlaysis during the resolve pass: */
  unsigned int resolve_omittable : 1;
  /* Records whether the variable is mutated and used before
     the body of its binding, so that itmust be allocated at latest
     after it's RHS expression is evaluated: */
  unsigned int must_allocate_immediately : 1;
  /* The type desired by use positions for unboxing purposes;
     set by the optimizer: */
  unsigned int arg_type : SCHEME_MAX_LOCAL_TYPE_BITS;
  /* The type provided by the binding position, mainly for unboxing
     purposes; set by the optimizer and potentially refined by the
     resolve pass (especially for function arguments whose types are
     set via local_type_map): */
  unsigned int val_type : SCHEME_MAX_LOCAL_TYPE_BITS;
  /* Unboxing might be disabled because allocation of boxes would
     be moved past a continuation: */
  unsigned int escapes_after_k_tick : 1;
  /* During unresolve, indicates whether references should be
     converted to calls: */
  unsigned int is_ref_arg : 1;

  Scheme_Object *name;

  /* `mode` determines which union is active: */
  union {
    struct {
      /* Maps the variable into the letrec-check pass's frames: */
      struct Letrec_Check_Frame *frame;
      int frame_pos;
    } letrec_check;
    struct {
      /* Constant- and copy-propagation information: */
      Scheme_Object *known_val;
      /* Whether `known_val` must be cleared when the variable's
         only use is duplicated: */
      int clear_known_on_multi_use;
      /* Number of `lambda` wrappers, which is relevant for
         accumulating closures, etc.: */
      int lambda_depth;
      /* Vitual continuation-capture clock for the variable's
         initialation, used to detect potential captures of
         allocation: */
      int init_kclock;
      /* Transitive uses record uses that become used if
         the variable itself is used; which is relevant
         for analyzing a letrec-bound function that might
         not get called: */
      Scheme_Hash_Table *transitive_uses;
    } optimize;
    struct {
      /* Records the position where the variable will be
         on the runstack, counting down from the enclosing
         procedure's starting point (i.e., backwards from the
         run-time direction): */
      int co_depth;
      /* Records a lexical depth for the purposes of sorting
         variables (as needed to make compilation deterministic): */
      int lex_depth;
      /* Information on closure-converstion of this
         variable's binding: */
      Scheme_Object *lifted;
    } resolve;
  };
} Scheme_IR_Local;

#define SCHEME_VAR(v) ((Scheme_IR_Local *)v)

#define SCHEME_USE_COUNT_INF    7

#define SCHEME_VAR_MODE_NONE         0
#define SCHEME_VAR_MODE_COMPILE      1
#define SCHEME_VAR_MODE_LETREC_CHECK 2
#define SCHEME_VAR_MODE_OPTIMIZE     3
#define SCHEME_VAR_MODE_RESOLVE      4

typedef struct {
  Scheme_Inclhash_Object iso; /* keyex used for flags */
  mzshort num_args; /* doesn't include rator, so arguments are at args[1]...args[num_args] */
  Scheme_Object *args[mzFLEX_ARRAY_DECL];
  /* After array of f & args, array of chars for eval type */
} Scheme_App_Rec;

#define SCHEME_APPN_FLAGS(app) MZ_OPT_HASH_KEY(&(app)->iso)
/* For all application types, throgh optimization, the low bits of the flags
   are used to hold an index for an application indicate that it's the Nth
   application of an identifier, which is useful to type inference.
   The same bits are used after resolve for app2 and app3 to indicate
   lookahead types (as below) */

/* A value of N means that the application is the (N-1)th
   application of a variable, where 0 means "unknown". */
#define APPN_POSITION_MASK  SCHEME_USE_COUNT_INF

/* Lookahead types for evaluating application arguments. */
/* 4 cases + else => magic number for some compilers doing a switch? */
enum {
  SCHEME_EVAL_CONSTANT = 0,
  SCHEME_EVAL_GLOBAL,
  SCHEME_EVAL_LOCAL,
  SCHEME_EVAL_LOCAL_UNBOX,
  SCHEME_EVAL_GENERAL
};

/* Flags to indicate to SFS pass that a [tail] application doesn't
   need clearing before it (because the call is to a immediate
   primitive or a Racket-implemented function). */
#define APPN_FLAG_SFS_TAIL (1 << 13)
#define APPN_FLAG_IMMED (1 << 12)
/* The compiler may determine that a call is omittable; usually that
   information is encoded in the primitive itself, but sometimes the
   optimizer can figure out more (e.g., based on known types of the
   arguments): */
#define APPN_FLAG_OMITTABLE (1 << 11)
#define APPN_FLAG_MASK (APPN_FLAG_OMITTABLE | APPN_FLAG_IMMED | APPN_FLAG_SFS_TAIL)

typedef struct {
  Scheme_Inclhash_Object iso; /* keyex used for flags */
  Scheme_Object *rator;
  Scheme_Object *rand;
} Scheme_App2_Rec;

typedef struct {
  Scheme_Inclhash_Object iso; /* keyex used for flags */
  Scheme_Object *rator;
  Scheme_Object *rand1;
  Scheme_Object *rand2;
} Scheme_App3_Rec;

typedef struct {
  Scheme_Object so;
  Scheme_Object *test;
  Scheme_Object *tbranch;
  Scheme_Object *fbranch;
} Scheme_Branch_Rec;

typedef struct {
  Scheme_Inclhash_Object iso; /* keyex used to disable module table */
  mzshort max_let_depth;
  Scheme_Object *code;
  struct Resolve_Prefix *prefix; /* NULL => a wrapper for a JITted module in `code' */
  Scheme_Object *binding_namess; /* list of <phase> to hash of <symbol> to <id>;
                                    additions to the top-level bindings table */
} Scheme_Compilation_Top;

/* A `let' or `letrec' form is compiled to the intermediate
   format (used during the optimization pass) as a Scheme_IR_Let_Header
   with a chain of Scheme_IR_Let_Value records as its body,
   where there's one Scheme_IR_Let_Value for each binding
   clause. The body of the `let...' form is the body of the innermost
   Scheme_IR_Let_Value record.
*/

typedef struct Scheme_IR_Let_Header {
  Scheme_Inclhash_Object iso; /* keyex used for recursive */
  mzshort count;       /* total number of bindings */
  mzshort num_clauses; /* number of binding clauses */
  Scheme_Object *body;
} Scheme_IR_Let_Header;

#define SCHEME_LET_FLAGS(lh) MZ_OPT_HASH_KEY(&lh->iso)
#define SCHEME_LET_RECURSIVE 0x1

typedef struct Scheme_IR_Let_Value {
  Scheme_Inclhash_Object iso; /* keyex used for set-starting */
  mzshort count;
  Scheme_Object *value;
  Scheme_Object *body;
  Scheme_IR_Local **vars;
} Scheme_IR_Let_Value;

#define SCHEME_IRLV_FLAGS(irlv) MZ_OPT_HASH_KEY(&(irlv)->iso)
#define SCHEME_IRLV_NO_GROUP_LATER_USES 0x1
#define SCHEME_IRLV_NO_GROUP_USES 0x2

typedef struct {
  Scheme_Object so;
  Scheme_Object *key;
  Scheme_Object *val;
  Scheme_Object *body;
} Scheme_With_Continuation_Mark;

#define HIGH_BIT_TO_DISABLE_HASHING 0x2000

typedef struct Scheme_Local {
  Scheme_Inclhash_Object iso; /* keyex used for flags and type info (and can't be hashed) */
  mzshort position;
#ifdef MZ_PRECISE_GC
# ifdef MZSHORT_IS_SHORT
  /* Everything has to be at least 2 words in size. */
  int x;
# endif
#endif
} Scheme_Local;

#define SCHEME_LOCAL_POS(obj)    (((Scheme_Local *)(obj))->position)
#define SCHEME_LOCAL_FLAGS(obj)  MZ_OPT_HASH_KEY(&((Scheme_Local *)(obj))->iso)

#define SCHEME_LOCAL_CLEAR_ON_READ 1
#define SCHEME_LOCAL_OTHER_CLEARS  2
#define SCHEME_LOCAL_TYPE_OFFSET   2

#define SCHEME_GET_LOCAL_FLAGS(obj)  (SCHEME_LOCAL_FLAGS(obj) & ~HIGH_BIT_TO_DISABLE_HASHING)
#define SCHEME_GET_LOCAL_TYPE(obj)  ((SCHEME_GET_LOCAL_FLAGS(obj) > 2) ? (SCHEME_GET_LOCAL_FLAGS(obj) - 2) : 0)

typedef struct Scheme_Toplevel {
  Scheme_Inclhash_Object iso; /* keyex used for flags (and can't be hashed) */
  mzshort depth;
  int position;
} Scheme_Toplevel;

#define SCHEME_TOPLEVEL_DEPTH(obj)    (((Scheme_Toplevel *)(obj))->depth)
#define SCHEME_TOPLEVEL_POS(obj)    (((Scheme_Toplevel *)(obj))->position)
#define SCHEME_TOPLEVEL_FLAGS(obj)  MZ_OPT_HASH_KEY(&((Scheme_Toplevel *)(obj))->iso)

/* The MASK pull out one of the levels for reference (CONST,
   FIXED, READY, or UNKNOWN) or one of the two levels for a
   definition (SEAL or not) */
#define SCHEME_TOPLEVEL_FLAGS_MASK 0x3

/* CONST means that a toplevel is READY and always has the "same" value,
   even for different instantiations or phases. "Same" means that the result
   is a procedure or would be ok to duplicate in the source. */
#define SCHEME_TOPLEVEL_CONST   3
/* FIXED is READY plus a promise of no mutation, but the value is
   not necessarily constant across different instantations or phases. */
#define SCHEME_TOPLEVEL_FIXED   2
/* READY means that the toplevel will have a value (i.e., the variable
   is defined), though it might be mutated later */
#define SCHEME_TOPLEVEL_READY   1
/* UNKNOWN means that the variable might not even be defined by the time the
   toplevel reference is executed */
#define SCHEME_TOPLEVEL_UNKNOWN   0

#define SCHEME_TOPLEVEL_SEAL   0x1

/* MUTATED is used on the toplevel for a definition, and only until
   after resolving; it records whether a toplevel is `set!'ed */
#define SCHEME_TOPLEVEL_MUTATED 0x4

typedef struct Scheme_Quote_Syntax {
  Scheme_Object so; /* scheme_quote_syntax_type */
  mzshort depth;
  mzshort position;
  mzshort midpoint;
} Scheme_Quote_Syntax;

typedef struct Scheme_Let_Value {
  Scheme_Inclhash_Object iso; /* keyex used for autobox */
  mzshort count;
  mzshort position;
  Scheme_Object *value;
  Scheme_Object *body;
} Scheme_Let_Value;

#define SCHEME_LET_VALUE_AUTOBOX(lv) MZ_OPT_HASH_KEY(&lv->iso)

typedef struct Scheme_Let_One {
  Scheme_Inclhash_Object iso; /* keyex used for eval_type + flonum/unused (and can't be hashed) */
  Scheme_Object *value;
  Scheme_Object *body;
} Scheme_Let_One;

#define SCHEME_LET_EVAL_TYPE(lh) MZ_OPT_HASH_KEY(&lh->iso)
#define LET_ONE_UNUSED 0x8

#define LET_ONE_TYPE_SHIFT 4
#define LET_ONE_TYPE_MASK  (SCHEME_MAX_LOCAL_TYPE_MASK << 4)
#define SCHEME_LET_ONE_TYPE(lo) (SCHEME_LET_EVAL_TYPE(lo) >> LET_ONE_TYPE_SHIFT)

typedef struct Scheme_Let_Void {
  Scheme_Inclhash_Object iso; /* keyex used for autobox */
  mzshort count;
  Scheme_Object *body;
} Scheme_Let_Void;

#define SCHEME_LET_VOID_AUTOBOX(lv) MZ_OPT_HASH_KEY(&lv->iso)

typedef struct Scheme_Letrec {
  Scheme_Object so;
  mzshort count;
  Scheme_Object **procs;
  Scheme_Object *body;
} Scheme_Letrec;

typedef struct {
  Scheme_Object so;
  mzshort count;
  Scheme_Object *array[mzFLEX_ARRAY_DECL];
} Scheme_Sequence;

typedef struct {
  Scheme_Object so;
  mzshort count;
  Scheme_Object *name; /* see note below */
#ifdef MZ_USE_JIT
  struct Scheme_Native_Lambda *native_code; /* generated by lightning */
#endif
  Scheme_Object *array[mzFLEX_ARRAY_DECL];
} Scheme_Case_Lambda;
/* If count is not 0, then check array[0] for LAMBDA_IS_METHOD.
   Otherwise, name is a boxed symbol (or #f) to indicate a method. */

#define scheme_make_prim_w_arity2(f, n, mina, maxa, minr, maxr) \
  scheme_make_prim_w_everything(f, 1, n, mina, maxa, 0, minr, maxr)

Scheme_Object *scheme_unclose_case_lambda(Scheme_Object *expr, int jit);

Scheme_Object *scheme_native_stack_trace(void);
void scheme_clean_native_symtab(void);
void scheme_clean_cust_box_list(void);
#ifndef MZ_PRECISE_GC
void scheme_notify_code_gc(void);
#endif

#ifdef USE_THREAD_LOCAL
# define BOTTOM_VARIABLE GC_variable_stack
# define EXTRA_NATIVE_ARGUMENT , &BOTTOM_VARIABLE
# define EXTRA_NATIVE_ARGUMENT_TYPE , void* thdloc
#else
# define EXTRA_NATIVE_ARGUMENT /* empty */
# define EXTRA_NATIVE_ARGUMENT_TYPE /* empty */
#endif

typedef struct Scheme_Object *(Scheme_Native_Proc)(void *d, int argc, struct Scheme_Object *argv[] 
                                                   EXTRA_NATIVE_ARGUMENT_TYPE);

/*========================================================================*/
/*                              control flow                              */
/*========================================================================*/

Scheme_Object *scheme_handle_stack_overflow(Scheme_Object *(*k)(void));
int scheme_is_stack_too_shallow();

THREAD_LOCAL_DECL(extern struct Scheme_Overflow_Jmp *scheme_overflow_jmp);
THREAD_LOCAL_DECL(extern void *scheme_overflow_stack_start);

#ifdef MZ_PRECISE_GC
# define PROMPT_STACK(id) &__gc_var_stack__
#else
# define PROMPT_STACK(id) ((void *)(&id))
#endif

struct Scheme_Overflow_Jmp *scheme_prune_jmpup(struct Scheme_Overflow_Jmp *jmp, void *stack_boundary);

void scheme_jmpup_free(Scheme_Jumpup_Buf *);
void *scheme_enlarge_runstack(intptr_t size, void *(*k)());
int scheme_check_runstack(intptr_t size);

#ifndef MZ_PRECISE_GC
void scheme_init_setjumpup(void);
void scheme_init_ephemerons(void);
#endif

#ifdef MZ_PRECISE_GC
void scheme_flush_stack_copy_cache(void);
#endif

typedef struct Scheme_Dynamic_State {
    struct Scheme_Comp_Env * volatile current_local_env;
    Scheme_Object * volatile scope;
    Scheme_Object * volatile use_scope;
    Scheme_Object * volatile name;
    Scheme_Object * volatile modidx;
    Scheme_Env    * volatile menv;
} Scheme_Dynamic_State;

void scheme_set_dynamic_state(Scheme_Dynamic_State *state, struct Scheme_Comp_Env *env,
                              Scheme_Object *scope, Scheme_Object *use_scope,
                              Scheme_Object *name, 
                              Scheme_Env *menv,
                              Scheme_Object *modidx);
void *scheme_top_level_do(void *(*k)(void), int eb);
void *scheme_top_level_do_worker(void *(*k)(void), int eb, int newthread, Scheme_Dynamic_State *dyn_state);

Scheme_Object *scheme_call_ec(int argc, Scheme_Object *argv[]);

uintptr_t scheme_get_deeper_address(void);

#ifdef DO_STACK_CHECK
void scheme_init_stack_limit (void);
#endif


typedef struct Scheme_Saved_Stack {
  MZTAG_IF_REQUIRED
  Scheme_Object **runstack_start;
  intptr_t runstack_offset;
  intptr_t runstack_size;
  struct Scheme_Saved_Stack *prev;
} Scheme_Saved_Stack;

typedef struct Scheme_Cont_Mark {
  /* Precise GC: We leave out the tag and make sure everything
     is a pointer, then allocate with GC_malloc_allow_interior */
  Scheme_Object *key;
  Scheme_Object *val;
  Scheme_Object *cache; /* chain and/or shortcut */
  MZ_MARK_POS_TYPE pos; /* Odd numbers - so they look like non-pointers */
} Scheme_Cont_Mark;
#define LOG_CONT_MARK_WORD_COUNT 2

void scheme_new_mark_segment(Scheme_Thread *p);

typedef struct Scheme_Cont_Mark_Chain {
  Scheme_Inclhash_Object iso; /* 0x1 => next is from different meta-continuation */
  Scheme_Object *key;
  Scheme_Object *val;
  MZ_MARK_POS_TYPE pos;
  struct Scheme_Cont_Mark_Chain *next;
} Scheme_Cont_Mark_Chain;

#define SCHEME_MARK_CHAIN_FLAG(c) MZ_OPT_HASH_KEY(&(c)->iso)

typedef struct Scheme_Cont_Mark_Set {
  Scheme_Object so;
  struct Scheme_Cont_Mark_Chain *chain;
  intptr_t cmpos;
  Scheme_Object *native_stack_trace;
} Scheme_Cont_Mark_Set;

#define SCHEME_LOG_MARK_SEGMENT_SIZE 6
#define SCHEME_MARK_SEGMENT_SIZE (1 << SCHEME_LOG_MARK_SEGMENT_SIZE)
#define SCHEME_MARK_SEGMENT_MASK (SCHEME_MARK_SEGMENT_SIZE - 1)

typedef struct Scheme_Stack_State {
  intptr_t runstack_offset;
  MZ_MARK_POS_TYPE cont_mark_pos;
  MZ_MARK_STACK_TYPE cont_mark_stack;
} Scheme_Stack_State;

typedef struct Scheme_Dynamic_Wind {
  MZTAG_IF_REQUIRED
  int depth;
  void *id; /* generated as needed */
  void *data;
  Scheme_Object *prompt_tag; /* If not NULL, indicates a fake D-W record for prompt boundary */
  void (*pre)(void *);
  void (*post)(void *);
  mz_jmp_buf *saveerr;
  int next_meta; /* amount to move forward in the meta-continuation chain, starting with next */
  struct Scheme_Stack_State envss;
  struct Scheme_Dynamic_Wind *prev;
} Scheme_Dynamic_Wind;

typedef struct Scheme_Cont_Jmp {
  MZTAG_IF_REQUIRED
  Scheme_Jumpup_Buf buf;
} Scheme_Cont_Jmp;

typedef struct Scheme_Cont {
  Scheme_Object so;
  char composable, has_prompt_dw, need_meta_prompt, skip_dws;
  struct Scheme_Meta_Continuation *meta_continuation;
  Scheme_Object *meta_continuation_src; /* a weak reference to the mc cloned, for use in detecting sharing */
  Scheme_Cont_Jmp *buf_ptr; /* indirection allows sharing */
  Scheme_Dynamic_Wind *dw;
  int next_meta;
  Scheme_Continuation_Jump_State cjs;
  Scheme_Stack_State ss;
  struct Scheme_Prompt *barrier_prompt; /* NULL if no barrier between cont and prompt */
  Scheme_Object **runstack_start;
  intptr_t runstack_size;
  Scheme_Saved_Stack *runstack_saved;
  Scheme_Object *prompt_tag;
  mz_jmp_buf *prompt_buf; /* needed for meta-prompt */
  MZ_MARK_POS_TYPE meta_tail_pos; /* to recognize opportunity for meta-tail calls */
  MZ_MARK_POS_TYPE cont_mark_pos_bottom; /* to splice cont mark values with meta-cont */
  void *prompt_stack_start;
  Scheme_Saved_Stack *runstack_copied;
  Scheme_Thread **runstack_owner;
  Scheme_Cont_Mark *cont_mark_stack_copied;
  Scheme_Thread **cont_mark_stack_owner;
  intptr_t cont_mark_total; /* size of the copied array plus cont_mark_offset */
  intptr_t cont_mark_offset; /* after the array, the original mark stack had this much */
  intptr_t cont_mark_nonshare; /* amount to skip for sub-cont sharing */
  void *stack_start;
  Scheme_Object *prompt_id; /* allows direct-jump optimization */
  Scheme_Config *init_config;
  Scheme_Object *init_break_cell;
#ifdef MZ_USE_JIT
  Scheme_Object *native_trace;
#endif
  struct Scheme_Overflow *save_overflow;
  mz_jmp_buf *savebuf; /* save old error buffer here */

  Scheme_Object *escape_cont;
  int orig_escape_cont;

  /* Arguments passed to a continuation invocation to the continuation restorer: */
  Scheme_Object *value; /* argument(s) to continuation */
  struct Scheme_Overflow *resume_to; /* meta-continuation return */
  char empty_to_next_mc;
  struct Scheme_Cont *use_next_cont; /* more meta-continuation return */
  int common_dw_depth; /* id for common dw record */
  Scheme_Dynamic_Wind *common_dw; /* shared part with source cont */
  int common_next_meta; /* for common_dw */
  Scheme_Object *extra_marks; /* vector of extra keys and marks to add to meta-cont */
  struct Scheme_Prompt *shortcut_prompt; /* prompt common to save and restore enabling shortcut */
} Scheme_Cont;

typedef struct Scheme_Escaping_Cont {
  Scheme_Object so;
  struct Scheme_Stack_State envss;
  struct Scheme_Prompt *barrier_prompt;
#ifdef MZ_USE_JIT
  Scheme_Object *native_trace;
#endif
  mz_jmp_buf *saveerr, *myerr;
} Scheme_Escaping_Cont;

#define SCHEME_CONT_F(obj) (((Scheme_Escaping_Cont *)(obj))->f)

int scheme_escape_continuation_ok(Scheme_Object *);

#define scheme_save_env_stack_w_thread(ss, p) \
    (ss.runstack_offset = MZ_RUNSTACK - MZ_RUNSTACK_START, \
     ss.cont_mark_stack = MZ_CONT_MARK_STACK, ss.cont_mark_pos = MZ_CONT_MARK_POS)
#define scheme_restore_env_stack_w_thread(ss, p) \
    (MZ_RUNSTACK = MZ_RUNSTACK_START + ss.runstack_offset, \
     MZ_CONT_MARK_STACK = ss.cont_mark_stack, MZ_CONT_MARK_POS = ss.cont_mark_pos)
#define scheme_save_env_stack(ss) \
    scheme_save_env_stack_w_thread(ss, scheme_current_thread)
#define scheme_restore_env_stack(ss) \
    scheme_restore_env_stack_w_thread(ss, scheme_current_thread)

void scheme_takeover_stacks(Scheme_Thread *p);

typedef struct Scheme_Overflow_Jmp {
  MZTAG_IF_REQUIRED
  char captured; /* set to 1 if possibly captured in a continuation */
  Scheme_Jumpup_Buf cont; /* continuation after value obtained in overflowed */
  mz_jmp_buf *savebuf; /* save old error buffer pointer here */
} Scheme_Overflow_Jmp;

typedef struct Scheme_Overflow {
  MZTAG_IF_REQUIRED
  char eot;      /* set to 1 => pseudo-overflow: continuation is to exit the thread */
  Scheme_Overflow_Jmp *jmp; /* overflow data, so it can be shared when an overflow chain is cloned; */
  void *id;                 /* identity of overflow record; generated as needed, and often == jmp */
  void *stack_start;
  struct Scheme_Overflow *prev; /* old overflow info */
} Scheme_Overflow;

#if defined(UNIX_FIND_STACK_BOUNDS) || defined(WINDOWS_FIND_STACK_BOUNDS) \
    || defined(MACOS_FIND_STACK_BOUNDS) || defined(ASSUME_FIXED_STACK_SIZE) \
    || defined(PALM_FIND_STACK_BOUNDS) || defined(PTHREAD_STACKSEG_FIND_STACK_BOUNDS)
# define USE_STACK_BOUNDARY_VAR
THREAD_LOCAL_DECL(extern uintptr_t scheme_stack_boundary);
/* Same as scheme_stack_boundary, but set to an extreme value when feul auto-expires,
   so that JIT-generated code can check just one variable: */
THREAD_LOCAL_DECL(extern uintptr_t volatile scheme_jit_stack_boundary);
#endif

typedef struct Scheme_Meta_Continuation {
  MZTAG_IF_REQUIRED
  char pseudo; /* if set, don't treat it as a prompt */
  char empty_to_next; /* when pseudo, if the continuation is empty to the next one */
  char cm_caches; /* cached info in copied cm */
  char cm_shared; /* cm is shared, so copy before setting cache entries */
  int copy_after_captured; /* for mutating a meta-continuation in set_cont_stack_mark */
  int depth;
  Scheme_Object *prompt_tag;
  /* The C stack: */
  Scheme_Overflow *overflow;
  MZ_MARK_POS_TYPE meta_tail_pos; /* to recognize opportunity for meta-tail calls */  
  MZ_MARK_POS_TYPE cont_mark_pos_bottom; /* to splice cont mark values with meta-cont */
  /* Cont mark info: */
  MZ_MARK_STACK_TYPE cont_mark_stack;
  MZ_MARK_POS_TYPE cont_mark_pos;
  intptr_t cont_mark_total, cont_mark_offset;
  Scheme_Cont_Mark *cont_mark_stack_copied;
  /* Continuation (whose cont-mark info is the same as above) */
  struct Scheme_Cont *cont;
  /* Next: */
  struct Scheme_Meta_Continuation *next;
} Scheme_Meta_Continuation;

typedef struct Scheme_Prompt {
  Scheme_Object so;
  char is_barrier, has_chaperone;
  Scheme_Object *tag;
  Scheme_Object *id;                  /* created as needed; allows direct-jump optimization for cont app */
  void *stack_boundary;               /* where to stop copying the C stack */
  void *boundary_overflow_id;         /* indicates the C stack segment */
  MZ_MARK_STACK_TYPE mark_boundary;   /* where to stop copying cont marks */
  MZ_MARK_POS_TYPE boundary_mark_pos; /* mark position of prompt */
  Scheme_Object **runstack_boundary_start; /* which stack has runstack_boundary */
  intptr_t runstack_boundary_offset;      /* where to stop copying the Scheme stack */
  mz_jmp_buf *prompt_buf;             /* to jump directly to the prompt */
  intptr_t runstack_size;                 /* needed for restore */
} Scheme_Prompt;

/* Compiler helper: */
#define ESCAPED_BEFORE_HERE  return NULL

Scheme_Object *scheme_compose_continuation(Scheme_Cont *c, int num_rands, Scheme_Object *value);
Scheme_Overflow *scheme_get_thread_end_overflow(void);
void scheme_end_current_thread(void);
void scheme_ensure_dw_id(Scheme_Dynamic_Wind *dw);
void scheme_apply_dw_in_meta(Scheme_Dynamic_Wind *dw, int post, int mc_depth, struct Scheme_Cont *recheck);

void scheme_drop_prompt_meta_continuations(Scheme_Object *prompt_tag);

struct Scheme_Prompt *scheme_get_barrier_prompt(struct Scheme_Meta_Continuation **_meta_cont,
                                                MZ_MARK_POS_TYPE *_pos);
Scheme_Prompt *scheme_get_prompt(Scheme_Object *prompt_tag, Scheme_Meta_Continuation **_meta_cont,
                                 MZ_MARK_POS_TYPE *_pos);
int scheme_is_cm_deeper(struct Scheme_Meta_Continuation *m1, MZ_MARK_POS_TYPE p1,
                        struct Scheme_Meta_Continuation *m2, MZ_MARK_POS_TYPE p2);
void scheme_recheck_prompt_and_barrier(struct Scheme_Cont *c);

Scheme_Object *scheme_all_current_continuation_marks(void);

void scheme_about_to_move_C_stack(void);

Scheme_Object *scheme_apply_with_dynamic_state(Scheme_Object *rator, int num_rands, Scheme_Object **rands, Scheme_Dynamic_State *dyn_state);
Scheme_Object *scheme_apply_multi_with_dynamic_state(Scheme_Object *rator, int num_rands, Scheme_Object **rands, Scheme_Dynamic_State *dyn_state);

Scheme_Object *scheme_jump_to_continuation(Scheme_Object *obj, int num_rands, Scheme_Object **rands, 
                                           Scheme_Object **old_runstack, int can_ec);

Scheme_Object *scheme_chaperone_do_continuation_mark(const char *name, int is_get, Scheme_Object *key, Scheme_Object *val);

XFORM_NONGCING Scheme_Object *scheme_get_immediate_cc_mark(Scheme_Object *key, Scheme_Object *def_val);
Scheme_Object *scheme_chaperone_get_immediate_cc_mark(Scheme_Object *key, Scheme_Object *def_val);

/*========================================================================*/
/*                         semaphores and locks                           */
/*========================================================================*/

typedef struct Scheme_Channel_Syncer {
  Scheme_Object so;
  Scheme_Thread *p;
  char in_line, picked;
  struct Scheme_Channel_Syncer *prev, *next;
  struct Syncing *syncing;
  Scheme_Object *obj;
  int syncing_i;
} Scheme_Channel_Syncer;

typedef struct Scheme_Sema {
  Scheme_Object so;
  Scheme_Channel_Syncer *first, *last;
  intptr_t value;
} Scheme_Sema;

typedef struct Scheme_Channel {
  Scheme_Object so;
  Scheme_Channel_Syncer *put_first, *put_last;
  Scheme_Channel_Syncer *get_first, *get_last;
} Scheme_Channel;

typedef struct Scheme_Channel_Put {
  Scheme_Object so;
  Scheme_Channel *ch;
  Scheme_Object *val;
} Scheme_Channel_Put;

#define GENERIC_BLOCKED -1
#define NOT_BLOCKED 0
#define SLEEP_BLOCKED 1

typedef struct Evt_Set {
  Scheme_Inclhash_Object iso; /* 0x1 => unflattened */
  int argc;
  Scheme_Object **argv; /* no evt sets; nested sets get flattened */
  struct Evt **ws;
} Evt_Set;

#define SCHEME_EVTSETP(o) SAME_TYPE(SCHEME_TYPE(o), scheme_evt_set_type)
#define SCHEME_EVTSET_UNFLATTENEDP(o) SCHEME_IMMUTABLEP(o)
#define SCHEME_SET_EVTSET_UNFLATTENED(o) SCHEME_SET_IMMUTABLE(o)

typedef struct Syncing {
  MZTAG_IF_REQUIRED
  Evt_Set *set;
  int result, start_pos;
  double sleep_end;
  float timeout;

  Scheme_Object **wrapss;
  Scheme_Object **nackss;
  char *reposts;
  Scheme_Accept_Sync *accepts;

  Scheme_Thread *disable_break; /* when result is set */
  Scheme_Thread *thread; /* set when syncing to allow in flight place message cleanup */
} Syncing;

int scheme_wait_semas_chs(int n, Scheme_Object **o, int just_try, Syncing *syncing);
Scheme_Object *scheme_make_sema_repost(Scheme_Object *sema);

Scheme_Object *scheme_wrap_evt(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_poll_evt(int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_do_chaperone_evt(const char*, int, int, Scheme_Object *argv[]);

extern Scheme_Object *scheme_always_ready_evt;

void scheme_get_outof_line(Scheme_Channel_Syncer *ch_w);
void scheme_get_back_into_line(Scheme_Channel_Syncer *ch_w);
void scheme_post_syncing_nacks(Syncing *syncing);

int scheme_try_channel_get(Scheme_Object *ch);
int scheme_try_channel_put(Scheme_Object *ch, Scheme_Object *v);

intptr_t scheme_get_semaphore_init(const char *who, int n, Scheme_Object **p);

/*========================================================================*/
/*                                 numbers                                */
/*========================================================================*/

#ifdef MPW_C
/* Optimizer bug! */
# define scheme_exact_zero ((Scheme_Object *)0x1)
# define scheme_exact_one ((Scheme_Object *)0x3)
#else
# define scheme_exact_zero scheme_make_integer(0)
# define scheme_exact_one scheme_make_integer(1)
#endif

#ifdef MZ_LONG_DOUBLE
# define MZ_LONG_DOUBLE_AND(x) (x)
#else
# define MZ_LONG_DOUBLE_AND(x) 0
#endif

#ifdef MZ_LONG_DOUBLE_API_IS_EXTERNAL
# define MZ_LONG_DOUBLE_AVAIL_AND(x) MZ_LONG_DOUBLE_AND(long_double_available() && (x))
# define WHEN_LONG_DOUBLE_UNSUPPORTED(what) \
  if (!long_double_available()) {                                       \
    what;                                                               \
  }
# define CHECK_MZ_LONG_DOUBLE_UNSUPPORTED(who) \
  if (!long_double_available()) {                                        \
    scheme_raise_exn(MZEXN_FAIL_UNSUPPORTED, who ": " NOT_SUPPORTED_STR); \
    ESCAPED_BEFORE_HERE;                                                \
  }
#else
# define WHEN_LONG_DOUBLE_UNSUPPORTED(what) /* empty */
# define CHECK_MZ_LONG_DOUBLE_UNSUPPORTED(who) /* empty */
# define MZ_LONG_DOUBLE_AVAIL_AND(x) MZ_LONG_DOUBLE_AND(x)
#endif

void scheme_configure_floating_point(void);

/****** Bignums *******/

#ifdef USE_LONG_LONG_FOR_BIGDIG
typedef unsigned long long bigdig;
#else
typedef uintptr_t bigdig;
#endif

typedef struct {
  Scheme_Inclhash_Object iso;
  intptr_t len;
  bigdig *digits;
} Scheme_Bignum;

#ifdef MZ_PRECISE_GC
# define SCHEME_BIGPOS(b) (MZ_OPT_HASH_KEY(&((Scheme_Bignum *)b)->iso) & 0x1)
# define SCHEME_SET_BIGPOS(b, v) MZ_OPT_HASH_KEY(&((Scheme_Bignum *)b)->iso) = ((v) | SCHEME_BIGINLINE(b))
# define SCHEME_BIGINLINE(b) (MZ_OPT_HASH_KEY(&((Scheme_Bignum *)b)->iso) & 0x2)
# define SCHEME_SET_BIGINLINE(b) MZ_OPT_HASH_KEY(&((Scheme_Bignum *)b)->iso) |= (0x2 | SCHEME_BIGPOS(b))
#else
# define SCHEME_BIGPOS(b) MZ_OPT_HASH_KEY(&((Scheme_Bignum *)b)->iso)
# define SCHEME_SET_BIGPOS(b, v) SCHEME_BIGPOS(b) = v
#endif

#define SCHEME_BIGLEN(b) (((Scheme_Bignum *)b)->len)
#define SCHEME_BIGDIG(b) (((Scheme_Bignum *)b)->digits)

typedef struct {
  Scheme_Bignum o;
  bigdig v[1];
} Small_Bignum;

XFORM_NONGCING Scheme_Object *scheme_make_small_bignum(intptr_t v, Small_Bignum *s);
char *scheme_number_to_string(int radix, Scheme_Object *obj);
char *scheme_double_to_string (double d, char* s, int slen, int was_single, int *used_buffer);
#ifdef MZ_LONG_DOUBLE
char *scheme_long_double_to_string (long_double d, char* s, int slen, int *used_buffer);
#endif

Scheme_Object *scheme_bignum_copy(const Scheme_Object *n);

XFORM_NONGCING int scheme_bignum_get_int_val(const Scheme_Object *o, intptr_t *v);
XFORM_NONGCING int scheme_bignum_get_unsigned_int_val(const Scheme_Object *o, uintptr_t *v);
XFORM_NONGCING int scheme_bignum_get_long_long_val(const Scheme_Object *o, mzlonglong *v);
XFORM_NONGCING int scheme_bignum_get_unsigned_long_long_val(const Scheme_Object *o, umzlonglong *v);

XFORM_NONGCING int scheme_bignum_eq(const Scheme_Object *a, const Scheme_Object *b);
XFORM_NONGCING int scheme_bignum_lt(const Scheme_Object *a, const Scheme_Object *b);
XFORM_NONGCING int scheme_bignum_gt(const Scheme_Object *a, const Scheme_Object *b);
XFORM_NONGCING int scheme_bignum_le(const Scheme_Object *a, const Scheme_Object *b);
XFORM_NONGCING int scheme_bignum_ge(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_negate(const Scheme_Object *n);
Scheme_Object *scheme_bignum_add(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_subtract(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_add1(const Scheme_Object *n);
Scheme_Object *scheme_bignum_sub1(const Scheme_Object *n);
Scheme_Object *scheme_bignum_multiply(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_max(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_min(const Scheme_Object *a, const Scheme_Object *b);
void scheme_bignum_divide(const Scheme_Object *n, const Scheme_Object *d,
			  Scheme_Object **qp, Scheme_Object **rp, int norm);
Scheme_Object *scheme_generic_integer_power(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_gcd(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_integer_sqrt(const Scheme_Object *n);
Scheme_Object *scheme_integer_sqrt_rem(const Scheme_Object *n, Scheme_Object **r);
Scheme_Object *scheme_bignum_and(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_or(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_xor(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_bignum_not(const Scheme_Object *a);
Scheme_Object *scheme_bignum_shift(const Scheme_Object *a, intptr_t shift);

XFORM_NONGCING double scheme_bignum_to_double_inf_info(const Scheme_Object *n, intptr_t just_use, intptr_t *only_need);
#ifdef MZ_LONG_DOUBLE
XFORM_NONGCING long_double scheme_bignum_to_long_double_inf_info(const Scheme_Object *n, intptr_t just_use, intptr_t *only_need);
#endif
#ifdef MZ_USE_SINGLE_FLOATS
XFORM_NONGCING float scheme_bignum_to_float_inf_info(const Scheme_Object *n, intptr_t just_use, intptr_t *only_need);
#else
# define scheme_bignum_to_float_inf_info scheme_bignum_to_double_inf_info
#endif

void scheme_clear_bignum_cache(void);

intptr_t scheme_integer_length(Scheme_Object *n);

char *scheme_push_c_numeric_locale();
void scheme_pop_c_numeric_locale(char *prev);

/****** Rational numbers *******/

typedef struct {
  Scheme_Object so;
  Scheme_Object *num;
  Scheme_Object *denom;
} Scheme_Rational;

typedef Scheme_Rational Small_Rational;

XFORM_NONGCING Scheme_Object *scheme_make_small_rational(intptr_t n, Small_Rational *space);
XFORM_NONGCING Scheme_Object *scheme_make_small_bn_rational(Scheme_Object *n, Small_Rational *space);
Scheme_Object *scheme_integer_to_rational(const Scheme_Object *n);
Scheme_Object *scheme_make_fixnum_rational(intptr_t n, intptr_t d);
XFORM_NONGCING int scheme_rational_eq(const Scheme_Object *a, const Scheme_Object *b);
int scheme_rational_lt(const Scheme_Object *a, const Scheme_Object *b);
int scheme_rational_gt(const Scheme_Object *a, const Scheme_Object *b);
int scheme_rational_le(const Scheme_Object *a, const Scheme_Object *b);
int scheme_rational_ge(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_rational_negate(const Scheme_Object *n);
Scheme_Object *scheme_rational_add(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_rational_subtract(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_rational_add1(const Scheme_Object *n);
Scheme_Object *scheme_rational_sub1(const Scheme_Object *n);
Scheme_Object *scheme_rational_multiply(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_rational_max(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_rational_min(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_rational_divide(const Scheme_Object *n, const Scheme_Object *d);
Scheme_Object *scheme_rational_power(const Scheme_Object *a, const Scheme_Object *b);
XFORM_NONGCING int scheme_is_rational_positive(const Scheme_Object *o);
Scheme_Object *scheme_rational_floor(const Scheme_Object *a);
Scheme_Object *scheme_rational_truncate(const Scheme_Object *a);
Scheme_Object *scheme_rational_ceiling(const Scheme_Object *a);
Scheme_Object *scheme_rational_round(const Scheme_Object *a);
Scheme_Object *scheme_rational_sqrt(const Scheme_Object *n);

/****** Complex numbers *******/

typedef struct {
  Scheme_Object so;
  Scheme_Object *r;
  Scheme_Object *i;
} Scheme_Complex;

typedef Scheme_Complex Small_Complex;

#define _scheme_complex_real_part(n) (((Scheme_Complex *)(n))->r)
#define _scheme_complex_imaginary_part(n) (((Scheme_Complex *)(n))->i)

Scheme_Object *scheme_make_small_complex(const Scheme_Object *n, Small_Complex *space);
Scheme_Object *scheme_real_to_complex(const Scheme_Object *n);
int scheme_complex_eq(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_complex_negate(const Scheme_Object *n);
Scheme_Object *scheme_complex_add(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_complex_subtract(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_complex_add1(const Scheme_Object *n);
Scheme_Object *scheme_complex_sub1(const Scheme_Object *n);
Scheme_Object *scheme_complex_multiply(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_complex_divide(const Scheme_Object *n, const Scheme_Object *d);
Scheme_Object *scheme_complex_power(const Scheme_Object *a, const Scheme_Object *b);
Scheme_Object *scheme_complex_sqrt(const Scheme_Object *a);
XFORM_NONGCING int scheme_is_complex_exact(const Scheme_Object *o);

/****** Inexacts ******/

#define REAL_NUMBER_STR "real number"

int scheme_check_double(const char *where, double v, const char *dest);
#ifdef MZ_LONG_DOUBLE
int scheme_check_long_double(const char *where, long_double v, const char *dest);
#endif
#ifdef MZ_USE_SINGLE_FLOATS
int scheme_check_float(const char *where, float v, const char *dest);
#else
# define scheme_check_float scheme_check_double
#endif

double scheme_get_val_as_double(const Scheme_Object *n);
XFORM_NONGCING int scheme_minus_zero_p(double d);

#ifdef MZ_LONG_DOUBLE
long_double scheme_get_val_as_long_double(const Scheme_Object *n);
XFORM_NONGCING int scheme_long_minus_zero_p(long_double d);
#else
# define scheme_long_minus_zero_p(d) scheme_minus_zero_p(d)
#endif

#ifdef MZ_USE_SINGLE_FLOATS
float scheme_get_val_as_float(const Scheme_Object *n);
#endif

#if !defined(USE_IEEE_FP_PREDS) && !defined(USE_SCO_IEEE_PREDS) \
    && !defined(USE_OSF_FP_PREDS) && !defined(USE_PALM_INF_TESTS) \
    && !defined(USE_MSVC_FP_PREDS)
# define MZ_IS_POS_INFINITY(d) ((d) == scheme_infinity_val)
# define MZ_IS_NEG_INFINITY(d) ((d) == scheme_minus_infinity_val)
# ifdef NAN_EQUALS_ANYTHING
#  define MZ_IS_NAN(d) (((d) == 1.0) && ((d) == 2.0))
# else
#  ifdef DEFEAT_FP_COMP_OPTIMIZATION
extern int scheme_both_nan(double a, double b);
#   define MZ_IS_NAN(d) (scheme_both_nan(d, d))
#  else
#   define MZ_IS_NAN(d) (!((d) == (d)))
#  endif
# endif
#else
# ifdef USE_SCO_IEEE_PREDS
#  include <ieeefp.h>
#  define MZ_IS_POS_INFINITY(d) (fpclass(d) == FP_PINF)
#  define MZ_IS_NEG_INFINITY(d) (fpclass(d) == FP_NINF)
#  define MZ_IS_NAN(d) isnan(d)
# else
#  ifdef USE_PALM_INF_TESTS
#   define MZ_IS_POS_INFINITY(d) scheme_is_pos_inf(d)
#   define MZ_IS_NEG_INFINITY(d) scheme_is_neg_inf(d)
#   define MZ_IS_NAN(d) scheme_is_nan(d)
extern int scheme_is_pos_inf(double);
extern int scheme_is_neg_inf(double);
extern int scheme_is_nan(double);
#  else
#   ifdef USE_OSF_FP_PREDS
#    include <math.h>
#    include <fp_class.h>
#    define MZ_IS_POS_INFINITY(d) (fp_class(d) == FP_POS_INF)
#    define MZ_IS_NEG_INFINITY(d) (fp_class(d) == FP_NEG_INF)
#    define MZ_IS_NAN(d) isnan(d)
#   else
#    ifdef USE_CARBON_FP_PREDS
#     define MZ_IS_INFINITY(d) (!__isfinite(d))
#     define MZ_IS_POS_INFINITY(d) (!__isfinite(d) && (d > 0))
#     define MZ_IS_NEG_INFINITY(d) (!__isfinite(d) && (d < 0))
#     define MZ_IS_NAN(d) __isnan(d)
#     define MZ_IS_NEG_ZERO(d) signbit(d)
#    else
#     ifdef USE_MSVC_FP_PREDS
#      include <float.h>
#      define MZ_IS_POS_INFINITY(d) (_fpclass(d) == _FPCLASS_PINF)
#      define MZ_IS_NEG_INFINITY(d) (_fpclass(d) == _FPCLASS_NINF)
#      define MZ_IS_NAN(d) _isnan(d)
#     else
       /* USE_IEEE_FP_PREDS */
#      include <math.h>
#      define MZ_IS_INFINITY(d) (isinf(d))
#      define MZ_IS_POS_INFINITY(d) (isinf(d) && (d > 0))
#      define MZ_IS_NEG_INFINITY(d) (isinf(d) && (d < 0))
#      define MZ_IS_NAN(d) isnan(d)
#      define MZ_IS_NEG_ZERO(d) signbit(d)
#     endif
#    endif
#   endif
#  endif
# endif
#endif

#ifdef MZ_LONG_DOUBLE_API_IS_EXTERNAL
# define MZ_IS_LONG_INFINITY(d) long_double_is_infinity(d)
# define MZ_IS_LONG_POS_INFINITY(d) long_double_is_pos_infinity(d)
# define MZ_IS_LONG_NEG_INFINITY(d) long_double_is_neg_infinity(d)
# define MZ_IS_LONG_NAN(d) long_double_is_nan(d)
#else
# define MZ_IS_LONG_INFINITY(d) MZ_IS_INFINITY(d)
# define MZ_IS_LONG_POS_INFINITY(d) MZ_IS_POS_INFINITY(d)
# define MZ_IS_LONG_NEG_INFINITY(d) MZ_IS_NEG_INFINITY(d)
# define MZ_IS_LONG_NAN(d) MZ_IS_NAN(d)
#endif

#ifndef MZ_IS_INFINITY
# define MZ_IS_INFINITY(d) (MZ_IS_POS_INFINITY(d) || MZ_IS_NEG_INFINITY(d))
#endif

#define IZI_REAL_PART(n) (((Scheme_Complex *)(n))->r)

extern double scheme_infinity_val, scheme_minus_infinity_val;
extern double scheme_floating_point_zero;
extern double scheme_floating_point_nzero;
extern Scheme_Object *scheme_zerod, *scheme_nzerod, *scheme_pi, *scheme_half_pi, *scheme_plus_i, *scheme_minus_i;
extern Scheme_Object *scheme_inf_object, *scheme_minus_inf_object, *scheme_nan_object;
#ifdef MZ_LONG_DOUBLE
extern long_double scheme_long_infinity_val, scheme_long_minus_infinity_val;
extern long_double scheme_long_floating_point_zero;
extern long_double scheme_long_floating_point_nzero;
extern Scheme_Object *scheme_zerol, *scheme_nzerol, *scheme_long_scheme_pi;
extern Scheme_Object *scheme_long_inf_object, *scheme_long_minus_inf_object, *scheme_long_nan_object;
#endif
#ifdef MZ_USE_SINGLE_FLOATS
extern Scheme_Object *scheme_zerof, *scheme_nzerof, *scheme_single_scheme_pi;
extern Scheme_Object *scheme_single_inf_object, *scheme_single_minus_inf_object, *scheme_single_nan_object;
#endif

XFORM_NONGCING double scheme_double_random(Scheme_Object *rand_state);

/****** General numeric ******/

Scheme_Object *scheme_read_number(const mzchar *str, intptr_t len,
				  int is_float,
				  int is_not_float,
				  int decimal_means_float,
				  int radix, int radix_set,
				  Scheme_Object *port,
				  int *div_by_zero,
				  int test_only,
				  Scheme_Object *stxsrc, intptr_t line, intptr_t col, intptr_t pos, intptr_t span,
				  Scheme_Object *indentation);

Scheme_Object *scheme_bin_gcd(const Scheme_Object *n1, const Scheme_Object *n2);
Scheme_Object *scheme_bin_quotient(const Scheme_Object *n1, const Scheme_Object *n2);
Scheme_Object *scheme_bin_mult(const Scheme_Object *n1, const Scheme_Object *n2);
Scheme_Object *scheme_bin_div(const Scheme_Object *n1, const Scheme_Object *n2);
Scheme_Object *scheme_bin_plus(const Scheme_Object *n1, const Scheme_Object *n2);
Scheme_Object *scheme_bin_minus(const Scheme_Object *n1, const Scheme_Object *n2);
int scheme_bin_eq(const Scheme_Object *n1, const Scheme_Object *n2);
int scheme_bin_lt(const Scheme_Object *n1, const Scheme_Object *n2);
int scheme_bin_gt(const Scheme_Object *n1, const Scheme_Object *n2);
int scheme_bin_gt_eq(const Scheme_Object *n1, const Scheme_Object *n2);
int scheme_bin_lt_eq(const Scheme_Object *n1, const Scheme_Object *n2);

Scheme_Object *scheme_bin_quotient_remainder(const Scheme_Object *n1, const Scheme_Object *n2, Scheme_Object **_rem);

Scheme_Object *scheme_sub1(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_add1(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_odd_p(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_even_p(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_expt(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_modulo(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_sqrt(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_abs(int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_inexact_to_exact(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_exact_to_inexact(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_inexact_p(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_TO_DOUBLE(const Scheme_Object *n);
#ifdef MZ_LONG_DOUBLE
Scheme_Object *scheme_TO_LONG_DOUBLE(const Scheme_Object *n);
#endif
Scheme_Object *scheme_to_bignum(const Scheme_Object *o);
XFORM_NONGCING int scheme_is_integer(const Scheme_Object *o);
XFORM_NONGCING int scheme_is_zero(const Scheme_Object *o);
XFORM_NONGCING int scheme_is_negative(const Scheme_Object *o);
XFORM_NONGCING int scheme_is_positive(const Scheme_Object *o);
Scheme_Object *scheme_make_polar(int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_bitwise_shift(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_bitwise_and(int argc, Scheme_Object *argv[]);

int scheme_exact_p(Scheme_Object *n);
int scheme_nonneg_exact_p(Scheme_Object *n);

Scheme_Object *scheme_floor(int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_bytes_to_integer(char *str, int slen, int sgned, int rshft, int mask);

#define scheme_make_integer_value_from_time(t) scheme_make_integer_value((intptr_t)t)
#define scheme_get_time_val(o, v) scheme_get_int_val(o, v)
#define UNBUNDLE_TIME_TYPE intptr_t

/***** Random number generator *****/

#ifdef MZ_BSD_RANDOM_GENERATOR
# define MZ_RANDOM_STATE_DEG 31
typedef struct {
  Scheme_Object so;
  short fpos, rpos;
  long state[MZ_RANDOM_STATE_DEG];
} Scheme_Random_State;
#else
typedef struct {
  Scheme_Object so;
  double x10, x11, x12, x20, x21, x22; 
} Scheme_Random_State;
#endif

Scheme_Object *scheme_make_random_state(intptr_t seed);
intptr_t scheme_rand(Scheme_Random_State *rs);

/***** flonums *****/

double scheme_double_truncate(double x);
double scheme_double_round(double x);
double scheme_double_floor(double x);
double scheme_double_ceiling(double x);
double scheme_double_sin(double x);
double scheme_double_cos(double x);
double scheme_double_tan(double x);
double scheme_double_asin(double x);
double scheme_double_acos(double x);
double scheme_double_atan(double x);
double scheme_double_log(double x);
double scheme_double_exp(double x);
double scheme_double_expt(double x, double y);

/***** extflonums *****/
#ifdef MZ_LONG_DOUBLE
long_double scheme_long_double_truncate(long_double x);
long_double scheme_long_double_round(long_double x);
long_double scheme_long_double_floor(long_double x);
long_double scheme_long_double_ceiling(long_double x);
long_double scheme_long_double_sin(long_double x);
long_double scheme_long_double_cos(long_double x);
long_double scheme_long_double_tan(long_double x);
long_double scheme_long_double_asin(long_double x);
long_double scheme_long_double_acos(long_double x);
long_double scheme_long_double_atan(long_double x);
long_double scheme_long_double_log(long_double x);
long_double scheme_long_double_exp(long_double x);
long_double scheme_long_double_expt(long_double x, long_double y);
#endif
/*========================================================================*/
/*                     read, eval, print                                  */
/*========================================================================*/

/* A "prefix" is put on the stack and captured by closures to captures
   a set of top-level or module variables and syntax
   objects. Top-level and module variables are packed together in a
   prefix on the theory that a lot of them may be captured in a single
   closure, and so it's better to keep one layer of hierarchy.  For
   3m, special GC cooperation allows a prefix's set of variables to be
   pruned (i.e., dropped from the prefix) for slots that are not used
   by any closure, when the prefix is accessed only by closures. */
typedef struct Scheme_Prefix
{
  Scheme_Inclhash_Object iso; /* scheme_prefix_type; 0x1 => incremental-mode fixup chain */
  int num_slots, num_toplevels, num_stxes;
#ifdef MZ_PRECISE_GC
  struct Scheme_Prefix *next_final; /* for special GC handling */
  struct Scheme_Object *fixup_chain; /* for special GC handling */
#endif
#ifdef MZ_GC_BACKTRACE
  Scheme_Object *backpointer;
#endif
  Scheme_Object *a[mzFLEX_ARRAY_DECL]; /* array of objects */
  /* followed by an array of `int's for tl_map uses */
} Scheme_Prefix;

#define SCHEME_PREFIX_FLAGS(obj) MZ_OPT_HASH_KEY(&(obj)->iso)

#define PREFIX_TO_USE_BITS(pf) \
  (int *)((char *)pf + sizeof(Scheme_Prefix) + ((pf->num_slots - mzFLEX_DELTA) * sizeof(Scheme_Object *)))

#define LOAD_ON_DEMAND
void scheme_clear_delayed_load_cache();

#define _scheme_do_eval(obj, env, v) \
  ((SCHEME_INTP(obj) || !SCHEME_STRTAG_VAL(_SCHEME_TYPE(obj))) \
   ? obj : scheme_do_eval(obj, -1, env, v))
#define q_scheme_eval_linked(obj) _scheme_do_eval(obj, 1)
#define q_scheme_tail_eval(obj) scheme_tail_eval(obj)

Scheme_Object *scheme_eval_linked_expr(Scheme_Object *expr);
Scheme_Object *scheme_eval_linked_expr_multi(Scheme_Object *expr);
Scheme_Object *scheme_eval_linked_expr_multi_with_dynamic_state(Scheme_Object *obj, Scheme_Dynamic_State *dyn_state);

Scheme_Object *_scheme_apply_to_list (Scheme_Object *rator, Scheme_Object *rands);
Scheme_Object *_scheme_tail_apply_to_list (Scheme_Object *rator, Scheme_Object *rands);

Scheme_Object *_scheme_apply_native(Scheme_Object *obj, int num_rands, Scheme_Object **rands);

Scheme_Object *scheme_internal_read(Scheme_Object *port, Scheme_Object *stxsrc, int crc, int cantfail, 
				    int recur, int expose_comment, int pre_char, Scheme_Object *readtable,
				    Scheme_Object *magic_sym, Scheme_Object *magic_val,
                                    Scheme_Object *delay_load_info);
void scheme_internal_display(Scheme_Object *obj, Scheme_Object *port);
void scheme_internal_write(Scheme_Object *obj, Scheme_Object *port);
void scheme_internal_print(Scheme_Object *obj, Scheme_Object *port, Scheme_Object *quote_depth);

Scheme_Object *scheme_read_language(Scheme_Object *port, int nonlang_ok);

#define _scheme_eval_linked_expr(obj) scheme_do_eval(obj,-1,NULL,1)
#define _scheme_eval_linked_expr_multi(obj) scheme_do_eval(obj,-1,NULL,-1)
#define _scheme_eval_linked_expr_wp(obj, p) scheme_do_eval_w_thread(obj,-1,NULL,1,p)
#define _scheme_eval_linked_expr_multi_wp(obj, p) scheme_do_eval_w_thread(obj,-1,NULL,-1,p)

Scheme_Object *scheme_named_map_1(char *,
				  Scheme_Object *(*fun)(Scheme_Object*, Scheme_Object *form),
				  Scheme_Object *lst, Scheme_Object *form);

XFORM_NONGCING int scheme_strncmp(const char *a, const char *b, int len);

#define _scheme_make_char(ch) scheme_make_character(ch)

Scheme_Object *scheme_default_eval_handler(int, Scheme_Object *[]);
Scheme_Object *scheme_default_compile_handler(int, Scheme_Object *[]);
Scheme_Object *scheme_default_print_handler(int, Scheme_Object *[]);
Scheme_Object *scheme_default_prompt_read_handler(int, Scheme_Object *[]);
Scheme_Object *scheme_default_read_input_port_handler(int argc, Scheme_Object *[]);
Scheme_Object *scheme_default_read_handler(int argc, Scheme_Object *[]);

extern Scheme_Object *scheme_eof_object_p_proc;
extern Scheme_Object *scheme_default_global_print_handler;

/* Type readers & writers for compiled code data */
void scheme_install_type_reader(Scheme_Type type, Scheme_Type_Reader f);
void scheme_install_type_writer(Scheme_Type type, Scheme_Type_Writer f);

Scheme_Object *scheme_make_default_readtable(void);
Scheme_Object *scheme_read_intern(Scheme_Object *o);

Scheme_Object *_scheme_apply_from_native(Scheme_Object *rator,
					 int argc,
					 Scheme_Object **argv);
Scheme_Object *_scheme_apply_multi_from_native(Scheme_Object *rator,
					       int argc,
					       Scheme_Object **argv);
Scheme_Object *_scheme_tail_apply_from_native(Scheme_Object *rator,
					      int argc,
					      Scheme_Object **argv);

Scheme_Object *scheme_force_value_same_mark(Scheme_Object *);
Scheme_Object *scheme_force_one_value_same_mark(Scheme_Object *);

void scheme_flush_stack_cache(void);

struct Scheme_Load_Delay;
Scheme_Object *scheme_load_delayed_code(int pos, struct Scheme_Load_Delay *ld);

intptr_t scheme_get_print_width(void);

#include "../utils/schiptr.h"

/*========================================================================*/
/*                          compile and link                              */
/*========================================================================*/

typedef struct Comp_Prefix
{
  MZTAG_IF_REQUIRED
  int num_toplevels, num_stxes, non_phaseless;
  Scheme_Hash_Table *toplevels; /* buckets for toplevel/module variables */
  Scheme_Hash_Table *inline_variants; /* position -> inline_variant */
  Scheme_Object *unbound; /* identifiers (and lists of phase-1 shifted unbounds) that were unbound at compile */
  Scheme_Hash_Table *stxes;     /* syntax objects */
} Comp_Prefix;

typedef Scheme_Object *(*Scheme_Expand_Result_Adjust_Proc)(Scheme_Object *stx, Scheme_Object *arg);

typedef struct Scheme_Comp_Env
{
  MZTAG_IF_REQUIRED
  short flags;          /* used for expanding/compiling */
  Scheme_Env *genv;     /* top-level environment */
  Scheme_Object *insp;  /* code inspector for checking protected */
  Comp_Prefix *prefix;  /* stack base info: globals and stxes */

  Scheme_Object *scopes; /* can be NULL, a scope, a scope set, or (cons scope-set nobind-scope) */

  Scheme_Object *value_name; /* propagated down */
  Scheme_Object *observer; /* parameter's value (to avoid looking up every time) */

  /* local bindings; */
  mzshort num_bindings; /* number of `values' slots */
  Scheme_Object **binders; /* identifiers */
  Scheme_Object **bindings; /* symbols */
  Scheme_Object **vals; /* compile-time values */
  Scheme_Object **shadower_deltas;
  Scheme_IR_Local **vars;
  int *use;
  int max_use, any_use;
  
  Scheme_Object *lifts;

  Scheme_Hash_Table *binding_namess; /* <phase> -> (<symbol> -> <id>); additions to the environment's
                                        bindings table made during a particular compilation */

  mzshort rename_var_count;      /* number of non-NULL `values' when `renames' was computed */
  mzshort rename_rstart;         /* leftover rstart from previous round; see env.c */
  Scheme_Hash_Table *dup_check;  /* table for finding colliding symbols in `values' */

  Scheme_Object *intdef_name;    /* syntax-local-context name for INTDEF frames */

  Scheme_Object *in_modidx;      /* during lookup/expand in macro */

  Scheme_Hash_Tree *skip_table; /* for jumping ahead in the chain */
  int skip_depth;                /* depth in simple frames, used to trigger skip_table creation */

  Scheme_Expand_Result_Adjust_Proc expand_result_adjust;
  Scheme_Object *expand_result_adjust_arg;

  struct Scheme_Comp_Env *next;
  struct Scheme_Comp_Env *use_scopes_next; /* fast-forward for use-site scope revert */
} Scheme_Comp_Env;

#define LAMBDA_HAS_REST 1
#define LAMBDA_HAS_TYPED_ARGS 2
#define LAMBDA_PRESERVES_MARKS 4
#define LAMBDA_NEED_REST_CLEAR 8
#define LAMBDA_IS_METHOD 16
#define LAMBDA_SINGLE_RESULT 32
#define LAMBDA_RESULT_TENTATIVE 64
#define LAMBDA_VALIDATED 128
#define LAMBDA_SFS 256
/* BITS 8-15 (overlaps LAMBDA_SFS) used by write_lambda() */

typedef struct Scheme_Compile_Expand_Info
{
  /* allocated as atomic */
  short comp;
  short comp_flags;
  char dont_mark_local_use;
  char resolve_module_ids;
  char pre_unwrapped;
  char testing_constantness;
  char substitute_bindings;
  int depth;
  int env_already;
} Scheme_Compile_Expand_Info;

#define COMP_ALLOW_SET_UNDEFINED 0x1
#define COMP_CAN_INLINE 0x2
#define COMP_ENFORCE_CONSTS 0x4

typedef Scheme_Compile_Expand_Info Scheme_Compile_Info;
typedef Scheme_Compile_Expand_Info Scheme_Expand_Info;

typedef struct Resolve_Prefix
{
  Scheme_Object so;
  int num_toplevels, num_stxes, num_lifts;
  Scheme_Object **toplevels;
  Scheme_Object **stxes; /* simplified */
  Scheme_Object *delay_info_rpair; /* (rcons refcount Scheme_Load_Delay*) */
  /* An inspector or symbol to identify bindings that are created as
     part of the module's expansion, so that a suitable inspector can
     be associated with those bindings (through a syntax-object
     "shift") when the code is re-loaded. */
  Scheme_Object *src_insp_desc;
} Resolve_Prefix;

typedef struct Resolve_Info Resolve_Info;

/* Scheme_IR_Lambda_Info is used to store extra closure information
   before a closure mapping is resolved. */
typedef struct {
  MZTAG_IF_REQUIRED
  Scheme_Hash_Table *base_closure;
  Scheme_IR_Local **vars;
  Scheme_Object **arg_types; /* predicates for the arguments, as determined by callers */
  short *arg_type_contributors; /* bitmap of applications that have provided type info;
                                   when the number of calls is know, this information
                                   can reveal when all callers have checked in; the
                                   contributor SCHEME_USE_COUNT_INF is an anonymous
                                   contributor; if a contributor set is non-empty;
                                   then NULL for a type mean "top" */
  char has_tl, has_nonleaf, is_dup;
  int body_size, body_psize;
} Scheme_IR_Lambda_Info;

typedef struct Optimize_Info Optimize_Info;

typedef struct Scheme_Object *
(Scheme_Syntax)(struct Scheme_Object *form, struct Scheme_Comp_Env *env,
		Scheme_Compile_Info *rec, int drec);

typedef struct Scheme_Object *
(Scheme_Syntax_Expander)(struct Scheme_Object *form, struct Scheme_Comp_Env *env,
			 Scheme_Expand_Info *rec, int drec);

typedef struct CPort Mz_CPort;

typedef struct Scheme_Lambda
{
  Scheme_Inclhash_Object iso; /* keyex used for flags */
  mzshort num_params; /* includes collecting arg if has_rest */
  mzshort max_let_depth;
  mzshort closure_size; /* the number of closed-over variables */
  union {
    Scheme_IR_Lambda_Info *ir_info; /* used until resolve pass */
    mzshort *closure_map; /* after resolve pass: 
                             contains closure_size elements mapping closed-over var to stack positions.

                             If LAMBDA_HAS_TYPED_ARGS, that array is followed by bit array with
                             LAMBDA_TYPE_BITS_PER_ARG bits per args then per closed-over

                             total size = closure_size + (closure_size + num_params) * LAMBDA_TYPE_BITS_PER_ARG */
  };
  Scheme_Object *body;
  Scheme_Object *name; /* name or (vector name src line col pos span generated?) */
  void *tl_map; /* fixnum or bit array (as array of `int's) indicating which globals+lifts in prefix are used */
#ifdef MZ_USE_JIT
  union {
    struct Scheme_Lambda *jit_clone;
    struct Scheme_Native_Lambda *native_code; /* generated by lightning */
  } u;
  Scheme_Object *context; /* e.g., a letrec that binds the closure */
#endif
} Scheme_Lambda;

#define SCHEME_LAMBDA_FLAGS(obj) MZ_OPT_HASH_KEY(&(obj)->iso)

#define LAMBDA_TYPE_BITS_PER_ARG 4
#define LAMBDA_TYPE_BOXED 1
#define LAMBDA_TYPE_TYPE_OFFSET 1

XFORM_NONGCING void scheme_boxmap_set(mzshort *boxmap, int j, int bit, int delta);
XFORM_NONGCING int scheme_boxmap_get(mzshort *boxmap, int j, int delta);
XFORM_NONGCING int scheme_boxmap_size(int n);

int scheme_has_method_property(Scheme_Object *code);

typedef struct Scheme_Closure {
  Scheme_Object so;
  Scheme_Lambda *code;
  Scheme_Object *vals[mzFLEX_ARRAY_DECL];
} Scheme_Closure;

#define SCHEME_CLOSURE_CODE(c) ((Scheme_Closure *)c)->code
#define SCHEME_CLOSURE_ENV(c)  ((Scheme_Closure *)c)->vals

#define ZERO_SIZED_CLOSUREP(closure) !(closure->code->closure_size)

typedef struct Scheme_Native_Lambda {
  Scheme_Inclhash_Object iso; /* type tag only set when needed, but
                                 flags always needed */
  Scheme_Native_Proc *start_code; /* When not yet JITted, this is = to
                                     scheme_on_demand_jit_code */  
  union {
    void *tail_code;                       /* For non-case-lambda */
    mzshort *arities;                      /* For case-lambda */
  } u;
  void *arity_code;
  mzshort max_let_depth; /* In bytes instead of words */
  mzshort closure_size; /* If this is negative, then this is a
                           case-lambda, and the number of cases is 
                           (-closure-size)-1 */
  union {
    struct Scheme_Lambda *orig_code; /* For not-yet-JITted
                                              non-case-lambda */
    Scheme_Object *name;
  } u2;
  void *tl_map;
#ifdef MZ_PRECISE_GC
  void **retained; /* inside code */
#endif
#if defined(MZ_USE_JIT_ARM) && !defined(MZ_PRECISE_GC)
# define NEED_RETAIN_CODE_POINTERS
  /* Thumb code is off by one, need real start for GC */
  void *retain_code;
#endif
  void *eq_key; /* for `procedure-closure-contents-eq?` */
} Scheme_Native_Lambda;

#define SCHEME_NATIVE_LAMBDA_FLAGS(obj) MZ_OPT_HASH_KEY(&(obj)->iso)

/* This flag is set pre-JIT: */
#define NATIVE_SPECIALIZED 0x1
/* Other flags are in "jit.h" */

typedef struct {
  Scheme_Object so;
  Scheme_Native_Lambda *code;
  Scheme_Object *vals[mzFLEX_ARRAY_DECL];
} Scheme_Native_Closure;

Scheme_Native_Lambda *scheme_generate_lambda(Scheme_Lambda *obj, int drop_code, 
                                             Scheme_Native_Lambda *case_lam);

typedef struct Scheme_Current_LWC {
  /* !! All of these fields are treated as atomic by the GC !! */
  Scheme_Object **runstack_start;
  MZ_MARK_STACK_TYPE cont_mark_stack_start;
  MZ_MARK_POS_TYPE cont_mark_pos_start;
  void *stack_start;
  Scheme_Object **runstack_end;
  Scheme_Object **runstack_base_end;
  MZ_MARK_STACK_TYPE cont_mark_stack_end;
  MZ_MARK_POS_TYPE cont_mark_pos_end;
  void *frame_end;
  void *stack_end;
  void *original_dest;
  void *saved_v1;
  double saved_save_fp;
#ifdef MZ_LONG_DOUBLE
  long_double saved_save_extfp;
#endif
} Scheme_Current_LWC;

void scheme_init_thread_lwc(void);
void scheme_fill_lwc_start(void);
void scheme_fill_lwc_end(void);
void scheme_fill_stack_lwc_end(void);
void scheme_clear_lwc(void);

THREAD_LOCAL_DECL(MZ_EXTERN Scheme_Current_LWC *scheme_current_lwc);

Scheme_Object *scheme_call_as_lightweight_continuation(Scheme_Native_Proc *code,
                                                       void *data,
                                                       int argc, 
                                                       Scheme_Object **argv);
void *scheme_save_lightweight_continuation_stack(Scheme_Current_LWC *lwc);
Scheme_Object *scheme_apply_lightweight_continuation_stack(Scheme_Current_LWC *lwc, void *stack, 
                                                           Scheme_Object *result);
struct Scheme_Lightweight_Continuation;
typedef struct Scheme_Lightweight_Continuation Scheme_Lightweight_Continuation;
Scheme_Lightweight_Continuation *scheme_capture_lightweight_continuation(Scheme_Thread *p,
                                                                         Scheme_Current_LWC *p_lwc,
                                                                         void **storage);
Scheme_Object *scheme_apply_lightweight_continuation(Scheme_Lightweight_Continuation *captured,
                                                     Scheme_Object *result,
                                                     int result_is_rs_argv,
                                                     intptr_t min_stacksize);
Scheme_Object **scheme_adjust_runstack_argument(Scheme_Lightweight_Continuation *captured,
                                                Scheme_Object **arg);

Scheme_Lightweight_Continuation *scheme_restore_lightweight_continuation_marks(Scheme_Lightweight_Continuation *lw);

int scheme_can_apply_lightweight_continuation(Scheme_Lightweight_Continuation *captured,
                                              int check_overflow);

int scheme_push_marks_from_thread(Scheme_Thread *p2, Scheme_Cont_Frame_Data *d);
int scheme_push_marks_from_lightweight_continuation(Scheme_Lightweight_Continuation *captured, 
                                                    Scheme_Cont_Frame_Data *d);

#define scheme_new_frame(n) scheme_new_special_frame(n, 0)
#define scheme_extend_env(f, e) (f->basic.next = e, f)
#define scheme_next_frame(e) ((e)->basic.next)
#define scheme_settable_frame(f, s) ((f)->basic.has_set_bang = (s))
#define scheme_get_frame_settable(f) ((f)->basic.has_set_bang)
#define scheme_get_binding(f, n) ((f)->values[n])

int scheme_is_module_begin_env(Scheme_Comp_Env *env);
Scheme_Object *scheme_make_toplevel(mzshort depth, int position, int resolved, int flags);

#define MAX_CONST_LOCAL_POS 64
#define MAX_CONST_LOCAL_TYPES 2
#define MAX_CONST_LOCAL_FLAG_VAL (2 + SCHEME_MAX_LOCAL_TYPE)

#define MAX_CONST_TOPLEVEL_DEPTH 16
#define MAX_CONST_TOPLEVEL_POS 16

#define ASSERT_IS_VARIABLE_BUCKET(b) /* if (((Scheme_Object *)b)->type != scheme_variable_type) abort() */

Scheme_Comp_Env *scheme_new_comp_env(Scheme_Env *genv, Scheme_Object *insp, Scheme_Object *scopes, int flags);
Scheme_Comp_Env *scheme_new_expand_env(Scheme_Env *genv, Scheme_Object *insp, Scheme_Object *scopes, int flags);

Scheme_Object *scheme_namespace_lookup_value(Scheme_Object *sym, Scheme_Env *genv, 
                                             Scheme_Object **_id, int *_use_map);
Scheme_Object *scheme_get_shadower(Scheme_Object *sym, Scheme_Comp_Env *env, int only_generated);
Scheme_Object *scheme_do_local_lift_expr(const char *who, int stx_pos, 
                                         int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_local_lift_context(Scheme_Comp_Env *env);
Scheme_Object *scheme_local_lift_end_statement(Scheme_Object *expr, Scheme_Object *local_scope, 
                                               Scheme_Comp_Env *env);
Scheme_Object *scheme_local_lift_module(Scheme_Object *expr, Scheme_Object *local_scope, 
                                        Scheme_Comp_Env *env);
Scheme_Object *scheme_local_lift_require(Scheme_Object *form, Scheme_Object *orig_form,
                                         intptr_t phase, Scheme_Object *local_scope, 
                                         Scheme_Comp_Env *env);
Scheme_Object *scheme_local_lift_provide(Scheme_Object *form, Scheme_Object *local_scope, 
                                         Scheme_Comp_Env *env);
Scheme_Comp_Env *scheme_get_module_lift_env(Scheme_Comp_Env *env);

void scheme_check_identifier(const char *formname, Scheme_Object *id,
			     const char *where,
			     Scheme_Comp_Env *env,
			     Scheme_Object *form);

Scheme_Object *scheme_check_immediate_macro(Scheme_Object *first,
					    Scheme_Comp_Env *env,
					    Scheme_Compile_Expand_Info *erec, int drec,
					    Scheme_Object **current_val,
                                            int keep_name);

Scheme_Object *scheme_apply_macro(Scheme_Object *name, Scheme_Env *menv,
				  Scheme_Object *f, Scheme_Object *code,
				  Scheme_Comp_Env *env, Scheme_Object *boundname,
                                  Scheme_Compile_Expand_Info *rec, int drec,
                                  int for_set,
                                  int scope_macro_use);

Scheme_Comp_Env *scheme_new_compilation_frame(int num_bindings, int flags, 
                                              Scheme_Object *scope,
					      Scheme_Comp_Env *env);
void scheme_add_compilation_binding(int index, Scheme_Object *val,
				    Scheme_Comp_Env *frame);
void scheme_add_compilation_frame_use_site_scope(Scheme_Comp_Env *frame,
                                            Scheme_Object *use_site_scope);
void scheme_add_compilation_frame_intdef_scope(Scheme_Comp_Env *frame,
                                              Scheme_Object *intdef_scope);
Scheme_Comp_Env *scheme_add_compilation_frame(Scheme_Object *vals, Scheme_Object *scope,
					      Scheme_Comp_Env *env, int flags);

Scheme_Comp_Env *scheme_no_defines(Scheme_Comp_Env *env);

Scheme_Object *scheme_compile_lookup(Scheme_Object *symbol, Scheme_Comp_Env *env, int flags, 
				     Scheme_Object *in_modidx, 
				     Scheme_Env **_menv, int *_protected,
                                     Scheme_Object **_local_binder, int *_need_macro_scope,
                                     Scheme_Object **_inline_variant);
int scheme_is_imported(Scheme_Object *var, Scheme_Comp_Env *env);

Scheme_Object *scheme_extract_unsafe(Scheme_Object *o);
Scheme_Object *scheme_extract_flfxnum(Scheme_Object *o);
Scheme_Object *scheme_extract_extfl(Scheme_Object *o);
Scheme_Object *scheme_extract_futures(Scheme_Object *o);
Scheme_Object *scheme_extract_foreign(Scheme_Object *o);

typedef Scheme_Object *(*Scheme_Lift_Capture_Proc)(Scheme_Object *, Scheme_Object **, Scheme_Object *, Scheme_Comp_Env *);
void scheme_frame_captures_lifts(Scheme_Comp_Env *env, Scheme_Lift_Capture_Proc cp, Scheme_Object *data, 
                                 Scheme_Object *end_stmts, Scheme_Object *context_key, 
                                 Scheme_Object *require_lifts, Scheme_Object *provide_lifts,
                                 Scheme_Object *module_lifts);
void scheme_propagate_require_lift_capture(Scheme_Comp_Env *orig_env, Scheme_Comp_Env *env);
Scheme_Object *scheme_frame_get_lifts(Scheme_Comp_Env *env);
Scheme_Object *scheme_frame_get_end_statement_lifts(Scheme_Comp_Env *env);
Scheme_Object *scheme_frame_get_end_modules(Scheme_Comp_Env *env);
Scheme_Object *scheme_frame_get_require_lifts(Scheme_Comp_Env *env);
Scheme_Object *scheme_frame_get_provide_lifts(Scheme_Comp_Env *env);
Scheme_Object *scheme_generate_lifts_key(void);
Scheme_Object *scheme_top_level_lifts_key(Scheme_Comp_Env *env);
Scheme_Comp_Env *scheme_get_env_for_lifts(Scheme_Comp_Env *env);

Scheme_Object *scheme_toplevel_require_for_expand(Scheme_Object *module_path, 
                                                  intptr_t phase,
                                                  Scheme_Comp_Env *cenv,
                                                  Scheme_Object *scope);
Scheme_Object *scheme_parse_lifted_require(Scheme_Object *module_path,
                                           intptr_t phase,
                                           Scheme_Object *scope,
                                           void *data,
                                           Scheme_Object **_ref_expr,
                                           struct Scheme_Comp_Env *cenv);

void scheme_add_local_syntax(int cnt, Scheme_Comp_Env *env);
void scheme_set_local_syntax(int pos, Scheme_Object *name, Scheme_Object *val,
			     Scheme_Comp_Env *env, int replace_value);

Scheme_Object *scheme_clone_vector(Scheme_Object *data, int skip, int set_type);

Scheme_Object *scheme_make_closure(Scheme_Thread *p,
				   Scheme_Object *compiled_code,
				   int close);
Scheme_Closure *scheme_malloc_empty_closure(void);

Scheme_Object *scheme_make_native_closure(Scheme_Native_Lambda *code);
Scheme_Object *scheme_make_native_case_closure(Scheme_Native_Lambda *code);

void scheme_reset_app2_eval_type(Scheme_App2_Rec *app);
void scheme_reset_app3_eval_type(Scheme_App3_Rec *app);

Scheme_Native_Lambda *scheme_generate_case_lambda(Scheme_Case_Lambda *cl);

void scheme_delay_load_closure(Scheme_Lambda *data);

Scheme_Object *scheme_intdef_bind_identifiers(Scheme_Object *intdef);
  
#define scheme_add_good_binding(i,v,f) (f->values[i] = v)

Scheme_Object *scheme_compiled_void(void);

int scheme_check_top_identifier_bound(Scheme_Object *symbol, Scheme_Env *genv, int disallow_unbound);

Scheme_Object *scheme_register_toplevel_in_prefix(Scheme_Object *var, Scheme_Comp_Env *env,
						  Scheme_Compile_Info *rec, int drec,
                                                  int imported, Scheme_Object *inline_variant);
Scheme_Object *scheme_register_toplevel_in_comp_prefix(Scheme_Object *var, Comp_Prefix *cp,
                                                       int imported, Scheme_Object *inline_variant);
void scheme_register_unbound_toplevel(Scheme_Comp_Env *env, Scheme_Object *id);
Scheme_Object *scheme_register_stx_in_prefix(Scheme_Object *var, Scheme_Comp_Env *env,
					     Scheme_Compile_Info *rec, int drec);
Scheme_Object *scheme_register_stx_in_comp_prefix(Scheme_Object *var, Comp_Prefix *cp);
void scheme_register_unsafe_in_prefix(Scheme_Comp_Env *env,
                                      Scheme_Compile_Info *rec, int drec,
                                      Scheme_Env *menv);
void scheme_merge_undefineds(Scheme_Comp_Env *exp_env, Scheme_Comp_Env *env);

void scheme_bind_syntaxes(const char *where, Scheme_Object *names, Scheme_Object *a, 
                          Scheme_Env *exp_env, Scheme_Object *insp, 
                          Scheme_Compile_Expand_Info *rec, int drec, Scheme_Object *observer,
                          Scheme_Comp_Env *stx_env, Scheme_Comp_Env *rhs_env,
                          int *_pos, Scheme_Object *rename_rib, int replace_value);
int scheme_is_sub_env(Scheme_Comp_Env *stx_env, Scheme_Comp_Env *env);

typedef struct SFS_Info SFS_Info;

SFS_Info *scheme_new_sfs_info(int depth);
Scheme_Object *scheme_sfs(Scheme_Object *expr, SFS_Info *info, int max_let_depth);
Scheme_Object *scheme_sfs_expr(Scheme_Object *expr, SFS_Info *si, int self_pos);

void scheme_sfs_used(SFS_Info *info, int pos);
void scheme_sfs_push(SFS_Info *info, int count, int track);
void scheme_sfs_start_sequence(SFS_Info *si, int cnt, int last_is_tail);

Scheme_Object *scheme_sfs_add_clears(Scheme_Object *expr, Scheme_Object *clears, int pre);

typedef struct Scheme_Object *(*Scheme_Syntax_SFSer)(Scheme_Object *data, SFS_Info *info);

typedef struct Scheme_Set_Bang {
  Scheme_Object so;
  int set_undef;
  Scheme_Object *var, *val;
} Scheme_Set_Bang;

Scheme_Object *scheme_protect_quote(Scheme_Object *expr);

Scheme_Object *scheme_letrec_check_expr(Scheme_Object *);

Scheme_Object *scheme_optimize_expr(Scheme_Object *, Optimize_Info *, int context);

/* Context uses result as a boolean: */
#define OPT_CONTEXT_BOOLEAN    0x1
/* Context might duplicate the expression: */
#define OPT_CONTEXT_NO_SINGLE  0x2
/* Context checks that result is a single value and is non-tail w.r.t. to same clock as bindig: */
#define OPT_CONTEXT_SINGLED    0x4
#define OPT_CONTEXT_TYPE_SHIFT 4
#define OPT_CONTEXT_TYPE_MASK  (SCHEME_MAX_LOCAL_TYPE_MASK << OPT_CONTEXT_TYPE_SHIFT)
#define OPT_CONTEXT_TYPE(oc)   ((oc & OPT_CONTEXT_TYPE_MASK) >> OPT_CONTEXT_TYPE_SHIFT)
#define OPT_CONTEXT_APP_COUNT_SHIFT (OPT_CONTEXT_TYPE_SHIFT + SCHEME_MAX_LOCAL_TYPE_BITS)
#define OPT_CONTEXT_APP_COUNT(oc) ((oc >> OPT_CONTEXT_APP_COUNT_SHIFT) & SCHEME_USE_COUNT_INF)

#define scheme_optimize_result_context(c) (c & (~(OPT_CONTEXT_TYPE_MASK | OPT_CONTEXT_NO_SINGLE | OPT_CONTEXT_SINGLED)))
#define scheme_optimize_tail_context(c)   scheme_optimize_result_context(c) 

int scheme_ir_duplicate_ok(Scheme_Object *o, int cross_mod);
int scheme_is_statically_proc(Scheme_Object *value, Optimize_Info *info, int flags);
XFORM_NONGCING int scheme_predicate_to_local_type(Scheme_Object *pred);
Scheme_Object *scheme_make_noninline_proc(Scheme_Object *e);
Scheme_Object *scheme_optimize_extract_tail_inside(Scheme_Object *t2);

Scheme_Object *scheme_resolve_expr(Scheme_Object *, Resolve_Info *);
Scheme_Object *scheme_resolve_list(Scheme_Object *, Resolve_Info *);
Scheme_Object *scheme_unresolve(Scheme_Object *, int argv, int *_has_cases,
                                Comp_Prefix *cp, Scheme_Env *env, Scheme_Object *insp, intptr_t ref_phase,
                                Scheme_Object *from_modidx, Scheme_Object *to_modidx);
Scheme_Object *scheme_unresolve_top(Scheme_Object *, Comp_Prefix **, int comp_flags);

int scheme_check_leaf_rator(Scheme_Object *le);

int scheme_is_ir_lambda(Scheme_Object *o, int can_be_closed, int can_be_liftable);

Scheme_Object *scheme_resolve_lets(Scheme_Object *form, Resolve_Info *info);

Resolve_Prefix *scheme_resolve_prefix(int phase, Comp_Prefix *cp, Scheme_Object *insp_desc);
Resolve_Prefix *scheme_remap_prefix(Resolve_Prefix *rp, Resolve_Info *ri);

Resolve_Info *scheme_resolve_info_create(Resolve_Prefix *rp);
void scheme_resolve_info_enforce_const(Resolve_Info *, int enforce_const);
int scheme_resolve_info_max_let_depth(Resolve_Info *ri);
int scheme_resolve_info_use_jit(Resolve_Info *ri);

void scheme_enable_expression_resolve_lifts(Resolve_Info *ri);
Scheme_Object *scheme_merge_expression_resolve_lifts(Scheme_Object *expr, Resolve_Prefix *rp, Resolve_Info *ri);

Optimize_Info *scheme_optimize_info_create(Comp_Prefix *cp, Scheme_Env *env, Scheme_Object *insp, int get_logger);
void scheme_optimize_info_enforce_const(Optimize_Info *, int enforce_const);
void scheme_optimize_info_set_context(Optimize_Info *, Scheme_Object *ctx);
void scheme_optimize_info_never_inline(Optimize_Info *);

char *scheme_optimize_info_context(Optimize_Info *);
Scheme_Logger *scheme_optimize_info_logger(Optimize_Info *);

Scheme_Object *scheme_toplevel_to_flagged_toplevel(Scheme_Object *tl, int flags);

int scheme_expr_produces_local_type(Scheme_Object *expr, int *_involves_k_cross);

Scheme_Object *scheme_make_primitive_syntax(Scheme_Syntax *syntax,
                                            Scheme_Syntax_Expander *exp);

Scheme_Object *scheme_compile_expr(Scheme_Object *form, Scheme_Comp_Env *env,
				   Scheme_Compile_Info *rec, int drec);

Scheme_Object *scheme_compile_expr_lift_to_let(Scheme_Object *form, Scheme_Comp_Env *env,
					       Scheme_Compile_Info *rec, int drec);

Scheme_Object *scheme_pair_lifted(Scheme_Object *_ip, Scheme_Object **_ids, Scheme_Object *expr, 
                                  Scheme_Comp_Env *env);
Scheme_Object *scheme_add_lifts_as_let(Scheme_Object *obj, Scheme_Object *l, Scheme_Comp_Env *env,
                                       Scheme_Object *orig_form, int comp_rev);

void scheme_add_core_stop_form(int pos, Scheme_Object *sym, Scheme_Comp_Env *env);

void scheme_default_compile_rec(Scheme_Compile_Info *src, int drec);
void scheme_compile_rec_done_local(Scheme_Compile_Info *src, int drec);
void scheme_init_compile_recs(Scheme_Compile_Info *src, int drec,
			      Scheme_Compile_Info *dest, int n);
void scheme_merge_compile_recs(Scheme_Compile_Info *src, int drec,
			       Scheme_Compile_Info *dest, int n);
void scheme_init_lambda_rec(Scheme_Compile_Info *src, int drec,
			    Scheme_Compile_Info *lam, int dlrec);
void scheme_merge_lambda_rec(Scheme_Compile_Info *src, int drec,
			    Scheme_Compile_Info *lam, int dlrec);


void scheme_init_expand_recs(Scheme_Expand_Info *src, int drec,
			     Scheme_Expand_Info *dest, int n);

Scheme_Object *scheme_make_sequence_compilation(Scheme_Object *compiled_list,
						int strip_values,
                                                int resolved);

Scheme_App_Rec *scheme_malloc_application(int n);
void scheme_finish_application(Scheme_App_Rec *app);

Scheme_Sequence *scheme_malloc_sequence(int count);

Scheme_Object *scheme_jit_expr(Scheme_Object *);
Scheme_Object *scheme_jit_closure(Scheme_Object *, Scheme_Object *context);
void scheme_jit_fill_threadlocal_table();

#ifdef MZ_USE_JIT
void scheme_on_demand_generate_lambda(Scheme_Native_Closure *nc, int argc, Scheme_Object **argv, int delta);
#endif

struct Start_Module_Args;

#ifdef MZ_USE_JIT
void *scheme_module_run_start(Scheme_Env *menv, Scheme_Env *env, Scheme_Object *name);
void *scheme_module_exprun_start(Scheme_Env *menv, int phase_plus_set_ns, Scheme_Object *name);
void *scheme_module_start_start(struct Start_Module_Args *a, Scheme_Object *name);
#endif
void *scheme_module_run_finish(Scheme_Env *menv, Scheme_Env *env);
void *scheme_module_exprun_finish(Scheme_Env *menv, int set_ns);
void *scheme_module_start_finish(struct Start_Module_Args *a);

Scheme_Object *scheme_build_closure_name(Scheme_Object *code, Scheme_Comp_Env *env);

#define SCHEME_SYNTAX(obj)     SCHEME_PTR1_VAL(obj)
#define SCHEME_SYNTAX_EXP(obj) SCHEME_PTR2_VAL(obj)

int scheme_env_check_reset_any_use(Scheme_Comp_Env *frame);
int scheme_env_max_use_above(Scheme_Comp_Env *frame, int pos);
void scheme_mark_all_use(Scheme_Comp_Env *frame);
void scheme_env_make_variables(Scheme_Comp_Env *frame);
void scheme_set_compilation_variables(Scheme_Comp_Env *frame, Scheme_IR_Local **vars,
                                      int pos, int count);

/* flags reported by scheme_resolve_info_flags */
#define SCHEME_INFO_BOXED 0x1
#define SCHEME_INFO_TYPED_VAL_SHIFT 4
#define SCHEME_INFO_TYPED_VAL_MASK (SCHEME_MAX_LOCAL_TYPE_MASK << SCHEME_INFO_TYPED_VAL_SHIFT)

/* flags used with scheme_new_frame */
#define SCHEME_TOPLEVEL_FRAME      (1 << 0)
#define SCHEME_MODULE_FRAME        (1 << 1)
#define SCHEME_MODULE_BEGIN_FRAME  (1 << 2)
#define SCHEME_LAMBDA_FRAME        (1 << 3)
#define SCHEME_INTDEF_FRAME        (1 << 4)
#define SCHEME_USE_SCOPES_TO_NEXT  (1 << 5)
#define SCHEME_CAPTURE_WITHOUT_RENAME (1 << 6)
#define SCHEME_FOR_STOPS           (1 << 7)
#define SCHEME_FOR_INTDEF          (1 << 8)
#define SCHEME_CAPTURE_LIFTED      (1 << 9)
#define SCHEME_INTDEF_SHADOW       (1 << 10)
#define SCHEME_POST_BIND_FRAME     (1 << 11)
#define SCHEME_NESTED_MODULE_FRAME (1 << 12)
#define SCHEME_KEEP_SCOPES_FRAME   (1 << 13)
#define SCHEME_TMP_TL_BIND_FRAME   (1 << 14)

#define SCHEME_REC_BINDING_FRAME (SCHEME_TOPLEVEL_FRAME | SCHEME_MODULE_BEGIN_FRAME \
                                  | SCHEME_INTDEF_FRAME | SCHEME_FOR_INTDEF)

/* Flags used with scheme_static_distance */
#define SCHEME_ELIM_CONST 1
#define SCHEME_APP_POS 2
#define SCHEME_SETTING 4
#define SCHEME_ENV_CONSTANTS_OK 8
#define SCHEME_GLOB_ALWAYS_REFERENCE 16
#define SCHEME_MUST_INDRECT 32
#define SCHEME_LINKING_REF 64
#define SCHEME_DONT_MARK_USE 128
#define SCHEME_OUT_OF_CONTEXT_OK 256
#define SCHEME_NULL_FOR_UNBOUND 512
#define SCHEME_RESOLVE_MODIDS 1024
#define SCHEME_NO_CERT_CHECKS 2048
#define SCHEME_REFERENCING 4096
#define SCHEME_OUT_OF_CONTEXT_LOCAL 8192
#define SCHEME_STOP_AT_FREE_EQ 16384

Scheme_Hash_Table *scheme_map_constants_to_globals(void);
const char *scheme_look_for_primitive(void *code);

Scheme_Object *scheme_expand_expr(Scheme_Object *form, Scheme_Comp_Env *env,
				  Scheme_Expand_Info *erec, int drec);
Scheme_Object *scheme_expand_expr_lift_to_let(Scheme_Object *form, Scheme_Comp_Env *env,
					      Scheme_Expand_Info *erec, int drec);

Scheme_Object *scheme_flatten_begin(Scheme_Object *expr, Scheme_Object *append_onto);

Scheme_Object *scheme_make_svector(mzshort v, mzshort *a);

#define SCHEME_SVEC_LEN(obj) (((Scheme_Simple_Object *)(obj))->u.svector_val.len)
#define SCHEME_SVEC_VEC(obj) (((Scheme_Simple_Object *)(obj))->u.svector_val.vec)

Scheme_Object *scheme_hash_percent_name(const char *name, int len);

Scheme_Object *scheme_make_branch(Scheme_Object *test,
				  Scheme_Object *tbranch,
				  Scheme_Object *fbranch);

int scheme_is_toplevel(Scheme_Comp_Env *env);
int scheme_is_nested_module(Scheme_Comp_Env *env);
Scheme_Comp_Env *scheme_extend_as_toplevel(Scheme_Comp_Env *env);

Scheme_Env *scheme_make_empty_env(void);
void scheme_prepare_exp_env(Scheme_Env *env);
void scheme_prepare_template_env(Scheme_Env *env);
void scheme_prepare_label_env(Scheme_Env *env);
void scheme_prepare_env_stx_context(Scheme_Env *env);

XFORM_NONGCING Scheme_Object *scheme_env_phase(Scheme_Env *env);
Scheme_Env *scheme_find_env_at_phase(Scheme_Env *env, Scheme_Object *phase);

int scheme_omittable_expr(Scheme_Object *o, int vals, int fuel, int flags,
                          Optimize_Info *opt_info, Optimize_Info *warn_info);
#define OMITTABLE_RESOLVED          0x1
#define OMITTABLE_KEEP_VARS         0x2
#define OMITTABLE_KEEP_MUTABLE_VARS 0x4
#define OMITTABLE_IGNORE_APPN_OMIT  0x8
#define OMITTABLE_IGNORE_MAKE_STRUCT_TYPE 0x10

int scheme_might_invoke_call_cc(Scheme_Object *value);
int scheme_is_liftable(Scheme_Object *o, Scheme_Hash_Tree *exclude_vars, int fuel, int as_rator, int or_escape);
int scheme_is_functional_nonfailing_primitive(Scheme_Object *rator, int num_args, int expected_vals);

typedef struct {
  int uses_super;
  int super_field_count; /* total fields (must == constructor-supplied fields) in superstruct */
  int field_count;       /* total fields in this struct */
  int init_field_count;  /* number of fields supplied to the constructor; usually == field_count */
  int normal_ops;  /* are selectors and predicates in the usual order? */
  int indexed_ops; /* do selectors have the index built in (as opposed to taking an index argument)? */
  int authentic; /* conservatively 0 is ok */
  int num_gets, num_sets;
} Simple_Struct_Type_Info;

Scheme_Object *scheme_is_simple_make_struct_type(Scheme_Object *app, int vals, int flags,
                                                 int *_auto_e_depth, 
                                                 Simple_Struct_Type_Info *_stinfo,
                                                 Scheme_Object **_parent_identity,
                                                 Scheme_Hash_Table *top_level_consts, 
                                                 Scheme_Hash_Table *inline_variants,
                                                 Scheme_Hash_Table *top_level_table,
                                                 Scheme_Object **runstack, int rs_delta,
                                                 Scheme_Object **symbols, Scheme_Hash_Table *symbol_table,
                                                 Scheme_Object **_name,
                                                 int fuel);
int scheme_is_simple_make_struct_type_property(Scheme_Object *app, int vals, int flags,
                                               int *_has_guard,
                                               Scheme_Hash_Table *top_level_consts, 
                                               Scheme_Hash_Table *inline_variants,
                                               Scheme_Hash_Table *top_level_table,
                                               Scheme_Object **runstack, int rs_delta,
                                               Scheme_Object **symbols, Scheme_Hash_Table *symbol_table,
                                               int fuel);
#define CHECK_STRUCT_TYPE_RESOLVED         0x1
#define CHECK_STRUCT_TYPE_ALWAYS_SUCCEED   0x2
#define CHECK_STRUCT_TYPE_DELAY_AUTO_CHECK 0x4

Scheme_Object *scheme_intern_struct_proc_shape(int shape);
intptr_t scheme_get_struct_proc_shape(int k, Simple_Struct_Type_Info *sinfo);
Scheme_Object *scheme_make_struct_proc_shape(intptr_t k, Scheme_Object *identity);
#define STRUCT_PROC_SHAPE_STRUCT  0
#define STRUCT_PROC_SHAPE_CONSTR  1
#define STRUCT_PROC_SHAPE_PRED    2
#define STRUCT_PROC_SHAPE_GETTER  3
#define STRUCT_PROC_SHAPE_SETTER  4
#define STRUCT_PROC_SHAPE_OTHER   5
#define STRUCT_PROC_SHAPE_MASK    0xF
#define STRUCT_PROC_SHAPE_AUTHENTIC 0x10
#define STRUCT_PROC_SHAPE_SHIFT   5

typedef struct Scheme_Struct_Proc_Shape {
  Scheme_Object so;
  intptr_t mode;
  Scheme_Object *identity; /* sequence of pairs that identity the struct type */
} Scheme_Struct_Proc_Shape;
#define SCHEME_PROC_SHAPE_MODE(obj)     ((Scheme_Struct_Proc_Shape *)obj)->mode
#define SCHEME_PROC_SHAPE_IDENTITY(obj) ((Scheme_Struct_Proc_Shape *)obj)->identity

Scheme_Object *scheme_intern_struct_prop_proc_shape(int shape);
intptr_t scheme_get_struct_property_proc_shape(int k, int has_guard);
Scheme_Object *scheme_make_struct_property_proc_shape(intptr_t k);
#define STRUCT_PROP_PROC_SHAPE_PROP          0
#define STRUCT_PROP_PROC_SHAPE_GUARDED_PROP  1
#define STRUCT_PROP_PROC_SHAPE_PRED          2
#define STRUCT_PROP_PROC_SHAPE_GETTER        3
#define SCHEME_PROP_PROC_SHAPE_MODE(obj) ((Scheme_Small_Object *)obj)->u.int_val

Scheme_Object *scheme_get_or_check_procedure_shape(Scheme_Object *e, Scheme_Object *expected);
int scheme_check_structure_shape(Scheme_Object *e, Scheme_Object *expected);
int scheme_decode_struct_shape(Scheme_Object *shape, intptr_t *_v);
int scheme_check_structure_property_shape(Scheme_Object *e, Scheme_Object *expected);
int scheme_decode_struct_prop_shape(Scheme_Object *shape, intptr_t *_v);
int scheme_closure_preserves_marks(Scheme_Object *p);
int scheme_native_closure_preserves_marks(Scheme_Object *p);
int scheme_native_closure_is_single_result(Scheme_Object *rator);

int scheme_get_eval_type(Scheme_Object *obj);

Scheme_Object *scheme_make_application(Scheme_Object *v, Optimize_Info *info);
Scheme_Object *scheme_try_apply(Scheme_Object *f, Scheme_Object *args, Optimize_Info *info);
int scheme_is_foldable_prim(Scheme_Object *f);

Scheme_Object *scheme_get_stop_expander(void);

void scheme_define_parse(Scheme_Object *form,
			 Scheme_Object **vars, Scheme_Object **val,
			 int defmacro,
			 Scheme_Comp_Env *env,
                         int no_toplevel_check);

void scheme_shadow(Scheme_Env *env, Scheme_Object *n, Scheme_Object *val, int as_var);
void scheme_binding_names_from_module(Scheme_Env *menv);
void scheme_install_binding_names(Scheme_Object *binding_namess, Scheme_Env *env);
Scheme_Hash_Table *scheme_get_binding_names_table(Scheme_Env *env);

int scheme_prefix_depth(Resolve_Prefix *rp);
Scheme_Object **scheme_push_prefix(Scheme_Env *genv, int already_linked, Resolve_Prefix *rp,
				   Scheme_Object *src_modix, Scheme_Object *now_modix,
				   int src_phase, int now_phase,
                                   Scheme_Env *dummy_env, Scheme_Object *insp);
void scheme_pop_prefix(Scheme_Object **rs);
Scheme_Object *scheme_suspend_prefix(Scheme_Object **rs);
Scheme_Object **scheme_resume_prefix(Scheme_Object *v);

Scheme_Object *scheme_eval_clone(Scheme_Object *expr);
Resolve_Prefix *scheme_prefix_eval_clone(Resolve_Prefix *rp);
Scheme_Object *scheme_module_eval_clone(Scheme_Object *data);
Scheme_Object *scheme_syntaxes_eval_clone(Scheme_Object *form);

Scheme_Object *scheme_make_environment_dummy(Scheme_Comp_Env *env);
Scheme_Env *scheme_environment_from_dummy(Scheme_Object *dummy);

void scheme_validate_code(Mz_CPort *port, Scheme_Object *code,
                          int depth,
			  int num_toplevels, int num_stxes, int num_lifts, void *tl_use_map,
                          Scheme_Object **toplevels,
                          int code_vec);

typedef mzshort **Validate_TLS;
struct Validate_Clearing;

void scheme_validate_closure(Mz_CPort *port, Scheme_Object *expr, 
                             char *closure_stack, Validate_TLS tls,
                             int num_toplevels, int num_stxes, int num_lifts, void *tl_use_map,
                             mzshort *tl_state, mzshort tl_timestamp,
                             int self_pos_in_closure, Scheme_Hash_Tree *procs,
                             Scheme_Hash_Table **_st_ht);

#define TRACK_ILL_FORMED_CATCH_LINES 1
#if TRACK_ILL_FORMED_CATCH_LINES
void scheme_ill_formed(Mz_CPort *port, const char *file, int line);
# define scheme_ill_formed_code(port) scheme_ill_formed(port, __FILE__, __LINE__)
#else
void scheme_ill_formed(Mz_CPort *port);
# define scheme_ill_formed_code(port) scheme_ill_formed(port)
#endif

Scheme_Object *scheme_check_name_property(Scheme_Object *stx, Scheme_Object *current_name);

Scheme_Object *scheme_make_lifted_defn(Scheme_Object *sys_wraps, Scheme_Object **_id, Scheme_Object *expr, Scheme_Comp_Env *env);

typedef struct Scheme_Marshal_Tables {
  MZTAG_IF_REQUIRED  
  int pass, print_now;
  Scheme_Hash_Table *symtab;
  Scheme_Hash_Table *st_refs;
  Scheme_Object *st_ref_stack;
  Scheme_Hash_Table *reachable_scopes; /* filled on -1 pass */
  Scheme_Object *reachable_scope_stack; /* used on -1 pass */
  Scheme_Hash_Table *pending_reachable_ids; /* use on -1 pass */
  Scheme_Hash_Table *conditionally_reachable_scopes; /* filled/used on -1 pass */
  Scheme_Hash_Table *intern_map;  /* filled on first pass */
  Scheme_Hash_Table *identity_map; /* filled on first pass */
  Scheme_Hash_Table *top_map;     /* used on every pass */
  Scheme_Hash_Table *key_map;     /* set after first pass, used on later passes */
  Scheme_Hash_Table *delay_map;   /* set during first pass, used on later passes */
  Scheme_Hash_Table *rn_saved;    /* maps each original object to generated marshaling */
  Scheme_Object **cdata_map;      /* for delay-load wrappers */
  int cdata_counter;              /* used with cdata_map */
  intptr_t *shared_offsets;      /* set in second pass */
  Scheme_Hash_Table *path_cache; /* cache for path-to-relative resolution */
  intptr_t sorted_keys_count;
  intptr_t inspector_counter;    /* for deterministic symbol allocation */
  Scheme_Object **sorted_keys;
} Scheme_Marshal_Tables;

void scheme_marshal_using_key(Scheme_Marshal_Tables *mt, Scheme_Object *key);
Scheme_Object *scheme_marshal_lookup(Scheme_Marshal_Tables *mt, Scheme_Object *a);
Scheme_Object *scheme_marshal_wrap_set(Scheme_Marshal_Tables *mt, Scheme_Object *a, Scheme_Object *v);
void scheme_marshal_push_refs(Scheme_Marshal_Tables *mt);
void scheme_marshal_pop_refs(Scheme_Marshal_Tables *mt, int keep);

typedef struct Scheme_Unmarshal_Tables {
  MZTAG_IF_REQUIRED
  Scheme_Hash_Table *rns;
  Scheme_Hash_Table *current_rns; /* in-progress unmarshal, commit to `rns` at end */
  Scheme_Hash_Table *multi_scope_pairs; /* records conversions */
  Scheme_Hash_Table *current_multi_scope_pairs; /* commit to `multi_scope_pairs` at end */
  struct CPort *rp;
  char *decoded;
  mzlonglong bytecode_hash;
} Scheme_Unmarshal_Tables;

Scheme_Object *scheme_unmarshal_wrap_get(Scheme_Unmarshal_Tables *ut, 
                                         Scheme_Object *wraps_key, 
                                         int *_decoded);
void scheme_unmarshal_wrap_set(Scheme_Unmarshal_Tables *ut, 
                               Scheme_Object *wraps_key, 
                               Scheme_Object *v);

Scheme_Object *scheme_make_marshal_shared(Scheme_Object *v);

int scheme_is_rename_transformer(Scheme_Object *o);
int scheme_is_binding_rename_transformer(Scheme_Object *o);
Scheme_Object *scheme_rename_transformer_id(Scheme_Object *o, Scheme_Comp_Env *env);
int scheme_is_set_transformer(Scheme_Object *o);
Scheme_Object *scheme_set_transformer_proc(Scheme_Object *o);

int scheme_is_expansion_context_symbol(Scheme_Object *v);
int scheme_expansion_contexts_include(Scheme_Object *o, Scheme_Object *ctx);
Scheme_Object *scheme_frame_to_expansion_context_symbol(int flags);

Scheme_Object *scheme_top_level_require_execute(Scheme_Object *data);
Scheme_Object *scheme_case_lambda_execute(Scheme_Object *expr);

Scheme_Object *scheme_module_jit(Scheme_Object *data);
Scheme_Object *scheme_top_level_require_jit(Scheme_Object *data);
Scheme_Object *scheme_case_lambda_jit(Scheme_Object *expr);

/*========================================================================*/
/*                         namespaces and modules                         */
/*========================================================================*/

typedef struct Scheme_Module_Registry {
  Scheme_Object so; /* scheme_module_registry_type */
  Scheme_Hash_Table *loaded; /* symbol -> module ; loaded modules,
                                shared with modules in same space */
  Scheme_Hash_Table *exports; /* symbol -> module-exports */
} Scheme_Module_Registry;

struct Scheme_Env {
  Scheme_Object so; /* scheme_namespace_type */

  signed char disallow_unbound, rename_set_ready;

  struct Scheme_Module *module; /* NULL => top-level */

  Scheme_Module_Registry *module_registry;
  Scheme_Module_Registry *module_pre_registry; /* for expanding submodules */
  Scheme_Object *guard_insp; /* instantiation-time inspector, for granting
                                protected access */
  Scheme_Object *access_insp; /* for gaining protected access */

  Scheme_Object *stx_context; /* encapsulates scopes, shifts, etc. */
  Scheme_Object *tmp_bind_scope; /* for compiling top-level definitions */

  Scheme_Bucket_Table *syntax;
  struct Scheme_Env *exp_env;
  struct Scheme_Env *template_env;
  struct Scheme_Env *label_env;
  struct Scheme_Env *instance_env; /* shortcut to env where module is instantiated */
  struct Scheme_Env *reader_env; /* namespace to use for #reader or #lang */

  Scheme_Hash_Table *shadowed_syntax; /* top level only */

  Scheme_Object *lift_key; /* for `syntax-local-lift-context' */

  /* Per-instance: */
  intptr_t phase, mod_phase;
  Scheme_Object *link_midx;
  Scheme_Object *require_names, *et_require_names, *tt_require_names, *dt_require_names; /* resolved */
  Scheme_Hash_Table *other_require_names;
  char *running; /* array of size `num_phases' if `module' and `mod_phase==0' */
  char attached, ran;
  Scheme_Object *did_starts;
  Scheme_Object *available_next[2];

  Scheme_Bucket_Table *toplevel;
  Scheme_Object *modchain; /* Vector of:
			       1. symbol -> env ; running modules,
			           shared with instances in same phase
			       2. modchain for next phase (or #f)
                               3. modchain for previous phase (or #f) */

  Scheme_Hash_Table *modvars; /* for scheme_module_variable_type hashing */

  Scheme_Object *weak_self_link; /* for Scheme_Bucket_With_Home */

  /* The `binding_names` table can be an immutable or mutable hash table: */
  Scheme_Object *binding_names; /* maps symbols to identifiers */
  short binding_names_need_shift; /* => binding names are from module, and need a shift */
  short interactive_bindings; /* => module namespace is interactive and shadowing is needed */

  int id_counter;
};

/* A module access path (or "idx") is a pair: sexp * symbol-or-#f
   The symbol is the resolved module name, or #f if it's not
   yet resolved. */

/* A Scheme_Module corresponds to a module declaration. A module
   instantiation is reprsented by a Scheme_Env */

typedef struct Scheme_Module_Export_Info {
  MZTAG_IF_REQUIRED
  char *provide_protects;            /* 1 => protected, 0 => not */
  Scheme_Object **indirect_provides; /* symbols (internal names) */
  int num_indirect_provides;

  /* Only if needed to reconstruct the renaming: */
  Scheme_Object **indirect_syntax_provides; /* symbols (internal names) */
  int num_indirect_syntax_provides;

  Scheme_Hash_Table *accessible; /* (symbol -> ...) */
} Scheme_Module_Export_Info;

typedef struct Scheme_Module
{
  Scheme_Object so; /* scheme_module_type */
  short predefined;

  Scheme_Object *phaseless; /* NULL, #t, or shared `toplevel' hash table */

  Scheme_Object *code_key;

  Scheme_Object *modname;
  Scheme_Object *modsrc;

  Scheme_Object *et_requires;  /* list of symbol-or-module-path-index */
  Scheme_Object *requires;     /* list of symbol-or-module-path-index */
  Scheme_Object *tt_requires;  /* list of symbol-or-module-path-index */
  Scheme_Object *dt_requires;  /* list of symbol-or-module-path-index */
  Scheme_Hash_Table *other_requires;  /* phase to list of symbol-or-module-path-index */

  Scheme_Invoke_Proc prim_body;
  Scheme_Invoke_Proc prim_et_body;

  Scheme_Object **bodies; /* array `num_phases' long */

  struct Scheme_Module_Exports *me;

  int num_phases;
  Scheme_Module_Export_Info **exp_infos; /* array `num_phases' long */

  Scheme_Object *self_modidx;

  /* These tables are unshifted, so they are relative to self_modidx
     and must be shifted as they are installed into an environment.
     The tables can be immutable or immutable hash tables, or they can
     be a vectors that should be converted to an immutable hash
     table. */
  Scheme_Object *binding_names; /* maps symbols to identifiers */
  Scheme_Object *et_binding_names; /* maps symbols to identifiers */
  Scheme_Object *other_binding_names; /* maps phases to maps symbols to identifiers */

  Scheme_Object *insp; /* declaration-time inspector, for module instantiation
                          and enabling access to protected imports */

  Scheme_Object *lang_info; /* NULL or vector */

  Scheme_Object *hints; /* set by expansion; moved to properties */
  Scheme_Object *ii_src; /* set by compile, temporary */
  Comp_Prefix *comp_prefix; /* set by body compile, temporary */
  void **super_bxs_info; /* set by expansion; temporary */
  Scheme_Object **sub_iidx_ptrs; /* set by expansion; temporary */

  int max_let_depth;
  Resolve_Prefix *prefix;

  Scheme_Object *dummy; /* for accessing the environment */

  Scheme_Env *primitive;

  Scheme_Object *rn_stx; /* NULL, #t, a stx for a rename, a vector of stxes, or a pair to delay shifts */

  Scheme_Object *submodule_path; /* path to this module relative to enclosing top-level module */
  Scheme_Object *pre_submodules, *post_submodules; /* list of modules (when compiled or loaded as a group) */
  Scheme_Object *pre_submodule_names; /* list of symbols (in expand mode) */
  Scheme_Object *supermodule; /* supermodule for which this is in {pre,post}_submodules */
  Scheme_Object *submodule_ancestry; /* set by compile/expand, temporary */
} Scheme_Module;

typedef struct Scheme_Module_Phase_Exports
{
  Scheme_Object so;

  Scheme_Object *phase_index;

  Scheme_Object *src_modidx;  /* same as in enclosing Scheme_Module_Exports */

  Scheme_Object **provides;          /* symbols (extenal names) */
  Scheme_Object **provide_srcs;      /* module access paths, #f for self */
  Scheme_Object **provide_src_names; /* symbols (original internal names) */
  Scheme_Object **provide_nominal_srcs; /* import source if re-exported; NULL or array of lists */
  int *provide_src_phases;          /* NULL, or src phase for for-syntax import */
  int num_provides;
  int num_var_provides;              /* non-syntax listed first in provides */

  Scheme_Hash_Table *ht;             /* maps external names to array indices; created lazily */
} Scheme_Module_Phase_Exports;

typedef struct Scheme_Module_Exports
{
  /* Scheme_Module_Exports is separate from Scheme_Module
     so that we can create a global table mapping export
     keys to exports. This mapping is used to lazily 
     unmarshal syntax-object context. */
  MZTAG_IF_REQUIRED

  /* Most common phases: */
  Scheme_Module_Phase_Exports *rt; /* run time? phase 0*/
  Scheme_Module_Phase_Exports *et; /* expansion time? phase 1 */
  Scheme_Module_Phase_Exports *dt; /* */
                            
  /* All others: */
  Scheme_Hash_Table *other_phases;

  Scheme_Object *src_modidx;  /* the one used in marshalled syntax */
  Scheme_Object *modsrc; /* module source as loaded */
} Scheme_Module_Exports;

typedef struct Scheme_Modidx {
  Scheme_Object so; /* scheme_module_index_type */

  Scheme_Object *path;
  Scheme_Object *base;
  Scheme_Object *resolved;
  Scheme_Object *shift_cache; /* vector */
  struct Scheme_Modidx *cache_next;
} Scheme_Modidx;

typedef struct Module_Variable {
  Scheme_Inclhash_Object iso; /* see SCHEME_MODVAR_... flags */
  Scheme_Object *modidx;
  Scheme_Object *sym;
  Scheme_Object *insp; /* for checking protected/unexported access */
  Scheme_Object *shape; /* NULL or a symbol encoding "type" information */
  int pos, mod_phase;
} Module_Variable;

/* See SCHEME_TOPLEVEL_...: */
#define SCHEME_MODVAR_CONST 0x1
#define SCHEME_MODVAR_FIXED 0x2

#define SCHEME_MODVAR_FLAGS(pr) MZ_OPT_HASH_KEY(&((Module_Variable *)pr)->iso)

#define SCHEME_VARREF_FLAGS(pr) MZ_OPT_HASH_KEY(&((Scheme_Simple_Object *)pr)->iso)

void scheme_add_global_keyword(const char *name, Scheme_Object *v, Scheme_Env *env);
void scheme_add_global_keyword_symbol(Scheme_Object *name, Scheme_Object *v, Scheme_Env *env);
void scheme_add_global_constant(const char *name, Scheme_Object *v, Scheme_Env *env);
void scheme_add_global_constant_symbol(Scheme_Object *name, Scheme_Object *v, Scheme_Env *env);

#define GLOBAL_FOLDING_PRIM(name, func, a1, a2, a3, env)      scheme_add_global_constant(name, scheme_make_folding_prim(func, name, a1, a2, a3), env)
#define GLOBAL_IMMED_PRIM(name, func, a1, a2, env)            scheme_add_global_constant(name, scheme_make_immed_prim(func, name, a1, a2), env)
#define GLOBAL_PARAMETER(name, func, constant, env)           scheme_add_global_constant(name, scheme_register_parameter(func, name, constant), env)
#define GLOBAL_PRIM_W_ARITY(name, func, a1, a2, env)          scheme_add_global_constant(name, scheme_make_prim_w_arity(func, name, a1, a2), env)
#define GLOBAL_PRIM_W_ARITY2(name, func, a1, a2, a3, a4, env) scheme_add_global_constant(name, scheme_make_prim_w_arity2(func, name, a1, a2, a3, a4), env)
#define GLOBAL_NONCM_PRIM(name, func, a1, a2, env)            scheme_add_global_constant(name, scheme_make_noncm_prim(func, name, a1, a2), env)

#define GLOBAL_FOLDING_PRIM_UNARY_INLINED(name, func, a1, a2, a3, env)      do {\
  Scheme_Object *p; \
  p = scheme_make_folding_prim(func, name, a1, a2, a3); \
  SCHEME_PRIM_PROC_FLAGS(p) |= scheme_intern_prim_opt_flags(SCHEME_PRIM_IS_UNARY_INLINED); \
  scheme_add_global_constant(name, p, env); \
} while(0)


Scheme_Object *scheme_global_binding(Scheme_Object *id, Scheme_Env *env, int for_top_level);
Scheme_Object *scheme_future_global_binding(Scheme_Object *id, Scheme_Env *env);

Scheme_Object *scheme_sys_wraps(Scheme_Comp_Env *env);
Scheme_Object *scheme_sys_wraps_phase(Scheme_Object *phase);

THREAD_LOCAL_DECL(extern Scheme_Bucket_Table *scheme_module_code_cache);
Scheme_Object *scheme_module_execute(Scheme_Object *data, Scheme_Env *genv);

Scheme_Env *scheme_new_module_env(Scheme_Env *env, Scheme_Module *m,
                                  int new_exp_module_tree, int new_pre_registry);
int scheme_is_module_env(Scheme_Comp_Env *env);

Scheme_Env *scheme_make_env_like(Scheme_Env *base);

Scheme_Object *scheme_module_resolve(Scheme_Object *modidx, int load_it);
Scheme_Env *scheme_module_access(Scheme_Object *modname, Scheme_Env *env, intptr_t rev_mod_phase);

int scheme_module_export_position(Scheme_Object *modname, Scheme_Env *env, Scheme_Object *varname);

Scheme_Module_Exports *scheme_make_module_exports();

Scheme_Object *scheme_check_accessible_in_module_instance(Scheme_Env *env,
                                                          Scheme_Object *symbol, Scheme_Object *stx, 
                                                          Scheme_Object *current_insp, Scheme_Object *binding_insp,
                                                          int position, int want_pos,
                                                          int *_protected, int *_unexported, 
                                                          Scheme_Env *from_env, int *_would_complain,
                                                          Scheme_Object **_is_constant);
Scheme_Object *scheme_check_accessible_in_module_name(Scheme_Object *modidx, intptr_t mod_phase, Scheme_Env *env,
                                                      Scheme_Object *symbol, int position,
                                                      Scheme_Object *current_insp, Scheme_Object *binding_insp,
                                                      Scheme_Object **_is_constant);
Scheme_Object *scheme_module_syntax(Scheme_Object *modname, Scheme_Env *env, Scheme_Object *name, int mod_phase);

Scheme_Object *scheme_modidx_shift(Scheme_Object *modidx,
				   Scheme_Object *shift_from_modidx,
				   Scheme_Object *shift_to_modidx);

Scheme_Object *scheme_modidx_submodule(Scheme_Object *modidx);
Scheme_Object *scheme_get_submodule_empty_self_modidx(Scheme_Object *submodule_path, int can_cache);

#define SCHEME_RMPP(o) (SAME_TYPE(SCHEME_TYPE((o)), scheme_resolved_module_path_type))
#define SCHEME_MODNAMEP(obj)  (SAME_TYPE(SCHEME_TYPE(obj), scheme_resolved_module_path_type))

Scheme_Object *scheme_intern_resolved_module_path(Scheme_Object *o);
Scheme_Object *scheme_resolved_module_path_value(Scheme_Object *rmp);
int scheme_resolved_module_path_value_matches(Scheme_Object *rmp, Scheme_Object *o);

Scheme_Object *scheme_resolved_module_path_to_modidx(Scheme_Object *rmp);

Scheme_Object *scheme_hash_module_variable(Scheme_Env *env, Scheme_Object *modidx, 
					   Scheme_Object *stxsym, Scheme_Object *insp,
					   int pos, intptr_t mod_phase, int is_constant,
                                           Scheme_Object *shape);


Scheme_Env *scheme_get_kernel_env();
int scheme_is_kernel_env();
Scheme_Env *scheme_get_unsafe_env();
Scheme_Env *scheme_get_flfxnum_env();
Scheme_Env *scheme_get_extfl_env();
Scheme_Env *scheme_get_futures_env();
Scheme_Env *scheme_get_foreign_env();

void scheme_install_initial_module_set(Scheme_Env *env);
Scheme_Bucket_Table *scheme_clone_toplevel(Scheme_Bucket_Table *ht, Scheme_Env *home);

Scheme_Env *scheme_copy_module_env(Scheme_Env *menv, Scheme_Env *ns, Scheme_Object *modchain, int clone);

Scheme_Module *scheme_extract_compiled_module(Scheme_Object *o);

int scheme_is_kernel_modname(Scheme_Object *modname);
int scheme_is_unsafe_modname(Scheme_Object *modname);
int scheme_is_flfxnum_modname(Scheme_Object *modname);
int scheme_is_extfl_modname(Scheme_Object *modname);
int scheme_is_futures_modname(Scheme_Object *modname);
int scheme_is_foreign_modname(Scheme_Object *modname);

void scheme_clear_modidx_cache(void);
void scheme_clear_shift_cache(void);
void scheme_clear_prompt_cache(void);

Scheme_Object *scheme_module_imported_list(Scheme_Env *genv, Scheme_Object *bindings, Scheme_Object *modpath,
                                           Scheme_Object *mode);
Scheme_Object *scheme_module_exported_list(Scheme_Object *modpath, Scheme_Env *genv);

void scheme_prepare_compile_env(Scheme_Env *env);

Scheme_Object *scheme_module_to_namespace(Scheme_Object *name, Scheme_Env *env);
void scheme_prep_namespace_rename(Scheme_Env *menv);

Scheme_Object *scheme_string_to_submodule_path(char *_s, intptr_t len);
char *scheme_submodule_path_to_string(Scheme_Object *p, intptr_t *_len);

Scheme_Object *scheme_annotate_existing_submodules(Scheme_Object *orig_fm, int incl_star);

Scheme_Object *scheme_get_modsrc(Scheme_Module *m);

Scheme_Object *scheme_prune_bindings_table(Scheme_Object *bindings, Scheme_Object *rn_stx, Scheme_Object *phase);

/*========================================================================*/
/*                         errors and exceptions                          */
/*========================================================================*/

#define NOT_SUPPORTED_STR "unsupported on this platform"

int scheme_last_error_is_racket(int errid);

void scheme_read_err(Scheme_Object *port,
		     Scheme_Object *stxsrc,
		     intptr_t line, intptr_t column, intptr_t pos, intptr_t span,
		     int is_eof, Scheme_Object *indentation,
		     const char *detail, ...);
Scheme_Object *scheme_numr_err(Scheme_Object *complain,
                               Scheme_Object *stxsrc,
                               intptr_t line, intptr_t column, intptr_t pos, intptr_t span,
                               Scheme_Object *indentation,
                               const char *detail, ...);
char *scheme_extract_indentation_suggestions(Scheme_Object *indentation);

void scheme_wrong_syntax(const char *where,
			 Scheme_Object *local_form,
			 Scheme_Object *form,
			 const char *detail, ...);
void scheme_unbound_syntax(const char *where,
                           Scheme_Object *local_form,
                           Scheme_Object *form,
                           const char *detail, ...);
void scheme_wrong_syntax_with_more_sources(const char *where,
                                           Scheme_Object *detail_form,
                                           Scheme_Object *form,
                                           Scheme_Object *extra_sources,
                                           const char *detail, ...);
extern const char *scheme_compile_stx_string;
extern const char *scheme_expand_stx_string;
extern const char *scheme_application_stx_string;
extern const char *scheme_set_stx_string;
extern const char *scheme_var_ref_string;
extern const char *scheme_begin_stx_string;

void scheme_wrong_rator(Scheme_Object *rator, int argc, Scheme_Object **argv);

void scheme_wrong_chaperoned(const char *who, const char *what, Scheme_Object *orig, Scheme_Object *naya);

void scheme_rktio_error(const char *name, const char *what);

void scheme_non_fixnum_result(const char *name, Scheme_Object *o);

void scheme_raise_out_of_memory(const char *where, const char *msg, ...);

char *scheme_make_srcloc_string(Scheme_Object *stx, intptr_t *len);

uintptr_t scheme_get_max_symbol_length();
void scheme_ensure_max_symbol_length(uintptr_t);

char *scheme_make_arity_expect_string(const char *map_name,
                                      Scheme_Object *proc,
				      int argc, Scheme_Object **argv,
				      intptr_t *len);

intptr_t scheme_extract_index(const char *name, int pos, int argc,
			  Scheme_Object **argv, intptr_t top, int false_ok);

void scheme_get_substring_indices(const char *name, Scheme_Object *str,
				  int argc, Scheme_Object **argv,
				  int spos, int fpos, intptr_t *_start, intptr_t *_finish);
void scheme_do_get_substring_indices(const char *name, Scheme_Object *str,
                                     int argc, Scheme_Object **argv,
                                     int spos, int fpos, intptr_t *_start, intptr_t *_finish, intptr_t len);

void scheme_out_of_range(const char *name, const char *what, const char *which,
                         Scheme_Object *i, Scheme_Object *s,
                         intptr_t start, intptr_t len);

const char *scheme_number_suffix(int);

const char *scheme_hostname_error(int err);

#define IMPROPER_LIST_FORM "illegal use of `.'"

int scheme_byte_string_has_null(Scheme_Object *o);
int scheme_any_string_has_null(Scheme_Object *o);
#define CHAR_STRING_W_NO_NULLS "string-no-nuls?"

Scheme_Object *scheme_do_exit(int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_make_arity(mzshort minc, mzshort maxc);
Scheme_Object *scheme_arity(Scheme_Object *p);

typedef struct {
  MZTAG_IF_REQUIRED
  Scheme_Object *syms[5];
  int count;
  intptr_t phase;
  Scheme_Hash_Table *ht;
} DupCheckRecord;

void scheme_begin_dup_symbol_check(DupCheckRecord *r, Scheme_Comp_Env *e);
void scheme_dup_symbol_check(DupCheckRecord *r, const char *where,
			     Scheme_Object *symbol, char *what,
			     Scheme_Object *form);

Scheme_Object *scheme_special_comment_value(Scheme_Object *o);

Scheme_Object *scheme_get_stack_trace(Scheme_Object *mark_set);

XFORM_NONGCING int scheme_fast_check_arity(Scheme_Object *v, int a);
Scheme_Object *scheme_get_or_check_arity(Scheme_Object *p, intptr_t a);
int scheme_native_arity_check(Scheme_Object *closure, int argc);
Scheme_Object *scheme_get_native_arity(Scheme_Object *closure, int mode);

struct Scheme_Logger {
  Scheme_Object so;
  Scheme_Object *name;
  Scheme_Logger *parent;
  int want_level;
  Scheme_Object *want_name_level_cache; /* vector */
  Scheme_Object **root_timestamp;
  intptr_t local_timestamp; /* determines when want_level is up-to-date */
  Scheme_Object *syslog_level; /* (list* <level-int> <name-sym> ... <level-int>) */
  Scheme_Object *stderr_level;
  Scheme_Object *propagate_level; /* can be NULL */
  Scheme_Object *readers; /* list of (cons (make-weak-box <reader>) <sema>) */
};

typedef struct Scheme_Log_Reader {
  Scheme_Object so;
  Scheme_Object *level; /* (list* <level-int> <name-sym> ... <level-int>) */
  Scheme_Object *sema;
  Scheme_Object *head, *tail;
} Scheme_Log_Reader;

Scheme_Logger *scheme_make_logger(Scheme_Logger *parent, Scheme_Object *name);

char *scheme_optimize_context_to_string(Scheme_Object *context);

void scheme_write_proc_context(Scheme_Object *port, int print_width,
                               Scheme_Object *name, 
                               Scheme_Object *src, Scheme_Object *line, 
                               Scheme_Object *col, Scheme_Object *pos,
                               int generated);

#ifdef MZ_USE_MZRT
void scheme_init_glib_log_queue(void);
void scheme_check_glib_log_messages(void);
#endif

/*========================================================================*/
/*                         filesystem utilities                           */
/*========================================================================*/

#ifdef USE_TRANSITIONAL_64_FILE_OPS
# define BIG_OFF_T_IZE(n) n ## 64
# define mz_off_t off64_t
#else
# define BIG_OFF_T_IZE(n) n
# if defined(DOS_FILE_SYSTEM)
#  define mz_off_t mzlonglong
# else
#  define mz_off_t off_t
# endif
#endif

int scheme_is_relative_path(const char *s, intptr_t len, int kind);
int scheme_is_complete_path(const char *s, intptr_t len, int kind);

#ifdef DOS_FILE_SYSTEM
__declspec(dllexport) wchar_t *scheme_get_dll_path(wchar_t *s);
__declspec(dllexport) void scheme_set_dll_path(wchar_t *p);
#endif

Scheme_Object *scheme_get_file_directory(const char *filename);

char *scheme_normal_path_seps(char *s, int *_len, int delta);

int scheme_is_regular_file(char *filename);

void scheme_do_format(const char *procname, Scheme_Object *port,
		      const mzchar *format, int flen,
		      int fpos, int offset, int argc, Scheme_Object **argv);

Scheme_Object *scheme_load_with_clrd(int argc, Scheme_Object *argv[], char *who, int handler_param);

Scheme_Object *scheme_default_load_extension(int argc, Scheme_Object **argv);

Scheme_Object *scheme_remove_current_directory_prefix(Scheme_Object *fn);

#ifdef DOS_FILE_SYSTEM
int scheme_is_special_filename(const char *_f, int not_nul);
# define NUM_SPECIAL_FILE_KINDS 30
#endif

char *scheme_get_exec_path(void);
Scheme_Object *scheme_get_run_cmd(void);

Scheme_Object *scheme_get_fd_identity(Scheme_Object *port, intptr_t fd, char *path, int noerr);

Scheme_Object *scheme_extract_relative_to(Scheme_Object *obj, Scheme_Object *dir, Scheme_Hash_Table *cache);

Scheme_Object *scheme_find_links_path(int argc, Scheme_Object *argv[]);

#ifdef DOS_FILE_SYSTEM
wchar_t *scheme_path_to_wide_path(const char *who, const char *p);
#endif

/*========================================================================*/
/*                               ports                                    */
/*========================================================================*/

THREAD_LOCAL_DECL(extern int scheme_active_but_sleeping);

struct rktio_fd_t;

typedef struct Scheme_Indexed_String {
  MZTAG_IF_REQUIRED
  char *string;
  int size;
  int index;
  union {
    int hot; /* output port */
    int pos; /* input port */
  } u;
} Scheme_Indexed_String;

typedef struct Scheme_Pipe {
  MZTAG_IF_REQUIRED
  unsigned char *buf;
  intptr_t buflen, bufmax;
  intptr_t bufmaxextra; /* due to peeks, bufmax can effectively grow */
  intptr_t bufstart, bufend;
  int eof;
  Scheme_Object *wakeup_on_read;
  Scheme_Object *wakeup_on_write;
} Scheme_Pipe;

extern Scheme_Object *scheme_string_input_port_type;
extern Scheme_Object *scheme_string_output_port_type;
extern Scheme_Object *scheme_user_input_port_type;
extern Scheme_Object *scheme_user_output_port_type;
extern Scheme_Object *scheme_pipe_read_port_type;
extern Scheme_Object *scheme_pipe_write_port_type;
extern Scheme_Object *scheme_null_output_port_type;
extern Scheme_Object *scheme_tcp_input_port_type;
extern Scheme_Object *scheme_tcp_output_port_type;

THREAD_LOCAL_DECL(extern int scheme_force_port_closed);

void scheme_flush_orig_outputs(void);
void scheme_flush_if_output_fds(Scheme_Object *o);
Scheme_Object *scheme_file_stream_port_p(int, Scheme_Object *[]);
Scheme_Object *scheme_terminal_port_p(int, Scheme_Object *[]);
Scheme_Object *scheme_do_open_input_file(char *name, int offset, int argc, Scheme_Object *argv[], 
                                         int internal, int for_module);
Scheme_Object *scheme_do_open_output_file(char *name, int offset, int argc, Scheme_Object *argv[], int and_read, 
                                          int internal);
Scheme_Object *scheme_file_position(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_file_position_star(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_file_truncate(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_file_buffer(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_file_identity(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_file_try_lock(int argc, Scheme_Object **argv);
Scheme_Object *scheme_file_unlock(int argc, Scheme_Object **argv);

void scheme_reserve_file_descriptor(void);
void scheme_release_file_descriptor(void);

int scheme_get_port_rktio_file_descriptor(Scheme_Object *p, struct rktio_fd_t **_fd);
Scheme_Object *scheme_make_rktio_fd_input_port(struct rktio_fd_t *rfd, Scheme_Object *name);
Scheme_Object *scheme_make_rktio_fd_output_port(struct rktio_fd_t *rfd, Scheme_Object *name, int read_too);

struct rktio_fd_t *scheme_get_port_rktio_socket(Scheme_Object *p);
void scheme_rktio_socket_to_input_port(struct rktio_fd_t *fd, Scheme_Object *name, int takeover,
                                       Scheme_Object **_inp);
void scheme_rktio_socket_to_output_port(struct rktio_fd_t *fd, Scheme_Object *name, int takeover,
                                        Scheme_Object **_outp);

void scheme_fs_change_properties(int *_supported, int *_scalable, int *_low_latency, int *_file_level);

THREAD_LOCAL_DECL(extern struct rktio_ltps_t *scheme_semaphore_fd_set);
THREAD_LOCAL_DECL(extern Scheme_Hash_Table *scheme_semaphore_fd_mapping);

intptr_t scheme_get_byte_string_or_ch_put(const char *who,
				      Scheme_Object *port,
				      char *buffer, intptr_t offset, intptr_t size,
				      int only_avail,
				      int peek, Scheme_Object *peek_skip,
				      Scheme_Object *unless_evt,
				      Scheme_Object *target_ch);

Scheme_Object *scheme_get_special(Scheme_Object *inport, Scheme_Object *stxsrc, intptr_t line, intptr_t col, intptr_t pos, int peek, 
				  Scheme_Hash_Table **for_read);
Scheme_Object *scheme_get_ready_read_special(Scheme_Object *port, Scheme_Object *stxsrc, Scheme_Hash_Table **ht);
void scheme_set_in_read_mark(Scheme_Object *stxsrc, Scheme_Hash_Table **ht);
Scheme_Object *scheme_get_special_proc(Scheme_Object *inport);
void scheme_bad_time_for_special(const char *name, Scheme_Object *port);
extern int scheme_special_ok;

int scheme_user_port_byte_probably_ready(Scheme_Input_Port *ip, Scheme_Schedule_Info *sinfo);
int scheme_user_port_write_probably_ready(Scheme_Output_Port *op, Scheme_Schedule_Info *sinfo);
int scheme_is_user_port(Scheme_Object *port);

int scheme_byte_ready_or_user_port_ready(Scheme_Object *p, Scheme_Schedule_Info *sinfo);

int scheme_pipe_char_count(Scheme_Object *p);
Scheme_Object *scheme_port_name(Scheme_Object *p);
intptr_t scheme_port_closed_p (Scheme_Object *port);

#define CURRENT_INPUT_PORT(config) scheme_get_param(config, MZCONFIG_INPUT_PORT)
#define CURRENT_OUTPUT_PORT(config) scheme_get_param(config, MZCONFIG_OUTPUT_PORT)
#define CHECK_PORT_CLOSED(who, kind, port, closed) if (closed) scheme_raise_exn(MZEXN_FAIL, "%s: " kind " port is closed", who);

#define MAX_UTF8_CHAR_BYTES 6

intptr_t scheme_redirect_write_bytes(Scheme_Output_Port *op,
                                     const char *str, intptr_t d, intptr_t len,
                                     int rarely_block, int enable_break);
int scheme_redirect_write_special (Scheme_Output_Port *op, Scheme_Object *v, int nonblock);
intptr_t scheme_redirect_get_or_peek_bytes(Scheme_Input_Port *orig_port,
                                           Scheme_Input_Port *port,
                                           char *buffer, intptr_t offset, intptr_t size,
                                           int nonblock,
                                           int peek, Scheme_Object *peek_skip,
                                           Scheme_Object *unless,
                                           Scheme_Schedule_Info *sinfo);

Scheme_Object *scheme_filesystem_change_evt(Scheme_Object *path, int flags, int report_errs);
void scheme_filesystem_change_evt_cancel(Scheme_Object *evt, void *ignored_data);

void scheme_init_fd_semaphores(void);
void scheme_release_fd_semaphores(void);

void scheme_check_fd_semaphores(void);
Scheme_Object *scheme_rktio_fd_to_semaphore(struct rktio_fd_t *fd, int mode);

struct rktio_envvars_t;
struct rktio_envvars_t *scheme_environment_variables_to_envvars(Scheme_Object *ev);

/*========================================================================*/
/*                         memory debugging                               */
/*========================================================================*/

#ifdef MEMORY_COUNTING_ON
extern intptr_t scheme_type_table_count;
extern intptr_t scheme_misc_count;

Scheme_Object *scheme_dump_memory_count(int c, Scheme_Object *a[]);

intptr_t scheme_count_closure(Scheme_Object **o, mzshort len, Scheme_Hash_Table *ht);

intptr_t scheme_count_envbox(Scheme_Object *root, Scheme_Hash_Table *ht);
intptr_t scheme_count_memory(Scheme_Object *root, Scheme_Hash_Table *ht);
void scheme_count_input_port(Scheme_Object *port, intptr_t *s, intptr_t *e, Scheme_Hash_Table *ht);
void scheme_count_output_port(Scheme_Object *port, intptr_t *s, intptr_t *e, Scheme_Hash_Table *ht);

void scheme_count_struct_info(Scheme_Object *o, intptr_t *s, intptr_t *e, Scheme_Hash_Table *ht);

#ifndef NO_OBJECT_SYSTEM
void scheme_count_object(Scheme_Object *o, intptr_t *s, intptr_t *e, Scheme_Hash_Table *ht);
void scheme_count_class(Scheme_Object *o, intptr_t *s, intptr_t *e, Scheme_Hash_Table *ht);
void scheme_count_class_data(Scheme_Object *o, intptr_t *s, intptr_t *e, Scheme_Hash_Table *ht);
void scheme_count_generic(Scheme_Object *o, intptr_t *s, intptr_t *e, Scheme_Hash_Table *ht);
#endif
#endif

/*========================================================================*/
/*                           miscellaneous                                */
/*========================================================================*/

Scheme_Object *scheme_checked_car(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_cdr(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_caar(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_cadr(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_cdar(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_cddr(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_length(Scheme_Object *v);
Scheme_Object *scheme_checked_list_tail(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_list_ref(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_mcar(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_mcdr(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_set_mcar (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_set_mcdr (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_vector_ref(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_vector_set(int argc, Scheme_Object **argv);
Scheme_Object *scheme_string_length(Scheme_Object *v);
Scheme_Object *scheme_string_eq_2(Scheme_Object *str1, Scheme_Object *str2);
Scheme_Object *scheme_checked_string_ref(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_string_set(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_byte_string_length(Scheme_Object *v);
Scheme_Object *scheme_byte_string_eq_2(Scheme_Object *str1, Scheme_Object *str2);
Scheme_Object *scheme_checked_byte_string_ref(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_byte_string_set(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_syntax_e(int argc, Scheme_Object **argv);
Scheme_Object *scheme_vector_length(Scheme_Object *v);
Scheme_Object *scheme_checked_flvector_ref(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_flvector_set(int argc, Scheme_Object **argv);
Scheme_Object *scheme_flvector_length(Scheme_Object *v);
Scheme_Object *scheme_checked_extflvector_ref(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_extflvector_set(int argc, Scheme_Object **argv);
Scheme_Object *scheme_extflvector_length(Scheme_Object *v);
Scheme_Vector *scheme_alloc_fxvector(intptr_t size);
Scheme_Object *scheme_checked_fxvector_ref(int argc, Scheme_Object **argv);
Scheme_Object *scheme_checked_fxvector_set(int argc, Scheme_Object **argv);
Scheme_Object *scheme_fxvector_length(Scheme_Object *v);
Scheme_Object *scheme_checked_real_part (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_imag_part (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_make_rectangular (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_flreal_part (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_flimag_part (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_make_flrectangular (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_procedure_arity_includes(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_char_to_integer (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_integer_to_char (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_checked_make_vector (int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_check_not_undefined (int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_check_assign_not_undefined (int argc, Scheme_Object *argv[]);

Scheme_Object *scheme_chaperone_vector_copy(Scheme_Object *obj);
Scheme_Object *scheme_chaperone_hash_table_copy(Scheme_Object *obj);

void scheme_bad_vec_index(char *name, Scheme_Object *i, 
                          const char *what, Scheme_Object *vec, 
                          intptr_t bottom, intptr_t len);

Scheme_Bucket_Table *scheme_make_weak_equal_table(void);
Scheme_Bucket_Table *scheme_make_weak_eqv_table(void);
Scheme_Bucket_Table *scheme_make_nonlock_equal_bucket_table(void);

int scheme_hash_table_equal_rec(Scheme_Hash_Table *t1, Scheme_Object *orig_t1,
                                Scheme_Hash_Table *t2, Scheme_Object *orig_t2,
                                void *eql);
int scheme_bucket_table_equal_rec(Scheme_Bucket_Table *t1, Scheme_Object *orig_t1,
                                  Scheme_Bucket_Table *t2, Scheme_Object *orig_t2,
                                  void *eql);
int scheme_hash_tree_equal_rec(Scheme_Hash_Tree *t1, Scheme_Object *orig_t1,
                               Scheme_Hash_Tree *t2, Scheme_Object *orig_t2,
                               void *eql);
Scheme_Object *scheme_hash_tree_copy(Scheme_Object *v);
Scheme_Hash_Tree *scheme_make_hash_tree_of_type(Scheme_Type stype);

Scheme_Hash_Tree *scheme_make_hash_tree_placeholder(int kind);
void scheme_hash_tree_tie_placeholder(Scheme_Hash_Tree *t, Scheme_Hash_Tree *base);
XFORM_NONGCING Scheme_Hash_Tree *scheme_hash_tree_resolve_placeholder(Scheme_Hash_Tree *t);
int scheme_hash_tree_kind(Scheme_Hash_Tree *t);
int scheme_hash_tree_subset_of(Scheme_Hash_Tree *t1, Scheme_Hash_Tree *t2);
XFORM_NONGCING int scheme_eq_hash_tree_subset_of(Scheme_Hash_Tree *t1, Scheme_Hash_Tree *t2);
XFORM_NONGCING int scheme_eq_hash_tree_subset_match_of(Scheme_Hash_Tree *t1, Scheme_Hash_Tree *t2);
intptr_t scheme_hash_tree_key_hash(Scheme_Hash_Tree *t1);

void scheme_set_root_param(int p, Scheme_Object *v);

int scheme_equal_modix_eq(Scheme_Object *obj1, Scheme_Object *obj2);
Scheme_Hash_Table *scheme_make_hash_table_equal_modix_eq();

Scheme_Object *scheme_intern_exact_parallel_symbol(const char *name, uintptr_t len);
Scheme_Object *scheme_symbol_append(Scheme_Object *s1, Scheme_Object *s2);
Scheme_Object *scheme_copy_list(Scheme_Object *l);

Scheme_Object *scheme_append_strings(Scheme_Object *s1, Scheme_Object *s2);

void scheme_reset_hash_table(Scheme_Hash_Table *ht, int *history);

XFORM_NONGCING void scheme_set_distinct_eq_hash(Scheme_Object *var2);

XFORM_NONGCING Scheme_Object *scheme_regexp_source(Scheme_Object *re);
int scheme_regexp_is_byte(Scheme_Object *re);
int scheme_regexp_is_pregexp(Scheme_Object *re);
Scheme_Object *scheme_make_regexp(Scheme_Object *str, int byte, int pcre, int * volatile result_is_err_string);
int scheme_is_pregexp(Scheme_Object *o);
void scheme_clear_rx_buffers(void);

int scheme_regexp_match_p(Scheme_Object *regexp, Scheme_Object *target);

Scheme_Object *scheme_gensym(Scheme_Object *base);
Scheme_Object *scheme_symbol_to_string(Scheme_Object *sym);

char *scheme_strdup_and_free(const char *str);

Scheme_Object *scheme_maybe_build_path(Scheme_Object *base, Scheme_Object *elem);

#ifdef SCHEME_BIG_ENDIAN
# define MZ_UCS4_NAME "UCS-4BE"
#else
# define MZ_UCS4_NAME "UCS-4LE"
#endif

#define SCHEME_SYM_UNINTERNEDP(o) (MZ_OPT_HASH_KEY(&((Scheme_Symbol *)(o))->iso) & 0x1)
#define SCHEME_SYM_PARALLELP(o) (MZ_OPT_HASH_KEY(&((Scheme_Symbol *)(o))->iso) & 0x2)
#define SCHEME_SYM_WEIRDP(o) (MZ_OPT_HASH_KEY(&((Scheme_Symbol *)(o))->iso) & 0x3)

Scheme_Object *scheme_current_library_collection_paths(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_current_library_collection_links(int argc, Scheme_Object *argv[]);
Scheme_Object *scheme_compiled_file_roots(int argc, Scheme_Object *argv[]);

int scheme_can_enable_write_permission(void);

#ifdef MZ_USE_JIT
int scheme_can_inline_fp_op();
int scheme_can_inline_fp_comp();
#else
# define scheme_can_inline_fp_op() 0
# define scheme_can_inline_fp_comp() 0
#endif

/* To suppress compiler warnings when it's difficult to avoid them otherwise: */
void scheme_unused_object(Scheme_Object*);
void scheme_unused_intptr(intptr_t);

intptr_t scheme_check_overflow(intptr_t n, intptr_t m, intptr_t a);

Scheme_Object *scheme_make_environment_variables(Scheme_Hash_Tree *ht);
void *scheme_environment_variables_to_block(Scheme_Object *env, int *_need_free);

int scheme_compare_equal(void *v1, void *v2);

/*========================================================================*/
/*                           places                                       */
/*========================================================================*/

#if defined(MZ_USE_PLACES)
# if defined(MZ_PRECISE_GC)
typedef struct Scheme_Symbol_Parts {
  Scheme_Hash_Table *table;
  int kind;
  unsigned int len;
  const char *name;
} Scheme_Symbol_Parts;

void scheme_spawn_master_place();
# endif
#endif

typedef struct Scheme_Place_Async_Channel {
  Scheme_Object so;
  intptr_t in;
  intptr_t out;
  intptr_t count;
  intptr_t size;
  intptr_t delta;
  intptr_t wr_ref, rd_ref; /* ref counts on readers and writers */
#if defined(MZ_USE_PLACES)
  mzrt_mutex *lock; /* no allocation while this lock is held */
#endif
  Scheme_Object **msgs;
  void **msg_memory;
  Scheme_Object **msg_chains; /* lists embedded in message blocks; specially traversed during GC */
  intptr_t mem_size;
  intptr_t reported_size; /* size reported to master GC; avoid reporting too often */
  void *wakeup_signal;
} Scheme_Place_Async_Channel;

typedef struct Scheme_Place_Bi_Channel_Link {
  /* all pointers; allocated as an array */
  Scheme_Place_Async_Channel *sendch;
  Scheme_Place_Async_Channel *recvch;
  struct Scheme_Place_Bi_Channel_Link *prev, *next;
} Scheme_Place_Bi_Channel_Link;

typedef struct Scheme_Place_Bi_Channel {
  Scheme_Object so;
  Scheme_Place_Bi_Channel_Link *link;
} Scheme_Place_Bi_Channel;

void scheme_free_place_bi_channels();

typedef struct Scheme_Place {
  Scheme_Object so;
  struct Scheme_Place_Object *place_obj;
  Scheme_Object *channel;
  Scheme_Custodian_Reference *mref;
  intptr_t result; /* set when place_obj becomes NULL */
#ifdef MZ_PRECISE_GC
  struct GC_Thread_Info *gc_info; /* managed by the GC */
#endif
  Scheme_Object *pumper_threads; /* Scheme_Vector of scheme threads */

  struct Scheme_Place *prev, *next; /* keeping a list of child places */
} Scheme_Place;

typedef struct Scheme_Place_Object {
  Scheme_Object so;
#if defined(MZ_USE_PLACES)
  mzrt_mutex *lock; /* no allocation or place-channel locks while this lock is held */
  mzrt_sema *pause;
#endif
  char die;
  char dead;
  char pbreak;
  char pausing;
  intptr_t refcount;
  void *signal_handle;
  void *parent_signal_handle; /* set to NULL when the place terminates */
  intptr_t result; /* initialized to 1, reset when parent_signal_handle becomes NULL */

  int id;
  intptr_t memory_use; /* set by inform hook on GC, used by GC for memory accounting */
  intptr_t prev_notify_memory_use; /* if memory_use > use_factor * prev_notify_memory_use, alert parent */
  double use_factor;
  intptr_t memory_limit; /* custodian-based limit on the place's memory use */
  uintptr_t *parent_need_gc; /* ptr to a variable in parent to force a GC (triggering accounting) */
} Scheme_Place_Object;

typedef struct Scheme_Serialized_File_FD {
  Scheme_Object so;
  Scheme_Object *name;
  struct rktio_fd_t *fd;
  intptr_t type;
  char flush_mode;
} Scheme_Serialized_File_FD;

typedef struct Scheme_Serialized_Socket_FD {
  Scheme_Object so;
  Scheme_Object *name;
  struct rktio_fd_t *fd;
  intptr_t type;
} Scheme_Serialized_Socket_FD;

int scheme_get_serialized_fd_flags(Scheme_Object* p, Scheme_Serialized_File_FD *so);
intptr_t scheme_dup_socket(intptr_t fd);
intptr_t scheme_dup_file(intptr_t fd);
void scheme_close_socket_fd(intptr_t fd);
void scheme_close_file_fd(intptr_t fd);
int scheme_os_pipe(intptr_t *fds, int near_index);
void scheme_tcp_abandon_port(Scheme_Object *port);
intptr_t scheme_socket_errno();
intptr_t scheme_errno();
void scheme_socket_to_input_port(intptr_t s, Scheme_Object *name, int takeover, Scheme_Object **_inp);
void scheme_socket_to_output_port(intptr_t s, Scheme_Object *name, int takeover, Scheme_Object **_outp);

#define SCHEME_PLACE_OBJECTP(o) (SCHEME_TYPE(o) == scheme_place_object_type)

#ifdef MZ_USE_PLACES
Scheme_Env *scheme_place_instance_init(void *stack_base, struct NewGC *, intptr_t memory_limit);
#endif
Scheme_Object *scheme_make_place_object();
void scheme_place_instance_destroy(int force);
void scheme_kill_green_thread_timer();
void scheme_place_check_for_interruption();
void scheme_place_set_memory_use(intptr_t amt);
void scheme_place_check_memory_use();
void scheme_clear_place_ifs_stack();

Scheme_Object **scheme_extract_sorted_keys(Scheme_Object *ht);
void scheme_sort_resolve_ir_local_array(Scheme_IR_Local **a, intptr_t count);

#ifdef MZ_USE_PLACES
Scheme_Object *scheme_place_make_async_channel();
void scheme_place_async_channel_send(Scheme_Object *ch, Scheme_Object *uo);
Scheme_Object *scheme_place_async_channel_receive(Scheme_Object *ch);
#endif

void scheme_process_global_lock(void);
void scheme_process_global_unlock(void);

#endif /* __mzscheme_private__ */
