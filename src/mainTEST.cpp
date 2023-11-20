#include <vector>
#include <ranges>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include <gtest/gtest.h>

// Ignore this whole file. Just for GTest

using namespace bddHelper;

int main()
{
  bdd_init(100000, 10000);
  bdd_setvarnum(BDDHelper::nTotalVars);
  ::testing::InitGoogleTest();
  auto res = RUN_ALL_TESTS();
  bdd_done();
  return res;
}
