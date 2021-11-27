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
  main.cpp
  Initializes the OctoWS2811 library and starts the first light effect
*/

#include <Arduino.h>
#include <OctoWS2811.h>
#include "config.h"

#define OCTO_FRAMEBUFFER_SIZE (MAX_LEDS_PER_CHANNEL * 6)
#define OCTO_DRAWINGBUFFER_SIZE (MAX_LEDS_PER_CHANNEL * 6)

OctoWS2811 * Octo;
int FrameBuffer[OCTO_FRAMEBUFFER_SIZE];
int DrawingBuffer[OCTO_DRAWINGBUFFER_SIZE];

/*
  List drawing function declarations to be listed in DrawRoutines
*/
void drawLineDance();
void drawCandyCane();
void drawTrainTracks();

/*
  This function pointer list determines the order at which drawing
  routines are cycled through. If a drawing routine is not added to
  this list, it will not be called.
*/
uint32_t CurrentDrawingRoutine = 0;
void (* DrawRoutines[])() =
{
  drawLineDance,
  drawCandyCane,
  drawTrainTracks
};

void enableLevelShifter();
void setupButton();

void setup()
{
  memset(DrawingBuffer, 0, sizeof(DrawingBuffer));
  memset(FrameBuffer, 0, sizeof(FrameBuffer));
  Octo = new OctoWS2811(MAX_LEDS_PER_CHANNEL, FrameBuffer, DrawingBuffer, OCTO_CONFIG);
  Octo->begin();
  enableLevelShifter();
  setupButton();
  pinMode(PIN_RANDOM, INPUT);
  randomSeed(analogRead(PIN_RANDOM));
}

void loop()
{
  for(int led = 0; led < MAX_LEDS_PER_CHANNEL; led++)
  {
    Octo->setPixel(led, 0);
  }
  DrawRoutines[CurrentDrawingRoutine]();
  Octo->show();
}

void changeDraw()
{
  if(++CurrentDrawingRoutine >= sizeof(DrawRoutines) / sizeof(DrawRoutines[0]))
  {
    CurrentDrawingRoutine = 0;
  }
}

void enableLevelShifter()
{
  pinMode(PIN_LEVEL_SHIFTER_EN, OUTPUT);
  digitalWrite(PIN_LEVEL_SHIFTER_EN, 0);
}

void setupButton()
{
  pinMode(PIN_BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), changeDraw, FALLING);
}
