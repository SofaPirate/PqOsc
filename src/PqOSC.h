#ifndef PQ_OSC_H_
#define PQ_OSC_H_

#include <PqCore.h>
#include <MicroOsc.h>
#include <MicroOscSlip.h>
#include <MicroOscUdp.h>

namespace pq
{
    // ===============================================================
    class OscIn; // forward declaration

    // OscIn class ----------------------------------------------------------- |
    class OscIn : public Unit
    {
    protected:
        // Shared static container containing all units. Static because it is shared between all OscIn units.
        static HybridArrayList<OscIn*, PLAQUETTE_MAX_UNITS>& oscInList();

        const char *_address;
        float _value;
        MicroOsc &_microOsc;
        bool _messageReceived : 1;
        bool _valueUpdated : 1;
        uint8_t _unused : 6;

    protected:
        void begin() override
        {
            _messageReceived = _valueUpdated = false;
        }

        void step() override {
            _valueUpdated = _messageReceived;
            _messageReceived = false;
        }

        /// Returns true iff an event of a certain type has been triggered.
        bool eventTriggered(EventType eventType) override {
            switch (eventType) {
            case EVENT_UPDATE: return updated();
            default:           return Unit::eventTriggered(eventType);
            }
        }    

        void receive(float f)
        {
            _value = f;
            _messageReceived = true;
        }

    public:
        const char *address() const
        {
            return _address;
        }

        OscIn(MicroOsc &osc, const char *address, Engine &engine = Engine::primary())
            : Unit(engine), _microOsc(osc), _address(address), _value(0), _valueUpdated(false)
        {
            oscInList().add(this);
        }

        float get() override
        {
            return _value;
        }

        /// Returns true iff value was changed.
        virtual bool updated() const { return _valueUpdated; }

        /// Registers event callback on finish event.
        virtual void onUpdate(EventCallback callback) { onEvent(callback, EVENT_UPDATE); }

        // ----------------------------------------------------------------------- |
        // handleOSCMessageCallback ---------------------------------------------- |
        static void handleOSCMessageCallback(MicroOscMessage &message);
        // ---------------------------------------------------------------------- |

    private:
        // DISABLE THE PUT
        float put(float f) override
        {
            //_value = f;
            return f;
        }

    };

    // OscSlip class -------------------------------------------------------- |
    template <const size_t MICRO_OSC_IN_SIZE>
    class OscSlip : public Unit, public MicroOscSlip<MICRO_OSC_IN_SIZE>
    {
    private:
        SerialType* _serial = nullptr;
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
        // Constructor with default value for _iic_address
        OscSlip(Stream &stream, Engine &engine = Engine::primary())
            : Unit(engine), MicroOscSlip<MICRO_OSC_IN_SIZE>(stream)
        {
        }

        OscSlip(SerialType &serial, unsigned long baudRate, Engine &engine = Engine::primary())
            : Unit(engine), MicroOscSlip<MICRO_OSC_IN_SIZE>(serial), _serial(&serial), _baudRate(baudRate)
        {
        }

        OscSlip(unsigned long baudRate, Engine &engine = Engine::primary())
            : OscSlip(PLAQUETTE_DEFAULT_SERIAL, baudRate, engine)
        {
        }

    };
    // ----------------------------------------------------------------------- |

    // OscUdp class ---------------------------------------------------------- |
    // OSC UDP Communication
    template <const size_t MICRO_OSC_IN_SIZE>
    class OscUdp : public Unit, public MicroOscUdp<MICRO_OSC_IN_SIZE>
    {
    protected:
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
        OscUdp(UDP &udp, Engine &engine = Engine::primary())
            : OscUdp(udp, 0, engine)
        {
        }

        OscUdp(UDP &udp, unsigned int receivePort, Engine &engine = Engine::primary())
            : Unit(engine), 
              MicroOscUdp<MICRO_OSC_IN_SIZE>(udp), 
              _receivePort(receivePort)
        {
        }

        OscUdp(UDP &udp, IPAddress destinationIp, unsigned int destinationPort, Engine &engine = Engine::primary())
            : OscUdp(udp, 0, destinationIp, destinationPort, engine)
        {
        }

        OscUdp(UDP &udp, unsigned int receivePort, IPAddress destinationIp, unsigned int destinationPort, Engine &engine = Engine::primary())
            : Unit(engine), 
              MicroOscUdp<MICRO_OSC_IN_SIZE>(udp, destinationIp, destinationPort),
              _receivePort(receivePort)
        {
        }
    };
    // ----------------------------------------------------------------------- |

    // OscOut class ---------------------------------------------------------- |
    class OscOut : public Unit
    {
    protected:
        MicroOsc &_microOsc;
        const char *_address;
        float _value;

        char _typeTag;

        bool _needToSend;

    public:
        OscOut(MicroOsc &osc, const char *address, Engine& engine = Engine::primary()) 
            : OscOut(osc, address, 'f', engine)
        {}

        OscOut(MicroOsc &osc, const char *address, char typeTag, Engine& engine = Engine::primary())
            : Unit(engine), _microOsc(osc), _address(address), _value(0), _typeTag(typeTag), _needToSend(false)
        {}

        float put(float f) override
        {
            _value = f;
            _needToSend = true;
            return get();
        }

        float get() override
        {
            return _value;
        }

    protected:
        void step() override
        {
            if (_needToSend)
            {
                _sendMessage();
                _needToSend = false;
            }
        }

        void _sendMessage();

    };
    // ----------------------------------------------------------------------- |
}
#endif