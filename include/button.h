#ifndef button_h
#define button_h

#include <Arduino.h>

// For button events
struct ButtonEvents {
  const unsigned long minDuration;
  bool pressed;
};

// Physical button representation
template <int N>
struct Button {
  const uint8_t pin;
  bool pressed;
  ButtonEvents* events[N];
  static const int eventLength = N;
};

#endif