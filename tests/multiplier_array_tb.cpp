#include <systemc.h>
#include "../src/multiplier_array.h"
#include "../src/clk.h"

int sc_main (int argc, char* argv[]) {
    static const int N_PORTS = 2;
    sc_vector<sc_signal<float>>  in1{"in1", N_PORTS};
    sc_vector<sc_signal<float>>  in2{"in2", N_PORTS};
    sc_vector<sc_signal<float>>  out1{"out1", N_PORTS};

    MultiplierArray multiplierArray("MultiplierArray");
    multiplierArray.input1(in1);
    multiplierArray.input2(in2);
    multiplierArray.output(out1);

    sc_trace_file *wf = sc_create_vcd_trace_file("multiplier_array");
    sc_trace(wf, in1[0], "in1(0)");
    sc_trace(wf, in1[1], "in1(1)");
    sc_trace(wf, in2[0], "in2(0)");
    sc_trace(wf, in2[1], "in2(1)");
    sc_trace(wf, out1[0], "out1(0)");
    sc_trace(wf, out1[1], "out1(1)");

    // Инициализация тестовых сценариев
    sc_start(1, SC_NS);  // Запускаем симуляцию на 1 наносекунду, чтобы 'сбросить' значения

    // Пример тестового сценария:
    in1[0].write(5.0f);
    in1[1].write(3.0f);
    in2[0].write(2.0f);
    in2[1].write(7.0f);

    sc_start(10, SC_NS);  // Запускаем симуляцию на 10 наносекунд

    // Здесь можно добавить другие сценарии тестирования, меняя значения входов и вызывая sc_start для продолжения симуляции

    sc_close_vcd_trace_file(wf);  // Закрываем файл трассировки
    return 0; // Завершаем симуляцию
}
