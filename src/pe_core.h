#include <systemc.h>
#include "memory.h"
#include "neural_math.h"

enum class ProcessingStage {
    START,
    READ_DATA_SEND_ADDR,
    COMPUTE_WAIT,
    WRITE_RESULT,
    NEW,
    CALCULATE,
    IDLE
};

template<int ADDR_BITS, int DATA_BITS, int POCKET_SIZE, int PE_CORES>
SC_MODULE(PeCore) {
    // Input ports
    sc_in_clk clk_i;

    // Bus ports
    sc_in<bool> bus_rd;
    sc_in<bool> bus_wr;
    sc_in<sc_uint<ADDR_BITS>> bus_addr;
    sc_in<float> bus_data_in{"bus_data_in"};
    sc_out<float> bus_data_out{"bus_data_out"};

    // Bus with pe cores
    sc_out<bool>  sn_wr;
    sc_out<sc_uint<ADDR_BITS>> sn_index;
    sc_out<float> sn_data;
    sc_vector<sc_in<bool>>  sn_wr_i{"sn_wr", PE_CORES-1};
    sc_vector<sc_in<sc_uint<ADDR_BITS>>> sn_index_i{"sn_index", PE_CORES-1};
    sc_vector<sc_in<float>> sn_data_i{"sn_data", PE_CORES-1};

    // Local Memory ports
    sc_out<bool> local_memory_enable;
    sc_out<bool> local_memory_rd;
    sc_out<bool> local_memory_wr;
    sc_out<bool> local_memory_single_channel;
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
    int neuron_index;
    sc_uint<ADDR_BITS> res_addr;
    int size;
    int neurons_count;

    // Constructor
    SC_CTOR(PeCore) : neuralMath("neuralMath") {
        neuralMath.clock(clk_i);
        neuralMath.busy(math_busy);
        neuralMath.reset(math_reset);
        neuralMath.enable(math_enable);
        neuralMath.inputs(math_inputs);
        neuralMath.weights(math_weights);
        neuralMath.output(math_output);

        SC_THREAD(core_process);
        sensitive << clk_i.pos();
    }

    sc_uint<ADDR_BITS> index_core;
    void init(sc_uint<ADDR_BITS> core_index) {
        index_core = core_index;
    }

    bool enable;
    bool busy;
    // Core processing method
    void core_process() {
        while (true) {  // Infinite loop for the thread
            local_memory_enable.write(false);
            local_memory_wr.write(false);
            local_memory_rd.write(false);
            local_memory_single_channel.write(false);
            sn_wr.write(false);

            // load data
            if ((bus_addr.read() >= (0x1000*(index_core+1))) && (bus_addr.read() <= (0x1000*(index_core+1))+0xFFF)) {
                if (bus_wr) {
                    local_memory_addr.write(bus_addr.read() - (0x1000 * (index_core + 1)));
                    local_memory_enable.write(true);
                    local_memory_wr.write(true);
                    local_memory_single_channel.write(true);
                    local_memory_data_bo[0].write(bus_data_in.read());
                }
                wait();  // Pause and wait for next trigger
                continue;
            }

            // start
            if (bus_addr.read() == 0x100 && bus_wr.read()) {
                cout << "start core: " <<  index_core << endl;
                enable = true;
                stage = ProcessingStage::START;
            }
            read_data_to_pe_cores();
            if (enable) {
                if (stage == ProcessingStage::START){
                    busy = true;
                    math_reset.write(true);
                    math_enable.write(false);
                    stage = ProcessingStage::READ_DATA_SEND_ADDR;
                    wait();
                    continue;
                }
                if (stage == ProcessingStage::READ_DATA_SEND_ADDR){
                    cout << "READ_DATA_SEND_ADDR" << endl;
                    auto results = lm_read(0);
                    neuron_index = results[0];
                    res_addr = results[1];
                    size = results[2];
                    neurons_count = results[3];
                    cout << "neuron_index " << neuron_index << endl;
                    cout << "res_addr " << res_addr << endl;
                    cout << "size " << size << endl;
                    cout << "neurons_count " << neurons_count << endl;
                    stage = ProcessingStage::CALCULATE;
                    wait();
                    continue;
                }
                if (stage == ProcessingStage::CALCULATE){
                    math_reset.write(false);
                    math_enable.write(true);
                    for (int i = 0; i < neurons_count; ++i) {
                        int temp_size = 0;
                        while (temp_size < size) {
                            const int addr = 4+(temp_size);
                            cout << "size "<< size << endl;
                            cout << "read_ddd "<< addr << endl;
                            cout << "POCKET_SIZE "<< POCKET_SIZE << endl;
                            auto results = lm_read(addr);
                            for (int k = 0; k < (POCKET_SIZE/2); ++k) {
//                                if (temp_size < size) {
                                    cout << "math_inputs("<<i<<")["<< index_core <<"]("<<temp_size<<"): " << results[k * 2] << endl;
                                    cout << "math_weights("<<i<<")["<< index_core <<"]("<<temp_size<<"): " << results[(k * 2) + 1] << endl;
                                    math_inputs[k].write(results[k * 2]);
                                    math_weights[k].write(results[(k * 2) + 1]);
//                                }
//                                else{
//                                    math_inputs[k].write(0);
//                                    math_weights[k].write(0);
//                                }
                            }
                            temp_size = temp_size + POCKET_SIZE;
                            cout << "temp_size "<< temp_size << endl;
                        }
                        wait();
                        for (int k = 0; k < (POCKET_SIZE / 2); ++k) {
                            math_inputs[k].write(0);
                            math_weights[k].write(0);
                        }
                        math_enable.write(false);
                        while (math_busy.read()) {
                            cout << "Result["<< index_core <<"]: " << math_output.read() << endl;
                            wait();
                        }
//                        lm_write(res_addr, math_output.read());
                        cout << "Result["<< index_core <<"]: " << math_output.read() << endl;
                        stage = ProcessingStage::IDLE;
//                        send_data_to_pe_cores(math_output.read());
                    }
                    wait();
                    continue;
                }
                if (stage == ProcessingStage::IDLE){
                    wait();
                    continue;
                }
            }
            wait();  // Pause and wait for next trigger
        }
    }


    void compute_set() {
        cout << "COMPUTE_SET" << endl;
        local_memory_enable.write(true);
        local_memory_rd.write(true);
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
        cout << "WRITE_RESULT " << math_output.read() << endl;
        local_memory_enable.write(true);
        local_memory_rd.write(false);
        local_memory_wr.write(true);
        local_memory_addr.write(res_addr);
        local_memory_data_bo[0].write(math_output.read());
        send_data_to_pe_cores(math_output.read());
        stage = ProcessingStage::NEW;
    }

    void read_data_to_pe_cores(){
        for (int i = 0; i < (PE_CORES-1); ++i) {
            if (sn_wr_i[i].read()){
                cout << "read_data_to_pe_cores " << sn_data_i[i].read() << endl;
            }
        }
    }

    void send_data_to_pe_cores(float data){
        sn_wr.write(true);
        sn_index.write(0);
        sn_data.write(data);
    }

    void new_stage() {
        local_memory_enable.write(false);
        local_memory_rd.write(false);
        local_memory_wr.write(true);
    }

    void lm_write(sc_uint<ADDR_BITS> addr, float data){
        local_memory_enable.write(true);
        local_memory_rd.write(false);
        local_memory_wr.write(true);
        local_memory_addr.write(res_addr);
        local_memory_data_bo[0].write(math_output.read());
    }

    std::vector<float> lm_read(sc_uint<ADDR_BITS> addr){
        local_memory_enable.write(true);
        local_memory_rd.write(true);
        local_memory_wr.write(false);
        local_memory_addr.write(addr);
        wait();
        local_memory_rd.write(false);
        std::vector<float> results;
        results.reserve(POCKET_SIZE);
        for (int i = 0; i < POCKET_SIZE; ++i) {
            results.push_back(local_memory_data_bi[i].read());
        }
        return results;
    }
};
