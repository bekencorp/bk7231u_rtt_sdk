/**
 **************************************************************************************
 * @file    psram.h
 * @brief   Driver API PSRAM
 *
 * @author  Aixing.Li
 * @version V1.0.0
 *
 * &copy; 2017 BEKEN Corporation Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __PSRAM_H__
#define __PSRAM_H__

#include <stdint.h>

int32_t spi_psram_init(void);

int32_t spi_psram_burst_set(uint32_t burst_size);

int32_t spi_psram_read_id(uint8_t id[12]);

uint32_t spi_psram_read(uint32_t addr, uint8_t* buffer, uint32_t size);

uint32_t spi_psram_write(uint32_t addr, uint8_t* buffer, uint32_t size);

#endif//__PSRAM_H__
