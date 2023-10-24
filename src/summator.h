#include <systemc.h>
#include "./floating_point_adder.h"

SC_MODULE(SumModule) {
    sc_in_clk     clock;
    sc_in<bool>   reset;
    sc_in<float> input1;
    sc_in<float> input2;
    sc_out<float> output;

    FloatingPointAdder *adder1;

    sc_signal<float> adder1Result;

    sc_signal<float> intermediateResult;  // Для хранения промежуточных результатов.

    void sum () {
        if (reset.read()) {  // Асинхронный сброс
            output.write(0);
            intermediateResult.write(0.0);  // При сбросе обнуляем промежуточный результат.
        } else if (clock.posedge()) { // Если положительный фронт
            float sum = adder1Result.read() + intermediateResult.read();  // Сложение текущего входа и промежуточного результата.
            intermediateResult.write(sum);  // Сохранение нового промежуточного результата.
            output.write(sum);
        }
    }

    SC_CTOR(SumModule) {
        adder1 = new FloatingPointAdder("adder1");

        adder1->input1(input1);
        adder1->input2(input2);
        adder1->result(adder1Result);

        SC_METHOD(sum);
        sensitive << reset;
        sensitive << clock.pos();  // Реагируем только на положительный фронт тактового сигнала
    }
};
