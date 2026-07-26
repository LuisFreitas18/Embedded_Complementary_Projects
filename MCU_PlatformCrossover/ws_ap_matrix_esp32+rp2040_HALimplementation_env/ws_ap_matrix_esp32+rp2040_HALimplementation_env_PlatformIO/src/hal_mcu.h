#ifndef HAL_MCU_H
#define HAL_MCU_H

#include "config.h"
#include <stdint.h>

extern int matrix[ROWS][COLS];

void init_nvs(void);
void load_matrix_from_nvs(void);
void print_matrix(void);

void wifi_init_softap(void);
void start_webserver(void);

#endif // HAL_MCU_H