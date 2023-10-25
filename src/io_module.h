#include <systemc.h>
#include <queue>

template<int ADDR_BITS>
SC_MODULE(IoModule) {
    sc_in_clk clk_i;
    sc_out<bool> bus_rd;
    sc_out<bool> bus_wr;
    sc_out<sc_uint<ADDR_BITS>> bus_addr;
    sc_out<float> bus_data_in{"bus_data_in"};
    sc_in<float> bus_data_out{"bus_data_out"};

    struct BusData {
        sc_uint<ADDR_BITS> addr;
        float data;
    };

    std::vector<BusData> data_vector;
    size_t current_index = 0;
    void process() {
//        cout << "Process triggered at " << sc_time_stamp() << endl; // Добавьте эту строку

        if (current_index < data_vector.size()) {
            BusData& current_data = data_vector[current_index];
            bus_addr.write(current_data.addr);
            bus_data_in.write(current_data.data);
            bus_wr.write(true);
            current_index++;
        }
    }

    SC_CTOR(IoModule) {
        SC_METHOD(process);
        sensitive << clk_i.pos();

        data_vector.push_back(BusData{0, 0.9990631251789995});
        data_vector.push_back(BusData{1, 0.7550705154311167});
        data_vector.push_back(BusData{2, 0.9950311958727136});
        data_vector.push_back(BusData{3, 0.6481739272549456});
        data_vector.push_back(BusData{4, 0.9965130706156015});
        data_vector.push_back(BusData{5, -0.03327157936852162});
        data_vector.push_back(BusData{6, 0.9987707591591141});
        data_vector.push_back(BusData{7, -0.42765877612887726});

        data_vector.push_back(BusData{8, 0.999039373995782});
        data_vector.push_back(BusData{9, 0.2268078469768019});
        data_vector.push_back(BusData{10, 0.9572634478083041});
        data_vector.push_back(BusData{11, 0.5068606953692869});
        data_vector.push_back(BusData{12, 0.007089524427052066});
        data_vector.push_back(BusData{13, 0.7370486058746694});
        data_vector.push_back(BusData{14, 0.9973003050392526});
        data_vector.push_back(BusData{15, 0.5651521802118312});

        data_vector.push_back(BusData{16, 0.9882247596998172});
        data_vector.push_back(BusData{17, 0.11509488104106606});
        data_vector.push_back(BusData{18, 0.9012400716710605});
        data_vector.push_back(BusData{19, 1.1424770795739387});
        data_vector.push_back(BusData{20, 0.9683287440476079});
        data_vector.push_back(BusData{21, 3.1055546438514123});
        data_vector.push_back(BusData{22, 0.9964592661223179});
        data_vector.push_back(BusData{23, 0.7880725968841713});

        data_vector.push_back(BusData{24, 0.2479846432195069});
        data_vector.push_back(BusData{25, 0.3983616222528344});
        data_vector.push_back(BusData{26, 0.003428754170688519});
        data_vector.push_back(BusData{27, 0.8743129908062628});
        data_vector.push_back(BusData{28, 0.9931021038679053});
        data_vector.push_back(BusData{29, 0.269989261270187});
        data_vector.push_back(BusData{30, 0.9993619882449931});
        data_vector.push_back(BusData{31, -0.24792302708954958});

        for (int i = 0; i < data_vector.size(); ++i) {
            data_vector[i].addr = data_vector[i].addr + 0x8000;
        }
    }
};
