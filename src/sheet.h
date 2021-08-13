#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <memory>
#include <vector>

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

private:
	Size sheet_size_;
	Size printable_size_;
	std::vector<std::vector<std::unique_ptr<Cell>>> cells_;

	void Resize(Position pos);
	void CheckPosition(Position pos) const;
	void UpdatePrintableSize();

	struct CellInterfaceValuePrinter {
		std::ostream& out;
		void operator()(const std::string& value);
		void operator()(double value);
		void operator()(FormulaError value);
	};
};