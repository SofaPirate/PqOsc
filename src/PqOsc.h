/*
 * PqOsc.h
 *
 * (c) 2025 Thomas O Fredericks :: tof(@)t-o-f(.)info
 * (c) 2025 Sofian Audry        :: info(@)sofianaudry(.)com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PQ_OSC_H_
#define PQ_OSC_H_

#include <PqCore.h>
#include <MicroOsc.h>
#include <MicroOscSlip.h>
#include <MicroOscUdp.h>

namespace pq
{

class OscIn; // forward declaration

/**
 * Receives OSC messages on a specific address.
 *
 * OscIn listens for incoming OSC messages matching its address and converts
 * the received value to a float. Use updated() to check for new messages
 * or onUpdate() for event-driven handling.
 */
class OscIn : public Unit
{
protected:
  /// Shared static container for all OscIn instances (for message routing).
  static HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& oscInList();

  // The OSC address to listen on.
  const char *_address;

  // The last received value.
  float _value;

  // Reference to the OSC transport layer.
  MicroOsc& _microOsc;

  // Flag indicating a message was received this frame.
  bool _messageReceived : 1;

  // Flag indicating value was updated last step.
  bool _valueUpdated : 1;

  // Reserved for future use.
  uint8_t _unused : 6;

protected:
  virtual void begin() override;
  virtual void step() override;

  /// Returns true iff an event of a certain type has been triggered.
  virtual bool eventTriggered(EventType eventType) override;

  /**
   * Receives a value from an incoming OSC message.
   * @param value the received value
   */
  void receive(float value);

public:
  /**
   * Constructor.
   * @param osc reference to the OSC transport (OscSlip or OscUdp)
   * @param address the OSC address to listen on (e.g., "/sensor")
   * @param engine the engine running this unit
   */
  OscIn(MicroOsc& osc, const char *address, Engine& engine = Engine::primary());

  /// Returns the OSC address.
  const char *address() const;

  /// Returns the current value.
  virtual float get() override;

  /// Returns true iff a new value was received last step.
  virtual bool updated() const;

  /// Registers callback for when a new value is received.
  virtual void onUpdate(EventCallback callback);

  /**
   * Static callback for handling incoming OSC messages.
   * Routes messages to matching OscIn instances.
   * @param message the received OSC message
   */
  static void handleOSCMessageCallback(MicroOsc& source, MicroOscMessage &message);

private:
  // Disabled: OscIn is input-only.
  virtual float put(float value) override;
};

/**
 * OSC transport layer using SLIP encoding over serial.
 *
 * OscSlip handles serial communication with SLIP (Serial Line Internet Protocol)
 * encoding for OSC messages. Use with OscIn and OscOut for bidirectional
 * OSC communication over a serial connection.
 *
 * @tparam MICRO_OSC_IN_SIZE buffer size for incoming messages
 */
template <const size_t MICRO_OSC_IN_SIZE>
class OscSlip : public Unit, public MicroOscSlip<MICRO_OSC_IN_SIZE>
{
private:
  // Pointer to serial port (if managed).
  SerialType* _serial = nullptr;

  // Baud rate for serial communication.
  unsigned long _baudRate = 0;

protected:
  void step() override
  {
    this->onOscMessageReceived(OscIn::handleOSCMessageCallback);
  }

  void begin() override
  {
    if (_serial)
    {
      // Safely start serial.
      beginSerial(*_serial, _baudRate);
    }
  }

public:
  /**
   * Constructor with existing stream.
   * @param stream reference to an already initialized stream
   * @param engine the engine running this unit
   */
  OscSlip(Stream& stream, Engine& engine = Engine::primary())
      : Unit(engine), MicroOscSlip<MICRO_OSC_IN_SIZE>(stream)
  {
  }

  /**
   * Constructor with serial port and baud rate.
   * @param serial reference to the serial port
   * @param baudRate baud rate for communication
   * @param engine the engine running this unit
   */
  OscSlip(SerialType& serial, unsigned long baudRate, Engine& engine = Engine::primary())
      : Unit(engine), MicroOscSlip<MICRO_OSC_IN_SIZE>(serial), _serial(&serial), _baudRate(baudRate)
  {
  }

  /**
   * Constructor with baud rate using default serial.
   * @param baudRate baud rate for communication
   * @param engine the engine running this unit
   */
  OscSlip(unsigned long baudRate, Engine& engine = Engine::primary())
      : OscSlip(PLAQUETTE_DEFAULT_SERIAL, baudRate, engine)
  {
  }
};

/**
 * OSC transport layer using UDP.
 *
 * OscUdp handles network communication for OSC messages over UDP.
 * Use with OscIn and OscOut for bidirectional OSC communication
 * over Ethernet or WiFi.
 *
 * @tparam MICRO_OSC_IN_SIZE buffer size for incoming messages
 */
template <const size_t MICRO_OSC_IN_SIZE>
class OscUdp : public Unit, public MicroOscUdp<MICRO_OSC_IN_SIZE>
{
protected:
  // UDP port for receiving messages.
  unsigned int _receivePort = 0;

protected:
  void step() override
  {
    this->onOscMessageReceived(OscIn::handleOSCMessageCallback);
  }

  void begin() override
  {
    if (_receivePort)
      this->udp->begin(_receivePort);
  }

public:
  /**
   * Constructor with UDP instance.
   * @param udp reference to the UDP instance
   * @param engine the engine running this unit
   */
  OscUdp(UDP& udp, Engine& engine = Engine::primary())
      : OscUdp(udp, 0, engine)
  {
  }

  /**
   * Constructor with UDP instance and receive port.
   * @param udp reference to the UDP instance
   * @param receivePort port to listen for incoming messages
   * @param engine the engine running this unit
   */
  OscUdp(UDP& udp, unsigned int receivePort, Engine& engine = Engine::primary())
      : Unit(engine),
        MicroOscUdp<MICRO_OSC_IN_SIZE>(udp),
        _receivePort(receivePort)
  {
  }

  /**
   * Constructor with UDP instance and destination.
   * @param udp reference to the UDP instance
   * @param destinationIp IP address to send messages to
   * @param destinationPort port to send messages to
   * @param engine the engine running this unit
   */
  OscUdp(UDP& udp, IPAddress destinationIp, unsigned int destinationPort, Engine& engine = Engine::primary())
      : OscUdp(udp, 0, destinationIp, destinationPort, engine)
  {
  }

  /**
   * Constructor with UDP instance, receive port, and destination.
   * @param udp reference to the UDP instance
   * @param receivePort port to listen for incoming messages
   * @param destinationIp IP address to send messages to
   * @param destinationPort port to send messages to
   * @param engine the engine running this unit
   */
  OscUdp(UDP& udp, unsigned int receivePort, IPAddress destinationIp, unsigned int destinationPort, Engine& engine = Engine::primary())
      : Unit(engine),
        MicroOscUdp<MICRO_OSC_IN_SIZE>(udp, destinationIp, destinationPort),
        _receivePort(receivePort)
  {
  }
};

/**
 * Sends OSC messages to a specific address.
 *
 * OscOut queues values received via put() or the flow operator (>>)
 * and sends them as OSC messages during step(). Supports multiple
 * OSC type tags for different data formats.
 */
class OscOut : public Unit
{
protected:
  // Reference to the OSC transport layer.
  MicroOsc& _microOsc;

  // The OSC address to send to.
  const char *_address;

  // The value to send.
  float _value;

  // OSC type tag ('f', 'i', 'd', etc.).
  char _typeTag;

  // Flag indicating a message needs to be sent.
  bool _needToSend;

public:
  /**
   * Constructor with default float type.
   * @param osc reference to the OSC transport (OscSlip or OscUdp)
   * @param address the OSC address to send to (e.g., "/led")
   * @param engine the engine running this unit
   */
  OscOut(MicroOsc& osc, const char *address, Engine& engine = Engine::primary());

  /**
   * Constructor with specified type tag.
   * @param osc reference to the OSC transport (OscSlip or OscUdp)
   * @param address the OSC address to send to (e.g., "/led")
   * @param typeTag OSC type tag: 'f' (float), 'i' (int), 'd' (double),
   *                'b' (blob), 's' (string), 'T' (true), 'F' (false),
   *                'N' (nil), 'I' (impulse)
   * @param engine the engine running this unit
   */
  OscOut(MicroOsc& osc, const char *address, char typeTag, Engine& engine = Engine::primary());

  /**
   * Queues a value to be sent as an OSC message.
   * @param value the value to send
   * @return the current value
   */
  virtual float put(float value) override;

  /// Returns the current value.
  virtual float get() override;

protected:
  virtual void step() override;

  /// Sends the queued OSC message.
  void _sendMessage();
};

}

#endif
