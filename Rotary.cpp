/* Rotary encoder handler for arduino.
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 *
 */

#include "Arduino.h"
#include "Rotary.h"

/*
 * The below state table has, for each state (row), the new state
 * to set based on the next encoder output. From left to right in,
 * the table, the encoder outputs are 00, 01, 10, 11, and the value
 * in that position is the new state to set.
 */

#define R_START 0x0

#define R_HALF_CCW_BEGIN 0x1
#define R_HALF_CW_BEGIN 0x2
#define R_HALF_START_M 0x3
#define R_HALF_CW_BEGIN_M 0x4
#define R_HALF_CCW_BEGIN_M 0x5
const unsigned char ttable_half[6][4] = {
    // R_START (00)
    {R_HALF_START_M,           R_HALF_CW_BEGIN,    R_HALF_CCW_BEGIN,  R_START},
    // R_HALF_CCW_BEGIN
    {R_HALF_START_M | DIR_CCW, R_START,            R_HALF_CCW_BEGIN,  R_START},
    // R_HALF_CW_BEGIN
    {R_HALF_START_M | DIR_CW,  R_HALF_CW_BEGIN,    R_START,           R_START},
    // R_HALF_START_M (11)
    {R_HALF_START_M,           R_HALF_CCW_BEGIN_M, R_HALF_CW_BEGIN_M, R_START},
    // R_HALF_CW_BEGIN_M
    {R_HALF_START_M,           R_HALF_START_M,     R_HALF_CW_BEGIN_M, R_START | DIR_CW},
    // R_HALF_CCW_BEGIN_M
    {R_HALF_START_M,           R_HALF_CCW_BEGIN_M, R_HALF_START_M,    R_START | DIR_CCW},
};

// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
    // R_START
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN,  R_START},
    // R_CW_FINAL
    {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
    // R_CW_BEGIN
    {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
    // R_CW_NEXT
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
    // R_CCW_BEGIN
    {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
    // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
    // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

/*
 * Constructor. Each arg is the pin number for each encoder contact.
 */
Rotary::Rotary(bool useHalfStepTable, bool flipLogicForPulldown)
{
  // Initialise state.
  state = R_START;
  // Don't invert read pin state by default
  inverter = flipLogicForPulldown ? 1 : 0;
  halfStep = useHalfStepTable;
}

unsigned char Rotary::process(unsigned char pin1State, unsigned char pin2State)
{
  // Grab state of input pins.
  unsigned char pinstate = ((inverter ^ pin2State << 1) | (inverter ^ pin1State));
  // Determine new state from the pins and state table.
  state = (halfStep ? ttable_half : ttable)[state & 0xf][pinstate];
  // Return emit bits, ie the generated event.
  return state & 0x30;
}