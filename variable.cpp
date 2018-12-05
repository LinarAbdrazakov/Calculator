//
// Created by Линар on 05/12/2018.
//
#include "std_lib_facilities.h"

struct Variable {
    string name;
    double value;
    bool is_const;

    Variable() {}
    Variable(string name, double value, bool is_const): name(name), value(value), is_const(is_const) {}
};