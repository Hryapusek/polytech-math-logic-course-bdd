#include "BDDFormulaBuilder.hpp"

BDDFormulaBuilder::BDDFormulaBuilder() :
  formula_(bdd_true())
{}

void BDDFormulaBuilder::addCondition(bdd formula)
{
  formula_ &= formula;
}

void BDDFormulaBuilder::addConditionTh(bdd formula)
{
  std::unique_lock lock(mut_);
  formula_ &= formula;
}

void BDDFormulaBuilder::setFormula(bdd formula)
{
  formula_ = formula;
}

bdd BDDFormulaBuilder::result()
{
  return formula_;
}
