#include "systemc.h"

template<int ADDR_BITS, int DATA_BITS>
SC_MODULE(Memory) {
    sc_in<bool> clk;
    sc_in<bool> enable;
    sc_in<bool> wr;
    sc_in<bool> rd;
    sc_in<sc_uint<ADDR_BITS>> address;
    sc_in<sc_uint<DATA_BITS>> data_in;
    sc_out<sc_uint<DATA_BITS>> data_out;

    // Локальная память
    sc_uint<DATA_BITS> mem[(1 << ADDR_BITS)];

    void proc(){
        if (!enable.read()) return;

    }

    // Конструктор модуля
    SC_CTOR(Memory) {
        SC_METHOD(proc)
        sensitive << clk.pos(); // Процесс срабатывает на положительный фронт сигнала тактирования
    }
};
