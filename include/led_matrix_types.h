/**
 _     _____ ____     __  __    _  _____ ____  _____  __
| |   | ____|  _ \   |  \/  |  / \|_   _|  _ \|_ _\ \/ /
| |   |  _| | | | |  | |\/| | / _ \ | | | |_) || | \  /
| |___| |___| |_| |  | |  | |/ ___ \| | |  _ < | | /  \
|_____|_____|____/___|_|  |_/_/   \_\_| |_| \_\___/_/\_\
                |_____|
****************************************************************************************************
*    @file           : led_matrix.cpp
*    @brief          : LED Matrix Panel public types
****************************************************************************************************
*    @author     Volodymyr Noha
*
*    @description:
*    This is a header file for the RGB full-color LED matrix panel, 2.5mm Pitch, 64x32 pixels
*    public types
*
*    @section  HISTORY
*    v1.0  - First version
*
*    @section  LICENSE
*    None
****************************************************************************************************
*/

#ifndef LED_MATRIX_TYPES_H_
#define LED_MATRIX_TYPES_H_

/* Include Adafruit GFX library */
#include "RGBmatrixPanel.h"
#include "bit_bmp.h"
#include "fonts.h"

typedef struct text_params_st {
    int x;            // X positinon
    int y;            // Y position
    char *str;        // string to display
    const GFXfont *f; // font pointer
    int color;        // text color
    int pixels_size;  // size in pixels
} text_params_t_st;

typedef enum {
    COLOR_RED       = 0xF800,
    COLOR_GREEN     = 0x07E0,
    COLOR_BLUE      = 0x001F,
    COLOR_YELLOW    = 0xFFE0,
    COLOR_CYAN      = 0x07FF,
    COLOR_MAGENTA   = 0xF81F,
    COLOR_WHITE     = 0xFFFF,
    COLOR_BLACK     = 0x0000,
} text_color_t_en;

typedef enum {
    SIZE_1_PIXEL   = 1,
    SIZE_2_PIXELS  = 2,
    SIZE_3_PIXELS  = 3,
    SIZE_4_PIXELS  = 4,
} text_size_t_en;

typedef enum {
    LED_MATRIX_SUCCESS = 0,
    LED_MATRIX_ERROR_INVALID_ARGUMENTS = -1,
} led_matrix_status_t;

#endif /* LED_MATRIX_TYPES_H_ */

