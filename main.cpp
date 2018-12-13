#include <iostream>
#include "std_lib_facilities.h"
#include "calculator.h"
#include <fstream>


int main() {
    try {

        ifstream fin("input.txt");

        Calculator calc{cin};

        calc.define_const("pi", 3.1415);
        calc.define_const("e", 2.71828);
        calc.define_const("c", 3e8);

        calc.hello();

        calc.calculate(cin);
        keep_window_open();
        return 0;
    }
    catch (runtime_error& e) {
        cerr << e.what() << endl;
        keep_window_open("~~");
        return 1;
    }
    catch (...) {
        cerr << "Исключение" << endl;
        keep_window_open("~~");
        return 2;
    }
}