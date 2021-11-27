/*
MIT License

Copyright (c) 2021 Chase Baker

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
  Chase Baker Nov 2021
  train_tracks.cpp
  Train Tracks will bring in train cars up and down the path. These cars
  change colors and direction over time.
*/

#include <Arduino.h>
#include "config.h"

#define TRAIN_CAR_LENGTH    6  /*Length of a train car in lights (LEDs)*/
#define TRAIN_CAR_PADDING   4  /*Padding distance between two train cars*/
#define TRAIN_TOP_SPEED     10  /*Top speed in lights (LEDs) per second*/
#define TRAIN_ACCEL         1.5  /*The acceleration of the train when speeding up or slowing down*/
#define TRAIN_STOP_TIME     2  /*Time, in seconds, train spends at 0 speed to change direction*/
#define TRAIN_TRAVEL_TIME   20 /*Time, in seconds, train spends traveling after reaching top speed before slowing down*/

struct train_car_s
{
  float position; //the position of the top (head) LED of the car
  uint32_t color;
  train_car_s * front_car; //The car in front of this car (toward positive position).
  train_car_s * back_car; //The car behind this car (toward negative position).
};

struct train_s
{
    train_car_s * head;
    train_car_s * tail;
    int car_count;
    float speed;
    float accel;
};

enum train_car_color //give to get_color_code() to get exact value to send to an LED
{
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    CYAN,
    BLUE,
    PURPLE,
    PINK,
    WHITE,
    COLOR_COUNT
};

int get_color_code(enum train_car_color);
void draw_car(train_car_s *);

//const int train_car_count = (MAX_LEDS_PER_CHANNEL / TRAIN_CAR_LENGTH) + 1;
enum train_car_color new_car_color;
static uint32_t alarm_time;

void drawTrainTracks()
{
  static uint32_t last_millis = 0;
  static train_s train =
  {
      .head = NULL,
      .tail = NULL,
      .car_count = 0,
      .speed = 0,
      .accel = TRAIN_ACCEL
  };
  enum train_direction {FORWARD, BACKWARD};
  //enum train_state {FORWARD, STOP, BACKWARD};
  static enum train_direction direction = FORWARD;
  static enum train_car_color new_car_color = (enum train_car_color)random(0, COLOR_COUNT);

  if(train.head == NULL) //Initialize the train on first invocation
  {
      train.head = new train_car_s();
      train.tail = train.head;
      train.head->front_car = NULL;
      train.head->back_car = NULL;
      train.head->position = TRAIN_CAR_LENGTH;
      train.car_count = 1;
      train.head->color = get_color_code(new_car_color);
      while(train.head->position < MAX_LEDS_PER_CHANNEL + TRAIN_CAR_PADDING + TRAIN_CAR_LENGTH) //instantiate all train cars
      {
          train_car_s * car = new train_car_s();
          car->back_car = train.head;
          train.head->front_car = car;
          car->color = get_color_code(new_car_color);
          car->position = train.head->position + TRAIN_CAR_PADDING + TRAIN_CAR_LENGTH;
          train.head = car;
          train.car_count++;
      }
      alarm_time = 0;
  }

  uint32_t current_millis = millis();

  //advance train according to speed/time
  float position_change = (current_millis - last_millis) / 1000.0 * train.speed;
  //float speed_change = (current_millis - last_millis) * train.accel;
  for(train_car_s * car = train.head; car != NULL; car = car->back_car)
  {
      car->position += position_change;
  }

  if(train.head->position <= MAX_LEDS_PER_CHANNEL)
  {
      //spawn a new head
      train.head->front_car = new train_car_s();
      train.head->front_car->back_car = train.head;
      train.head->front_car->front_car = NULL;
      train.head->front_car->color = get_color_code(new_car_color);
      train.head->front_car->position = train.head->position + TRAIN_CAR_LENGTH + TRAIN_CAR_PADDING;
      train.head = train.head->front_car;
      train.car_count++;
  }
  else if(train.head->position > MAX_LEDS_PER_CHANNEL + TRAIN_CAR_LENGTH + TRAIN_CAR_PADDING)
  {
      //delete head out-of-bounds
      train_car_s * new_head_car = train.head->back_car;
      new_head_car->front_car = NULL;
      delete train.head;
      train.head = new_head_car;
      train.car_count--;
  }

  if(train.tail->position > TRAIN_CAR_LENGTH)
  {
      //spawn a new tail
      train.tail->back_car = new train_car_s();
      train.tail->back_car->front_car = train.tail;
      train.tail->back_car->back_car = NULL;
      train.tail->back_car->color = get_color_code(new_car_color);
      train.tail->back_car->position = train.tail->position - TRAIN_CAR_LENGTH - TRAIN_CAR_PADDING;
      train.tail = train.tail->back_car;
      train.car_count++;
  }
  else if(train.tail->position < 0 - TRAIN_CAR_PADDING)
  {
      //delete tail out-of-bounds
      train_car_s * new_tail_car = train.tail->front_car;
      new_tail_car->back_car = NULL;
      delete train.tail;
      train.tail = new_tail_car;
      train.car_count--;
  }

  //draw train
  for(train_car_s * car = train.head; car != NULL; car = car->back_car)
  {
      draw_car(car);
  }

  if(train.accel)
  {
      train.speed += (current_millis - last_millis) / 1000.0 * train.accel;
      if(abs(train.speed) > TRAIN_TOP_SPEED) //clamp speed to top speed
      {
          train.speed = (train.speed > 0) ? TRAIN_TOP_SPEED : -TRAIN_TOP_SPEED;
          train.accel = 0;
          alarm_time = current_millis + TRAIN_TRAVEL_TIME * 1000;
      }
      else if((train.speed <= 0 && direction == FORWARD) ||
                (train.speed >= 0 && direction == BACKWARD)) //Stop if speed is opposite of direction
      {
          train.speed = 0;
          train.accel = 0;
          direction = (direction == FORWARD) ? BACKWARD : FORWARD; //Flip direction
          alarm_time = current_millis + TRAIN_STOP_TIME * 1000;
          train_car_color old_color = new_car_color;
          new_car_color = (enum train_car_color)random(0, COLOR_COUNT);
          if(new_car_color == old_color) //Prevent repeat colors
          {
              new_car_color = (enum train_car_color)((new_car_color + 1 == COLOR_COUNT) ? 0 : (new_car_color + 1));
          }
      }
  }
  else if(abs(train.speed) == TRAIN_TOP_SPEED && current_millis >= alarm_time) // Time to slow down
  {
      //direction = (train.speed > 0) ? BACKWARD : FORWARD;
      train.accel = (direction == FORWARD) ? -TRAIN_ACCEL : TRAIN_ACCEL;
      alarm_time = 0;
  }
  else if(train.speed == 0 && current_millis >= alarm_time) // Time to start moving
  {
      train.accel = (direction == FORWARD) ? TRAIN_ACCEL : -TRAIN_ACCEL;
      alarm_time = 0;
  }

  last_millis = current_millis;
}

void draw_car(train_car_s * car)
{
    int head_led = (int)car->position;
    float head_fade_amount = car->position - head_led; // Should be a decimal number [0 , 1)
    float tail_fade_amount = 1.0 - head_fade_amount;
    int tail_led = head_led - TRAIN_CAR_LENGTH;
    if(head_led > MAX_LEDS_PER_CHANNEL - 1)
    {
        head_led = MAX_LEDS_PER_CHANNEL - 1;
        head_fade_amount = 0;
    }
    else if(head_led <= 0 && car->position < 0)
    {
        return;
    }
    if(tail_led < 0)
    {
        tail_led = 0;
        tail_fade_amount = 0;
    }

    for(int led_index = head_led; led_index >= tail_led; led_index--)
    {
        Octo->setPixel(led_index, car->color);
    }

    uint8_t red_faded;
    uint8_t green_faded;
    uint8_t blue_faded;
    if(head_fade_amount)
    {
        red_faded = ((car->color & 0xFF00) >> 8) * head_fade_amount;
        green_faded = ((car->color & 0xFF0000) >> 16) * head_fade_amount;
        blue_faded = (car->color & 0xFF) * head_fade_amount;
        Octo->setPixel(head_led, (red_faded << 8) | (green_faded << 16) | (blue_faded));
    }
    if(tail_fade_amount)
    {
        red_faded = ((car->color & 0xFF00) >> 8) * tail_fade_amount;
        green_faded = ((car->color & 0xFF0000) >> 16) * tail_fade_amount;
        blue_faded = (car->color & 0xFF) * tail_fade_amount;
        Octo->setPixel(tail_led, (red_faded << 8) | (green_faded << 16) | (blue_faded));
    }

}

int get_color_code(enum train_car_color color)
{
    switch(color)
    {
        case RED:
        {
            return 0x0000FF00;
        }break;
        case ORANGE:
        {
            return 0x0030CC00;
        }break;
        case YELLOW:
        {
            return 0x00AAFF00;
        }break;
        case GREEN:
        {
            return 0x00FF0000;
        }break;
        case PURPLE:
        {
            return 0x0000AAAA;
        }break;
        case BLUE:
        {
            return 0x000000FF;
        }break;
        case CYAN:
        {
            return 0x00BB00BB;
        }break;
        case PINK:
        {
            return 0x0044FF44;
        }break;
        case WHITE:
        {
            return 0x00CCCCCC;
        }
        default:
        {
            return 0x00222222;
        }
    }
}
