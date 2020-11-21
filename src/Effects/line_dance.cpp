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
  line_dance.cpp
  The Line Dance effect spawns lines of color that crawl down the strand.
  Each line is either red, green, or blue. The lines are various sizes and
  have different crawling speeds. When two different colors overlap, they
  blend. For example, a red line that overlaps with a blue line will create a
  purple line.
*/

#include <Arduino.h>
#include "config.h"

struct line_s
{
  float position;
  int size; // In LEDs
  int color;
  float speed; //In LEDs per second
};

const int Max_Lines = 20;
const int Min_Line_Size = 5;
const int Max_Line_Size = 15;
const uint32_t Min_Line_Spawn_Alarm = 1000; //The minimum time, in milliseconds, between two line spawns.
const uint32_t Max_Line_Spawn_Alarm = 4000; //The maximum time, in milliseconds, between two line spawns.
const float Max_Line_Speed = 25;
const float Min_Line_Speed = 4;

struct line_s Lines[Max_Lines] = {0};
int LineCount;
uint32_t LineSpawnAlarm = 0;
void drawLineDance()
{
  static uint32_t last_millis = 0;
  uint32_t current_millis = millis();

  if(current_millis >= LineSpawnAlarm && LineCount < Max_Lines) //If it's time to spawn a new line, do so
  {
    for(int line_index = 0; line_index < Max_Lines && LineCount < Max_Lines; line_index++)
    {
      if(Lines[line_index].size == 0)
      {
        Lines[line_index].position = 0;
        Lines[line_index].size = random(Min_Line_Size, Max_Line_Size + 1);
        Lines[line_index].color = 0xFF << (random(0, 3) * 8); //Red, Green, or Blue
        Lines[line_index].speed = random(Min_Line_Speed * 1000, Max_Line_Speed * 1000 + 1) / 1000;
        LineSpawnAlarm = current_millis + random(Min_Line_Spawn_Alarm, Max_Line_Spawn_Alarm + 1);
        LineCount++;
        break;
      }
    }
  }

  for(int line_index = 0; line_index < Max_Lines; line_index++) //Draw each line
  {
    if(Lines[line_index].size == 0) //Skip line if it's not instantiated.
    {
      continue;
    }
    Lines[line_index].position += Lines[line_index].speed * (current_millis - last_millis) / 1000.0;
    if(Lines[line_index].position - Lines[line_index].size >= MAX_LEDS_PER_CHANNEL) //If the line has crawled off, delete it
    {
      Lines[line_index].size = 0;
      LineCount--;
      continue;
    }

    int head_led = (int)Lines[line_index].position;
    int head_fade = (int)((Lines[line_index].position - head_led) * (Lines[line_index].color + 1)); //Calculates the fade intensity of the head LED
    head_fade &= Lines[line_index].color;
    int tail_led = head_led - Lines[line_index].size;
    int tail_fade = Lines[line_index].color - head_fade; //Calculates the fade intensity of the last LED

    int aux_led = Octo->getPixel(head_led);
    if(head_fade > (aux_led & Lines[line_index].color))
    {
      aux_led &= ~Lines[line_index].color;
      aux_led |= head_fade;
    }
    Octo->setPixel(head_led, aux_led);

    aux_led = Octo->getPixel(tail_led);
    if(tail_fade > (aux_led & Lines[line_index].color))
    {
      aux_led &= ~Lines[line_index].color;
      aux_led |= tail_fade;
    }
    Octo->setPixel(tail_led, aux_led);

    for(int led = tail_led + 1; led < head_led; led++) //Everything between the head and tail LEDs are at full brightness
    {
      Octo->setPixel(led, Lines[line_index].color | Octo->getPixel(led));
    }
  }

  last_millis = current_millis;
}