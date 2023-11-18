#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include "Conditions.hpp"

using namespace bddHelper;

template < class T > using vect = std::vector< T >;

int main()
{
  bdd_init(300000000, 10000000);
  bdd_setvarnum(BDDHelper::nTotalVars);
  constexpr int nObjs = bddHelper::BDDHelper::nObjs;
  constexpr int nProps = bddHelper::BDDHelper::nProps;
  constexpr int nVals = bddHelper::BDDHelper::nVals;
  constexpr int nValueBits = bddHelper::BDDHelper::nValueBits;
  constexpr int nObjsVars = bddHelper::BDDHelper::nObjsVars;
  constexpr int nPropsVars = bddHelper::BDDHelper::nPropsVars;
  constexpr int nValuesVars = bddHelper::BDDHelper::nValuesVars;
  constexpr int nTotalVars = bddHelper::BDDHelper::nTotalVars;
  std::vector< bdd > vars;
  vect< bdd > o;
  vect< vect< bdd > > p;
  vect< vect< vect< bdd > > > v;
  vars = std::vector< bdd > (nTotalVars);
  {
    int i = 0;
    std::generate(std::begin(vars), std::end(vars),
      [&i]() {
      return bdd_ithvar(i++);
    });
  }
  o = std::vector< bdd > (vars.begin(), vars.begin() + nObjs);
  p = vect< vect< bdd > >(nObjs);
  for (auto objNum : std::views::iota(0, nObjs))
  {
    p[objNum] = vect< bdd >(vars.begin() + nObjsVars + objNum * nProps, vars.begin() + nObjs + objNum * nProps + nProps);
  }
  v = vect< vect< vect< bdd > > >(nObjs);
  for (auto objNum : std::views::iota(0, nObjs))
  {
    v[objNum] = vect< vect< bdd > >(nProps);
    for (auto propNum : std::views::iota(0, nProps))
    {
      auto baseIndex = nObjsVars + nPropsVars + objNum * nProps * nValueBits + propNum * nValueBits;
      v[objNum][propNum] = vect< bdd >{
        bdd_ithvar(baseIndex + 0),
        bdd_ithvar(baseIndex + 1),
        bdd_ithvar(baseIndex + 2),
        bdd_ithvar(baseIndex + 3) };
    }
  }
  BDDHelper h(o, p, v);
  BDDFormulaBuilder builder;
  conditions::addConditions(h, builder);
  std::cout << bdd_satcount(builder.result()) << '\n';
  bdd_done();
  return 0;
}
