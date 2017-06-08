// (C) Copyright Princeton University 2009
// Written by Christian Bienia
// Implementation of pthreads-style barriers based on pthread mutexes
// Use to replace pthread barriers on machines that have pthreads but not pthread barriers

// Some comments on this barrier implementation and directions for possible improvements:
//
//   * This is a straightforward two-phase centralized barrier implementation
//   * It's main advantage is that it only requries the pthreads library (no atomic instructions),
//     making it very portable and easy to maintain if libpthread is already installed
//   * The only more advanced feature is the ability to spin (if enabled). This should yield
//     significant performance improvements if the number of threads is less or equal to the
//     number of processors
//   * There are many ways to improve this implementation, just check the scientific literature.
//     Some suggestions are:
//       - Dynamically adapt maximum amount of spinning based on observed runtime behavior
//       - Tree-based or "butterfly" barriers (John M. Mellor-Crumm, TOCS 1991)
//       - Adaptive combining tree barriers (Michael L. Scott et al, IJPP 1994)

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "parsec_barrier.hpp"
#include <stdint.h>
typedef bool bool_t;
#include "arch_spinlock_backoff.h"

struct parsec_barrier {
	unsigned max;
	volatile unsigned n;
	volatile bool_t   is_arrival_phase;
#if defined(MCS_LOCK_BARRIER)
	mcs_lock lock;
#else
	backoff_lock lock;
#endif
};


//Internal debugging functions
#define NOT_IMPLEMENTED() Not_Implemented(__FUNCTION__,__FILE__,__LINE__);

static inline void Not_Implemented(const char* function,const char* file,unsigned int line) {
  fprintf(stderr, "ERROR: Something in function %s in file %s, line %u is not implemented.\n", function, file, line);
  exit(1);
}

#ifdef ENABLE_SPIN_BARRIER
//Counter type to use for spinning
typedef unsigned long long int spin_counter_t;

//Maximum amount of iterations to spin on flag before falling back to blocking
//NOTE: A value of 350 corresponds to about 1 us on modern computers
//Set value so we spin no more than 0.1 ms
static const spin_counter_t SPIN_COUNTER_MAX=350*100;
#endif //ENABLE_SPIN_BARRIER


//Barrier initialization & destruction
int parsec_barrier_init(parsec_barrier_t *barrier, const parsec_barrierattr_t *attr, unsigned count) {
	assert(attr == NULL);
	parsec_barrier_t pb = malloc(sizeof(struct parsec_barrier));

	if (pb == NULL) {
		assert(false);
		return ENOMEM;
	}

	pb->max = count;
	pb->n = 0;
	pb->is_arrival_phase = true;
#if defined(MCS_LOCK_BARRIER)
	__insn_mf();
	pb->lock = NULL;
#else
	backoff_initialize_lock(&(pb->lock));
#endif

	*barrier = pb;
	return 0;
}

int parsec_barrier_destroy(parsec_barrier_t *barrier) {
    free(*barrier);
    return 0;
}

//Barrier attribute initialization & destruction
int parsec_barrierattr_destroy(parsec_barrierattr_t *attr) {
  if(attr==NULL) return EINVAL;
  //simply do nothing
  return 0;
}

int parsec_barrierattr_init(parsec_barrierattr_t *attr) {
  if(attr==NULL) return EINVAL;
  //simply do nothing
  return 0;
}

//Barrier attribute modification
int parsec_barrierattr_getpshared(const parsec_barrierattr_t *attr, int *pshared) {
  if(attr==NULL || pshared==NULL) return EINVAL;
  *pshared = *attr;
  return 0;
}

int parsec_barrierattr_setpshared(parsec_barrierattr_t *attr, int pshared) {
  if(attr==NULL) return EINVAL;
  if(pshared!=PARSEC_PROCESS_SHARED && pshared!=PARSEC_PROCESS_PRIVATE) return EINVAL;
  //Currently we only support private barriers (the default)
  if(pshared!=PARSEC_PROCESS_PRIVATE) NOT_IMPLEMENTED();
  *attr = pshared;
  return 0;
}

//Barrier usage
int parsec_barrier_wait(parsec_barrier_t *barrier) {
	// Enter arrival phase
	while(!((*barrier)->is_arrival_phase)); // TODO: try flag?

	unsigned local_n;

#if defined(MCS_LOCK_BARRIER)
	mcs_lock_t me;
	lock_mcs(&((*barrier)->lock), &me);
	local_n = ++((*barrier)->n);
	unlock_mcs(&((*barrier)->lock), &me);
#else
	backoff_acquire_lock_without_preemption(&((*barrier)->lock));
	local_n = ++((*barrier)->n);
	backoff_release_lock(&((*barrier)->lock));
#endif

	if (local_n == (*barrier)->max) { // last thread is arrived
		(*barrier)->is_arrival_phase = false;
	}

	// Enter departure phase
	while(((*barrier)->is_arrival_phase));

#if defined(MCS_LOCK_BARRIER)
	lock_mcs(&((*barrier)->lock), &me);
	local_n = --((*barrier)->n);
	unlock_mcs(&((*barrier)->lock), &me);
#else
	backoff_acquire_lock_without_preemption(&((*barrier)->lock));
	local_n = --((*barrier)->n);
	backoff_release_lock(&((*barrier)->lock));
#endif

	if (local_n == 0) { // last thread is leaved
		(*barrier)->is_arrival_phase = true;
	}

	return 0;
}


