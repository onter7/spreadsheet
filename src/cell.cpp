#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <variant>


// Реализуйте следующие методы
Cell::Cell()
	: impl_(std::make_unique<EmptyImpl>()) {
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
	if (text.empty()) {
		Clear();
	}
	else if (text[0] == '=' && text.size() > 1u) {
		impl_ = std::make_unique<FormulaImpl>(text.substr(1));
	}
	else {
		impl_ = std::make_unique<TextImpl>(text);
	}
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

CellInterface::Value Cell::EmptyImpl::GetValue() const {
	return {};
}

std::string Cell::EmptyImpl::GetText() const {
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

Cell::FormulaImpl::FormulaImpl(std::string text) {
	using namespace std::literals;
	try {
		formula_ = ParseFormula(text);
	}
	catch (...) {
		throw FormulaException("Formula parsing error"s);
	}
}

Cell::Value Cell::FormulaImpl::GetValue() const {
	auto result{ formula_->Evaluate() };
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