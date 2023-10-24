// channel.h
#include "systemc.h"

template<int ADDR_BITS, int DATA_BITS>
class bus_interface : public sc_interface {
public:
    virtual void write(sc_uint<ADDR_BITS> addr,sc_uint<DATA_BITS> data) = 0;
    virtual sc_uint<DATA_BITS> read() = 0;
};

template<int ADDR_BITS, int DATA_BITS>
class bus_channel : public bus_interface<ADDR_BITS, DATA_BITS>, public sc_prim_channel {
    sc_uint<ADDR_BITS> addr;
    sc_uint<DATA_BITS> data;
    bool wr;
    bool rd;
public:
    void write(sc_uint<ADDR_BITS> addr, sc_uint<DATA_BITS> data) override {
        this->addr = addr;
        this->data = data;
        this->wr = true;
    }

    sc_uint<DATA_BITS> read() override {
        return data;
    }

//    sc_uint<ADDR_BITS> readAddr() override {
//        return addr;
//    }
};