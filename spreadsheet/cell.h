#pragma once

#include "common.h"
#include "formula.h"
#include <utility>
#include <unordered_set>
#include <optional>  


class Sheet;

class Cell : public CellInterface {
public:
    explicit Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    void InvalidateCache();
    bool IsCacheValid() const;

    // методы будут работать с зависимостями
    void UpdateReferences(const std::vector<Position>& new_references);
    void AddDependentCell(Position pos);
    void RemoveDependentCell(Position pos);

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;

    // ссылка на лист для доступа к другим ячейкам
    SheetInterface& sheet_;

    // значение кэша
    std::optional<Value> cache_;

    std::vector<Position> referenced_cells_;   // // ячейки на которые ссылается эта ячейка
    std::vector<Position> dependent_cells_;    // ячейки которые зависят от этой ячейки

    bool HasCircularDependency(Position pos, std::unordered_set<Position>& visited, std::unordered_set<Position>& in_stack) const;
};