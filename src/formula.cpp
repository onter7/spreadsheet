#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << "#DIV/0!"s;
}

namespace {
	class Formula : public FormulaInterface {
	public:
		explicit Formula(std::string expression)
			: ast_(ParseFormulaAST(expression)) {
		}

		Value Evaluate(const SheetInterface& sheet) const override {
			std::function<double(Position)> get_value_by_position = [&sheet](Position pos) {
				double result = 0.0;

				if (!pos.IsValid()) {
					throw FormulaError(FormulaError::Category::Ref);
				}

				const CellInterface* cell = sheet.GetCell(pos);
				if (!cell) {
					return result;
				}

				std::visit(CellValueGetter{ result }, cell->GetValue());

				return result;
			};

			try {
				return ast_.Execute(get_value_by_position);
			}
			catch (const FormulaError& formula_error) {
				return formula_error;
			}
		}

		std::string GetExpression() const override {
			std::stringstream ss;
			ast_.PrintFormula(ss);
			return ss.str();
		}

		std::vector<Position> GetReferencedCells() const override {
			const auto& cells = ast_.GetCells();
			return { cells.begin(), cells.end() };
		}

	private:
		FormulaAST ast_;

		struct CellValueGetter {
			double& result;
			void operator()(const double value) {
				result = value;
			}
			void operator()(const std::string& value) {
				try {
					result = std::stod(value);
				}
				catch (const std::invalid_argument& /* err */) {
					throw FormulaError(FormulaError::Category::Value);
				}
			}
			void operator()(const FormulaError& formula_error) {
				throw formula_error;
			}
		};
	};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	return std::make_unique<Formula>(std::move(expression));
}