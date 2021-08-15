#pragma once

#include "common.h"
#include "formula.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

class Cell : public CellInterface {
public:
	Cell(SheetInterface& sheet_);
	~Cell();

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;

	std::vector<Position> GetReferencedCells() const override;

private:
	class Impl {
	public:
		virtual Value GetValue() const = 0;
		virtual std::string GetText() const = 0;
		virtual void ClearCache() = 0;
		virtual std::vector<Position> GetReferencedCells() const = 0;
	};

	class EmptyImpl : public Impl {
	public:
		Value GetValue() const override;
		std::string GetText() const override;
		void ClearCache() override;
		std::vector<Position> GetReferencedCells() const override;
	};

	class TextImpl : public Impl {
	public:
		TextImpl(std::string value);
		Value GetValue() const override;
		std::string GetText() const override;
		void ClearCache() override;
		std::vector<Position> GetReferencedCells() const override;
	private:
		std::string value_;
	};

	class FormulaImpl : public Impl {
	public:
		FormulaImpl(const SheetInterface& sheet, std::string text);
		Value GetValue() const override;
		std::string GetText() const override;
		void ClearCache() override;
		std::vector<Position> GetReferencedCells() const override;
	private:
		const SheetInterface& sheet_;
		std::unique_ptr<FormulaInterface> formula_;
		std::optional<CellInterface::Value> cached_value_;
	};

	std::unique_ptr<Impl> impl_;
	SheetInterface& sheet_;
	std::unordered_set<Cell*> dependent_cells_;
	std::vector<Position> referenced_cells_;

	void ClearDependentCellsCache();
	void UpdateDependencies(std::unique_ptr<Impl>& new_impl);
};