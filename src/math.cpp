#include <systemc.h>
#include "../src/floating_point_multiplier.cpp"

SC_MODULE(MathModule) {
    sc_in<float> input1;
    sc_in<float> input2;
    sc_out<float> result;

    FloatingPointMultiplier* multiplier;

    void multiply() {
        float result_value = input1.read() * input2.read();
        result.write(result_value);
    }

    SC_CTOR(MathModule) {
        SC_METHOD(multiply);
        sensitive << input1 << input2;
    }
};