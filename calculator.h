//
// Created by Линар on 05/12/2018.
//

#ifndef TASK1_MIPT_CALCULATOR_CALCULATOR_H
#define TASK1_MIPT_CALCULATOR_CALCULATOR_H

#endif //TASK1_MIPT_CALCULATOR_CALCULATOR_H

#include "std_lib_facilities.h"

struct Variable {
    string name;
    double value;
    bool is_const;

    Variable() {}
    Variable(string name, double value, bool is_const): name(name), value(value), is_const(is_const) {}
};

class Token {
public:
    char kind;
    double value;
    string name;

    Token() {}
    Token(char ch)
            : kind{ ch }, value{ 0 } {}
    Token(char ch, double val)
            : kind{ ch }, value{ val } {}
    Token(char ch, string name)
            : kind{ ch }, name{ name } {}
};

class Token_stream {
public:
    Token get(istream& in);
    void putback(Token t);
    void ignore(istream& in, char c);

private:
    bool full {false};
    Token buffer;
};

class Calculator {
public:
    void define_const(string, double);
    void define_var(string, double);
    void exec();

    void calculate(istream& in);

private:
    Token_stream ts;
    vector <Variable> var_table;

    double primary(istream& in);
    double term(istream& in);
    double expression(istream& in);
    double get_value(string s);
    void set_value(string s, double d);
    bool is_declared(string var);
    double define_name(string var, double val, bool is_const);
    double declaration(istream& in, bool is_const);
    double change_variable(istream& in);
    double statement(istream& in);
    void clean_up_mess(istream& in);
};

