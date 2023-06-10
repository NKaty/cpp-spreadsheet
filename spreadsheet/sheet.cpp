#include "sheet.h"

#include "cell.h"

#include <algorithm>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Position is invalid");
  }

  if (auto *cell = GetCell(pos)) {
    cell->Set(std::move(text));
  } else {
    data_[pos] = std::make_unique<Cell>(*this);
    SetCell(pos, std::move(text));
  }
}

const CellInterface *Sheet::GetCell(Position pos) const {
  return const_cast<Sheet *>(this)->GetCell(pos);
}

CellInterface *Sheet::GetCell(Position pos) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Position is invalid");
  }
  return data_.count(pos) ? data_.at(pos).get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
  if (GetCell(pos)) {
    data_.at(pos).reset();
  }
}

Size Sheet::GetPrintableSize() const {
  int rows = 0;
  int cols = 0;

  for (auto &[pos, _] : data_) {
    if (data_.at(pos)) {
      rows = std::max(pos.row + 1, rows);
      cols = std::max(pos.col + 1, cols);
    }
  }

  return {rows, cols};
}

void Sheet::PrintValues(std::ostream &output) const {
  Print(output, [&output](const CellInterface *cell) {
    visit([&](const auto &value) { output << value; }, cell->GetValue());
  });
}

void Sheet::PrintTexts(std::ostream &output) const {
  Print(output, [&output](const CellInterface *cell) { output << cell->GetText(); });
}

std::unique_ptr<SheetInterface> CreateSheet() {
  return std::make_unique<Sheet>();
}