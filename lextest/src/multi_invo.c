/* ----------------------------------------------------------------------------
  Copyright (c) 2021, Microsoft Research, Daan Leijen
  This is free software; you can redistribute it and/or modify it
  under the terms of the MIT License. A copy of the license can be
  found in the "LICENSE" file at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "test.h"

/*-----------------------------------------------------------------
  Example programs
-----------------------------------------------------------------*/

void* multi_invo_action(void* arg) {
  UNUSED(arg);
  long k = 0;
  for (int i = 0; i < 100; i++) {
    k = multi_invo_ask();
  }
  exn_raise("");
  return mpe_voidp_long( k );
}

void* nested_action(void* arg) {
  multi_invo_handle(multi_invo_action, mpe_long_voidp(arg), NULL);
}

/*-----------------------------------------------------------------
  testing
-----------------------------------------------------------------*/
void multi_invo_run(void) {
  long init = 42;
  long res = 0;
  mpt_bench{ res = mpe_long_voidp(exn_handle(nested_action, NULL)); }
  mpt_printf("multi_invo    : %ld\n", res);
}

