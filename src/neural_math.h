#include <systemc.h>
#include "adder_mmm.h"
#include "adder_buffer.h"
#include "multiplier_array.h"
#include "sigmoid.h"

template<int N_PORTS>
SC_MODULE(NeuralMath) {
    sc_in_clk clock;
    sc_in<bool> reset;
    sc_in<bool> enable;
    sc_out<bool> busy;
    sc_vector<sc_in<float>> inputs{"inputs", N_PORTS};
    sc_vector<sc_in<float>> weights{"weights", N_PORTS};
    sc_out<float> output;

    MultiplierArray<N_PORTS> multiplierArray{"MultiplierArray"};
    AdderMMM<N_PORTS> adderMmm{"AdderMMM"};
    AdderBuffer adderBuffer{"AdderBuffer"};
    Sigmoid sigmoid{"Sigmoid"};

    sc_vector<sc_signal<float>> multiplierArrayInput1{"multiplierArrayInput1", N_PORTS};
    sc_vector<sc_signal<float>> multiplierArrayInput2{"multiplierArrayInput2", N_PORTS};
    sc_vector<sc_signal<float>> multiplierArrayOutputs{"multiplierArrayOutputs", N_PORTS};
    sc_signal<float> adderMmmOutput;
    sc_signal<float> adderBufferOutput;

    sc_signal<float> sigmoidInput;
    sc_signal<float> sigmoidOutput;

    sc_uint<2> count;

    void update_output() {
        if (reset.read()) {  // Асинхронный сброс
            output.write(0);
            busy.write(false);
        } else if (clock.posedge()) { // Если положительный фронт
            if (enable){
                busy.write(true);
                count = 2;
                output.write(0);
                for (int i = 0; i < N_PORTS; ++i) {
                    multiplierArrayInput1[i].write(inputs[i].read());
                    multiplierArrayInput2[i].write(weights[i].read());
                }
                return;
            }
            for (int i = 0; i < N_PORTS; ++i) {
                multiplierArrayInput1[i].write(0);
                multiplierArrayInput2[i].write(0);
            }
            sigmoidInput.write(adderBufferOutput.read());
            if (count <= 0) {
                busy.write(false);  // Not processing, so not busy
                output.write(sigmoidOutput.read());
            }
        }
        count--;
    }

    SC_CTOR(NeuralMath) {
        // Connect the multiplier array module
        multiplierArray.input1(multiplierArrayInput1);
        multiplierArray.input2(multiplierArrayInput2);
        multiplierArray.output(multiplierArrayOutputs);

        // Connect the adder module
        adderMmm.inputs(multiplierArrayOutputs);
        adderMmm.output(adderMmmOutput);

        adderBuffer.clock(clock);
        adderBuffer.reset(reset);
        adderBuffer.input(adderMmmOutput);
        adderBuffer.output(adderBufferOutput);

        sigmoid.input(sigmoidInput);
        sigmoid.output(sigmoidOutput);

        SC_METHOD(update_output);
        for (int i = 0; i < N_PORTS; ++i) {
            sensitive << inputs[i];
        }
        sensitive << reset;
        sensitive << clock.pos();  // Реагируем только на положительный фронт тактового сигнала
    }
};