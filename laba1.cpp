#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

using namespace std;

// Проверка на гласную
bool isVowel(wchar_t c) {
    wstring vowels = L"аеёиоуыэюяАЕЁИОУЫЭЮЯ";
    return vowels.find(c) != wstring::npos;
}

// Проверка на согласную
bool isConsonant(wchar_t c) {
    wstring consonants = L"бвгджзйклмнпрстфхцчшщБВГДЖЗЙКЛМНПРСТФХЦЧШЩ";
    return consonants.find(c) != wstring::npos;
}

// Проверка на цифру
bool isDigit(wchar_t c) {
    return c >= L'0' && c <= L'9';
}

// Добавление уникального символа
void addUnique(wstring& row, wchar_t c) {
    if (row.find(c) == wstring::npos) {
        row += c;
    }
}

int main() {
    // Настройка Unicode для консоли
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    wcout << L"Введите строку (до 50 символов):" << endl;
    wstring input;
    getline(wcin, input);

    if (input.length() > 50) {
        input = input.substr(0, 50);
    }

    // Зубчатый массив
    vector<wstring> jaggedArray(4);

    for (wchar_t c : input) {
        if (isVowel(c)) {
            addUnique(jaggedArray[0], c);
        }
        else if (isConsonant(c)) {
            addUnique(jaggedArray[1], c);
        }
        else if (isDigit(c)) {
            addUnique(jaggedArray[2], c);
        }
        else {
            if (c != L'@') {
                addUnique(jaggedArray[3], c);
            }
        }
    }

    // Вывод массива
    wcout << L"Массив:" << endl;
    for (int i = 0; i < 4; i++) {
        for (wchar_t c : jaggedArray[i]) {
            if (c == L' ') wcout << L"   ";
            else wcout << c << L" ";
        }
        wcout << endl;
    }

    wcout << L"Результат:" << endl;

    // Вывод раскрашенной строки
    for (wchar_t c : input) {
        if (isVowel(c)) {
            // Красный текст (4)
            SetConsoleTextAttribute(hConsole, 4);
            wcout << c;
        }
        else if (isConsonant(c)) {
            // Синий текст (1)
            SetConsoleTextAttribute(hConsole, 1);
            wcout << c;
        }
        else if (isDigit(c)) {
            // Зеленый текст (2)
            SetConsoleTextAttribute(hConsole, 2);
            wcout << c;
        }
        else if (c == L'@') {
            // Белый текст (7) - игнорируем раскраску
            SetConsoleTextAttribute(hConsole, 7);
            wcout << c;
        }
        else {

            SetConsoleTextAttribute(hConsole, BACKGROUND_RED | BACKGROUND_GREEN);

            SetConsoleTextAttribute(hConsole, BACKGROUND_RED | BACKGROUND_GREEN | FOREGROUND_INTENSITY); 

            SetConsoleTextAttribute(hConsole, 96);

            wcout << c;
        }
    }

    SetConsoleTextAttribute(hConsole, 7);
    wcout << endl;

    return 0;
}