#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    void UpdateDependencies(Position pos, const std::vector<Position>& old_refs, const std::vector<Position>& new_refs);

    void AddDependency(Position from, Position to);
    void RemoveDependency(Position from, Position to);

private:
    std::unordered_map<Position, std::unique_ptr<Cell>> sheet_;

    bool HasCircularDependency(Position pos, std::unordered_set<Position>& visited, std::unordered_set<Position>& in_stack) const;

    bool IsValidPosition(const Position& pos) const;
};