#include <systemc.h>
#include "memory.h"
#include "neural_math.h"

enum class ProcessingStage {
    START,
    GET_LAYER_SIZE,
    READ_DATA_SEND_ADDR,
    COMPUTE_WAIT,
    WRITE_RESULT,
    WAIT_DATA,
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

    bool enable;
    int current_group;
    // Core processing method
    int data_offset = 0;
    int index_calc = 0;
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
            sn_wr.write(false);
            if (enable) {
                if (stage == ProcessingStage::START){
                    math_reset.write(true);
                    math_enable.write(false);
                    stage = ProcessingStage::GET_LAYER_SIZE;
                    wait();
                    continue;
                }
                if (stage == ProcessingStage::GET_LAYER_SIZE){
                    auto results = lm_read(0);
                    layers_count = results[0];
                    current_layers = 0;
                    data_offset = 0;
                    cout << "layers_count " << layers_count << endl;
                    stage = ProcessingStage::READ_DATA_SEND_ADDR;
                    wait();
                    continue;
                }
                if (stage == ProcessingStage::READ_DATA_SEND_ADDR){
                    cout << "READ_DATA_SEND_ADDR" << endl;
                    cout << "current_layers " << current_layers << endl;
                    cout << "addr " << 1+(current_layers*3) << endl;
                    auto results = lm_read(1+(current_layers*3));
                    input_count = results[0];
                    group_count = results[1];
                    group_index = results[2];
                    cout << "input_count1 " << input_count << endl;
                    cout << "group_count " << group_count << endl;
                    cout << "group_index " << group_index << endl;
                    index_calc = 0;
                    stage = ProcessingStage::CALCULATE;
                    wait();
                    continue;
                }
                if (stage == ProcessingStage::CALCULATE){
                    current_group = group_index;
                    //                    for (int i = 0; i < 1; ++i) {
                    math_reset.write(true);
                    math_enable.write(false);
                    wait();
                    math_reset.write(false);
                    math_enable.write(true);

                    int temp_size = 0;
                    while (temp_size < (input_count*2+7)) {
                        cout << "data_offset " << data_offset << endl;
                        const int addr = 1+(layers_count*3)+data_offset+(index_calc*(input_count*2))+(temp_size);
                        cout << "input_count "<< input_count << endl;
                        cout << "temp_size "<< temp_size << endl;
                        cout << "addr "<< addr << endl;
                        cout << "offset_m "<< (index_calc*(input_count*2)) << endl;

                        auto data_from_memory = lm_read(addr);
                        for (int j = 0; j < data_from_memory.size(); ++j) {
                            cout << "data_from_memory[" << addr+j << "] " << data_from_memory[j] << endl;
                        }
                        for (int k = 0; k < (POCKET_SIZE/2); ++k) {
                            cout << "k " << (k * 2) + 1 << endl;
                            if ((temp_size/2)+k < input_count) {
                                cout << "math_inputs["<< index_core <<"]("<<(temp_size/2)+k<<"): " << data_from_memory[k * 2] << endl;
                                cout << "math_weights["<< index_core <<"]("<<(temp_size/2)+k<<"): " << data_from_memory[(k * 2) + 1] << endl;
                                math_inputs[k].write(data_from_memory[k * 2]);
                                math_weights[k].write(data_from_memory[(k * 2) + 1]);
                            }
                            else{
                                cout << "math_inputs["<< index_core <<"]("<<(temp_size/2)+k<<"): " << 0 << endl;
                                cout << "math_weights["<< index_core <<"]("<<(temp_size/2)+k<<"): " << 0 << endl;
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

                    cout << "test data_offset" << data_offset << endl;
                    cout << "test (2*group_count*input_count)" << (2*group_count*input_count) << endl;
                    cout << "test (group_index)" << (group_index) << endl;
                    sc_uint<ADDR_BITS> addr = 1+(layers_count*3) + data_offset + (2*group_count*input_count) + (index_calc*POCKET_SIZE) + (index_core*2);
                    cout << "test data_math " << math_output.read() << endl;
                    cout << "test addr_math " << addr << endl;

                    lm_write(res_addr, math_output.read());
                    cout << "Result ADDR "<< addr << endl;
                    cout << "Result["<< index_core <<"]: " << math_output.read() << endl;
                    send_data_to_pe_cores(addr,math_output.read());
                    index_calc++;
                    if (index_calc < group_count){
                        wait();
                        continue;
                    }

                    stage = ProcessingStage::WAIT_DATA;
                    data_offset += group_count*input_count;
                    wait();
                    continue;
                }
                if (stage == ProcessingStage::WAIT_DATA){
                    bool is_wait = false;
                    for (const auto & j : sn_wr_i) {
                        if (j.read()){
                            is_wait = true;
                        }
                    }
                    if (busy_write_data || is_wait || sn_wr){
                        wait();
                        continue;
                    }
                    cout << "go next" << endl;
                    current_layers++;
                    if (current_layers >= layers_count) {
                        stage = ProcessingStage::IDLE;
                    }
                    else{
                        stage = ProcessingStage::READ_DATA_SEND_ADDR;
//                        stage = ProcessingStage::IDLE;
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

    bool busy_write_data;
    void read_data_to_pe_cores(){
        busy_write_data = true;
        std::vector<sc_uint<ADDR_BITS>> addr_vec;
        std::vector<float> data_vec;
        for (int i = 0; i < (PE_CORES-1); ++i) {
            if (sn_wr_i[i].read()){
                cout << "sn_index_i " << sn_index_i[i].read() << endl;
                cout << "read_data_to_pe_cores " << sn_data_i[i].read() << endl;
//                lm_write(sn_index_i[i].read(),sn_data_i[i].read());
                addr_vec.push_back(sn_index_i[i].read());
                data_vec.push_back(sn_data_i[i].read());
            }
        }
        for (int i = 0; i < addr_vec.size(); ++i) {
            lm_write(addr_vec[i],data_vec[i]);
        }
        busy_write_data = false;
    }

    void send_data_to_pe_cores(sc_uint<ADDR_BITS> index,float data){
        sn_wr.write(true);
        sn_index.write(index);
        sn_data.write(data);
    }

    void lm_write(sc_uint<ADDR_BITS> addr_wr, float data_wr){
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
