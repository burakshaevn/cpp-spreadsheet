#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::UpdateDependencies(Position pos, const std::vector<Position>& old_refs, const std::vector<Position>& new_refs) {
    std::unordered_set<Position> old_refs_set(old_refs.begin(), old_refs.end());
    std::unordered_set<Position> new_refs_set(new_refs.begin(), new_refs.end());

    // удаляем зависимости, которые есть в старых ссылках, но отсутствуют в новых
    for (const auto& ref : old_refs) {
        if (new_refs_set.find(ref) == new_refs_set.end()) {
            RemoveDependency(ref, pos);
        }
    }

    // добавляем зависимости, которые есть в новых ссылках, но отсутствуют в старых
    for (const auto& ref : new_refs) {
        if (old_refs_set.find(ref) == old_refs_set.end()) {
            AddDependency(ref, pos);
        }
    }
}

void Sheet::AddDependency(Position from, Position to) {
    auto* cell = dynamic_cast<Cell*>(GetCell(from));
    if (cell) {
        cell->AddDependentCell(to);  // добавление зависимой ячейки
    }
}

void Sheet::RemoveDependency(Position from, Position to) {
    auto* cell = dynamic_cast<Cell*>(GetCell(from));
    if (cell) {
        cell->RemoveDependentCell(to);  // удаление зависимой ячейки
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!IsValidPosition(pos)) {
        throw InvalidPositionException("Invalid position");
    }
    if (text[0] == '=') {
        try {
            auto formula_ast = ParseFormula(text.substr(1));
            for (const auto& cell_ref : formula_ast->GetReferencedCells()) {
                if (!IsValidPosition(cell_ref)) {
                    throw FormulaException("Invalid cell reference in formula");
                }
                if (GetCell(cell_ref) == nullptr) {
                    sheet_[cell_ref] = std::make_unique<Cell>(*this);
                }
            }
        }
        catch (const FormulaException&) {
            throw;
        }
    }
    Cell* cell = dynamic_cast<Cell*>(GetCell(pos));
    if (!cell) {
        sheet_[pos] = std::make_unique<Cell>(*this);
        cell = sheet_[pos].get();
    }

    std::string old_text = cell->GetText();
    auto old_references = cell->GetReferencedCells();

    try {
        cell->Set(std::move(text)); 
    }
    catch (const FormulaException& e) {
        cell->Set(std::move(old_text));
        throw;
    }
    catch (const CircularDependencyException& e) {
        cell->Set(std::move(old_text));
        cell->UpdateReferences(old_references); // Вернуть старые зависимости
        throw;
    }

    // Обновление зависимостей в таблице
    UpdateDependencies(pos, old_references, cell->GetReferencedCells());
}


const CellInterface* Sheet::GetCell(Position pos) const {
    if (!IsValidPosition(pos)) {
        throw InvalidPositionException("Invalid position");
    }
    auto it = sheet_.find(pos);
    return (it != sheet_.end() && it->second) ? it->second.get() : nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!IsValidPosition(pos)) {
        throw InvalidPositionException("Invalid position");
    }
    auto it = sheet_.find(pos);
    return (it != sheet_.end() && it->second) ? it->second.get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!IsValidPosition(pos)) {
        throw InvalidPositionException("Invalid position");
    }

    auto it = sheet_.find(pos);
    if (it != sheet_.end()) {
        // очищаем ячейку
        it->second->Clear();

        // проверяем, есть ли ссылки на эту ячейку из других ячеек
        bool has_references = false;
        for (const auto& [key, cell] : sheet_) {
            if (!cell->GetReferencedCells().empty()) {
                const auto& refs = cell->GetReferencedCells();
                if (std::find(refs.begin(), refs.end(), pos) != refs.end()) {
                    has_references = true;
                    break;
                }
            }
        }

        // если на ячейку нет ссылок и она пустая, удаляем её
        if (!has_references && it->second->GetText().empty()) {
            sheet_.erase(it);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    if (sheet_.empty()) {
        return { 0, 0 };
    }
    int max_row = 0;
    int max_col = 0;
    for (const auto& [pos, cell] : sheet_) {
        if (!cell->GetText().empty()) {
            max_row = std::max(max_row, pos.row + 1);
            max_col = std::max(max_col, pos.col + 1);
        }
    }
    return { max_row, max_col };
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            Position pos{ row, col };
            const CellInterface* cell = GetCell(pos); // ячейка по позиции
            if (cell) {
                std::visit([&output](const auto& value) { output << value; }, cell->GetValue());
            }

            if (col < size.cols - 1) {
                output << '\t';  // разделитель табуляции между столбцами
            }
        }
        output << '\n';  // перевод строки после каждой строки таблицы
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            Position pos{ row, col };
            const CellInterface* cell = GetCell(pos); // получаем ячейку по позиции
            if (cell) {
                output << cell->GetText();
            }

            if (col < size.cols - 1) {
                output << '\t';  // разделитель табуляции между столбцами
            }
        }
        output << '\n';  // перевод строки после каждой строки таблицы
    }
}

bool Sheet::IsValidPosition(const Position& pos) const {
    return pos.IsValid();
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
