#include <systemc.h>
#include "memory.h"
#include "neural_math.h"

template<int ADDR_BITS, int DATA_BITS, int POCKET_SIZE>
SC_MODULE(PeCore) {
    sc_in_clk clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> enable_i;
    sc_out<bool> busy_o;

    sc_out<bool>   local_memory_enable;
    sc_out<bool>   local_memory_rd;
    sc_out<bool>   local_memory_wr;
    sc_out<sc_uint<ADDR_BITS>>  local_memory_addr;
    sc_vector<sc_out<sc_uint<DATA_BITS>>>  local_memory_data_bo{"local_memory_data_bo", POCKET_SIZE};
    sc_vector<sc_in<sc_uint<DATA_BITS>>>  local_memory_data_bi{"local_memory_data_bi", POCKET_SIZE};

//    sc_in<sc_uint<ADDR_BITS>> addr_bo;
//    sc_in<sc_uint<DATA_BITS>> data_bo;
//    sc_out<sc_uint<DATA_BITS>> data_bi;
//    sc_in<bool> wr_o;
//    sc_in<bool> rd_o;



    NeuralMath<POCKET_SIZE> neuralMath{"NeuralMath"};


    sc_vector<sc_signal<float>> math_inputs{"inputs", POCKET_SIZE};
    sc_vector<sc_signal<float>> math_weights{"weights", POCKET_SIZE};
    sc_signal<float> math_output{"output", POCKET_SIZE};
    sc_signal<bool> math_clock;
    sc_signal<bool> math_reset;
    sc_signal<bool> math_enable;
    sc_signal<bool> math_busy;


    void process() {

    }

    SC_CTOR(PeCore) {

        neuralMath.clock(clk_i);
        neuralMath.busy(math_busy);
        neuralMath.reset(math_reset);
        neuralMath.enable(math_enable);
        neuralMath.inputs(math_inputs);
        neuralMath.weights(math_weights);
        neuralMath.output(math_output);


        SC_METHOD(process);
        sensitive << clk_i.pos();
    }
};


