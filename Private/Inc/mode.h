#ifndef INC_MODE_H_
#define INC_MODE_H_

#include "main.h"
#include "Mymain.h"

#define CCMRAM __attribute__((section("ccmram")))

CCMRAM void PV_Grid(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);

CCMRAM void PV_Grid_Storage(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);

CCMRAM void PV_AC_Storage(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);

CCMRAM void PV_DC_Storage(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);

CCMRAM void PV_AC_V(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);

CCMRAM void PV_AC_I(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);
	
CCMRAM void PV_DC_V(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);

CCMRAM void PV_DC_I(uint8_t *key,uint32_t *ad_value,uint32_t *av_value);


#endif
