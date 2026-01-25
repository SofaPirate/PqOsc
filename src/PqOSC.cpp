/*
 * PqOsc.cpp
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
#include "PqOSC.h"

namespace pq {

// Static singleton list of all OscIn instances.
HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& OscIn::oscInList() {
  static HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS> instance;
  return instance;
}

OscIn::OscIn(MicroOsc& osc, const char *address, Engine& engine)
    : Unit(engine), _microOsc(osc), _address(address), _value(0), _valueUpdated(false)
{
  oscInList().add(this);
}

void OscIn::begin() {
  _messageReceived = _valueUpdated = false;
}

void OscIn::step() {
  _valueUpdated = _messageReceived;
  _messageReceived = false;
}

bool OscIn::eventTriggered(EventType eventType) {
  switch (eventType) {
    case EVENT_UPDATE: return updated();
    default:           return Unit::eventTriggered(eventType);
  }
}

void OscIn::receive(float value) {
  _value = value;
  _messageReceived = true;
}

const char* OscIn::address() const {
  return _address;
}

float OscIn::get() {
  return _value;
}

bool OscIn::updated() const {
  return _valueUpdated;
}

void OscIn::onUpdate(EventCallback callback) {
  onEvent(callback, EVENT_UPDATE);
}

float OscIn::put(float value) {
  // Disabled: OscIn is input-only.
  return value;
}

void OscIn::handleOSCMessageCallback(MicroOscMessage &message)
{
  HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& oscInputs = oscInList();
  for (size_t i = 0; i < oscInputs.size(); i++)
  {
    OscIn *oscIn = oscInputs[i];

    // Check if message matches this OscIn's source and address.
    if (message.checkSource(&oscIn->_microOsc) && message.checkOscAddress(oscIn->address()))
    {
      // Get type tag.
      char typeTag;
      message.copyTypeTags(&typeTag, sizeof(typeTag));

      // Convert OSC value to float based on type tag.
      float value = 0;
      switch (typeTag) {
        case 'f': // float
          value = message.nextAsFloat();
          break;

        case 'd': // double
          value = static_cast<float>(message.nextAsDouble());
          break;

        case 'i': // int32
          value = static_cast<float>(message.nextAsInt());
          break;

        case 'b': { // blob (extract float if size matches)
          const unsigned char *blobData = 0;
          if (message.nextAsBlob(&blobData) == sizeof(float)) {
            memcpy(&value, blobData, sizeof(float));
          }
          break;
        }

        case 's': // string (parse as float)
          value = atof(message.nextAsString());
          break;

        case 'N': // nil
        case 'I': // impulse
        case 'T': // true
          value = 1;
          break;

        case 'F': // false
          value = 0;
          break;

        // Unsupported types.
        case 't': // OSC timetag
        case 'm': // MIDI message
        case 'h': // int64
        default:
          value = 0;
      }

      // Send value to the OscIn instance.
      oscIn->receive(value);
    }
  }
}

OscOut::OscOut(MicroOsc& osc, const char *address, Engine& engine)
    : OscOut(osc, address, 'f', engine)
{
}

OscOut::OscOut(MicroOsc& osc, const char *address, char typeTag, Engine& engine)
    : Unit(engine), _microOsc(osc), _address(address), _value(0), _typeTag(typeTag), _needToSend(false)
{
}

float OscOut::put(float value) {
  _value = value;
  _needToSend = true;
  return get();
}

float OscOut::get() {
  return _value;
}

void OscOut::step() {
  if (_needToSend) {
    _sendMessage();
    _needToSend = false;
  }
}

void OscOut::_sendMessage() {
  switch (_typeTag) {
    // Value types.
    case 'f': // float
      _microOsc.sendFloat(_address, _value);
      break;

    case 'd': // double
      _microOsc.sendDouble(_address, _value);
      break;

    case 'i': // int32
      _microOsc.sendInt(_address, round(_value));
      break;

    case 'b': // blob
      _microOsc.sendBlob(_address, (unsigned char *) &_value, sizeof(_value));
      break;

    case 's': { // string
      char str[32];
      sprintf(str, "%f", _value);
      _microOsc.sendString(_address, str);
      break;
    }

    // Trigger types.
    case 'T': // true
      _microOsc.sendTrue(_address);
      break;

    case 'F': // false
      _microOsc.sendFalse(_address);
      break;

    case 'N': // nil
      _microOsc.sendNull(_address);
      break;

    case 'I': // impulse
      _microOsc.sendImpulse(_address);
      break;

    // Unsupported types.
    case 't': // OSC timetag
    case 'm': // MIDI message
    case 'h': // int64
    default:
      break;
  }
}

}
