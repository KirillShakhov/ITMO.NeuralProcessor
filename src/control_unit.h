#include <systemc.h>
#include <cmath>

enum class ControlUnitStage {
    GET_CONFIG,
    SEND_INPUTS,
    SEND_WEIGHTS,
    IDLE
};

template<int ADDR_BITS, int SHARED_MEMORY_OFFSET, int PE_CORE>
SC_MODULE(ControlUnit) {
    sc_in_clk clk_i;
    sc_in<bool> enable;
    sc_out<bool> bus_rd;
    sc_out<bool> bus_wr;
    sc_out<sc_uint<ADDR_BITS>> bus_addr;
    sc_out<float> bus_data_in;
    sc_in<float> bus_data_out;


    ControlUnitStage stage;

    // Config
    sc_uint<ADDR_BITS> result_addr;
    int input_size;
    int weight_size;
    std::vector<int> weights_layers;

    void process() {
        while (true) {
            if (!enable) {
                stage = ControlUnitStage::GET_CONFIG;
                wait();
                continue;
            }
            bus_wr.write(false);
            bus_rd.write(false);
            if (stage == ControlUnitStage::GET_CONFIG) {
                result_addr = read(SHARED_MEMORY_OFFSET);
                input_size = read(SHARED_MEMORY_OFFSET+1);
                cout << "result_addr: " << result_addr << endl;
                cout << "input_size: " << input_size << endl;
                weight_size = read(SHARED_MEMORY_OFFSET + 2 + input_size);
                cout << "weight_size: " << weight_size << endl;
                for (int i = 0; i < weight_size; ++i) {
                    int w = read(SHARED_MEMORY_OFFSET + 2 + input_size + 1 + i);
                    cout << "weights_layer: " << w << endl;
                    weights_layers.push_back(w);
                }
                stage = ControlUnitStage::SEND_INPUTS;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::SEND_INPUTS) {
                for (int i = 0; i < PE_CORE; ++i) {
                    for (int j = 0; j < input_size; ++j) {
                        float in = read(SHARED_MEMORY_OFFSET + 2 + j);
                        cout << "in " << in << endl;
                        write((0x1000*(i+1))+3+(j*2), in);
                    }
                }
                stage = ControlUnitStage::SEND_WEIGHTS;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::SEND_WEIGHTS) {
                bool next;
                for (int j = 0; j < 1; ++j) {
                    int layer_size = input_size;
                    for (int i = 0; i < PE_CORE; ++i) {
                        int first_index_neuron = weights_layers[j]/PE_CORE*i;
                        int first_address = 0x1000*(i+1);
                        cout << "first_index_neuron " << first_index_neuron << endl;
                        cout << "first_address " << first_address << endl;
                        write(first_address, first_index_neuron);
                        write(first_address+1, 2000);
                        write(first_address+2, layer_size);
                        for (int k = 0; k < weights_layers[j]; ++k) {
                            float w = read(SHARED_MEMORY_OFFSET + 2 + layer_size + 1 + weight_size + k*layer_size + i*(weights_layers[j]*layer_size/PE_CORE));
                            cout << "w" << k << ": " << w << endl;
                            cout << "addr " << first_address+3+(k*2)+1 << endl;
                            write(first_address+3+(k*2)+1, w);
                        }
                    }
                    write(0x100, 1);
                    bus_addr.write(111);
                    bus_wr.write(false);
                    bus_rd.write(false);
                }
                stage = ControlUnitStage::IDLE;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::IDLE) {
                wait();
                continue;
            }
            wait();
        }
    }

    float read(sc_uint<ADDR_BITS> addr){
        bus_addr.write(addr);
        bus_rd.write(true);
        bus_wr.write(false);
        wait();
        wait();
        bus_rd.write(false);
        return bus_data_out.read();
    }

    void write(sc_uint<ADDR_BITS> addr, float data){
        bus_addr.write(addr);
        bus_data_in.write(data);
        bus_rd.write(false);
        bus_wr.write(true);
        wait();
        bus_wr.write(false);
    }


    SC_CTOR(ControlUnit) {
        SC_THREAD(process);
        sensitive << clk_i.pos();
    }
};
