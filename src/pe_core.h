#include <systemc.h>
#include "memory.h"
#include "neural_math.h"

enum class ProcessingStage {
    IDLE,
    READ_DATA_SEND_ADDR,
    READ_DATA_GET_DATA,
    COMPUTE_SET,
    COMPUTE_WAIT,
    WRITE_RESULT,
    NEW
};

template<int ADDR_BITS, int DATA_BITS, int POCKET_SIZE>
SC_MODULE(PeCore) {
    // Input ports
    sc_in_clk clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> enable_i;
    sc_out<bool> busy_o;

    // Bus ports
    sc_in<bool> bus_rd;
    sc_in<bool> bus_wr;
    sc_in<sc_uint<ADDR_BITS>> bus_addr;
    sc_vector<sc_in<float>> bus_data_in{"bus_data_in", POCKET_SIZE};
    sc_vector<sc_out<float>> bus_data_out{"bus_data_out", POCKET_SIZE};

    // Local Memory ports
    sc_out<bool> local_memory_enable;
    sc_out<bool> local_memory_rd;
    sc_out<bool> local_memory_wr;
    sc_out<sc_uint<ADDR_BITS>> local_memory_addr;
    sc_vector<sc_out<float>> local_memory_data_bo{"local_memory_data_bo", POCKET_SIZE};
    sc_vector<sc_in<float>> local_memory_data_bi{"local_memory_data_bi", POCKET_SIZE};

    // Other signals and components
    NeuralMath<POCKET_SIZE> neuralMath;
    sc_vector<sc_signal<float>> math_inputs{"inputs", POCKET_SIZE};
    sc_vector<sc_signal<float>> math_weights{"weights", POCKET_SIZE};
    sc_signal<float> math_output;
    sc_signal<bool> math_reset;
    sc_signal<bool> math_enable;
    sc_signal<bool> math_busy;

    // Internal state variables
    ProcessingStage stage;
    sc_uint<ADDR_BITS> res_addr;
    sc_uint<ADDR_BITS> size;

    // Constructor
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

    // Core processing method
    void core_process() {
        if (rst_i) {
            reset_core();
            return;
        }

        if (enable_i) {
            switch (stage) {
                case ProcessingStage::IDLE:
                    initialize_core();
                    break;
                case ProcessingStage::READ_DATA_SEND_ADDR:
                    read_data_send_addr();
                    break;
                case ProcessingStage::READ_DATA_GET_DATA:
                    read_data_get_data();
                    break;
                case ProcessingStage::COMPUTE_SET:
                    compute_set();
                    break;
                case ProcessingStage::COMPUTE_WAIT:
                    compute_wait();
                    break;
                case ProcessingStage::WRITE_RESULT:
                    write_result();
                    break;
                case ProcessingStage::NEW:
                    new_stage();
                    break;
            }
        }
    }

    // Helper functions for each stage
    void reset_core() {
        busy_o.write(false);
        stage = ProcessingStage::IDLE;
    }

    void initialize_core() {
        busy_o.write(true);
        math_reset.write(true);
        math_enable.write(false);
        stage = ProcessingStage::READ_DATA_SEND_ADDR;
    }

    void read_data_send_addr() {
        cout << "READ_DATA_SEND_ADDR" << endl;
        local_memory_enable.write(true);
        local_memory_rd.write(true);
        local_memory_wr.write(false);
        local_memory_addr.write(0);
        stage = ProcessingStage::READ_DATA_GET_DATA;
        math_reset.write(false);
        math_enable.write(true);
    }

    void read_data_get_data() {
        cout << "READ_DATA_GET_DATA" << endl;
        res_addr = local_memory_data_bi[0].read();
        size = local_memory_data_bi[1].read();
        cout << "size " << size << endl;
        local_memory_enable.write(true);
        local_memory_rd.write(true);
        local_memory_wr.write(false);
        local_memory_addr.write(2);
        stage = ProcessingStage::COMPUTE_SET;
    }

    void compute_set() {
        cout << "COMPUTE_SET" << endl;
        local_memory_enable.write(true);
        local_memory_addr.write(local_memory_addr.read() + POCKET_SIZE);
        for (int i = 0; i < (POCKET_SIZE / 2); ++i) {
            math_inputs[i].write(local_memory_data_bi[i * 2].read());
            math_weights[i].write(local_memory_data_bi[(i * 2) + 1].read());
        }
        size -= POCKET_SIZE / 2;
        if (size <= 0) {
            stage = ProcessingStage::COMPUTE_WAIT;
        }
    }

    void compute_wait() {
        cout << "COMPUTE_WAIT" << endl;
        for (int i = 0; i < (POCKET_SIZE / 2); ++i) {
            math_inputs[i].write(0);
            math_weights[i].write(0);
        }
        math_enable.write(false);
        if (!math_busy.read()) {
            stage = ProcessingStage::WRITE_RESULT;
        }
    }

    void write_result() {
        cout << "WRITE_RESULT" << endl;
        local_memory_enable.write(true);
        local_memory_rd.write(false);
        local_memory_wr.write(true);
        local_memory_addr.write(res_addr);
        local_memory_data_bo[0].write(math_output.read());
        stage = ProcessingStage::NEW;
    }

    void new_stage() {
        local_memory_enable.write(false);
        local_memory_rd.write(false);
        local_memory_wr.write(true);
    }
};
