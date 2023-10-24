#include <systemc.h>
#include "../src/adder_mmm.h"
#include "../src/clk.h"

int sc_main (int argc, char* argv[]) {
    static const int N_PORTS = 3;
    sc_vector<sc_signal<float>>  inputs{"inputs", N_PORTS};
    sc_signal<float>             output{"output", N_PORTS};

    AdderMMM<N_PORTS> adderMmm("AdderArray");
    adderMmm.inputs(inputs);
    adderMmm.output(output);

    sc_trace_file *wf = sc_create_vcd_trace_file("adder_mmm");
    for (int i = 0; i < N_PORTS; ++i) {
        sc_trace(wf, inputs[i], "in"+std::to_string(i));
    }
    sc_trace(wf, output, "output");

    // Инициализация тестовых сценариев
    sc_start(1, SC_NS);  // Запускаем симуляцию на 1 наносекунду, чтобы 'сбросить' значения

    // Пример тестового сценария:
    inputs[0].write(5.0f);
    inputs[1].write(3.0f);
    inputs[2].write(13.0f);

    sc_start(10, SC_NS);  // Запускаем симуляцию на 10 наносекунд

    // Здесь можно добавить другие сценарии тестирования, меняя значения входов и вызывая sc_start для продолжения симуляции

    sc_close_vcd_trace_file(wf);  // Закрываем файл трассировки
    return 0; // Завершаем симуляцию
}
