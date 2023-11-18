#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include "Conditions.hpp"

using namespace bddHelper;

int main()
{
  bdd_init(10000000, 10000000);
  bdd_setvarnum(BDDHelper::nTotalVars);
  constexpr int nObjs = bddHelper::BDDHelper::nObjs;
  constexpr int nProps = bddHelper::BDDHelper::nProps;
  constexpr int nVals = bddHelper::BDDHelper::nVals;
  constexpr int nObjsVars = bddHelper::BDDHelper::nObjsVars;
  constexpr int nPropsVars = bddHelper::BDDHelper::nPropsVars;
  constexpr int nValuesVars = bddHelper::BDDHelper::nValuesVars;
  constexpr int nTotalVars = bddHelper::BDDHelper::nTotalVars;
  std::vector< bdd > vars;
  std::vector< bdd > objs;
  std::vector< bdd > props;
  std::vector< bdd > vals;
  using namespace bddHelper;
  vars = std::vector< bdd > (nTotalVars);
  {
    int i = 0;
    std::generate(std::begin(vars), std::end(vars),
      [&i]() {
      return bdd_ithvar(i++);
    });
  }
  objs = std::vector< bdd > (vars.begin(), vars.begin() + nObjs);
  props = std::vector< bdd > (vars.begin() + nObjsVars, vars.begin() + nObjsVars + nPropsVars);
  vals = std::vector< bdd > (vars.begin() + nObjsVars + nPropsVars, vars.end());
  BDDHelper h(objs, props, vals);
  BDDFormulaBuilder builder;
  conditions::addConditions(h, builder);
  std::cout << bdd_satcount(builder.result()) << '\n';
  bdd_done();
  return 0;
}
