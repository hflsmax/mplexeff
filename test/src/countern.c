/* ----------------------------------------------------------------------------
  Copyright (c) 2021, Microsoft Research, Daan Leijen
  This is free software; you can redistribute it and/or modify it
  under the terms of the MIT License. A copy of the license can be
  found in the "LICENSE" file at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
   Counter10: state under 10 readers
-----------------------------------------------------------------------------*/
#include "test.h"

/*-----------------------------------------------------------------
  Benchmark
-----------------------------------------------------------------*/

typedef struct counter_handlers_s {
  mpe_frame_handle_t* state_handler;
} counter_handlers_t;

static void* bench_counter(mpe_frame_handle_t* h_r, void* arg) {
  mpe_frame_handle_t* h_s = ((counter_handlers_t*)arg)->state_handler;
  long count = 0;
  long i;
  while ((i = state_get(h_s)) > 0) {
    //trace_printf("counter: %i\n", i);
    state_set(h_s, i-1);
    count++;
  }
  return mpe_voidp_long(count);
}


/*-----------------------------------------------------------------
  Run
-----------------------------------------------------------------*/
static void* bench_reader1(mpe_frame_handle_t* h, void* arg) {
  counter_handlers_t handlers_;
  counter_handlers_t* handlers;
  if (h->frame.effect == MPE_EFFECT(state)) {
    handlers_.state_handler = h;
    handlers = &handlers_;
  } else {
    handlers = (counter_handlers_t*)arg;
  }
  return reader_handle(&bench_counter, 1, (void*)handlers);
}
static void* bench_reader2(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader1, 2, arg);
}
static void* bench_reader3(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader2, 3, arg);
}
static void* bench_reader4(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader3, 4, arg);
}
static void* bench_reader5(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader4,5,arg);
}
static void* bench_reader6(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader5, 6, arg);
}
static void* bench_reader7(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader6, 7, arg);
}
static void* bench_reader8(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader7, 8, arg);
}
static void* bench_reader9(mpe_frame_handle_t* h_r, void* arg) {
  return reader_handle(&bench_reader8, 9, arg);
}
static void* bench_reader10(mpe_frame_handle_t* h_s, void* arg) {
  counter_handlers_t handlers_;
  handlers_.state_handler = h_s;
  return reader_handle(&bench_reader9, 10, &handlers_);
}

static void test(long count) {
  long res = 0;
  /*mpt_bench{ res = mpe_long_voidp(state_handle(&bench_reader1,count,NULL)); }
  mpt_printf("counter1  : %ld\n", res);
  mpt_assert(res == count, "counter1");*/

  mpt_bench{ res = mpe_long_voidp(ostate_handle(&bench_reader1,count,NULL)); }
  mpt_printf("ocounter1 : %ld\n", res);
  mpt_assert(res == count, "ocounter1");

  mpt_bench{ res = mpe_long_voidp(state_handle(&bench_reader10,count,NULL)); }  
  mpt_printf("counter10 : %ld\n", res);
  mpt_assert(res == count, "counter10");
  
  mpt_bench{ res = mpe_long_voidp(ostate_handle(&bench_reader10,count,NULL)); }
  mpt_printf("ocounter10: %ld\n", res);
  mpt_assert(res == count, "ocounter10");
}

void countern_run(void) {
  #ifdef NDEBUG
  test(10010010L);
  #else
  test(100100L);
  #endif
}

