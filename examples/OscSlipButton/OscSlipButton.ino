/**
 * OscSlipButton
 *
 * Sends a boolean OSC message when a button is pressed or released.
 * Demonstrates using a specific type tag ('i' for integer) with OscOut.
 *
 * The circuit:
 * - Momentary push button attached between pin 2 and GND.
 *
 * Test with the Pure Data patch included in the extras directory.
 *
 * Created in 2025 by Sofian Audry
 */
#include <Plaquette.h>
#include <PqOsc.h>

// Button on pin 2.
DigitalIn button(2, INTERNAL_PULLUP);

// Create the OSC SLIP node (128 byte buffer, 115200 baud).
OscSlip<128> oscSlip(115200);

// Link an output address to the node with integer type.
// ie. 1 for true, 0 for false
OscOut oscOutButton(oscSlip, "/button", 'i');

void step() {
  // Send button state when changed.
  if (button.changed()) {
    button >> oscOutButton;
  }
}
