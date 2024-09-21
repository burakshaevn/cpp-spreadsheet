#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
    class Formula : public FormulaInterface {
    public: 

        Formula(std::string expression)
            try
            : ast_(ParseFormulaAST(std::move(expression)))
        {}
        catch (const FormulaException& e) {
            throw FormulaException("Error parsing formula");
        }

        Value Evaluate(const SheetInterface& sheet) const override {
            /*Должен вычислять значение формулы и возвращать число, если формулу удалось вычислить. 
            Если не удалось, должен возвращать ошибку вычисления FormulaError. 
            Для этого в файле common.h объявлен метод вывода ошибки в поток, а в файле formula.cpp он реализован.*/
            try {
                return ast_.Execute(sheet);
            } 
            catch (const FormulaError& e) {
                return e;
            }
        }

        std::string GetExpression() const override { 
            std::ostringstream oss;
            ast_.PrintFormula(oss);
            return oss.str();
        } 
         
        std::vector<Position> GetReferencedCells() const {
            auto positions = ast_.GetCells();
            std::unordered_set<Position> unique_positions;

            for (const auto& pos : positions) {
                unique_positions.insert(pos);  
            }

            return std::vector<Position>(unique_positions.begin(), unique_positions.end());
        }



    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}