#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include "Conditions.hpp"
#include "PrintHelper.hpp"

using namespace bddHelper;

template < class T > using vect = std::vector< T >;

constexpr int nObjs = bddHelper::BDDHelper::nObjs;
constexpr int nProps = bddHelper::BDDHelper::nProps;
constexpr int nVals = bddHelper::BDDHelper::nVals;
constexpr int nValueBits = bddHelper::BDDHelper::nValueBits;
constexpr int nValuesVars = bddHelper::BDDHelper::nValuesVars;
constexpr int nTotalVars = bddHelper::BDDHelper::nTotalVars;

std::once_flag once;
std::string varset;

void fun(char *varset_, int size)
{
  std::call_once(once, [&]() {
    varset = std::string(varset_, size);
  });
}

void printProp(Property prop, int valNum)
{
  switch (prop)
  {
    case Property::ANIMAL:
      std::cout << to_string(static_cast< Animal >(valNum)) << '\n';
      break;

    case Property::COLOR:
      std::cout << to_string(static_cast< Color >(valNum)) << '\n';
      break;

    case Property::NATION:
      std::cout << to_string(static_cast< Nation >(valNum)) << '\n';
      break;

    case Property::PLANT:
      std::cout << to_string(static_cast< Plant >(valNum)) << '\n';
      break;
  }
}

void printObjects()
{
  for (auto objNum : std::views::iota(0, nObjs))
  {
    auto obj = static_cast< Object >(objNum);
    std::cout << to_string(obj) << " {\n";
    for (auto propNum : std::views::iota(0, nProps))
    {
      auto prop = static_cast< Property >(propNum);
      std::cout << '\t' << to_string(prop) << ": ";
      auto baseIndex = objNum * nProps * nValueBits + propNum * nValueBits;
      int valNum = (varset.at(baseIndex)) * 2 * 2 * 2 +
                   (varset.at(baseIndex + 1)) * 2 * 2 +
                   (varset.at(baseIndex + 2)) * 2 +
                   (varset.at(baseIndex + 3));
      printProp(prop, valNum);
    }
    std::cout << "}\n";
  }
}

int main()
{
  bdd_init(300000000, 10000000);
  bdd_setvarnum(BDDHelper::nTotalVars);
  std::vector< bdd > vars;
  using namespace bddHelper;
  vars = std::vector< bdd > (nTotalVars);
  {
    int i = 0;
    std::generate(std::begin(vars), std::end(vars),
      [&i]() {
      return bdd_ithvar(i++);
    });
  }
  auto v = vect< vect< vect< bdd > > >(nObjs);
  for (auto objNum : std::views::iota(0, nObjs))
  {
    v[objNum] = vect< vect< bdd > >(nProps);
    for (auto propNum : std::views::iota(0, nProps))
    {
      auto baseIndex = objNum * nProps * nValueBits + propNum * nValueBits;
      v[objNum][propNum] = vect< bdd >{
        bdd_ithvar(baseIndex + 0),
        bdd_ithvar(baseIndex + 1),
        bdd_ithvar(baseIndex + 2),
        bdd_ithvar(baseIndex + 3) };
    }
  }
  BDDHelper h(v);
  BDDFormulaBuilder builder;
  conditions::addConditions(h, builder);
  std::cout << "Bdd formula created.\n";
  std::cout << bdd_satcount(builder.result()) << '\n';
  bdd_allsat(builder.result(), fun);
  printObjects();
  bdd_done();
  return 0;
}
