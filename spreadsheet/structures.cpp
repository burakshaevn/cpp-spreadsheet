#include "common.h"

#include <cctype>
#include <sstream>
#include <regex>
#include <cctype>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = { -1, -1 };

bool Position::operator==(const Position rhs) const {
    return (row == rhs.row) && (col == rhs.col);
}

bool Position::operator<(const Position rhs) const {
    return (row < rhs.row) && (col < rhs.col);
}

// Проверяет валидность позиции,
// то есть что ячейка(row, col) не выходит за ограничения ниже и что значения полей row и col неотрицательны.
// Position::NONE невалидна.
bool Position::IsValid() const {
    return ((row >= 0 && row < MAX_ROWS) && (col >= 0 && col < MAX_COLS));
}

// Возвращает строку — позицию в формате пользовательского индекса.
// если позиция невалидна, метод должен вернуть пустую строку.
std::string Position::ToString() const { 
    if (!IsValid()) {
        return std::string();
    }
    const int user_row = row + 1;
    std::string user_col;

    int column_index = col;
    while (column_index >= 0) {
        user_col.insert(user_col.begin(), 'A' + (column_index % 26));
        column_index = column_index / 26 - 1;
    }
    return user_col + std::to_string(user_row);
}

// Возвращает позицию, соответствующую индексу, заданному в str
Position Position::FromString(std::string_view str) {
    std::regex pos_regex("^([A-Z]+)([0-9]+)$");
    std::smatch match;

    std::string str_copy(str);

    // если индекс задан в неверном формате — “abc”, “111”, “12jfd”, 
    // тогда функция должна вернуть дефолтную позицию Position::NONE
    if (std::regex_match(str_copy, match, pos_regex)) {
        std::string row_str = match[1].str();
        std::string col_str = match[2].str();
        int col = 0;
        for (char ch : row_str) {
            if (!std::isupper(ch)) {
                return Position::NONE;
            }
            col = col * 26 + (ch - 'A' + 1);
            if (col > MAX_COLS) {
                return Position::NONE;
            }
        }
        const int row = std::stoull(col_str) - 1;
        // если индедекс выходит за предельные значения
        if (row >= 0 && row < MAX_ROWS && col >= 1 && col <= MAX_COLS) {
            Position pos;
            pos.row = row;
            pos.col = col - 1;
            return pos;
        }
    }

    return Position::NONE;
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}