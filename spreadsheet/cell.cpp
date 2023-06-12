#include "cell.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <functional>

using namespace std::literals::string_literals;

// Cell
Cell::Cell(SheetInterface &sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
  std::unique_ptr<Impl> tmp;
  const bool is_formula = text[0] == FORMULA_SIGN && text.size() > 1;
  if (text.empty()) {
    tmp = std::make_unique<EmptyImpl>();
  } else if (is_formula) {
    tmp = std::make_unique<FormulaImpl>(text.substr(1));
  } else {
    tmp = std::make_unique<TextImpl>(std::move(text));
  }
  if (is_formula && HasCircularDependencies(*tmp)) {
    throw CircularDependencyException("Circular dependencies");
  }
  ClearReferences();
  impl_ = std::move(tmp);
  AddReferences();
  InvalidateCache();
  InvalidateDependentCellsCache();
}

void Cell::Clear() {
  impl_ = std::make_unique<EmptyImpl>();
  InvalidateCache();
}

Cell::Value Cell::GetValue() const {
  if (!cache_.has_value()) {
    cache_ = impl_->GetValue(sheet_);
  }
  return cache_.value();
}

std::string Cell::GetText() const {
  return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
  return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
  return !GetReferencedCells().empty();
}

bool Cell::HasCircularDependencies(const Impl &impl) {
  std::unordered_map<const Cell *, bool> visiting;
  visiting[this] = true;
  std::unordered_set<const Cell *> visited;

  std::function<bool(const Cell *)>
      traverse = [&visited, &visiting, this, &traverse](const Cell *current_cell) {
    if (visited.find(current_cell) == visited.end()) {
      if (visiting[current_cell]) {
        return true;
      }
      visited.insert(current_cell);
      visiting[current_cell] = true;

      for (const auto &pos : current_cell->GetReferencedCells()) {
        const auto *cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
        if (visiting[cell]) {
          return true;
        }
        if (visited.find(cell) == visited.end() && traverse(cell)) {
          return true;
        }
      }
    }
    visiting[current_cell] = false;
    return false;
  };

  for (const auto pos : impl.GetReferencedCells()) {
    if (auto *cell = dynamic_cast<Cell *>(sheet_.GetCell(pos))) {
      if (traverse(cell)) {
        return true;
      }
    }
    return false;
  }

  return false;
}

void Cell::ClearReferences() {
  for (const Position &pos : GetReferencedCells()) {
    auto *cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    if (cell) {
      cell->dependent_cells_.erase(this);
    }
  }
}

void Cell::AddReferences() {
  for (auto &pos : GetReferencedCells()) {
    auto *cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    if (!cell) {
      sheet_.SetCell(pos, "");
      cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    }
    cell->dependent_cells_.insert(this);
  }
}

void Cell::InvalidateCache() {
  cache_ = std::nullopt;
}

void Cell::InvalidateDependentCellsCache() {
  std::unordered_set<const Cell *> visited;
  for (auto cell : dependent_cells_) {
    if (visited.find(cell) == visited.end()) {
      visited.insert(cell);
      cell->InvalidateCache();
      cell->InvalidateDependentCellsCache();
    }
  }
}

// Cell::EmptyImpl
std::string Cell::EmptyImpl::GetText() const {
  return ""s;
}

Cell::Value Cell::EmptyImpl::GetValue(const SheetInterface &sheet) const {
  return ""s;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
  return {};
}

// Cell::TextImpl
Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

std::string Cell::TextImpl::GetText() const {
  return text_;
}

Cell::Value Cell::TextImpl::GetValue(const SheetInterface &sheet) const {
  return !text_.empty() && text_[0] == ESCAPE_SIGN ? text_.substr(1) : text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
  return {};
}

// Cell::FormulaImpl
Cell::FormulaImpl::FormulaImpl(std::string formula) : formula_(ParseFormula(std::move(formula))) {}

std::string Cell::FormulaImpl::GetText() const {
  return FORMULA_SIGN + formula_->GetExpression();
}

Cell::Value Cell::FormulaImpl::GetValue(const SheetInterface &sheet) const {
  const auto value = formula_->Evaluate(sheet);
  if (std::holds_alternative<double>(value)) {
    return std::get<double>(value);
  }
  return std::get<FormulaError>(value);
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
  return formula_->GetReferencedCells();
}
