/*
   xorshiro128plus.h

*/

#ifndef   XORSHIRO128PLUS_H_
#define   XORSHIRO128PLUS_H_

#include <stdint.h>

#define NUM_SEED 4

extern void seed(uint32_t seed[NUM_SEED]);
extern uint32_t get_seed(int sidx);
extern uint32_t next(void);
extern void jump(void);
extern void long_jump(void);

#endif /* XORSHIRO128PLUS_H_ */
