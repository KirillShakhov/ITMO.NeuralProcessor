#include <systemc.h>
#include <cmath>

enum class ControlUnitStage {
    GET_CONFIG,
    SEND_INPUTS,
    SEND_WEIGHTS,
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
                input_size = read(SHARED_MEMORY_OFFSET+1);
                cout << "result_addr: " << result_addr << endl;
                cout << "input_size: " << input_size << endl;
                weight_size = read(SHARED_MEMORY_OFFSET + 2 + input_size);
                cout << "weight_size: " << weight_size << endl;
                for (int i = 0; i < weight_size; ++i) {
                    int w = read(SHARED_MEMORY_OFFSET + 2 + input_size + 1 + i);
                    cout << "weights_layer: " << w << endl;
                    weights_layers.push_back(w);
                }
                stage = ControlUnitStage::SEND_INPUTS;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::SEND_INPUTS) {
                for (int core_i = 0; core_i < PE_CORE; ++core_i) {
                    for (int j = 0; j < input_size; ++j) {
                        float in = read(SHARED_MEMORY_OFFSET + 2 + j);
                        cout << "in " << in << endl;
                        int first_address = 0x1000*(core_i + 1);
                        const int service_info_count = 4;
                        write(first_address+service_info_count+(j*2), in);
                    }
                }
                stage = ControlUnitStage::SEND_WEIGHTS;
                wait();
                continue;
            }
            if (stage == ControlUnitStage::SEND_WEIGHTS) {
//                int all_weight = get_all_weight();
//                cout << "all_weight " << all_weight << endl;
//                cout << "get_layer_size() " << get_layer_size() << endl;
//                for (int i = 0; i < get_layer_size(); ++i) {
//                    cout << "get_layer_wright_count" << get_layer_wright_count(i) << endl;
//                }
//
//                int tmp = input_size;
//                for (int i = 0; i < all_weight; ++i) {
//                    float weight = read(SHARED_MEMORY_OFFSET + 2 + input_size + 1 + weight_size + i);
//
//
//                }



                int current_core = 0;
                cout << "get_layer_size() " <<  get_layer_size() << endl;
                for (int layer_index = 0; layer_index < get_layer_size(); ++layer_index) {
                    cout << "wright_count " <<  get_layer_weight_count(layer_index) << endl;
                    int weights_per_count = get_layer_weight_count(layer_index)/PE_CORE;
                    cout << "weights_per_count " <<  weights_per_count << endl;
                    cout << "groups " <<  weights_layers[layer_index] << endl;

                }

                std::vector<int> offsetCores(PE_CORE, 0); // Создаем вектор offset для каждого coreIndex

                int totalWeightsProcessed = 0; // Общее количество обработанных весов
                int layerIndex = 0; // Индекс текущего слоя

                int service_info = 1;
                for (int i = 0; i < PE_CORE; ++i) {
                    cout << "weight_size " << weight_size << endl;
                    write(getFirstAddressCore(i), weight_size);
                }
                for (int j = 0; j < weight_size; ++j) {
                    service_info++;
                    for (int i = 0; i < PE_CORE; ++i) {

                        cout << "wl " << get_layer_weight_count(i) << endl;
                        write(getFirstAddressCore(i), weights_layers[i]);
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


                    cout << "w" << i << ": " << weight << " layer: " << layerIndex << " group: " << groupIndex << " core: " << coreIndex << endl;
                    offsetCores[coreIndex]++;
                    int offset = offsetCores[coreIndex];
                    cout << "offset " << offset << endl;
                    write(offset, weight);

                    // Увеличить общее количество обработанных весов
                    totalWeightsProcessed++;
                }

//                cout << "w" << i << ": " << weight << " layer:" << layerIndex << " group: " << groupIndex << " core: " << coreIndex << endl;

//                i, weight, layer, group

//                cout << "w" << i << ": " << weight << " layer:" << layerIndex << " group: " << groupsInLayer << endl;


//                cout << "w" << i << ": " << weight << " layer:" << layerIndex << endl;


//                for (int j = -1; j < weight_size; ++j) {
//                    int in_layer_size = input_size;
//                    if (j >= 0) {
//                        in_layer_size = weights_layers[j];
//                    }
//                    for (int core_i = 0; core_i < PE_CORE; ++core_i) {
//                        const int service_info_count = 3;
//                        int first_index_neuron = weights_layers[j] / PE_CORE * core_i;
//                        int first_address = 0x1000*(core_i + 1);
//                        if (j >= 0) {
//                            first_address = 0x1000*(core_i + 1)+(service_info_count*(j+1))+(input_size*weights_layers[j]);
//                        }
//                        cout << "first_index_neuron " << first_index_neuron << endl;
//                        cout << "first_address " << first_address << endl;
//                        const int neuron_in_core_count = weights_layers[j]/PE_CORE;
//                        write(first_address, first_index_neuron);
//                        write(first_address+2, in_layer_size);
//                        write(first_address+3, neuron_in_core_count);
//                        cout << "weights_layers[j] " << weights_layers[j] << endl;
//                        cout << "in_layer_size " << in_layer_size << endl;
//                        cout << "neuron_in_core_count " << neuron_in_core_count << endl;
//                        int w_index = 0;
//                        for (int i = 0; i < in_layer_size; ++i) {
//                                float w = read(SHARED_MEMORY_OFFSET + 2 + input_size + 1 + weight_size + (weights_layers[j]*i) + (core_i*neuron_in_core_count));
//                                cout << "w" << i << ": " << w << endl;
//                                cout << "addr " << first_address + service_info_count + (w_index * 2) + 1 << endl;
//                                write(first_address + service_info_count + (w_index * 2) + 1, w);
//                                w_index++;
//                        }
//                    }
//                }
//                write(0x100, 1);
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

    int get_all_weight(){
        int all_weight = 0;
        int last_layer = input_size;
        for (int i = 0; i < weight_size; ++i) {
            all_weight = all_weight + (last_layer * weights_layers[i]);
            last_layer = weights_layers[i];
        }
        return all_weight;
    }

    int getFirstAddressCore(int core_i){
        return 0x1000*(core_i + 1);
    }

    int get_layer_size(){
        return weight_size;
    }

    int get_layer_weight_count(int layer_index){
        if (layer_index == 0){
            return input_size*weights_layers[0];
        }
        return weights_layers[layer_index-1]*weights_layers[layer_index];
    }

    int get_group_count(int layer_index){
        return weights_layers[layer_index];
    }

    float read(sc_uint<ADDR_BITS> addr){
        bus_addr.write(addr);
        bus_rd.write(true);
        bus_wr.write(false);
        wait();
        wait();
        bus_rd.write(false);
        return bus_data_out.read();
    }

    void write(sc_uint<ADDR_BITS> addr, float data){
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
