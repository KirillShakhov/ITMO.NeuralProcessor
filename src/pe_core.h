#include <systemc.h>
#include "memory.h"
#include "neural_math.h"

enum ProcessingStage {
    READ_DATA_SEND_ADDR,
    READ_DATA_GET_DATA,
    COMPUTE_SET,
    COMPUTE_WAIT,
    WRITE_RESULT,
    NEW,
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
    sc_vector<sc_out<float>>  local_memory_data_bo{"local_memory_data_bo", POCKET_SIZE};
    sc_vector<sc_in<float>>  local_memory_data_bi{"local_memory_data_bi", POCKET_SIZE};

//    sc_in<sc_uint<ADDR_BITS>> addr_bo;
//    sc_in<sc_uint<DATA_BITS>> data_bo;
//    sc_out<sc_uint<DATA_BITS>> data_bi;
//    sc_in<bool> wr_o;
//    sc_in<bool> rd_o;



    NeuralMath<(POCKET_SIZE)> neuralMath;


    sc_vector<sc_signal<float>> math_inputs{"inputs", (POCKET_SIZE)};
    sc_vector<sc_signal<float>> math_weights{"weights", (POCKET_SIZE)};
    sc_signal<float> math_output;
    sc_signal<bool> math_clock;
    sc_signal<bool> math_reset;
    sc_signal<bool> math_enable;
    sc_signal<bool> math_busy;

    ProcessingStage stage;
    sc_uint<ADDR_BITS> res_addr;
    sc_uint<ADDR_BITS> size;


    void core_process() {
        if (rst_i){
            busy_o = false;
            stage = IDLE;
            return;
        }
        if (enable_i){
            switch (stage) {
                case IDLE:
                    busy_o = true;
                    math_reset = true;
                    math_enable = false;
                    stage = READ_DATA_SEND_ADDR;
                    return;
                case READ_DATA_SEND_ADDR:
                    cout << "READ_DATA_SEND_ADDR" << endl;
                    local_memory_enable = true;
                    local_memory_rd = true;
                    local_memory_wr = false;
                    local_memory_addr = 0;
                    stage = READ_DATA_GET_DATA;
                    math_reset = false;
                    math_enable = true;
                    return;
                case READ_DATA_GET_DATA:
                    cout << "READ_DATA_GET_DATA" << endl;
                    res_addr = local_memory_data_bi[0];
                    size = local_memory_data_bi[1];
                    cout << "size " << size << endl;
                    local_memory_enable = true;
                    local_memory_rd = true;
                    local_memory_wr = false;
                    local_memory_addr = 2;
                    stage = COMPUTE_SET;
                    return;
                case COMPUTE_SET:
                    cout << "COMPUTE_SET" << endl;
                    local_memory_enable = true;
                    local_memory_addr = local_memory_addr.read()+(POCKET_SIZE);
                    for (int i = 0; i < (POCKET_SIZE/2); ++i) {
                        math_inputs[i].write(local_memory_data_bi[i*2].read());
                        math_weights[i].write(local_memory_data_bi[(i*2)+1].read());
                    }
                    size -= POCKET_SIZE/2;
                    if (size <= 0) {
                        stage = COMPUTE_WAIT;
                    }
                    return;
                case COMPUTE_WAIT:
                    cout << "COMPUTE_WAIT" << endl;
                    for (int i = 0; i < (POCKET_SIZE/2); ++i) {
                        math_inputs[i].write(0);
                        math_weights[i].write(0);
                    }
                    math_enable = false;
                    if (!math_busy.read()){
                        stage = WRITE_RESULT;
                    }
                    return;
                case WRITE_RESULT:
                    cout << "WRITE_RESULT" << endl;

                    local_memory_enable = true;
                    local_memory_rd = false;
                    local_memory_wr = true;
                    local_memory_addr = res_addr;
                    float resu;
                    resu = math_output.read();
                    local_memory_data_bo[0].write(resu);
                    stage = NEW;
                    return;
                case NEW:
                    local_memory_enable = false;
                    local_memory_rd = false;
                    local_memory_wr = true;
                    return;
            }
        }
    }

    SC_CTOR(PeCore) : neuralMath("neuralMath") {
        neuralMath.clock(clk_i);
        neuralMath.busy(math_busy);
        neuralMath.reset(math_reset);
        neuralMath.enable(math_enable);
        neuralMath.inputs(math_inputs);
        neuralMath.weights(math_weights);
        neuralMath.output(math_output);

        SC_METHOD(core_process);
        sensitive << clk_i.pos();
    }
};


