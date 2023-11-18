#include <iostream>
#include <vector>
#include <ranges>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include <gtest/gtest.h>

using namespace bddHelper;

/**
 * @return std::vector< bdd > with [ o2 o1 o0 p2 p1 p0 v2 v1 v0 ]
 * @note bdd_ithvar contains [o0 o1 o2 p0 p1 p2 v0 v1 v2]
 */
std::vector< bdd > createVariables()
{
  bdd_setvarnum(9);
  std::vector< bdd > vars(9);
  for (auto i : std::views::iota(0, 9))
  {
    auto varsIndex = i / 3 * 3 + (2 - i % 3);
    vars[i] = bdd_ithvar(varsIndex);
  }
  return vars;
}

int main()
{
  bdd_init(1000, 100);
  std::vector< bdd > vars = createVariables();
  BDDHelper h(vars);
  BDDFormulaBuilder builder;
  ::testing::InitGoogleTest();
  auto res = RUN_ALL_TESTS();
  bdd_done();
  return res;
}
