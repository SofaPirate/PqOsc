/*
 * PqOsc.cpp
 *
 * (c) 2025 Thomas O Fredericks :: tof(@)t-o-f(.)info
 * (c) 2026 Sofian Audry        :: info(@)sofianaudry(.)com
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

HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& OscIn::oscInList() {
  static HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS> instance;
  return instance;
}

void OscIn::handleOSCMessageCallback(MicroOscMessage &message)
{
    HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& oscInputs = oscInList();
    for (size_t i = 0; i < oscInputs.size(); i++)
    {
        OscIn *oscIn = oscInputs[i];

        if (message.checkSource(&oscIn->_microOsc) && message.checkOscAddress(oscIn->address()))
        {
            // Get typetag.
            char typeTag;
            message.copyTypeTags(&typeTag, sizeof(typeTag));

            // Get value.
            float value = 0;
            switch (typeTag) {
                case 'f': {
                    value = message.nextAsFloat();
                    break;
                }
                case 'd': {
                    value = static_cast<float>(message.nextAsDouble());
                    break;
                }
                case 'i': {
                    value = static_cast<float>(message.nextAsInt());
                    break;
                }
                case 'b': {
                    const unsigned char *blobData = 0;
                    if (message.nextAsBlob(&blobData) == sizeof(float)) {
                        memcpy(&value, blobData, sizeof(float));
                    }
                    break;
                }
                case 's': {
                    const char* str = message.nextAsString();
                    value = atof(str);
                    break;
                }

                case 'N':
                case 'I':
                case 'T': {
                    value = 1;
                    break;
                }

                case 'F': {
                    value = 0;
                    break;
                }

                // Unsupported.
                case 't': // osc timetag
                case 'm':
                case 'h':
                default:
                    value = 0;
            }

            // Set value.
            oscIn->receive(value);
        }
    }
}

void OscOut::_sendMessage() {

    switch (_typeTag) {

        // Values.
        case 'f': {
            _microOsc.sendFloat(_address, _value); 
            break;
        }
        case 'd': {
            _microOsc.sendDouble(_address, _value); 
            break;
        }
        case 'i': {
            _microOsc.sendInt(_address, round(_value)); 
            break;
        }
        case 'b': {
            _microOsc.sendBlob(_address, (unsigned char *) &_value, sizeof(_value)); 
            break;
        }
        case 's': {
            char str[32];
            sprintf(str, "%f", _value);
            _microOsc.sendString(_address, str);
            break;
        }

        // Triggers.
        case 'T': { // true
            _microOsc.sendTrue(_address); 
            break;
        }
        case 'F': { // false
            _microOsc.sendFalse(_address); 
            break;
        }
        case 'N': { // nil
            _microOsc.sendNull(_address); 
            break;
        }
        case 'I': { // impulse
            _microOsc.sendImpulse(_address); 
            break;
        }

        // Unsupported.
        case 't': // osc timetag
        case 'm':
        case 'h':
        default:;
    }

    // if (!isValue) {
    //     _value = 0.0;
    // }
}

}