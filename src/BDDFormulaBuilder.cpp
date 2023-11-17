#include "BDDFormulaBuilder.hpp"

BDDFormulaBuilder::BDDFormulaBuilder() :
  formula_(bdd_true())
{}

void BDDFormulaBuilder::addCondition(bdd formula)
{
  formula_ &= formula;
}

bdd BDDFormulaBuilder::result()
{
  return formula_;
}
