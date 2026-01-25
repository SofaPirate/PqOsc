/**
 * OscSlipEcho
 *
 * Echoes the value of the OSC message received on "/alpha" to "/beta".
 * (i.e., /alpha -> Arduino -> /beta)
 *
 * The circuit:
 * - No external hardware required. Uses serial communication.
 *
 * Test with the Pure Data patch included in the extras directory.
 *
 * Created in 2025 by Thomas O. Fredericks
 */
#include <Plaquette.h>
#include <PqOsc.h>

// Create the OSC SLIP node (128 byte buffer, 115200 baud).
OscSlip<128> oscSlip(115200);

// Link an input address to the node.
OscIn oscInAlpha(oscSlip, "/alpha");

// Link an output address to the node.
OscOut oscOutBeta(oscSlip, "/beta");

void step() {
  if (oscInAlpha.updated()) {
    // Send message only when new message received.
    oscInAlpha >> oscOutBeta;
  }
}
