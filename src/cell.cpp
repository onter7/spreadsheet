#include "cell.h"
#include "common.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <variant>

class Cell::Impl {
public:
	virtual CellInterface::Value GetValue() const = 0;
	virtual std::string GetText() const = 0;
	virtual void ClearCache() = 0;
	virtual std::vector<Position> GetReferencedCells() const = 0;
};

class Cell::EmptyImpl : public Cell::Impl {
public:
	CellInterface::Value GetValue() const override {
		return {};
	}
	std::string GetText() const override {
		return {};
	}
	void ClearCache() override {}
	std::vector<Position> GetReferencedCells() const override {
		return {};
	}
};

class Cell::TextImpl : public Cell::Impl {
public:
	TextImpl(std::string value)
		: value_(std::move(value)) {
	}
	CellInterface::Value GetValue() const override {
		return value_.front() == ESCAPE_SIGN ? value_.substr(1) : value_;
	}
	std::string GetText() const override {
		return value_;
	}
	void ClearCache() override {}
	std::vector<Position> GetReferencedCells() const override {
		return {};
	}
private:
	std::string value_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
	FormulaImpl(const SheetInterface& sheet, std::string text)
		: sheet_(sheet) {
		using namespace std::literals;
		try {
			formula_ = ParseFormula(text);
		}
		catch (...) {
			throw FormulaException("Formula parsing error"s);
		}
	}
	CellInterface::Value GetValue() const override {
		if (cached_value_ != std::nullopt) {
			return cached_value_.value();
		}
		auto result{ formula_->Evaluate(sheet_) };
		if (std::holds_alternative<double>(result)) {
			return std::get<double>(result);
		}
		else {
			return std::get<FormulaError>(result);
		}
	}
	std::string GetText() const override {
		return FORMULA_SIGN + formula_->GetExpression();
	}
	void ClearCache() override {
		cached_value_ = std::nullopt;
	}
	std::vector<Position> GetReferencedCells() const override {
		return formula_->GetReferencedCells();
	}
private:
	const SheetInterface& sheet_;
	std::unique_ptr<FormulaInterface> formula_;
	std::optional<CellInterface::Value> cached_value_;
};

Cell::Cell(SheetInterface& sheet)
	: impl_(std::make_unique<EmptyImpl>())
	, sheet_(sheet) {
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
	std::unique_ptr<Impl> tmp;
	if (text.empty()) {
		tmp = std::make_unique<EmptyImpl>();
	}
	else if (text[0] == FORMULA_SIGN && text.size() > 1u) {
		tmp = std::make_unique<FormulaImpl>(sheet_, text.substr(1));
	}
	else {
		tmp = std::make_unique<TextImpl>(text);
	}
	ClearDependentCellsCache();
	UpdateDependencies(tmp);
	impl_ = std::move(tmp);
}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return referenced_cells_;
}

void Cell::ClearDependentCellsCache() {
	for (auto dependent_cell : dependent_cells_) {
		dependent_cell->impl_->ClearCache();
		dependent_cell->ClearDependentCellsCache();
	}
}

void Cell::UpdateDependencies(std::unique_ptr<Impl>& new_impl) {
	for (const auto& ref_pos : GetReferencedCells()) {
		Cell* cell_ptr = reinterpret_cast<Cell*>(sheet_.GetCell(ref_pos));
		if (cell_ptr) {
			cell_ptr->dependent_cells_.erase(this);
		}
	}
	for (const auto& ref_pos : new_impl->GetReferencedCells()) {
		Cell* cell_ptr = reinterpret_cast<Cell*>(sheet_.GetCell(ref_pos));
		if (cell_ptr) {
			cell_ptr->dependent_cells_.insert(this);
		}
	}
	referenced_cells_ = new_impl->GetReferencedCells();
}