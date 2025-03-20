#pragma once

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

bool add_metadata(  int image_jpg_x, 
                    int image_jpg_y, 
                    uint8_t * image_jpg, 
                    int image_jpg_len,
                    uint8_t ** image_jpg_output,
                    int * image_jpg_output_size,
                    struct tm * tm);