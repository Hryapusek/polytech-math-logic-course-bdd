#ifndef BDD_FORMULA_BUILDER_HPP
#define BDD_FORMULA_BUILDER_HPP

#include "bdd.h"

class BDDFormulaBuilder
{
public:
  BDDFormulaBuilder();
  void addCondition(bdd formula);
  void setFormula(bdd formula);
  bdd result();

private:
  bdd formula_;
};

#endif
