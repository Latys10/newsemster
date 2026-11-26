#include "morse.h"

namespace morse {

namespace {

// Приводит букву к верхнему регистру; Ё считаем Е (в таблице Ё нет).
char32_t normalizeLetter(char32_t ch) {
    if (ch >= U'а' && ch <= U'я') return ch - 0x20;
    if (ch == U'ё' || ch == U'Ё') return U'Е';
    return ch;
}

bool isSpace(char32_t ch) {
    return ch == U' ' || ch == U'\t' || ch == U'\n' || ch == U'\r' || ch == 0x00A0;
}

}  // namespace

const std::map<char32_t, std::string>& encodeTable() {
    static const std::map<char32_t, std::string> table = {
        {U'А', "*-"},   {U'Б', "-***"}, {U'В', "*--"},   {U'Г', "--*"},
        {U'Д', "-**"},  {U'Е', "*"},    {U'Ж', "***-"},  {U'З', "--**"},
        {U'И', "**"},   {U'Й', "*---"}, {U'К', "-*-"},   {U'Л', "*-**"},
        {U'М', "--"},   {U'Н', "-*"},   {U'О', "---"},   {U'П', "*--*"},
        {U'Р', "*-*"},  {U'С', "***"},  {U'Т', "-"},     {U'У', "**-"},
        {U'Ф', "**-*"}, {U'Х', "****"}, {U'Ц', "-*-*"},  {U'Ч', "---*"},
        {U'Ш', "----"}, {U'Щ', "--*-"}, {U'Ъ', "*--*-*"}, {U'Ы', "-*--"},
        {U'Ь', "-**-"}, {U'Э', "**-**"}, {U'Ю', "**--"}, {U'Я', "*-*-"},
    };
    return table;
}

const std::map<std::string, char32_t>& decodeTable() {
    // Обратная таблица строится один раз из прямой.
    static const std::map<std::string, char32_t> table = [] {
        std::map<std::string, char32_t> inverse;
        for (const auto& [letter, code] : encodeTable()) inverse.emplace(code, letter);
        return inverse;
    }();
    return table;
}

bool isLetterCode(const std::string& token) {
    return decodeTable().count(token) != 0;
}

bool isValidToken(const std::string& token) {
    return token.empty() || isLetterCode(token);
}

Code encodeText(const Text& text, std::size_t* skipped) {
    if (skipped) *skipped = 0;
    Code code;
    bool pendingSpace = false;  // между словами был пробел
    bool anyLetter = false;     // уже была хотя бы одна буква
    for (char32_t raw : text) {
        if (isSpace(raw)) {
            pendingSpace = true;
            continue;
        }
        const auto it = encodeTable().find(normalizeLetter(raw));
        if (it == encodeTable().end()) {
            if (skipped) ++*skipped;
            continue;
        }
        if (pendingSpace && anyLetter) code += '/';  // пробел = лишний '/'
        pendingSpace = false;
        anyLetter = true;
        code += it->second;
        code += '/';
    }
    return code;
}

std::vector<std::string> splitTokens(const Code& code) {
    std::vector<std::string> tokens;
    std::string current;
    for (char ch : code) {
        if (ch == '/') {
            tokens.push_back(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    if (!current.empty()) tokens.push_back(current);  // хвост без завершающего '/'
    return tokens;
}

Code joinTokens(const std::vector<std::string>& tokens) {
    Code code;
    for (const std::string& token : tokens) {
        code += token;
        code += '/';
    }
    return code;
}

DecodeResult decodeCode(const Code& code) {
    DecodeResult result;
    const std::vector<std::string> tokens = splitTokens(code);
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].empty()) {
            result.text += U' ';
            continue;
        }
        const auto it = decodeTable().find(tokens[i]);
        if (it != decodeTable().end()) {
            result.text += it->second;
        } else {
            result.text += U'?';
            result.hasErrors = true;
            result.badTokens.push_back(i);
        }
    }
    return result;
}

Code normalizeCode(const std::string& raw) {
    Code code;
    for (std::size_t i = 0; i < raw.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(raw[i]);
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') continue;
        if (ch == '*' || ch == '-' || ch == '/' || ch == '?') {
            code += static_cast<char>(ch);
            continue;
        }
        throw std::invalid_argument("Недопустимый символ (код " + std::to_string(ch) +
                                    ") на позиции " + std::to_string(i + 1) +
                                    ". Разрешены только * - / ? и пробелы.");
    }
    return code;
}

std::string formatCode(const Code& code) {
    std::string out;
    out.reserve(code.size() * 2);
    for (char ch : code) {
        if (!out.empty()) out += ' ';
        out += ch;
    }
    return out;
}

}  // namespace morse
