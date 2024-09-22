#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>

class Cell::Impl {
public:
    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual ~Impl() = default;
};

class Cell::EmptyImpl : public Impl {
public:
    CellInterface::Value GetValue() const override {
        return "";
    }

    std::string GetText() const override {
        return "";
    }
};

class Cell::TextImpl : public Impl {
public:
    explicit TextImpl(std::string text) : text_(std::move(text)) {}

    CellInterface::Value GetValue() const override {
        if (!text_.empty() && text_.front() == ESCAPE_SIGN) {
            return text_.substr(1);
        }
        return text_;
    }

    std::string GetText() const override {
        return text_;
    }

private:
    std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:
    explicit FormulaImpl(std::string expression, const SheetInterface& sheet)
        : sheet_(sheet) {
        if (expression.empty() || expression[0] != FORMULA_SIGN) {
            throw FormulaException("Invalid formula");
        }
        // Парсинг формулы через функцию ParseFormula
        formula_ = ParseFormula(expression.substr(1));
    }

    CellInterface::Value GetValue() const override {
        if (!cache_) {
            // Вычисляем значение формулы через Evaluate, передавая ссылку на таблицу
            cache_ = formula_->Evaluate(sheet_);
        }

        // Возвращаем кэшированное значение
        if (std::holds_alternative<double>(*cache_)) {
            return std::get<double>(*cache_);
        }
        else {
            return std::get<FormulaError>(*cache_);
        }
    }

    std::string GetText() const override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    // сброс кэша при изменениях
    void InvalidateCache() {
        cache_.reset();
    }

    std::vector<Position> GetReferencedCells() const {
        return formula_->GetReferencedCells();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
    const SheetInterface& sheet_; // ссылка на таблицу
    mutable std::optional<std::variant<double, FormulaError>> cache_; // кеш результата вычислений
};

Cell::~Cell() = default;

Cell::Cell(SheetInterface& sheet)
    : impl_(std::make_unique<EmptyImpl>())
    , sheet_(sheet)
{}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else if (!text.empty() && text.front() == FORMULA_SIGN) {
        impl_ = std::make_unique<FormulaImpl>(std::move(text), sheet_);

        auto new_references = GetReferencedCells();
        std::unordered_set<Position> visited;
        std::unordered_set<Position> in_stack;

        for (const auto& ref : new_references) {
            if (HasCircularDependency(ref, visited, in_stack)) {
                throw CircularDependencyException("Circular dependency detected in cell.");
            }
        }

        UpdateReferences(new_references);
    }
    else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    Set("");
}

CellInterface::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    if (auto formula_impl = dynamic_cast<const FormulaImpl*>(impl_.get())) {
        return formula_impl->GetReferencedCells();
    }
    return {};
}

// методы будут работать с зависимостями
void Cell::InvalidateCache() {
    if (!cache_) {
        return;
    }
    cache_.reset();
    for (const Position& pos : dependent_cells_) {
        Cell* dependent = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        if (dependent) {
            dependent->InvalidateCache();
        }
    }
}

bool Cell::IsCacheValid() const {
    return cache_.has_value();
}

void Cell::UpdateReferences(const std::vector<Position>& new_references) {
    referenced_cells_ = new_references;
}

void Cell::AddDependentCell(Position pos) {
    dependent_cells_.push_back(pos);
}

void Cell::RemoveDependentCell(Position pos) {
    for (Position dep_pos : dependent_cells_) {
        Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(dep_pos));
        if (!dep_pos.IsValid() || !cell)
            continue;

        // Ищем позицию в referenced_cells_
        auto it = std::find(cell->referenced_cells_.begin(), cell->referenced_cells_.end(), pos);
        if (it != cell->referenced_cells_.end()) {
            cell->referenced_cells_.erase(it);  // Удаляем элемент по итератору
        }
    }
}

bool Cell::HasCircularDependency(Position pos, std::unordered_set<Position>& visited, std::unordered_set<Position>& in_stack) const {
    if (in_stack.count(pos)) {
        return true;  // Обнаружен цикл
    }

    if (visited.count(pos)) {
        return false;  // Ячейка уже проверена
    }

    visited.insert(pos);
    in_stack.insert(pos);

    const CellInterface* cell = sheet_.GetCell(pos);
    if (cell) {
        for (const auto& ref_pos : cell->GetReferencedCells()) {
            if (ref_pos.IsValid()) {
                if (HasCircularDependency(ref_pos, visited, in_stack)) {
                    return true;
                }
            }
        }
    }

    in_stack.erase(pos);
    return false;
}