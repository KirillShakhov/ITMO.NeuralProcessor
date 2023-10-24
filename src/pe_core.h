#include <systemc.h>
#include "memory.h"
#include "neural_math.h"

enum ProcessingStage {
    READ_DATA_SEND_ADDR,
    READ_DATA_GET_DATA,
    COMPUTE_SET,
    WRITE_RESULT,
    IDLE
};

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

    ProcessingStage stage;
    sc_uint<ADDR_BITS> size;

    void process() {
        if (rst_i){
            stage = READ_DATA_SEND_ADDR;
            busy_o = false;
            return;
        }
        if (enable_i){
            local_memory_enable = true;
            local_memory_addr = 0;
            local_memory_rd = true;
            local_memory_wr = false;
//            stage = READ_DATA_GET_DATA;
            cout << "result addr: " << local_memory_data_bi[0] << endl;
            cout << "size: " << local_memory_data_bi[1] << endl;
            return;
            switch (stage) {
                case READ_DATA_SEND_ADDR:
                    cout << "READ_DATA_SEND_ADDR" << endl;
//                    local_memory_enable = true;
//                    local_memory_addr = 0;
//                    local_memory_rd = true;
//                    local_memory_wr = false;
//                    stage = READ_DATA_GET_DATA;
                    return;
                case READ_DATA_GET_DATA:
                    cout << "READ_DATA_GET_DATA" << endl;

                    cout << "result addr: " << local_memory_data_bi[0].read() << endl;
                    cout << "size: " << local_memory_data_bi[1] << endl;

                    size = local_memory_data_bi[1].read();
                    local_memory_enable = true;
                    local_memory_addr = 2;
                    local_memory_rd = true;
                    local_memory_wr = false;
                    stage = IDLE;
                    math_reset = true;
                    return;
//                case COMPUTE_SET:
//                    cout << "COMPUTE_SET" << endl;
//                    math_reset = false;
//                    math_enable = true;
//                    for (int i = 0; i < POCKET_SIZE; ++i) {
//                        math_inputs[i].write(local_memory_data_bi[i]->read());
//                        cout << "data:" << local_memory_data_bi[i]->read() << endl;
//                    }
//                    size--;
//                    if (size <= 0) {
//                        math_enable = false;
//                        stage = WRITE_RESULT;
//                    }
//                    return;
//                case WRITE_RESULT:
//                    cout << "WRITE_RESULT" << endl;
//                    if (!math_busy) {
//                        stage = IDLE;
//                    }
//                    return;
//                case IDLE:
//                    cout << "IDLE" << endl;
//                    cout << "math_output: " << math_output << endl;
//                    return;
            }
        }
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
        sensitive << clk_i.pos() << rst_i.pos();
    }
};


