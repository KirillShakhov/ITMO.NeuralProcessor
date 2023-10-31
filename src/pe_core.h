#include <systemc.h>
#include "memory.h"
#include "neural_math.h"

enum class ProcessingStage {
    START,
    GET_LAYER_SIZE,
    READ_DATA_SEND_ADDR,
    CALCULATE,
    IDLE
};

template<int ADDR_BITS, int POCKET_SIZE, int PE_CORES>
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
    sc_out<bool> sn_wr;
    sc_out<sc_uint<ADDR_BITS>> sn_index;
    sc_out<float> sn_data;
    sc_vector<sc_in<bool>> sn_wr_i{"sn_wr", PE_CORES - 1};
    sc_vector<sc_in<sc_uint<ADDR_BITS>>> sn_index_i{"sn_index", PE_CORES - 1};
    sc_vector<sc_in<float>> sn_data_i{"sn_data", PE_CORES - 1};

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
    ProcessingStage stage = ProcessingStage::IDLE;
    int current_layers;
    int layers_count;
    int input_count;
    int group_count;
    int group_index;
    sc_uint<ADDR_BITS> res_addr;

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

    // Core processing method
    int data_offset;

    void core_process() {
        while (true) {
            local_memory_enable.write(false);
            local_memory_wr.write(false);
            local_memory_rd.write(false);
            local_memory_single_channel.write(false);
            sn_wr.write(false);

            // load data
            if ((bus_addr.read() >= (0x1000 * (index_core + 1))) &&
                (bus_addr.read() <= (0x1000 * (index_core + 1)) + 0xFFF)) {
                auto lm_addr = bus_addr.read() - (0x1000 * (index_core + 1));
                if (bus_wr) {
                    local_memory_addr.write(lm_addr);
                    local_memory_enable.write(true);
                    local_memory_wr.write(true);
                    local_memory_single_channel.write(true);
                    local_memory_data_bo[0].write(bus_data_in.read());
                }
                if (bus_rd) {
                    cout << "lm_addr " << lm_addr << endl;
                    local_memory_addr.write(lm_addr);
                    local_memory_enable.write(true);
                    local_memory_rd.write(true);
                    wait();
                    float res = local_memory_data_bi[0].read();
                    cout << "lm_res " << res << endl;
                    bus_data_out.write(res);
                }
                wait();  // Pause and wait for next trigger
                continue;
            }

            // start
            if ((bus_addr.read() == 0xFFF || bus_addr.read() == (0x100 * (index_core + 1))) && bus_wr.read()) {
                cout << "start core: " << index_core << endl;
                stage = ProcessingStage::START;
            }

            // free flag
            if (bus_addr.read() == ((0x100 * (index_core + 1)) + 0xFF) && bus_rd.read()) {
                if (stage == ProcessingStage::IDLE) {
                    bus_data_out.write(0x100);
                }
                else{
                    bus_data_out.write(0x0);
                }
            }
            read_data_to_pe_cores();
            sn_wr.write(false);

            if (stage == ProcessingStage::START) {
                math_reset.write(true);
                math_enable.write(false);
                stage = ProcessingStage::GET_LAYER_SIZE;
                wait();
                continue;
            }
            if (stage == ProcessingStage::GET_LAYER_SIZE) {
                auto results = lm_read(0);
                layers_count = results[0];
                current_layers = 0;
                data_offset = 0;
                cout << "layers_count " << layers_count << endl;
                stage = ProcessingStage::READ_DATA_SEND_ADDR;
                wait();
                continue;
            }
            if (stage == ProcessingStage::READ_DATA_SEND_ADDR) {
                cout << "READ_DATA_SEND_ADDR" << endl;
                cout << "current_layers " << current_layers << endl;
                cout << "addr " << 1 + (current_layers * 3) << endl;
                auto results = lm_read(1 + (current_layers * 3));
                input_count = results[0];
                group_count = results[1];
                group_index = results[2];
                cout << "input_count1 " << input_count << endl;
                cout << "group_count " << group_count << endl;
                cout << "group_index " << group_index << endl;
                stage = ProcessingStage::CALCULATE;
                wait();
                continue;
            }
            if (stage == ProcessingStage::CALCULATE) {
                for (int current_group = 0; current_group < group_count; ++current_group) {
                    math_reset.write(true);
                    math_enable.write(false);
                    wait();
                    math_reset.write(false);
                    math_enable.write(true);

                    int temp_size = 0;
                    while (temp_size < (input_count * 2 + 7)) {
                        cout << "data_offset " << data_offset << endl;
                        const int addr =
                                1 + (layers_count * 3) + (data_offset * 2) + (current_group * (input_count * 2)) +
                                (temp_size);
                        cout << "input_count " << input_count << endl;
                        cout << "temp_size " << temp_size << endl;
                        cout << "addr " << addr << endl;
                        cout << "offset_m " << (current_group * (input_count * 2)) << endl;

                        auto data_from_memory = lm_read(addr);
                        for (int j = 0; j < data_from_memory.size(); ++j) {
                            cout << "data_from_memory[" << addr + j << "] " << data_from_memory[j] << endl;
                        }
                        for (int k = 0; k < (POCKET_SIZE / 2); ++k) {
                            cout << "k " << (k * 2) + 1 << endl;
                            if ((temp_size / 2) + k < input_count) {
                                cout << "math_inputs[" << index_core << "](" << (temp_size / 2) + k << "): "
                                     << data_from_memory[k * 2] << endl;
                                cout << "math_weights[" << index_core << "](" << (temp_size / 2) + k << "): "
                                     << data_from_memory[(k * 2) + 1] << endl;
                                math_inputs[k].write(data_from_memory[k * 2]);
                                math_weights[k].write(data_from_memory[(k * 2) + 1]);
                            } else {
                                cout << "math_inputs[" << index_core << "](" << (temp_size / 2) + k << "): " << 0
                                     << endl;
                                cout << "math_weights[" << index_core << "](" << (temp_size / 2) + k << "): " << 0
                                     << endl;
                                math_inputs[k].write(0);
                                math_weights[k].write(0);
                            }
                        }
                        temp_size += POCKET_SIZE;
                        cout << "POCKET_SIZE " << POCKET_SIZE << endl;
                    }
                    wait();
                    for (int k = 0; k < (POCKET_SIZE / 2); ++k) {
                        math_inputs[k].write(0);
                        math_weights[k].write(0);
                    }
                    math_enable.write(false);
                    while (math_busy.read()) {
                        wait();
                    }

                    int result_neuron_index = index_core + (current_group * PE_CORES);
                    cout << "Result[" << result_neuron_index << "]: " << math_output.read() << endl;
                    if (current_layers == layers_count-1){
                        sendLastResults(result_neuron_index, math_output.read());
                    }
                    else {
                        sendResults(result_neuron_index, math_output.read());
                    }
                }
                // go next
                data_offset += group_count * input_count;
                current_layers++;
                // check
                if (current_layers < layers_count) {
                    stage = ProcessingStage::READ_DATA_SEND_ADDR;
                    continue;
                }
                stage = ProcessingStage::IDLE;
                cout << "IDLE" << endl;
                wait();
                continue;
            }

            if (stage == ProcessingStage::IDLE) {
                wait();
                continue;
            }

            wait();  // Pause and wait for next trigger
        }
    }

    void sendResults(int result_index, float data) {
        auto results = lm_read(1 + ((current_layers + 1) * 3));
        int next_input_count = results[0];
        int next_group_count = results[1];

        cout << "next_group_count " << next_group_count << endl;
        for (int next_group_i = 0; next_group_i < next_group_count; ++next_group_i) {
            sc_uint<ADDR_BITS> addr = 1 + (layers_count * 3) + (data_offset * 2) + (group_count * input_count * 2) +
                                      (next_group_i * next_input_count * 2) + (result_index * 2);
            cout << "sendResults addr " << addr << endl;
            send_data_to_pe_cores(addr, data);
            read_data_to_pe_cores();
        }
    }

    void sendLastResults(int result_index, float data) {
        sc_uint<ADDR_BITS> addr = 0xF00 + result_index;
        cout << "sendResults addr " << addr << endl;
        send_data_to_pe_cores(addr, data);
        read_data_to_pe_cores();
    }

    void read_data_to_pe_cores() {
        std::vector<sc_uint<ADDR_BITS>> addr_vec;
        std::vector<float> data_vec;
        for (int i = 0; i < (PE_CORES - 1); ++i) {
            if (sn_wr_i[i].read()) {
                cout << "sn_index_i " << sn_index_i[i].read() << endl;
                cout << "read_data_to_pe_cores " << sn_data_i[i].read() << endl;
                addr_vec.push_back(sn_index_i[i].read());
                data_vec.push_back(sn_data_i[i].read());
            }
        }
        for (int i = 0; i < addr_vec.size(); ++i) {
            lm_write(addr_vec[i], data_vec[i]);
        }
    }

    void send_data_to_pe_cores(sc_uint<ADDR_BITS> addr, float data) {
        lm_write(addr, data);
        sn_wr.write(true);
        sn_index.write(addr);
        sn_data.write(data);
    }

    void lm_write(sc_uint<ADDR_BITS> addr_wr, float data_wr) {
        local_memory_enable.write(true);
        local_memory_rd.write(false);
        local_memory_wr.write(true);
        local_memory_addr.write(addr_wr);
        local_memory_single_channel.write(true);
        local_memory_data_bo[0].write(data_wr);
        wait();
        local_memory_enable.write(false);
        local_memory_wr.write(false);
        local_memory_single_channel.write(false);
    }

    std::vector<float> lm_read(sc_uint<ADDR_BITS> addr) {
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
