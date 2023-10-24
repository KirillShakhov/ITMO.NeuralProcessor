#include <systemc.h>

SC_MODULE(ClockGenerator) {
        sc_out<bool> clk;

        void generate_clock() {
            while (true) {
                clk.write(true);   // Set clock signal high
                wait(5, SC_NS);    // Wait for half the clock period
                clk.write(false);  // Set clock signal low
                wait(5, SC_NS);    // Wait for half the clock period
            }
        }

        SC_CTOR(ClockGenerator) {
            SC_THREAD(generate_clock);
        }
};


