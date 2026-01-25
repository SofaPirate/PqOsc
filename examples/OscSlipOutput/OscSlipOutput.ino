/**
 * OscSlipOutput
 *
 * Outputs an OSC SLIP message every 100 milliseconds with the OSC address
 * "/wave" and the value of a sine wave.
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

// The input signal: a sine wave.
Wave wave(SINE);

// Create the OSC SLIP node (128 byte buffer, 115200 baud).
OscSlip<128> oscSlip(115200);

// Link an output address to the node.
OscOut oscOutWave(oscSlip, "/wave");

// Used to slow down message transmission.
Metronome ticker(0.1f); // 10 Hz

void step() {
  if (ticker) {
    wave >> oscOutWave;
  }
}
