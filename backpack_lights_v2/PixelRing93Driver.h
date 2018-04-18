/*
    PixelRing93Driver.h - 
    Library for driving a
    93 LED 6 ring WS2812 panel

    Author - Sam McAnelly
    Date Created - 4/18/2018

*/
#ifndef PixelRing93Driver_h
#define PinxelRing93Driver_h

#define COLOR_MODE_COUNT 7
#define HISTORY_BUFFER_CNT 3
#define LAYER_COUNT 6
#define LIGHT_RING_TOTAL 93

#include "Arduino.h"
#include <AdafruitNeoPixel.h>

//colors
const uint32_t red = 0xFF0000;
const uint32_t green = red >> 8;
const uint32_t blue = green >> 8;
const uint32_t white = 0xFFFFFF;
const uint32_t yellow = 0xFFFF00;
const uint32_t orange = 0xFF6600;

//arrays holding various color combinations for the different color modes
const uint32_t rgb_colors[3] = {red, green, blue};
const uint32_t bw_colors[3] = {blue, white, blue};
const uint32_t red_colors[3] = {red, yellow, orange};
const uint32_t dark_red_colors[3] = {red, 0x800000, 0x696969}; 
const uint32_t rwb_colors[3] = {red, white, blue};
const uint32_t grn_colors[3] = {green, yellow, white};
const uint32_t color_18[18];

const PROGMEM int rgb_arr_sz = 3;
const PROGMEM int color_18_sz = 18;

const PROGMEM int glitch_layer_counts[LAYER_COUNT] = {32, 24, 16, 12, 8, 1};
const PROGMEM int glitch_layer_offsets[LAYER_COUNT] = {0, 32, 56, 72, 84, 92};

const int normal_layer_counts[LAYER_COUNT] = {32, 24, 16, 12, 8, 1};
const int normal_layer_offsets[LAYER_COUNT] = {0, 32, 56, 72, 84, 92};

//ring layer index bank
const uint16_t layer_0_indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                         13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                         23, 24, 25, 26, 27, 28, 29, 30, 31}; 
const uint16_t layer_1_indices[] = {32, 33, 34, 35, 36, 37, 38, 39, 40,
                         41, 42, 43, 44, 45, 46, 47, 48, 49, 
                         50, 51, 52, 53, 54, 55};
const uint16_t layer_2_indices[] = {56, 57, 58, 59, 60, 61, 62, 63, 64,
                         65, 66, 67, 68, 69, 70, 71};
const uint16_t layer_3_indices[] = {72, 73, 74, 75, 76, 77, 78, 79, 80,
                         81, 82, 83, 84};
const uint16_t layer_4_indices[] = {85, 86, 87, 88, 89, 90, 91};
const uint16_t layer_5_indices[] = {92};

const uint16_t layer_indices[][LAYER_COUNT] = {layer_0_indices, layer_1_indices, layer_2_indices, layer_3_indices,
                                   layer_4_indices, layer_5_indices};

class PixelRing93Driver {
    public:
        PixelRing93Driver(int pin);


    private:
        uint32_t *color_arrays[COLOR_MODE_COUNT];
        uint32_t *curr_color_array;

        const PROGMEM int rgb_arr_sz = 3;
        const PROGMEM int color_18_sz = 18;

        int curr_c_array_size;
        int curr_c_array_idx = 0;
        int color_idx = 0;

        uint32_t brightness = 20;
        uint32_t delayTime = 10;

        bool spiral_direction = true;

        bool bool_op1 = false;
        bool bool_op2 = false;

        uint16_t history_idx = 0;
        uint16_t transform_idx = 0;
        uint32_t transform_saved_state[LIGHT_RING_TOTAL];
        uint32_t history[HISTORY_BUFFER_CNT][LIGHT_RING_TOTAL];




};

#endif