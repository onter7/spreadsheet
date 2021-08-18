#pragma once

#include "common.h"
#include "cell.h"

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

	struct PositionHasher {
		size_t operator()(const Position& pos) const {
			return static_cast<size_t>(pos.row) + 37 * static_cast<size_t>(pos.col);
		}
	};

	void Resize(Position pos);
	void ThrowIfInvalidPosition(Position pos) const;
	void UpdatePrintableSize();
	void ThrowIfCircularDependencyFound(const Position& src_pos, const std::vector<Position>& referenced_calls, std::unordered_set<Position, PositionHasher>& visited) const;
	CellInterface* GetCellImpl(Position pos) const;

	struct CellInterfaceValuePrinter {
		std::ostream& out;
		void operator()(const std::string& value);
		void operator()(double value);
		void operator()(FormulaError value);
	};
};