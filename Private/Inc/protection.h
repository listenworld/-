#ifndef INC_PROTECTION_H_
#define INC_PROTECTION_H_

#include "main.h"
#define CCMRAM __attribute__((section("ccmram")))


void ERROR_Testing(uint32_t DCccr,uint32_t ACccr,float dc_v,float dc_i,float ac_d,float ac_q,float gd_d,float gd_q);


#endif

