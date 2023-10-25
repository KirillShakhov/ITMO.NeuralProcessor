#include <systemc.h>

template<int ADDR_BITS, int ADDR>
SC_MODULE(BusMultiplexer) {
    // Входные порты для модуля 1
    sc_out<bool> module1_enable;
    sc_in<bool> module1_rd;
    sc_in<bool> module1_wr;
    sc_in<sc_uint<ADDR_BITS>> module1_addr;
    sc_in<float> module1_data_in;

    // Входные порты для модуля 2
    sc_out<bool> module2_enable;
    sc_in<bool> module2_rd;
    sc_in<bool> module2_wr;
    sc_in<sc_uint<ADDR_BITS>> module2_addr;
    sc_in<float> module2_data_in;

    // Выходные порты
    sc_out<bool> bus_rd;
    sc_out<bool> bus_wr;
    sc_out<sc_uint<ADDR_BITS>> bus_addr;
    sc_out<float> bus_data_in;
    sc_in<float> bus_data_out;

    // Сигнал для выбора активного модуля (0 = модуль 1, 1 = модуль 2)
    bool select = false;
    void process() {
        bool rd;
        bool wr;
        sc_uint<ADDR_BITS> addr;
        float data;
        if (!select) { // Если модуль 1 активен
            rd = module1_rd.read();
            wr = module1_wr.read();
            addr = module1_addr.read();
            data = module1_data_in.read();
        } else { // Если модуль 2 активен
            rd = module2_rd.read();
            wr = module2_wr.read();
            addr = module2_addr.read();
            data = module2_data_in.read();
        }
        if (addr == ADDR) {
            cout << "BusMultiplexer choose module 1" << endl;
            select = false;
        }
        if (addr == ADDR+1) {
            cout << "BusMultiplexer choose module 2" << endl;
            select = true;
        }
        bus_rd.write(rd);
        bus_wr.write(wr);
        bus_addr.write(addr);
        bus_data_in.write(data);
        module1_enable.write(!select);
        module2_enable.write(select);
    }

    SC_CTOR(BusMultiplexer) {
        SC_METHOD(process);
        sensitive << module1_rd << module1_wr << module1_addr << module1_data_in
                  << module2_rd << module2_wr << module2_addr << module2_data_in;
    }
};