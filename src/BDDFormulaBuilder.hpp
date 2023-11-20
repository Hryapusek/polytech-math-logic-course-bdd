#ifndef BDD_FORMULA_BUILDER_HPP
#define BDD_FORMULA_BUILDER_HPP

#include "bdd.h"
#include <mutex>

class BDDFormulaBuilder
{
public:
  /**
   * Create empty formula.
   * Later we will add conditions using addCondition.
   */
  BDDFormulaBuilder();
  /**
   * Adds condition to formula.
   */
  void addCondition(bdd formula);
  /**
   * Same as previous, but thread safe.
   */
  void addConditionTh(bdd formula);
  /**
   * Return result bdd conditions formula.
   */
  bdd result();

private:
  bdd formula_;
  std::mutex mut_;
};

#endif
