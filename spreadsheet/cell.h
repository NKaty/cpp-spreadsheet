#pragma once

#include "formula.h"
#include "sheet.h"

#include "memory"
#include "unordered_set"

class Cell : public CellInterface {
 public:
  Cell(SheetInterface &sheet);
  ~Cell() override;

  void Set(std::string text) override;
  void Clear();

  [[nodiscard]] Value GetValue() const override;
  [[nodiscard]] std::string GetText() const override;

  [[nodiscard]] std::vector<Position> GetReferencedCells() const override;

  bool IsReferenced() const;

 private:
  class Impl {
   public:
    virtual ~Impl() = default;

    [[nodiscard]] virtual Value GetValue(const SheetInterface &sheet) const = 0;

    [[nodiscard]] virtual std::string GetText() const = 0;

    [[nodiscard]] virtual std::vector<Position> GetReferencedCells() const = 0;
  };

  class EmptyImpl final : public Impl {
   public:
    [[nodiscard]] std::string GetText() const override;

    [[nodiscard]] Value GetValue(const SheetInterface &sheet) const override;

    [[nodiscard]] std::vector<Position> GetReferencedCells() const override;
  };

  class TextImpl final : public Impl {
   public:
    explicit TextImpl(std::string text);

    [[nodiscard]] std::string GetText() const override;

    [[nodiscard]] Value GetValue(const SheetInterface &sheet) const override;

    [[nodiscard]] std::vector<Position> GetReferencedCells() const override;

   private:
    std::string text_;
  };

  class FormulaImpl final : public Impl {
   public:
    explicit FormulaImpl(std::string formula);

    [[nodiscard]] std::string GetText() const override;

    [[nodiscard]] Value GetValue(const SheetInterface &sheet) const override;

    [[nodiscard]] std::vector<Position> GetReferencedCells() const override;

   private:
    std::unique_ptr<FormulaInterface> formula_;
  };

  std::unique_ptr<Impl> impl_;
  SheetInterface &sheet_;
  std::unordered_set<Cell *> dependent_cells_;
  mutable std::optional<Value> cache_;

  bool HasCircularDependencies(const Impl &impl);

  void ClearReferences();
  void AddReferences();

  void InvalidateCache();
  void InvalidateDependentCellsCache();
};
