#include <systemc.h>
#include <cmath>

enum class ControlUnitStage {
    GET_RESULT_ADDR_SEND,
    GET_RESULT_ADDR_GET,
    GET_CONFIG_SEND_ADDR,
    GET_CONFIG_GET_DATA,
    GET_CONFIG_INPUT_SIZE,
    GET_CONFIG_WEIGHT_COUNT,
    GET_CONFIG_WEIGHT_SIZE,
    ACTIVATE_PE_CORE,
    WRITE_WEIGHT,
    CALCULATE,
    STOP_WRITE_IN_CORE,
    SEND_INPUT_SIZE,
    SEND_INPUT,
    IDLE,
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
    int start_addr = 0;
    bool wait;


    // Config
    sc_uint<ADDR_BITS> result_addr;
    int input_size;
    int weight_size;
    std::vector<int> weights_layers;

    int core_loaded;
    int data_loaded;
    void process() {
        if (!enable) {
            start_addr = 0;
            stage = ControlUnitStage::GET_RESULT_ADDR_SEND;
            return;
        }
        bus_wr.write(false);
        bus_rd.write(false);
        if (wait){
            wait = false;
            return;
        }
        if (stage == ControlUnitStage::GET_RESULT_ADDR_SEND){
            bus_addr.write(SHARED_MEMORY_OFFSET);
            bus_rd.write(true);
            stage = ControlUnitStage::GET_RESULT_ADDR_GET;
            wait = true;
            return;
        }
        if (stage == ControlUnitStage::GET_RESULT_ADDR_GET){
            result_addr = bus_data_out.read();
            cout << "result_addr: " << result_addr << endl;
            bus_addr.write(SHARED_MEMORY_OFFSET+1);
            bus_rd.write(true);
            stage = ControlUnitStage::GET_CONFIG_INPUT_SIZE;
            wait = true;
            return;
        }
        if (stage == ControlUnitStage::GET_CONFIG_INPUT_SIZE){
            input_size = bus_data_out.read();
            cout << "input_size: " << input_size << endl;
            bus_addr.write(SHARED_MEMORY_OFFSET+2+input_size);
            bus_rd.write(true);
            stage = ControlUnitStage::GET_CONFIG_WEIGHT_SIZE;
            wait = true;
            return;
        }
        if (stage == ControlUnitStage::GET_CONFIG_WEIGHT_SIZE){
            weight_size = bus_data_out.read();
            cout << "weight_size: " << weight_size << endl;
            data_loaded = 0;
            stage = ControlUnitStage::GET_CONFIG_WEIGHT_COUNT;

            bus_rd.write(true);
            bus_addr.write(SHARED_MEMORY_OFFSET+2+input_size+1+data_loaded);
            data_loaded++;
            wait = true;
            return;
        }
        if (stage == ControlUnitStage::GET_CONFIG_WEIGHT_COUNT){
            if (data_loaded > weight_size) {
                bus_rd.write(false);
//                 pe activates
                bus_addr.write(0x0F00);
                bus_wr.write(true);
                bus_data_in.write(0x0);
                stage = ControlUnitStage::SEND_INPUT_SIZE;
                return;
            }
            bus_rd.write(true);
            bus_addr.write(SHARED_MEMORY_OFFSET+2+input_size+1+data_loaded);
            int weight_layer = bus_data_out.read();
            weights_layers.push_back(weight_layer);
            cout << "weight_layer: " << bus_data_out.read() << endl;
            data_loaded++;
            wait = true;
            return;
        }
        if (stage == ControlUnitStage::SEND_INPUT_SIZE){
            data_loaded = 0;
            bus_addr.write(SHARED_MEMORY_OFFSET+1);
            bus_rd.write(true);
            bus_data_in.write(input_size);
            stage = ControlUnitStage::SEND_INPUT;
            return;
        }
        if (stage == ControlUnitStage::SEND_INPUT){
            bus_rd.write(true);
            bus_addr.write(SHARED_MEMORY_OFFSET+(data_loaded));
            data_loaded++;
            if (data_loaded > input_size + 2 + 1){
                cout << "STOP" << endl;
                stage = ControlUnitStage::STOP_WRITE_IN_CORE;
            }
        }
        if (stage == ControlUnitStage::STOP_WRITE_IN_CORE) {
            bus_addr.write(0x0F01);
            bus_wr.write(true);
            stage = ControlUnitStage::WRITE_WEIGHT;
            core_loaded = 0;
            data_loaded = 0;
            return;
        }
        if (stage == ControlUnitStage::WRITE_WEIGHT) {
            if (core_loaded > PE_CORE){

            }
            return;
        }
        if (stage == ControlUnitStage::CALCULATE) {
            bus_addr.write(0x102);
            bus_wr.write(true);
            stage = ControlUnitStage::IDLE;
            return;
        }
        if (stage == ControlUnitStage::IDLE) {
            return;
        }
    }

    SC_CTOR(ControlUnit) {
        SC_METHOD(process);
        sensitive << clk_i.pos();
    }
};
