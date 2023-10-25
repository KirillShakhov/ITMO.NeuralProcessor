#include <systemc.h>
#include "../src/neural_math.h"
#include "../src/clk.h"

int sc_main (int argc, char* argv[]) {
    static const int N_PORTS = 2;
    sc_vector<sc_signal<float>>  inputs{"inputs", N_PORTS};
    sc_vector<sc_signal<float>>  weights{"weights", N_PORTS};
    sc_signal<float>             output{"output", N_PORTS};

    sc_signal<bool>   clock;
    sc_signal<bool>   reset;
    sc_signal<bool>   enable;
    sc_signal<bool>   busy;
    ClockGenerator clock_gen("clock_gen");
    clock_gen.clk(clock);

    NeuralMath<N_PORTS> neuralMath("NeuralMath");
    neuralMath.clock(clock);
    neuralMath.busy(busy);
    neuralMath.reset(reset);
    neuralMath.enable(enable);
    neuralMath.inputs(inputs);
    neuralMath.weights(weights);
    neuralMath.output(output);

    sc_trace_file *wf = sc_create_vcd_trace_file("neural_math");
    for (int i = 0; i < N_PORTS; ++i) {
        sc_trace(wf, inputs[i], "in"+std::to_string(i));
        sc_trace(wf, weights[i],"weights"+std::to_string(i));
    }
    sc_trace(wf, output, "output");
    sc_trace(wf, clock, "clock");
    sc_trace(wf, reset, "reset");
    sc_trace(wf, enable, "enable");
    sc_trace(wf, busy, "busy");

    reset = 1;
    sc_start(10, SC_NS);

    reset = 0;
    enable = 1;
    inputs[0].write(0.9477695660751321f);
    inputs[1].write(0.9937557133609726f);
    weights[0].write(-4.0961078688499155f);
    weights[1].write(-14.892351723504802f);
    sc_start(10, SC_NS);

    inputs[0].write(0.999224345493303f);
    inputs[1].write(0.9992826834871f);
    weights[0].write(-8.589540987991482f);
    weights[1].write(-9.79067155131993f);
    sc_start(10, SC_NS);

    inputs[0].write(0.9991566901389833f);
    inputs[1].write(0.9999998737712528f);
    weights[0].write(6.262679709682974f);
    weights[1].write(45.32539856128967f);
    sc_start(10, SC_NS);

    inputs[0].write(0.9259701252780933f);
    inputs[1].write(0.9993096918528903f);
    weights[0].write(6.072379717196129f);
    weights[1].write(-17.452001343590457f);
    sc_start(10, SC_NS);
    enable = 0;
    sc_start(150, SC_NS);

    cout << "res" << output << endl;

    sc_close_vcd_trace_file(wf);
    return 0;
}
