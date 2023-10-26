#include <systemc.h>
#include <cmath>

enum class ControlUnitStage {
    GET_CONFIG_SEND_ADDR,
    GET_CONFIG_GET_DATA,
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
    void process() {
        if (!enable) {
            start_addr = 0;
            return;
        }
        switch (stage) {
            case ControlUnitStage::GET_CONFIG_SEND_ADDR:
                bus_addr.write(start_addr);
                bus_wr.write(false);
                bus_rd.write(true);
                start_addr++;
                stage = ControlUnitStage::GET_CONFIG_GET_DATA;
                return;
            case ControlUnitStage::GET_CONFIG_GET_DATA:
                cout << "data: " << bus_data_out.read() << endl;
                bus_addr.write(0x10);
                bus_wr.write(true);
                return;
        }
    }

    void start(){
        cout << "Control Unit Started" << endl;
        stop();
    }

    void stop(){

    }

    SC_CTOR(ControlUnit) {
        SC_METHOD(process);
        sensitive << clk_i.pos();
    }
};
