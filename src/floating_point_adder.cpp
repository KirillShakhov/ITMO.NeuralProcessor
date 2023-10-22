#include <systemc.h>

SC_MODULE(FloatingPointAdder) {
    sc_in<float> input1;
    sc_in<float> input2;
    sc_out<float> result;

    void adder() {
        float result_value = input1.read() + input2.read();
        result.write(result_value);
    }

    SC_CTOR(FloatingPointAdder) {
        SC_METHOD(adder);
        sensitive << input1 << input2;
    }
};