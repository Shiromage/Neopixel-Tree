/*
MIT License

Copyright (c) 2020 Chase Baker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
  Chase Baker Nov 2020
  candy_cane.cpp
  The Candy Cane effect.
*/

#include <Arduino.h>
#include "config.h"

#define BASE_BRIGHTNESS 128
#define MAX_BRIGHTNESS  240

#define MAX_SPOTLIGHTS  5
#define SPOTLIGHT_BRIGHTNESS_OFFSET 40
#define SPOTLIGHT_MIN_SIZE  5
#define SPOTLIGHT_MAX_SIZE  25
#define SPOTLIGHT_MIN_LIFETIME  10000
#define SPOTLIGHT_MAX_LIFETIME  24000
#define SPOTLIGHT_MIN_SPEED 0.5
#define SPOTLIGHT_MAX_SPEED 2

const uint32_t Stripe_Sizes[] =
{
    26, 23, 21, 19, 17, 15, 12, 10, 7
};

struct spotlight_s
{
    float position;
    uint8_t intensity;
    uint32_t size;
    const float max_speed;
    float current_speed;
    uint32_t lifetime_left;
};

struct spotlight_s Spotlights[MAX_SPOTLIGHTS] = {0};

void drawCandyCane()
{
    static uint32_t last_millis = 0;
    uint32_t current_millis = millis();

    //Draw the white and red lines.
    int pixel = 0;
    for(int stripe = 0; stripe < sizeof(Stripe_Sizes) / sizeof(uint32_t); stripe++)
    {
        int color = (stripe % 2) ? (BASE_BRIGHTNESS | (BASE_BRIGHTNESS << 8) | (BASE_BRIGHTNESS << 16)) : (BASE_BRIGHTNESS << 8);
        for(int stripe_pixel = 0; stripe_pixel < Stripe_Sizes[stripe]; stripe_pixel++, pixel++)
        {
            Octo->setPixel(pixel, color);
        }
    }

    //Advance the age of each line.

    //Render each spotlight

    last_millis = current_millis;
}
