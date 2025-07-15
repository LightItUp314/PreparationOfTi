#ifndef FFT_H
#define FFT_H
#include "arm_math.h"
#include "stdbool.h"
void Myfft(void);
uint8_t shibie_basic(float *mag,float32_t*mag_max,float32_t mag_arr[]);
uint8_t find_min_index_diff_above_threshold(float32_t mag_arr[], uint32_t size, float32_t thred);
void guji_mf(uint16_t F,float32_t* mf,float32_t range_half);
bool shibie_2ask(int16_t adc_buff[],uint8_t range,float32_t thred);
uint8_t guji_2ask(void);
bool shibie_2fskor2psk(float32_t range_half,float32_t thred);
void guji_2fsk(uint16_t* Rb,float32_t* h);
void guji_2psk(uint16_t* Rb);
void guji_2ask_2(uint16_t* Rb,float32_t range_half,float32_t thred);
void guji_2fsk_2(uint16_t* Rb,float32_t* h,float32_t range_half,float32_t thred);
void guji_2psk_2(uint16_t* Rb);
#endif