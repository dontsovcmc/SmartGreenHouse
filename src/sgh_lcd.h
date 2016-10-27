
#ifndef SGH_MENU_H
#define SGH_MENU_H

#include "wire.h"
#include <LiquidCrystal_I2C.h>

#define SCREEN_WIDTH 16
extern LiquidCrystal_I2C lcd;

void print_message(const char *buf, bool clean = true);
void print_screen(const char *message, const char *button1, const char *button2);

#endif