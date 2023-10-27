#include <systemc.h>
#include <cmath>

enum class ControlUnitStage {
    GET_CONFIG,
    SEND_INPUTS,
    SEND_WEIGHTS,
    START_CORES,
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
    int service_info = 0;

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
                input_size = read(SHARED_MEMORY_OFFSET + 1);
                cout << "result_addr: " << result_addr << endl;
                cout << "input_size: " << input_size << endl;
                weight_size = read(SHARED_MEMORY_OFFSET + 2 + input_size);
                cout << "weight_size: " << weight_size << endl;
                for (int i = 0; i < weight_size; ++i) {
                    int w = read(SHARED_MEMORY_OFFSET + 2 + input_size + 1 + i);
                    cout << "weights_layer: " << w << endl;
                    weights_layers.push_back(w);
                }
                stage = ControlUnitStage::SEND_WEIGHTS;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::SEND_WEIGHTS) {


                int current_core = 0;
                cout << "get_layer_size() " << get_layer_size() << endl;
                for (int layer_index = 0; layer_index < get_layer_size(); ++layer_index) {
                    cout << "wright_count " << get_layer_weight_count(layer_index) << endl;
                    int weights_per_count = get_layer_weight_count(layer_index) / PE_CORE;
                    cout << "weights_per_count " << weights_per_count << endl;
                    cout << "groups " << weights_layers[layer_index] << endl;

                }

                std::vector<int> offsetCores(PE_CORE, 0); // Создаем вектор offset для каждого coreIndex

                int totalWeightsProcessed = 0; // Общее количество обработанных весов
                int layerIndex = 0; // Индекс текущего слоя

                for (int i = 0; i < PE_CORE; ++i) {
                    cout << "weight_size " << weight_size << endl;
                    write(getFirstAddressCore(i), weight_size);
                }
                service_info = weight_size * 3;
                for (int j = 0; j < weight_size; ++j) {
                    for (int i = 0; i < PE_CORE; ++i) {
                        int input_count = get_layer_weight_count(j) / weights_layers[j];
                        write(getFirstAddressCore(i) + 1 + (j * 3), input_count);
                        cout << "input_count " << input_count << endl;

                        int group_count = (weights_layers[j] + PE_CORE - 1) / PE_CORE;
                        write(getFirstAddressCore(i) + 1 + 1 + (j * 3), group_count);
                        cout << "group_count " << group_count << endl;

                        int index_neuron = group_count * i;
                        cout << "neuron_index " << index_neuron << endl;
                        write(getFirstAddressCore(i) + 1 + 2 + (j * 3), index_neuron);
                    }
                }


                for (int i = 0; i < get_all_weight(); ++i) {
                    float weight = read(SHARED_MEMORY_OFFSET + 2 + input_size + 1 + weight_size + i);

                    // Получить количество весов в текущем слое
                    int weightsInLayer = get_layer_weight_count(layerIndex);

                    // Получить количество групп в текущем слое
                    int groupsInLayer = weights_layers[layerIndex];

                    // Проверить, если обработаны все веса в текущем слое
                    if (totalWeightsProcessed == weightsInLayer) {
                        // Перейти к следующему слою
                        layerIndex++;
                        totalWeightsProcessed = 0;

                        // Обновить количество весов и групп в новом слое
                        weightsInLayer = get_layer_weight_count(layerIndex);
                        groupsInLayer = weights_layers[layerIndex];
                    }

                    // Определить группу (нейрон) в слое для текущего веса
                    int groupIndex = totalWeightsProcessed / (weightsInLayer / groupsInLayer);
                    int weightInGroup = weightsInLayer / groupsInLayer;
                    int coreIndex = (totalWeightsProcessed / (weightsInLayer / groupsInLayer)) % PE_CORE;

//                     Теперь у вас есть `weight`, который относится к `layerIndex` слою и может быть внутри одной из `groupIndex` групп весов.


                    cout << "w" << i << ": " << weight << " layer: " << layerIndex << " group: " << groupIndex
                         << " core: " << coreIndex << endl;
                    offsetCores[coreIndex]++;
                    int offset = offsetCores[coreIndex];
                    cout << "offset " << offset << endl;
                    int first_address = 0x1000 * (coreIndex + 1);
                    write(first_address + service_info + (offset * 2), weight);

                    // Увеличить общее количество обработанных весов
                    totalWeightsProcessed++;
                }
                stage = ControlUnitStage::SEND_INPUTS;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::SEND_INPUTS) {

                cout << "service_info " << service_info << endl;

                int group_count = weights_layers[0] / PE_CORE;
                for (int i = 0; i < input_size; ++i) {
                    float in = read(SHARED_MEMORY_OFFSET + 2 + i);
                    cout << "in " << in << endl;
                    int offset_in_addr = 0;
                    for (int j = 0; j < group_count; ++j) {
                        for (int core_i = 0; core_i < PE_CORE; ++core_i) {
                            int first_address = 0x1000 * (core_i + 1);
                            int addr = first_address + service_info + 1 + offset_in_addr + (i*2);
                            cout << "group_count " << group_count << endl;
                            cout << "in_addr " << addr << endl;
                            write(addr, in);
                        }
                        offset_in_addr += (input_size * 2);
                    }
                }

                stage = ControlUnitStage::START_CORES;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::START_CORES) {
                write(0x100, 1);
                bus_addr.write(111);
                bus_wr.write(false);
                bus_rd.write(false);
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

    int get_all_weight() {
        int all_weight = 0;
        int last_layer = input_size;
        for (int i = 0; i < weight_size; ++i) {
            all_weight = all_weight + (last_layer * weights_layers[i]);
            last_layer = weights_layers[i];
        }
        return all_weight;
    }

    int getFirstAddressCore(int core_i) {
        return 0x1000 * (core_i + 1);
    }

    int get_layer_size() {
        return weight_size;
    }

    int get_layer_weight_count(int layer_index) {
        if (layer_index == 0) {
            return input_size * weights_layers[0];
        }
        return weights_layers[layer_index - 1] * weights_layers[layer_index];
    }

    int get_group_count(int layer_index) {
        return weights_layers[layer_index];
    }

    float read(sc_uint<ADDR_BITS> addr) {
        bus_addr.write(addr);
        bus_rd.write(true);
        bus_wr.write(false);
        wait();
        wait();
        bus_rd.write(false);
        return bus_data_out.read();
    }

    void write(sc_uint<ADDR_BITS> addr, float data) {
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
