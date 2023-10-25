#include <systemc.h>
#include "pe_core.h"
#include "local_memory.h"

template<int ADDR_BITS, int DATA_BITS, int PE_CORES, int POCKET_SIZE>
SC_MODULE(NeuralProcessor) {
    sc_in<bool> clk_i;

    void process() {

    }

    std::vector<PeCore<ADDR_BITS, DATA_BITS, POCKET_SIZE>*> cores;
    std::vector<LocalMemory<ADDR_BITS, DATA_BITS, POCKET_SIZE>*> local_memories;
    sc_vector<sc_signal<bool>> pe_core_rst{"pe_core_rst", PE_CORES};
    sc_vector<sc_signal<bool>> pe_core_enable{"pe_core_enable", PE_CORES};
    sc_vector<sc_signal<bool>> pe_core_busy{"pe_core_busy", PE_CORES};

    sc_vector<sc_signal<bool>> local_memory_enable{"local_memory_enable", PE_CORES};
    sc_vector<sc_signal<sc_uint<ADDR_BITS>>> local_memory_addr{"local_memory_addr", PE_CORES};
    sc_vector<sc_signal<bool>> local_memory_rd{"local_memory_rd", PE_CORES};
    sc_vector<sc_signal<bool>> local_memory_wr{"local_memory_wr", PE_CORES};

    std::vector<sc_vector<sc_signal<float>>*> local_memory_data_in;
    std::vector<sc_vector<sc_signal<float>>*> local_memory_data_out;

    sc_vector<sc_signal<bool>> bus_rd{"bus_rd", PE_CORES};
    sc_vector<sc_signal<bool>> bus_wr{"bus_wr", PE_CORES};
    sc_vector<sc_signal<sc_uint<ADDR_BITS>>> bus_addr{"bus_addr", PE_CORES};
    std::vector<sc_vector<sc_signal<float>>*> bus_data_in;
    std::vector<sc_vector<sc_signal<float>>*> bus_data_out;

    SC_CTOR(NeuralProcessor) {
        for (int i = 0; i < PE_CORES; ++i) {
            std::string out_name = "local_memory_data_out_" + std::to_string(i+1);
            local_memory_data_out.push_back(new sc_vector<sc_signal<float>>(
                    out_name.c_str(),
                    POCKET_SIZE
            ));
            std::string in_name = "local_memory_data_in_" + std::to_string(i+1);
            local_memory_data_in.push_back(new sc_vector<sc_signal<float>>(
                    in_name.c_str(),
                    POCKET_SIZE
            ));
            std::string in_bus_name = "bus_data_in_" + std::to_string(i+1);
            bus_data_in.push_back(new sc_vector<sc_signal<float>>(
                    in_bus_name.c_str(),
                    POCKET_SIZE
            ));
            std::string out_bus_name = "bus_data_out_" + std::to_string(i+1);
            bus_data_out.push_back(new sc_vector<sc_signal<float>>(
                    out_bus_name.c_str(),
                    POCKET_SIZE
            ));
        }

        for (int i = 0; i < PE_CORES; ++i) {
            cores.push_back(new PeCore<ADDR_BITS, DATA_BITS, POCKET_SIZE>(("core" + std::to_string(i)).c_str()));
            local_memories.push_back(new LocalMemory<16, DATA_BITS, POCKET_SIZE>(("local_memory" + std::to_string(i)).c_str()));


            local_memories[i]->clk(clk_i);
            local_memories[i]->enable(local_memory_enable[i]);
            local_memories[i]->address(local_memory_addr[i]);
            local_memories[i]->rd(local_memory_rd[i]);
            local_memories[i]->wr(local_memory_wr[i]);
            local_memories[i]->data_in(*local_memory_data_in[i]);
            local_memories[i]->data_out(*local_memory_data_out[i]);

            cores[i]->init(i);
            cores[i]->clk_i(clk_i);
            cores[i]->rst_i(pe_core_rst[i]);
            cores[i]->enable_i(pe_core_enable[i]);
            cores[i]->busy_o(pe_core_busy[i]);
            // Local Memory
            cores[i]->local_memory_enable(local_memory_enable[i]);
            cores[i]->local_memory_addr(local_memory_addr[i]);
            cores[i]->local_memory_wr(local_memory_wr[i]);
            cores[i]->local_memory_rd(local_memory_rd[i]);
            cores[i]->local_memory_data_bi(*local_memory_data_out[i]);
            cores[i]->local_memory_data_bo(*local_memory_data_in[i]);
            // Bus
            cores[i]->bus_wr(bus_wr[i]);
            cores[i]->bus_rd(bus_rd[i]);
            cores[i]->bus_addr(bus_addr[i]);
            cores[i]->bus_data_in(*bus_data_in[i]);
            cores[i]->bus_data_out(*bus_data_out[i]);
        }



        SC_METHOD(process);
        sensitive << clk_i.pos();
    }

};
