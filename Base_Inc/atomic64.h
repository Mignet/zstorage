

#ifndef LIBDRM_ATOMICS_H
#define LIBDRM_ATOMICS_H

typedef struct {

      volatile int atomic;
} atomic_t;
  

#define atomic_read(x) ((x)->atomic)

#define atomic_set(x, val) ((x)->atomic = (val))

#define atomic_inc(x) ((int) __sync_fetch_and_add (&(x)->atomic, 1))

#define atomic_dec_and_test(x) (__sync_fetch_and_add (&(x)->atomic, -1) == 1)

#define atomic_add(x, v) ((int) __sync_add_and_fetch(&(x)->atomic, (v)))

#define atomic_dec(x, v) ((int) __sync_sub_and_fetch(&(x)->atomic, (v)))

#define atomic_sub_return(v, x) atomic_dec(x, v)

#define atomic_add_return(v, x) atomic_add(x, v)

#define atomic_cmpxchg(x, oldv, newv) __sync_val_compare_and_swap (&(x)->atomic, oldv, newv)

#define atomic_sub(x,v) atomic_dec(x,v)

#define atomic_init(a,v) atomic_set(a,v)


#endif

