#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <string>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
	CheckPosition(pos);
	Resize(pos);
	if (!cells_[pos.row][pos.col]) {
		cells_[pos.row][pos.col] = std::make_unique<Cell>();
	}
	cells_[pos.row][pos.col]->Set(text);
	UpdatePrintableSize();
}

const CellInterface* Sheet::GetCell(Position pos) const {
	CheckPosition(pos);
	if (pos.row >= sheet_size_.rows || pos.col >= sheet_size_.cols) {
		return nullptr;
	}
	const auto& cell = cells_.at(pos.row).at(pos.col);
	return cell->GetText().empty() ? nullptr : cell.get();
}
CellInterface* Sheet::GetCell(Position pos) {
	CheckPosition(pos);
	if (pos.row > sheet_size_.rows - 1 || pos.col > sheet_size_.cols - 1) {
		return nullptr;
	}
	auto& cell = cells_.at(pos.row).at(pos.col);
	return cell->GetText().empty() ? nullptr : cell.get();
}

void Sheet::ClearCell(Position pos) {
	CheckPosition(pos);
	if (pos.row > sheet_size_.rows - 1 || pos.col > sheet_size_.cols - 1) {
		return;
	}
	if (cells_[pos.row][pos.col]) {
		cells_[pos.row][pos.col]->Clear();
		UpdatePrintableSize();
	}
}

Size Sheet::GetPrintableSize() const {
	return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
	for (int row = 0; row < printable_size_.rows; ++row) {
		bool first = true;
		for (int col = 0; col < printable_size_.cols; ++col) {
			if (!first) {
				output << '\t';
			}
			else {
				first = false;
			}
			if (cells_[row][col]) {
				std::visit(CellInterfaceValuePrinter{ output }, cells_[row][col]->GetValue());
			}
		}
		output << '\n';
	}
}
void Sheet::PrintTexts(std::ostream& output) const {
	for (int row = 0; row < printable_size_.rows; ++row) {
		bool first = true;
		for (int col = 0; col < printable_size_.cols; ++col) {
			if (!first) {
				output << '\t';
			}
			else {
				first = false;
			}
			if (cells_[row][col]) {
				output << cells_[row][col]->GetText();
			}
		}
		output << '\n';
	}
}

void Sheet::Resize(Position pos) {
	if (pos.row >= sheet_size_.rows || pos.col >= sheet_size_.cols) {
		const int rows = pos.row + 1;
		if (rows > sheet_size_.rows) {
			const int old_rows = sheet_size_.rows;
			cells_.resize(rows);
			for (int row = old_rows; row < rows; ++row) {
				cells_[row].resize(sheet_size_.cols);
			}
			sheet_size_.rows = rows;
		}
		const int cols = pos.col + 1;
		if (cols > sheet_size_.cols) {
			for (auto& row : cells_) {
				row.resize(cols);
			}
			sheet_size_.cols = cols;
		}
	}
}

void Sheet::CheckPosition(Position pos) const {
	if (!pos.IsValid()) {
		throw InvalidPositionException("Position {"s + std::to_string(pos.row) + ","s + std::to_string(pos.col) + "} is invalid"s);
	}
}

void Sheet::UpdatePrintableSize() {
	int max_non_empty_row = Position::NONE.row;
	int max_non_empty_col = Position::NONE.col;
	for (int row = 0; row < sheet_size_.rows; ++row) {
		for (int col = 0; col < sheet_size_.cols; ++col) {
			if (cells_[row][col] && !cells_[row][col]->GetText().empty()) {
				if (row > max_non_empty_row) {
					max_non_empty_row = row;
				}
				if (col > max_non_empty_col) {
					max_non_empty_col = col;
				}
			}
		}
	}
	printable_size_ = { max_non_empty_row + 1, max_non_empty_col + 1 };
}

void Sheet::CellInterfaceValuePrinter::operator()(const std::string& value) {
	out << value;
}

void Sheet::CellInterfaceValuePrinter::operator()(double value) {
	out << value;
}

void Sheet::CellInterfaceValuePrinter::operator()(FormulaError value) {
	out << '#' << value.what() << '!';
}

std::unique_ptr<SheetInterface> CreateSheet() {
	return std::make_unique<Sheet>();
}