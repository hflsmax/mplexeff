/* ----------------------------------------------------------------------------
  Copyright (c) 2021, Microsoft Research, Daan Leijen
  This is free software; you can redistribute it and/or modify it
  under the terms of the MIT License. A copy of the license can be
  found in the "LICENSE" file at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
   Ambiguity with state
-----------------------------------------------------------------------------*/

#include "test.h"

/*-----------------------------------------------------------------
  Benchmark
-----------------------------------------------------------------*/

typedef struct amb_state_handlers_s {
  mpe_frame_handle_t* state_handler;
  mpe_frame_handle_t* amb_handler;
} amb_state_handlers_t;

static bool xxor(mpe_frame_handle_t* h_a) {
  bool x = amb_flip(h_a);
  bool y = amb_flip(h_a);
  return ((x && !y) || (!x && y));
}


static void* foo(mpe_frame_handle_t* h, void* arg) {
  mpe_frame_handle_t* h_a;
  mpe_frame_handle_t* h_s;

  amb_state_handlers_t *handlers = (amb_state_handlers_t*)(arg);

  if (h->frame.effect == MPE_EFFECT(state)) {
    h_s = h;
    h_a = handlers->amb_handler;
  } else {
    h_s = handlers->state_handler;
    h_a = h;
  }
  bool p = amb_flip(h_a);
  long i = state_get(h_s);
  state_set(h_s, i + 1);
  bool b = ((i > 0 && p) ? xxor(h_a) : false);
  return mpe_voidp_bool(b);
}


/*-----------------------------------------------------------------
  Test
-----------------------------------------------------------------*/

static void print_bool(void* arg) {
  mpt_printf("%s", mpe_bool_voidp(arg) ? "true" : "false" );
}

static void* hstate( mpe_frame_handle_t* h_a, void* arg ) {
  amb_state_handlers_t handlers_;
  handlers_.amb_handler = h_a;
  return state_handle( &foo, 0, &handlers_ );
}

static blist amb_state(void) {
  return amb_handle( &hstate, NULL );
}

static void* hamb( mpe_frame_handle_t* h_s, void* arg) {
  amb_state_handlers_t handlers_;
  handlers_.state_handler = h_s;
  return amb_handle(&foo, &handlers_);
}

static blist state_amb(void) {
  return mpe_blist_voidp(state_handle(&hamb, 0, NULL));
}

static void test() {
  blist xs = NULL;
  mpt_bench{ xs = amb_state(); }
  mpt_printf("amb-state : "); blist_println(xs, &print_bool); 
  mpt_assert(blist_length(xs)==2, "amb-state");
  blist_free(xs);

  mpt_bench{ xs = state_amb(); }
  mpt_printf("state-amb : "); blist_println(xs, &print_bool);
  mpt_assert(blist_length(xs) == 5, "state-amb");
  blist_free(xs);
}


void amb_state_run(void) {
  test();
}

