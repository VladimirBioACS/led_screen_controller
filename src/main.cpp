#include <Arduino.h>

/* Include Adafruit GFX library */
#include "RGBmatrixPanel.h"
#include "bit_bmp.h"
#include "fonts.h"
#include <string.h>
#include <stdlib.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}