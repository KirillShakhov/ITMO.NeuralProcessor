#include <systemc.h>

SC_MODULE(FloatingPointMultiplier) {
    sc_in<float> input1;
    sc_in<float> input2;
    sc_out<float> result;

    void multiply() {
        float result_value = input1.read() * input2.read();
        result.write(result_value);
    }

    SC_CTOR(FloatingPointMultiplier) {
        SC_METHOD(multiply);
        sensitive << input1 << input2;
    }
};