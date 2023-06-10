#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>
#include <map>

using namespace std::literals;

class Sheet : public SheetInterface {
 public:
  ~Sheet();

  void SetCell(Position pos, std::string text) override;

  const CellInterface *GetCell(Position pos) const override;
  CellInterface *GetCell(Position pos) override;

  void ClearCell(Position pos) override;

  Size GetPrintableSize() const override;

  void PrintValues(std::ostream &output) const override;
  void PrintTexts(std::ostream &output) const override;

 private:
  std::map<Position, std::unique_ptr<CellInterface>> data_;

  template<typename F>
  void Print(std::ostream &output, F cell_printer) const;
};

template<typename F>
void Sheet::Print(std::ostream &output, F cell_printer) const {
  const auto [rows, cols] = GetPrintableSize();
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      if (col != 0) {
        output << "\t"s;
      }
      if (auto *cell = GetCell({row, col})) {
        cell_printer(cell);
      }
    }
    output << "\n"s;
  }
}
