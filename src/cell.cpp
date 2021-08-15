#include "cell.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <variant>

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
	else if (text[0] == '=' && text.size() > 1u) {
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

CellInterface::Value Cell::EmptyImpl::GetValue() const {
	return {};
}

std::string Cell::EmptyImpl::GetText() const {
	return {};
}

void Cell::EmptyImpl::ClearCache() {}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
	return {};
}

Cell::TextImpl::TextImpl(std::string value)
	: value_(std::move(value)) {
}

CellInterface::Value Cell::TextImpl::GetValue() const {
	return value_.front() == '\'' ? value_.substr(1) : value_;
}

std::string Cell::TextImpl::GetText() const {
	return value_;
}

void Cell::TextImpl::ClearCache() {}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
	return {};
}

Cell::FormulaImpl::FormulaImpl(const SheetInterface& sheet, std::string text)
	: sheet_(sheet) {
	using namespace std::literals;
	try {
		formula_ = ParseFormula(text);
	}
	catch (...) {
		throw FormulaException("Formula parsing error"s);
	}
}

Cell::Value Cell::FormulaImpl::GetValue() const {
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

std::string Cell::FormulaImpl::GetText() const {
	return '=' + formula_->GetExpression();
}

void Cell::FormulaImpl::ClearCache() {
	cached_value_ = std::nullopt;
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
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