/**
 * OscSlipCallback
 *
 * Uses the onUpdate() callback to respond to incoming OSC messages.
 * This event-driven approach uses only begin() with no step() function.
 *
 * Echoes the value received on "/alpha" to "/beta".
 *
 * Test with the Pure Data patch included in the extras directory.
 *
 * Created in 2025 by Sofian Audry
 */
#include <Plaquette.h>
#include <PqOsc.h>

// Create the OSC SLIP node (128 byte buffer, 115200 baud).
OscSlip<128> oscSlip(115200);

// Link an input address to the node.
OscIn oscInAlpha(oscSlip, "/alpha");

// Link an output address to the node.
OscOut oscOutBeta(oscSlip, "/beta");

void begin() {
  oscInAlpha.onUpdate([]() {
    oscInAlpha >> oscOutBeta;
  });
}
