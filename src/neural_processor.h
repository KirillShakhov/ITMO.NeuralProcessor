#include <systemc.h>
#include "pe_core.h"
#include "shared_memory.h"
#include "local_memory.h"
#include "io_module.h"
#include "bus_multiplexer.h"
#include "control_unit.h"

template<int ADDR_BITS, int DATA_BITS, int PE_CORES, int POCKET_SIZE>
SC_MODULE(NeuralProcessor) {
    sc_in<bool> clk_i;

    std::vector<PeCore<ADDR_BITS, POCKET_SIZE, PE_CORES>*> cores;
    std::vector<LocalMemory<ADDR_BITS, POCKET_SIZE>*> local_memories;

    sc_vector<sc_signal<bool>> local_memory_enable{"local_memory_enable", PE_CORES};
    sc_vector<sc_signal<sc_uint<ADDR_BITS>>> local_memory_addr{"local_memory_addr", PE_CORES};
    sc_vector<sc_signal<bool>> local_memory_rd{"local_memory_rd", PE_CORES};
    sc_vector<sc_signal<bool>> local_memory_single_channel{"local_memory_single_channel", PE_CORES};
    sc_vector<sc_signal<bool>> local_memory_wr{"local_memory_wr", PE_CORES};

    std::vector<sc_vector<sc_signal<float>>*> local_memory_data_in;
    std::vector<sc_vector<sc_signal<float>>*> local_memory_data_out;

    sc_signal<bool, SC_MANY_WRITERS> bus_rd{"bus_rd"};
    sc_signal<bool, SC_MANY_WRITERS> bus_wr{"bus_wr"};
    sc_signal<sc_uint<ADDR_BITS>, SC_MANY_WRITERS> bus_addr{"bus_addr"};
    sc_signal<float, SC_MANY_WRITERS> bus_data_in{"bus_data_in"};
    sc_signal<float, SC_MANY_WRITERS> bus_data_out{"bus_data_out"};

    sc_signal<bool> io_enable{"io_enable"};
    sc_signal<bool> io_bus_rd{"io_bus_rd"};
    sc_signal<bool> io_bus_wr{"io_bus_wr"};
    sc_signal<sc_uint<ADDR_BITS>> io_bus_addr{"io_bus_addr"};
    sc_signal<float> io_bus_data_in{"io_bus_data_in"};

    sc_signal<bool> cu_enable{"cu_enable"};
    sc_signal<bool> cu_bus_rd{"cu_bus_rd"};
    sc_signal<bool> cu_bus_wr{"cu_bus_wr"};
    sc_signal<sc_uint<ADDR_BITS>> cu_bus_addr{"cu_bus_addr"};
    sc_signal<float> cu_bus_data_in{"cu_bus_data_in"};

    static const int SHARED_MEMORY_OFFSET = 0x8000;

    IoModule<ADDR_BITS> ioModule{"IoModule"};
    ControlUnit<ADDR_BITS, SHARED_MEMORY_OFFSET, PE_CORES> controlUnit{"ControlUnit"};
    SharedMemory<ADDR_BITS, SHARED_MEMORY_OFFSET> sharedMemory{"SharedMemory"};
    BusMultiplexer<ADDR_BITS, 0x10> busMultiplexer{"busMultiplexer"};

    std::vector<sc_signal<bool>*> sn_wr;
    std::vector<sc_signal<sc_uint<ADDR_BITS>>*> sn_index;
    std::vector<sc_signal<float>*> sn_data;

    void process(){
        if (bus_addr.read() == 0x0){
            cout << "Start" << endl;
//            bus_addr.write(10);
        }
    }

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
            std::string sn_wr_name = "sn_wr_" + std::to_string(i+1);
            sn_wr.push_back(new sc_signal<bool>(
                    sn_wr_name.c_str(),
                    POCKET_SIZE
            ));
            std::string sn_index_name = "sn_index_" + std::to_string(i+1);
            sn_index.push_back(new sc_signal<sc_uint<ADDR_BITS>>(
                    sn_index_name.c_str(),
                    POCKET_SIZE
            ));
            std::string sn_data_name = "sn_data_" + std::to_string(i+1);
            sn_data.push_back(new sc_signal<float>(
                    sn_data_name.c_str(),
                    POCKET_SIZE
            ));
        }

        for (int i = 0; i < PE_CORES; ++i) {
            cores.push_back(new PeCore<ADDR_BITS, POCKET_SIZE, PE_CORES>(("core" + std::to_string(i)).c_str()));
            local_memories.push_back(new LocalMemory<16, POCKET_SIZE>(("local_memory" + std::to_string(i)).c_str()));


            local_memories[i]->clk(clk_i);
            local_memories[i]->enable(local_memory_enable[i]);
            local_memories[i]->address(local_memory_addr[i]);
            local_memories[i]->rd(local_memory_rd[i]);
            local_memories[i]->wr(local_memory_wr[i]);
            local_memories[i]->single_channel(local_memory_single_channel[i]);
            local_memories[i]->data_in(*local_memory_data_in[i]);
            local_memories[i]->data_out(*local_memory_data_out[i]);

            cores[i]->init(i);
            cores[i]->clk_i(clk_i);
            // Local Memory
            cores[i]->local_memory_enable(local_memory_enable[i]);
            cores[i]->local_memory_addr(local_memory_addr[i]);
            cores[i]->local_memory_wr(local_memory_wr[i]);
            cores[i]->local_memory_rd(local_memory_rd[i]);
            cores[i]->local_memory_single_channel(local_memory_single_channel[i]);
            cores[i]->local_memory_data_bi(*local_memory_data_out[i]);
            cores[i]->local_memory_data_bo(*local_memory_data_in[i]);
            // Bus
            cores[i]->bus_wr(bus_wr);
            cores[i]->bus_rd(bus_rd);
            cores[i]->bus_addr(bus_addr);
            cores[i]->bus_data_in(bus_data_in);
            cores[i]->bus_data_out(bus_data_out);
            //
            cores[i]->sn_wr(*sn_wr[i]);
            cores[i]->sn_index(*sn_index[i]);
            cores[i]->sn_data(*sn_data[i]);

            for (int j = 0; j < PE_CORES; ++j) {
                if (i == j) continue;
                int index = (i < j) ? j-1 : j;
                cores[i]->sn_wr_i[index](*sn_wr[j]);
                cores[i]->sn_index_i[index](*sn_index[j]);
                cores[i]->sn_data_i[index](*sn_data[j]);
            }
        }

        ioModule.clk_i(clk_i);
        ioModule.enable(io_enable);
        ioModule.bus_addr(io_bus_addr);
        ioModule.bus_rd(io_bus_rd);
        ioModule.bus_wr(io_bus_wr);
        ioModule.bus_data_in(io_bus_data_in);
        ioModule.bus_data_out(bus_data_out);

        controlUnit.clk_i(clk_i);
        controlUnit.enable(cu_enable);
        controlUnit.bus_addr(cu_bus_addr);
        controlUnit.bus_rd(cu_bus_rd);
        controlUnit.bus_wr(cu_bus_wr);
        controlUnit.bus_data_in(cu_bus_data_in);
        controlUnit.bus_data_out(bus_data_out);

        busMultiplexer.bus_addr(bus_addr);
        busMultiplexer.bus_rd(bus_rd);
        busMultiplexer.bus_wr(bus_wr);
        busMultiplexer.bus_data_in(bus_data_in);
        busMultiplexer.bus_data_out(bus_data_out);

        busMultiplexer.module1_enable(io_enable);
        busMultiplexer.module1_addr(io_bus_addr);
        busMultiplexer.module1_rd(io_bus_rd);
        busMultiplexer.module1_wr(io_bus_wr);
        busMultiplexer.module1_data_in(io_bus_data_in);

        busMultiplexer.module2_enable(cu_enable);
        busMultiplexer.module2_addr(cu_bus_addr);
        busMultiplexer.module2_rd(cu_bus_rd);
        busMultiplexer.module2_wr(cu_bus_wr);
        busMultiplexer.module2_data_in(cu_bus_data_in);

        sharedMemory.clk_i(clk_i);
        sharedMemory.bus_addr(bus_addr);
        sharedMemory.bus_rd(bus_rd);
        sharedMemory.bus_wr(bus_wr);
        sharedMemory.bus_data_in(bus_data_in);
        sharedMemory.bus_data_out(bus_data_out);

        SC_METHOD(process);
        sensitive << clk_i.pos();
    }
};
