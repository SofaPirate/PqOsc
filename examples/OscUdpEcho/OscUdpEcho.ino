/**
 * OscUdpEcho
 *
 * Echoes the value of the OSC message received on "/alpha" to "/beta"
 * over UDP. (i.e., /alpha -> Arduino -> /beta)
 *
 * The circuit:
 * - Requires a board with Ethernet or WiFi capability.
 *
 * Uses the MicroNet library for networking.
 * Change "remoteName" to match your destination device's local DNS name.
 * Uncomment USE_WIFI to use WiFi instead of Ethernet.
 *
 * Created in 2025 by Thomas O. Fredericks
 */
#include <Plaquette.h>
#include <PqOsc.h>

// Uncomment the following line to use WiFi instead of Ethernet.
// #define USE_WIFI

#ifdef USE_WIFI
  #include <MicroNetWiFi.h>
  MicroNetWiFi myMicroNet;
  WiFiUDP myUdp;
#else
  #include <MicroNetEthernet.h>
  MicroNetEthernet myMicroNet(MicroNetEthernet::Configuration::ATOM_POE_WITH_ATOM_LITE);
  EthernetUDP myUdp;
#endif

const char* remoteName = "m3-air"; // change for your own local DNS name
const unsigned int receivePort = 8888;
const unsigned int sendPort = 7777;

// Create the OSC UDP node (128 byte buffer).
OscUdp<128> myOsc(myUdp, receivePort);

// Link an input address to the node.
OscIn oscInAlpha(myOsc, "/alpha");

// Link an output address to the node.
OscOut oscOutBeta(myOsc, "/beta");

// Serial monitor.
Monitor monitor(115200);

void prepare() {
  myMicroNet.begin("plaquette");
}

void begin() {
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

void step() {
  myMicroNet.update();

  if (oscInAlpha.updated()) {
    oscInAlpha >> oscOutBeta;
  }
}
