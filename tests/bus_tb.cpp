// module_A.h
#include "systemc.h"
#include "../src/bus_channel.cpp"

SC_MODULE(Module_A) {
    sc_port<bus_channel> out_port;

    void process() {
        out_port->write(42);
    }

    SC_CTOR(Module_A) {
        SC_THREAD(process);
    }
};

SC_MODULE(Module_B) {
    sc_port<bus_channel> in_port;
    sc_out<int> data_out;


    void process() {
        int data = in_port->read();
        data_out.write(data);
        cout << "Received data: " << data << endl;
    }

    SC_CTOR(Module_B) {
        SC_THREAD(process);
    }
};

int sc_main(int argc, char* argv[]) {
    Module_A mod_a("Module_A");
    Module_B mod_b("Module_B");
    bus_channel channel;

    sc_signal<int> data;

    mod_a.out_port(channel);
    mod_b.in_port(channel);
    mod_b.data_out(data);


    sc_start();
    return 0;
}