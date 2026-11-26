#pragma once
// Азбука Морзе: таблицы, кодирование, декодирование и вспомогательные функции.
// Ядро не зависит от Qt: текст хранится как std::u32string, код - как std::string.

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace morse {

// Поток символов '*', '-', '/', '?' без пробелов.
// Каждая буква записывается как её код и завершающий '/'.
// Пробел между словами - это ещё один '/' (пустой токен).
using Code = std::string;
using Text = std::u32string;

// Таблицы на std::map: буква -> код и код -> буква.
const std::map<char32_t, std::string>& encodeTable();
const std::map<std::string, char32_t>& decodeTable();

bool isLetterCode(const std::string& token);  // токен - код буквы из таблицы
bool isValidToken(const std::string& token);  // буква или пустой токен (пробел)

// Текст -> код. Символы вне таблицы пропускаются (их число пишется в *skipped).
Code encodeText(const Text& text, std::size_t* skipped = nullptr);

struct DecodeResult {
    Text text;                           // нераспознанные токены заменены на '?'
    bool hasErrors = false;
    std::vector<std::size_t> badTokens;  // номера нераспознанных токенов
};
DecodeResult decodeCode(const Code& code);

// Разбиение потока на токены по '/' и обратная склейка (канонический вид).
std::vector<std::string> splitTokens(const Code& code);
Code joinTokens(const std::vector<std::string>& tokens);

// Строка из файла/поля ввода -> поток (убирает пробелы, проверяет символы).
// Бросает std::invalid_argument при недопустимом символе.
Code normalizeCode(const std::string& raw);

// Поток -> строка для показа и сохранения: символы через пробел.
std::string formatCode(const Code& code);

}  // namespace morse
