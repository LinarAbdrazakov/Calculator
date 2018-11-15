#include <iostream>
#include "std_lib_facilities.h"


const char number = '8';         // t.kind == number означает, что t - число
const char quit = 'q';           // t.kind == quit означает, что t - лексема выхода
const char print = ';';          // t.kind == print означает, что t - лексема печати
const string prompt = "> ";
const string result = "= ";      // Используется для указания на то,
                                 // что даллее следует результат
const char name = 'a';           // Лексема Имя
const char let = 'L';            // Лексема let
const string declkey = "let";    // Ключевое слов let


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
    Token get();
    void putback(Token t);
    void ignore(char c);

private:
    bool full {false};
    Token buffer;
};

struct Variable {
    string name;
    double value;

    Variable() {}
    Variable(string name, double value): name(name), value(value) {}
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
        case '=':
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
            if(isalpha(ch)) {
                cin.putback(ch);
                string s = "";
                //cin >> s;
                char el = '\0';
                while(true){
                    cin.get(el);
                    if((el >= 'a' && el <= 'z')||(el >= 'A' && el <= 'Z')){
                        s += el;
                    } else if (el != ' '){
                        cin.putback(el);
                        break;
                    } else {
                        break;
                    }
                }
                if(s == declkey)
                    return Token(let); // Ключевое слово let
                return Token{name, s};
            }
            error("Неверная лексема");
    }
}
void Token_stream::ignore(char c) {      // Символ с представляет разновидность лексем
    if (full && c==buffer.kind) {        // Сначала проверяем буфер:
        full = false;
        return;
    }
    full = false;

    char ch = 0;
    while (cin >> ch) {
        if (ch == c) return;
    }
}

Token_stream ts;
vector <Variable> var_table;
double expression();
double get_value(string s);


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
        case 'a': {
            string var_name = t.name;
            return get_value(var_name);
        }
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

double get_value(string s) {
    for (const Variable& v : var_table)
        if(v.name == s) {
            return v.value;
        }
    error("get: неопределенная переменная ", s);
}

void set_value(string s, double d) {
    for (Variable& v : var_table){
        if (v.name == s) {
            v.value = d;
            return;
        }
    }
    error("set: неопределенная переменная ", s);
}

bool is_declared(string var) {
    // Есть ли переменная var в векторе var_table?
    for(const Variable& v : var_table) {
        if(v.name == var) return true;
    }
    return false;
}

double define_name(string var, double val) {
    // Добавляем пару (var, val) в вектор var_table
    if(is_declared(var)) error(var, " повторное объявление");
    var_table.push_back(Variable(var, val));
    return val;
}

double declaration() {
    // Считаем, что мы уже встретили ключевое слово "let"
    // Обрабатываем: Имя = Выражение
    // Объявление переменной с Именем с начальным значением,
    // заданным выражением
    Token t = ts.get();
    if(t.kind != name)
        error("в объявлении ожидается имя переменной");
    string var_name = t.name;

    Token t2 = ts.get();
    if (t2.kind != '=')
        error("Пропущен символ = в объявлении ", var_name);

    double d = expression();
    define_name(var_name, d);
    return d;
}

double statement() {
    Token t = ts.get();
    switch (t.kind) {
        case let:
            return declaration();
        default:
            ts.putback(t);
            return expression();
    }
}

void clean_up_mess() {
    ts.ignore(print);
}
void calculate() {
    double val = 0;
    while(cin)
        try {
            cout << prompt;
            Token t = ts.get();
            while (t.kind == print) t = ts.get(); // "Съедает" ';'
            if (t.kind == quit) return;
            ts.putback(t);
            val = statement();
            cout << result << val << '\n';
        }
        catch (exception& e) {
            cerr << e.what() << '\n';
            clean_up_mess();
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