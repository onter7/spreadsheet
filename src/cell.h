#pragma once

#include "common.h"
#include "formula.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

class Cell : public CellInterface {
private:
	class Impl;
	class EmptyImpl;
	class TextImpl;
	class FormulaImpl;

public:
	Cell(SheetInterface& sheet_);
	~Cell();

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;

	std::vector<Position> GetReferencedCells() const override;

private:
	std::unique_ptr<Impl> impl_;
	SheetInterface& sheet_;
	std::unordered_set<Cell*> dependent_cells_;
	std::vector<Position> referenced_cells_;

	void ClearDependentCellsCache();
	void UpdateDependencies(std::unique_ptr<Impl>& new_impl);
};