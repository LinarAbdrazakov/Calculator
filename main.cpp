#include <iostream>
#include <vector>
#include "std_lib_facilities.h"


const char number = '8'; // t.kind == number означает, что t - число
const char quit = 'q';   // t.kind == quit означает, что t - лексема выхода
const char print = ';';   // t.kind == print означает, что t - лексема печати
const string prompt = "> ";
const string result = "= "; // Используется для указания на то,
                            // что даллее следует результат


class Token {
public:
    char kind;
    double value;

    Token() {}
    Token(char ch)
            : kind{ ch }, value{ 0 } {}
    Token(char ch, double val)
            : kind{ ch }, value{ val } {}
};
class Token_stream {
public:
    Token get();
    void putback(Token t);
private:
    bool full {false};
    Token buffer;
};

void Token_stream::putback(Token t) {
    if (full) error("putback(): буфер заполнен");
    buffer = t;
    full = true;
}
Token Token_stream::get() {
    if (full) {          // Проверка наличия Token в буфере
        full = false;
        return buffer;
    }
    char ch;
    cin >> ch;           // Заметим, что оператор >> пропускает
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
            return Token{ch}; // Каждый символ представляет сам себя
        case '.':             // Число с плавающей точкой может начинаться с точки

            // Числовой литерал:
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
            cin.putback(ch); // Возврат цифры или точки во входной поток
            double val;
            cin >> val;      // Чтение числа с плавающей точкой
            return Token{number, val};
        }
        default:
            error("Неверная лексема");
    }
}

Token_stream ts;
double expression();

double primary() {
    Token t = ts.get();
    switch(t.kind) {
        case '(': {
            double d = expression();
            t = ts.get();
            if(t.kind != ')') {
                error("Требуется ')'");
            }
            return d;
        }
        case number:
            return t.value;
        case '-':
            return - primary();
        case '+':
            return primary();
        default:
            error("Требуется первичное выражение");
    }
}
double term() {
    double left = primary();
    Token t = ts.get();
    while(true){
        switch(t.kind){
            case '*':
                left *= primary();
                t = ts.get();
                break;
            case '/': {
                double d = primary();
                if (d == 0) {
                    error("Деление на нуль");
                }
                left /= d;
                t = ts.get();
                break;
            }
            case '%': {
                double d = primary();
                if(d == 0) error("%: деление на нуль");
                left = fmod(left, d);
                t = ts.get();
                break;
            }
            default:
                ts.putback(t);
                return left;
        }
    }
}
double expression() {
    double left = term();
    Token t = ts.get();
    while(true){
        switch(t.kind){
            case '+':
                left += term();
                t = ts.get();
                break;
            case '-':
                left -= term();
                t = ts.get();
                break;
            default:
                ts.putback(t);
                return left;
        }
    }
}

void calculate() {
    double val = 0;
    while(cin) {
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t = ts.get(); // "Съедает" ';'
        if (t.kind == quit) return;
        ts.putback(t);
        val = expression();
        cout << result << val << '\n';
    }
}

int main() {
    try {
        calculate();
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