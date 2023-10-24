#include <systemc.h>
#include "./floating_point_adder.h"

SC_MODULE(AdderBuffer) {
    sc_in_clk     clock;
    sc_in<bool>   reset;
    sc_in<float>  input;
    sc_out<float> output;

    sc_signal<float> intermediateResult;

    void sum () {
        if (reset.read()) {  // Асинхронный сброс
            output.write(0);
            intermediateResult.write(0.0);  // При сбросе обнуляем промежуточный результат.
        } else if (clock.posedge()) { // Если положительный фронт
            float sum = input.read() + intermediateResult.read();  // Сложение текущего входа и промежуточного результата.
            intermediateResult.write(sum);  // Сохранение нового промежуточного результата.
            output.write(sum);
        }
    }

    SC_CTOR(AdderBuffer) {
        SC_METHOD(sum);
        sensitive << reset;
        sensitive << clock.pos();  // Реагируем только на положительный фронт тактового сигнала
    }
};
