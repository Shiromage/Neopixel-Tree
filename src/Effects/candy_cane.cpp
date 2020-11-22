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

#define BASE_BRIGHTNESS 4
#define MAX_BRIGHTNESS  128

#define MAX_SPOTLIGHTS  5
#define SPOTLIGHT_MIN_SPAWN_TIME    2000
#define SPOTLIGHT_MAX_SPAWN_TIME    5000
#define SPOTLIGHT_BRIGHTNESS_OFFSET 80
#define SPOTLIGHT_INTENSITY_RAMP_TIME   1000
#define SPOTLIGHT_SPEED_RAMP_TIME       1000
#define SPOTLIGHT_MIN_RADIUS  5
#define SPOTLIGHT_MAX_RADIUS  15
#define SPOTLIGHT_MIN_LIFETIME  6000
#define SPOTLIGHT_MAX_LIFETIME  8000
#define SPOTLIGHT_MIN_SPEED 1
#define SPOTLIGHT_MAX_SPEED 3

const uint32_t Stripe_Sizes[] =
{
    26, 23, 21, 19, 17, 15, 12, 10, 7
};

struct spotlight_s
{
    float position;
    int8_t intensity; //The light intensity, which is a value between 0 and SPOTLIGHT_BRIGHTNESS_OFFSET
    uint32_t radius; // The size of the spotlight. If 0, the spotlight is not active
    float current_velocity;
    int lifetime_left; // The amount of time, in milliseconds, the spotlight has left to live
    uint32_t intensity_ramp_time; // The time it takes, in milliseconds, for the intensity to reach the max value from 0
    uint32_t speed_ramp_time; // The time it takes, in milliseconds, for the travel speed to reach the max value from minimum value
    int8_t ramp_direction; // 0 => no growth; 1 => positive growth; -1 => negative growth
};

struct spotlight_s Spotlights[MAX_SPOTLIGHTS] = {0};
int Spotlight_Spawn_Alarm = 0;

void drawCandyCane()
{
    static uint32_t last_millis = 0;
    delay(30);
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

    //Create a new spotlight if the alarm expires
    if(Spotlight_Spawn_Alarm <= current_millis)
    {
        for(int spotlight = 0; spotlight < MAX_SPOTLIGHTS; spotlight++)
        {
            if(Spotlights[spotlight].radius == 0)
            {
                Spotlights[spotlight].position = random(0, MAX_LEDS_PER_CHANNEL);
                Spotlights[spotlight].intensity = 0;
                Spotlights[spotlight].radius = random(SPOTLIGHT_MIN_RADIUS, SPOTLIGHT_MAX_RADIUS + 1);
                Spotlights[spotlight].lifetime_left = random(SPOTLIGHT_MIN_LIFETIME, SPOTLIGHT_MAX_LIFETIME + 1);
                Spotlights[spotlight].current_velocity = Spotlights[spotlight].lifetime_left % 2 ? SPOTLIGHT_MIN_SPEED : -SPOTLIGHT_MIN_SPEED;
                Spotlights[spotlight].ramp_direction = 1;
                if(Spotlights[spotlight].lifetime_left < SPOTLIGHT_INTENSITY_RAMP_TIME * 2)
                {
                    Spotlights[spotlight].intensity_ramp_time = Spotlights[spotlight].lifetime_left / 2;
                }
                else
                {
                    Spotlights[spotlight].intensity_ramp_time = SPOTLIGHT_INTENSITY_RAMP_TIME;
                }
                if(Spotlights[spotlight].lifetime_left < SPOTLIGHT_SPEED_RAMP_TIME * 2)
                {
                    Spotlights[spotlight].speed_ramp_time = Spotlights[spotlight].lifetime_left / 2;
                }
                else
                {
                    Spotlights[spotlight].speed_ramp_time = SPOTLIGHT_SPEED_RAMP_TIME;
                }
                Spotlight_Spawn_Alarm = current_millis + random(SPOTLIGHT_MIN_SPAWN_TIME, SPOTLIGHT_MAX_SPAWN_TIME + 1);
                //Serial.print("Spotlight "); Serial.print(spotlight); Serial.print(" at pos: "); Serial.println(Spotlights[spotlight].position);
                break;
            }
        }
        Spotlight_Spawn_Alarm = current_millis + random(SPOTLIGHT_MIN_SPAWN_TIME, SPOTLIGHT_MAX_SPAWN_TIME + 1);
    }

    // Advance the age of the spotlights
    for(int spotlight = 0; spotlight < MAX_SPOTLIGHTS; spotlight++)
    {
        if(Spotlights[spotlight].radius == 0)
        {
            continue;
        }
        Spotlights[spotlight].lifetime_left -= current_millis - last_millis;
        if(Spotlights[spotlight].lifetime_left <= 0)
        {
            Spotlights[spotlight].radius = 0;
            //Serial.print("Spotlight died by age: "); Serial.println(spotlight);
            //Serial.print("Sample time delta: "); Serial.println(current_millis - last_millis);
            continue;
        }

        Spotlights[spotlight].position += Spotlights[spotlight].current_velocity * ((current_millis - last_millis) / 1000.0);
        if(Spotlights[spotlight].position - Spotlights[spotlight].radius > MAX_LEDS_PER_CHANNEL - 1 ||
            Spotlights[spotlight].position + Spotlights[spotlight].radius < 0)
        {   //If the spotlight is out of sight, delete it and move on to the next spotlight.
            Spotlights[spotlight].radius = 0;
            //Serial.print("Spotlight died by travel: "); Serial.println(spotlight);
            continue;
        }

        if(Spotlights[spotlight].ramp_direction != 0)
        {
            // change velocity
            float velocity_step = (SPOTLIGHT_MAX_SPEED / ((float)Spotlights[spotlight].speed_ramp_time) * (current_millis - last_millis));
            velocity_step *= Spotlights[spotlight].ramp_direction;
            Spotlights[spotlight].current_velocity += velocity_step;
            if(abs(Spotlights[spotlight].current_velocity) >= SPOTLIGHT_MAX_SPEED)
            {
                Spotlights[spotlight].current_velocity = Spotlights[spotlight].current_velocity > 0 ? SPOTLIGHT_MAX_SPEED : -SPOTLIGHT_MAX_SPEED;
            }
            else if(abs(Spotlights[spotlight].current_velocity) < SPOTLIGHT_MIN_SPEED)
            {
                Spotlights[spotlight].current_velocity = Spotlights[spotlight].current_velocity > 0 ? SPOTLIGHT_MIN_SPEED : -SPOTLIGHT_MIN_SPEED;
            }

            // change intensity
            float intensity_step = (SPOTLIGHT_BRIGHTNESS_OFFSET / ((float)Spotlights[spotlight].intensity_ramp_time) * (current_millis - last_millis));
            intensity_step *= Spotlights[spotlight].ramp_direction;
            Spotlights[spotlight].intensity += (int)intensity_step;
            if(Spotlights[spotlight].intensity < 0)
            {
                Spotlights[spotlight].intensity = 0;
            }
            else if(Spotlights[spotlight].intensity > SPOTLIGHT_BRIGHTNESS_OFFSET)
            {
                Spotlights[spotlight].intensity = SPOTLIGHT_BRIGHTNESS_OFFSET;
                Spotlights[spotlight].ramp_direction = 0;
                //Serial.print("Spotlight reached full intensity: "); Serial.println(spotlight);
            }
        }
        else if(Spotlights[spotlight].lifetime_left < Spotlights[spotlight].intensity_ramp_time)
        {
            Spotlights[spotlight].ramp_direction = -1;
            Spotlights[spotlight].intensity_ramp_time = Spotlights[spotlight].lifetime_left - 200;
        }
    }

    //Render each spotlight
    for(int spotlight = 0; spotlight < MAX_SPOTLIGHTS; spotlight++)
    {
        if(Spotlights[spotlight].radius == 0)
        {
            continue;
        }
        int first_led = (int)Spotlights[spotlight].position - Spotlights[spotlight].radius;
        int last_led = first_led + 2 * Spotlights[spotlight].radius;
        if(first_led < 0)
            first_led = 0;
        if(last_led >= MAX_LEDS_PER_CHANNEL)
            last_led = MAX_LEDS_PER_CHANNEL - 1;

        for(int led = first_led; led <= last_led; led++)
        {
            int color_mask;
            if((Octo->getPixel(led) & 0xFF) == 0)
            {
                color_mask = 0xFF00;
            }
            else
            {
                color_mask = 0xFFFFFF;
            }
            int brightness = -(Spotlights[spotlight].intensity/(float)Spotlights[spotlight].radius)*abs(led - Spotlights[spotlight].position) + Spotlights[spotlight].intensity;
            brightness += (Octo->getPixel(led) & 0xFF00) >> 8;
            if(brightness > MAX_BRIGHTNESS)
            {
                brightness = MAX_BRIGHTNESS;
            }
            else if(brightness < BASE_BRIGHTNESS)
            {
                continue;
            }

            brightness = brightness | (brightness << 8) | (brightness << 16);
            brightness &= color_mask;
            Octo->setPixel(led, brightness);
        }
    }

    last_millis = current_millis;
}
