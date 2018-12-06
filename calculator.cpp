//
// Created by Линар on 05/12/2018.
//
#include <iostream>
#include "std_lib_facilities.h"
#include "variable.h"

const char number = '8';                // t.kind == number означает, что t - число
const char quit = 'q';                  // t.kind == quit означает, что t - лексема выхода
const char print = ';';                 // t.kind == print означает, что t - лексема печати
const string prompt = "> ";
const string result = "= ";             // Используется для указания на то,
// что даллее следует результат
const char name = 'a';                  // Лексема Имя
const char let = 'L';                   // Лексема let
const char const_var = 'C';
const char change = 'H';
const string declkey = "let";           // Ключевое слов let
const string declkey_const = "const";
const string declkey_change = "change";


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

void Token_stream::putback(Token t) {
    if (full) error("putback(): буфер заполнен");
    buffer = t;
    full = true;
}
Token Token_stream::get(istream& in) {
    if (full) {          // Проверка наличия Token в буфере
        full = false;
        return buffer;
    }
    char ch;
    in >> ch;           // Заметим, что оператор >> пропускает
    // пробельные символы

    switch (ch) {
        case ';':
        case 'q':
        case '(':
        case ')':
        case '+':
        case '-':
        case '/':
        case '*':
        case '%':
        case '=':
            return Token{ch}; // Каждый символ представляет сам себя
        case '.':             // Число с плавающей точкой может начинаться с точки

            // Числовой литерал:
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
            cin.putback(ch); // Возврат цифры или точки во входной поток
            double val;
            in >> val;      // Чтение числа с плавающей точкой
            return Token{number, val};
        }
        default:
            if(isalpha(ch)) {
                in.putback(ch);
                string s = "";
                //cin >> s;
                char el = '\0';
                while(true){
                    in.get(el);
                    if((el >= 'a' && el <= 'z')||(el >= 'A' && el <= 'Z')){
                        s += el;
                    } else if (el != ' '){
                        in.putback(el);
                        break;
                    } else {
                        break;
                    }
                }
                if(s == declkey)
                    return Token(let); // Ключевое слово let
                if(s == declkey_const)
                    return Token(const_var);
                if(s == declkey_change)
                    return Token(change);
                return Token{name, s};
            }
            error("Неверная лексема");
    }
}
void Token_stream::ignore(istream& in, char c) {      // Символ с представляет разновидность лексем
    if (full && c==buffer.kind) {        // Сначала проверяем буфер:
        full = false;
        return;
    }
    full = false;

    char ch = 0;
    while (in >> ch) {
        if (ch == c) return;
    }
}


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

double Calculator::primary(istream& in) {
    Token t = ts.get(in);
    switch(t.kind) {
        case '(': {
            double d = expression(in);
            t = ts.get(in);
            if(t.kind != ')') {
                error("Требуется ')'");
            }
            return d;
        }
        case number:
            return t.value;
        case '-':
            return - primary(in);
        case '+':
            return primary(in);
        case name: {
            string var_name = t.name;
            return get_value(var_name);
        }
        default:
            error("Требуется первичное выражение");
    }
}
double Calculator::term(istream& in) {
    double left = primary(in);
    Token t = ts.get(in);
    while(true){
        switch(t.kind){
            case '*':
                left *= primary(in);
                t = ts.get(in);
                break;
            case '/': {
                double d = primary(in);
                if (d == 0) {
                    error("/: деление на нуль");
                }
                left /= d;
                t = ts.get(in);
                break;
            }
            case '%': {
                double d = primary(in);
                if(d == 0) error("%: деление на нуль");
                left = fmod(left, d);
                t = ts.get(in);
                break;
            }
            default:
                ts.putback(t);
                return left;
        }
    }
}
double Calculator::expression(istream& in) {
    double left = term(in);
    Token t = ts.get(in);
    while(true){
        switch(t.kind){
            case '+':
                left += term(in);
                t = ts.get(in);
                break;
            case '-':
                left -= term(in);
                t = ts.get(in);
                break;
            default:
                ts.putback(t);
                return left;
        }
    }
}

double Calculator::get_value(string s) {
    for (const Variable& v : var_table)
        if(v.name == s) {
            return v.value;
        }
    error("get: неопределенная переменная ", s);
}
void Calculator::set_value(string s, double d) {
    for (Variable& v : var_table){
        if (v.name == s) {
            if(v.is_const) {
                error("set: нельзя изменять константы");
                return;
            }
            v.value = d;
            return;
        }
    }
    error("set: неопределенная переменная ", s);
}

bool Calculator::is_declared(string var) {
    // Есть ли переменная var в векторе var_table?
    for(const Variable& v : var_table) {
        if(v.name == var) return true;
    }
    return false;
}
double Calculator::define_name(string var, double val, bool is_const=false) {
    // Добавляем пару (var, val) в вектор var_table
    if(is_declared(var)) error(var, " повторное объявление");
    var_table.push_back(Variable(var, val, is_const));
    return val;
}
double Calculator::declaration(istream& in, bool is_const=false) {
    // Считаем, что мы уже встретили ключевое слово "let"
    // Обрабатываем: Имя = Выражение
    // Объявление переменной с Именем с начальным значением,
    // заданным выражением
    Token t = ts.get(in);
    if(t.kind != name)
        error("в объявлении ожидается имя переменной");
    string var_name = t.name;

    Token t2 = ts.get(in);
    if (t2.kind != '=')
        error("Пропущен символ = в объявлении ", var_name);

    double d = expression(in);
    define_name(var_name, d, is_const);
    return d;
}
double Calculator::change_variable(istream& in) {
    Token t = ts.get(in);
    if(t.kind != name)
        error("в изменении ожидается имя переменной");
    string var_name = t.name;

    Token t2 = ts.get(in);
    if (t2.kind != '=')
        error("Пропущен символ = в изменении ", var_name);

    double d = expression(in);
    set_value(var_name, d);
    return d;
}
double Calculator::statement(istream& in) {
    Token t = ts.get(in);
    switch (t.kind) {
        case let:
            return declaration(in, false);
        case const_var:
            return declaration(in, true);
        case change:
            return change_variable(in);
        default:
            ts.putback(t);
            return expression(in);
    }
}

void Calculator::clean_up_mess(istream& in) {
    ts.ignore(in, print);
}
void Calculator::calculate(istream& in) {
    double val = 0;
    while(in)
        try {
            cout << prompt;
            Token t = ts.get(in);
            while (t.kind == print) t = ts.get(in); // "Съедает" ';'
            if (t.kind == quit) return;
            ts.putback(t);
            val = statement(in);
            cout << result << val << '\n';
        }
        catch (exception& e) {
            cerr << e.what() << '\n';
            clean_up_mess(in);
        }
}

void Calculator::define_var(string var, double val) {
    define_name(var, val);
}
void Calculator::define_const(string var, double val) {
    define_name(var, val, true);
}