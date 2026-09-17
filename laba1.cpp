#include <iostream>     
#include <string>       
#include <vector>       
#include <windows.h>    
#include <locale>       
#include <clocale>      

// Функция проверяет, является ли символ гласной буквой
bool isVowel(char c) {
    std::string vowels = "аеёиоуыэюяАЕЁИОУЫЭЮЯ";
    return vowels.find(c) != std::string::npos;
}

// Функция проверяет, является ли символ согласной буквой
bool isConsonant(char c) {
    std::string consonants = "бвгджзйклмнпрстфхцчшщБВГДЖЗЙКЛМНПРСТФХЦЧШЩ";
    return consonants.find(c) != std::string::npos;
}

// Функция проверяет, является ли символ цифрой
bool isDigitChar(char c) {
    return c >= '0' && c <= '9';
}

// Функция проверяет, является ли символ знаком
bool isSymbol(char c) {
    return !isVowel(c) && !isConsonant(c) && !isDigitChar(c) && c != '@';
}

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(1251);

    std::string input;
    std::cout << "Введите строку:\n";
    std::getline(std::cin, input);

    if (input.length() > 50) {
        input = input.substr(0, 50);
    }

    //Зубчатый массив
    std::vector<std::vector<char>> jagged(4);

    // Проходим по каждому символу введённой строки
    for (char c : input) {
        int row = -1;

        // Определяем, в какую строку массива поместить символ
        if (isVowel(c))         row = 0;
        else if (isConsonant(c)) row = 1;
        else if (isDigitChar(c)) row = 2;
        else if (isSymbol(c))    row = 3;

        // Если символ не подошёл
        if (row == -1) continue;

        // Проверяем, есть ли уже такой символ в соответствующей строке массива
        bool exists = false;
        for (char existing : jagged[row]) {
            if (existing == c) {
                exists = true;
                break;
            }
        }

        // Если символа ещё нет — добавляем его в конец строки
        if (!exists) {
            jagged[row].push_back(c); 
        }
    }

    // Вывод "зубчатого" массива
    std::cout << "Массив:\n";
    for (int i = 0; i < 4; i++) {
        for (char c : jagged[i]) {
            std::cout << c << " ";
        }
        std::cout << "\n";
    }

    input += "+123АБВ";

    // Получаем дескриптор консоли для управления цветом
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    std::cout << "Результат:\n";

    // Проходим по каждому символу итоговой строки
    for (char c : input) {
        int color = 7;

        // Проверяем, есть ли символ в массиве гласных 
        bool found = false;
        for (char v : jagged[0]) {
            if (v == c) { color = 12; found = true; break; } 
        }
        // Если не нашли проверяем согласные
        if (!found) {
            for (char v : jagged[1]) {
                if (v == c) { color = 9; found = true; break; } 
            }
        }
        // Если не нашли — проверяем цифры
        if (!found) {
            for (char v : jagged[2]) {
                if (v == c) { color = 10; found = true; break; } 
            }
        }
        // Если не нашли — проверяем знаки 
        if (!found) {
            for (char v : jagged[3]) {
                if (v == c) { color = 14; found = true; break; } 
            }
        }

        // Символ '@' игнорируем при раскраске — оставляем белым
        if (c == '@') color = 7;

        // Устанавливаем цвет текста в консоли
        SetConsoleTextAttribute(hConsole, color);
        std::cout << c;
    }

    SetConsoleTextAttribute(hConsole, 7);
    std::cout << "\n";

    return 0;
}