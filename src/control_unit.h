#include <systemc.h>
#include <cmath>

enum class ControlUnitStage {
    GET_RESULT_ADDR_SEND,
    GET_RESULT_ADDR_GET,
    GET_CONFIG_SEND_ADDR,
    GET_CONFIG_GET_DATA,
    GET_CONFIG_INPUT_SIZE,
    GET_CONFIG_WEIGHT_SIZE,
    ACTIVATE_PE_CORE,
    CALCULATE,
    IDLE,
};

template<int ADDR_BITS, int SHARED_MEMORY_OFFSET>
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
            stage = ControlUnitStage::ACTIVATE_PE_CORE;
//            bus_addr.write(SHARED_MEMORY_OFFSET+1+input_size);
//            bus_rd.write(true);
//            stage = ControlUnitStage::GET_CONFIG_WEIGHT_SIZE;
            return;
        }
        if (stage == ControlUnitStage::ACTIVATE_PE_CORE){
            bus_addr.write(0x100);
            bus_wr.write(true);
            stage = ControlUnitStage::CALCULATE;
        }
        if (stage == ControlUnitStage::CALCULATE){

        }
        return;
        switch (stage) {
            case ControlUnitStage::GET_RESULT_ADDR_SEND:
                stage = ControlUnitStage::GET_CONFIG_SEND_ADDR;
                return;
            case ControlUnitStage::GET_CONFIG_SEND_ADDR:
//                sh_send_addr(start_addr);
                bus_addr.write(SHARED_MEMORY_OFFSET);
                bus_wr.write(false);
                bus_rd.write(true);

                start_addr++;
                stage = ControlUnitStage::GET_CONFIG_GET_DATA;
                return;
            case ControlUnitStage::GET_CONFIG_GET_DATA:
                cout << "data: " << bus_data_out.read() << endl;
//                stop();
                return;
        }
    }

    void sh_send_addr(sc_uint<ADDR_BITS> addr){
    }

    void stop(){
        bus_addr.write(0x10);
        bus_wr.write(true);
    }

    SC_CTOR(ControlUnit) {
        SC_METHOD(process);
        sensitive << clk_i.pos();
    }
};
