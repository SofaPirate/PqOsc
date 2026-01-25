#include "PqOSC.h"

namespace pq {

HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& OscIn::oscInList() {
  static HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS> instance;
  return instance;
}

void OscIn::handleOSCMessageCallback(MicroOscMessage &message)
{
    HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& oscInputs = oscInList();
    for (size_t i = 0; i != oscInputs.size(); i++)
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