# PqOsc

Open Sound Control (OSC) library for [Plaquette](https://github.com/SofaPirate/Plaquette)

GitHub repo : https://github.com/SofaPirate/PqOsc

To receive the data, your software must be able to process OSC SLIP. Here are a few demonstrations that show how to receive the OSC Slip data in Pure Data and Max (in French):
- [OSC SLIP dans Pd @ AidE](https://t-o-f.info/aide/#/pd/osc/slip/)
- [Pd : Relais OSC SLIP ⇄ UDP @ AidE](https://t-o-f.info/aide/#/pd/osc/relais/)
- [Max : Relais OSC SLIP ⇄ UDP @ AidE](https://t-o-f.info/aide/#/max/osc/relais/)
- There is also a Pure Data patch included in the `extras` directory.

## Examples

If you still want to try this library here are some examples. You can test these examples with the Pure Data patch included in the `extras` directory.

### Output only

This code outputs, every 100 milliseconds, an OSC SLIP message with the OSC address "/wave" and the value of a SINE Wave :
```cpp
#include <Plaquette.h>
#include <PqOsc.h>

// The input signal core wave.
Wave wave(SINE);

// Create the input and output node
OscSlip<128> oscSlip(115200); // use Serial by default

// Link an output address to the node
OscOut oscOutWave(oscSlip, "/wave");

// Used to slow down message transmission
Metronome ticker(0.1f); // 10 Hz

void step()
{
  if (ticker)
  {
    wave >> oscOutWave;
  }
}
```

Using Plaquette's [event management](https://plaquette.org/events.html) callbacks you can replace ``step()`` by:

```cpp
void begin() {
  ticker.onBang([]() // inline callback
  { 
    wave >> oscOutWave; 
  });
}
```

### Echo OSC (forward input to output) over SLIP

This code echoes the value of the OSC message received on the OSC address "/alpha" to the OSC address "/beta" (i.e. /alpha → Arduino → /beta):
```cpp
#include <Plaquette.h>
#include <PqOsc.h>

// Create the input and output node
OscSlip<128> oscSlip(115200);

// Link an input address to the node
OscIn oscInAlpha(oscSlip, "/alpha");

// Link an output address to the node
OscOut oscOutBeta(oscSlip, "/beta");

void step()
{
  if (oscInAlpha.updated()) 
  {
    // Send message only when new message received.
    oscInAlpha >> oscOutBeta;
  }
}
```

Event callback:
```cpp
void begin() {
  oscInAlpha.onUpdate([]() // inline callback
  { 
    oscInAlpha >> oscOutBeta;
  });
}
```

### Echo OSC (forward input to output) over UDP

The following code echoes every 100 milliseconds the value of the OSC message received on the OSC address "/alpha" to the OSC address "/beta" (i.e. /alpha → Arduino → /beta).

The code uses the [MicroNet](https://github.com/thomasfredericks/MicroNet) library to help with networking.

The microcontroller connects to the network with the name "plaquette" and it looks for a destination device on the network called "m3-air". Note that you should append ".local" tho these names when using when referencing them in your software. For example, in Pure Data, you would connect to "plaquette.local".

```cpp
#include <Plaquette.h>
#include <PqOsc.h>
#include <MicroNetEthernet.h>

const char* remoteName = "m3-air"; // change for your own local hostname
const unsigned int receivePort = 8888;
const unsigned int sendPort = 7777;

// MicroNet server over ethernet
MicroNetEthernet myMicroNet(MicroNetEthernet::Configuration::ATOM_POE_WITH_ATOM_LITE);

// UDP over ethernet
EthernetUDP myUdp;

// Create the input and output node
OscUdp<128> myOsc(myUdp, receivePort);
  
// Link an input address to the node
OscIn oscInAlpha(myOsc, "/alpha");

// Link an output address to the node
OscOut oscOutBeta(myOsc, "/beta");

// Serial monitor
Monitor monitor(115200);

void prepare() 
{
  myMicroNet.begin("plaquette");
}

void begin()
{
  println();
  print("My IP: ");
  print(myMicroNet.getIP());
  println();
  print("My name: ");
  print(myMicroNet.getName());
  println();

  print("Looking for: ");
  println(remoteName);
  println();

  IPAddress destinationIp = myMicroNet.resolveName(remoteName);
  myOsc.setDestination(destinationIp, sendPort);

  print("Found it at IP: ");
  print(destinationIp);
  println();
}

void step()
{
  myMicroNet.update();

  if (oscInAlpha.updated())
  {
    oscInAlpha >> oscOutBeta;
  }
}
```

If you are using wi-fi instead of ethernet, simply replace the MicroNet and UDP declarations with their wi-fi versions:

```cpp
MicroNetWiFi myMicroNet;

WiFiUDP myUdp;
```

### Type support

You can change output type by specifying it at construction.
For example, this will send the value as an integer:

```cpp
OscOut oscOutput(oscSlip, "/output", 'i');
```

Input type in ``OscIn`` are figured out at reception and are converted to float.

Please refer to [MicroOSC](https://github.com/thomasfredericks/MicroOsc/) for 
a complete reference on supported types.
