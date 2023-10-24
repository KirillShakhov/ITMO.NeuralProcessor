// channel.h
#include "systemc.h"
#include "bus_interface.h"

class bus_channel : public bus_interface, public sc_prim_channel {
    int data;
public:
    void write(int data) override {
        this->data = data;
    }

    int read() override {
        return data;
    }
};