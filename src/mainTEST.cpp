#include <iostream>
#include <vector>
#include <ranges>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include <gtest/gtest.h>

using namespace bddHelper;

int main()
{
  bdd_init(1000, 100);
  bdd_setvarnum(BDDHelper::nTotalVars);
  BDDFormulaBuilder builder;
  ::testing::InitGoogleTest();
  auto res = RUN_ALL_TESTS();
  bdd_done();
  return res;
}
