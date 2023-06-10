#include "formula.h"
#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe) {
  return output << "#DIV/0!";
}

class ValueVisitor {
 public:
  double operator()(double value) const {
    return value;
  }

  double operator()(const FormulaError &error) {
    throw error;
  }

  double operator()(const std::string &value) const {
    if (value.empty()) {
      return 0.;
    }
    std::istringstream iss(value);
    double num;
    if ((iss >> num) && (iss.eof())) {
      return num;
    }
    throw FormulaError(FormulaError::Category::Value);
  }
};

namespace {
class Formula : public FormulaInterface {
 public:
// Реализуйте следующие методы:
  explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {
    auto cell_list = ast_.GetCells();
    cell_list.unique();
    referenced_cells_ = {cell_list.begin(), cell_list.end()};
  }

  [[nodiscard]] Value Evaluate(const SheetInterface &sheet) const override {
    try {
      auto value_getter = [&sheet](Position pos) {
        if (!pos.IsValid()) {
          throw FormulaError(FormulaError::Category::Ref);
        }
        if (auto *cell = sheet.GetCell(pos)) {
          return std::visit(ValueVisitor{}, cell->GetValue());
        } else {
          return 0.;
        }
      };
      return ast_.Execute(value_getter);
    } catch (const FormulaError &error) {
      return error;
    }
  }

  [[nodiscard]] std::string GetExpression() const override {
    std::ostringstream out;
    ast_.PrintFormula(out);
    return out.str();
  }

  [[nodiscard]] std::vector<Position> GetReferencedCells() const override {
    return referenced_cells_;
  }

 private:
  FormulaAST ast_;
  std::vector<Position> referenced_cells_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
  try {
    return std::make_unique<Formula>(std::move(expression));
  } catch (...) {
    throw FormulaException("Parsing error");
  }
}