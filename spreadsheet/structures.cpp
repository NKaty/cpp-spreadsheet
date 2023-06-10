#include "common.h"

#include <cctype>
#include <sstream>
#include <tuple>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
  return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
  return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
  return col >= 0 && col < MAX_COLS && row >= 0 && row < MAX_ROWS;
}

std::string Position::ToString() const {
  if (!IsValid()) {
    return "";
  }

  std::string str;
  int col_count = col + 1;
  while (col_count > 0) {
    --col_count;
    str.insert(str.begin(), static_cast<char>(static_cast<int>('A') + col_count % 26));
    col_count /= 26;
  }

  return str + std::to_string(row + 1);
}

Position Position::FromString(std::string_view str) {
  const auto str_size = str.size();
  if (!str_size || str_size > MAX_POSITION_LENGTH) {
    return Position::NONE;
  }

  int col = 0;
  int row = 0;
  size_t i = 0;

  while (i < str_size && std::isalpha(str[i])) {
    if (i > MAX_POS_LETTER_COUNT || !std::isupper(str[i])) {
      return Position::NONE;
    }
    col = col * LETTERS + (str[i] - 'A' + 1);
    i++;
  }

  if (col > MAX_COLS) {
    return Position::NONE;
  }

  if (i >= str_size) {
    return Position::NONE;
  }

  while (i < str_size) {
    if (!std::isdigit(str[i])) {
      return Position::NONE;
    }
    row = row * 10 + (str[i] - '0');
    i++;
  }

  if (row < 1 || row > MAX_ROWS) {
    return Position::NONE;
  }

  return {row - 1, col - 1};
}

bool Size::operator==(Size rhs) const {
  return cols == rhs.cols && rows == rhs.rows;
}
