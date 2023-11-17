#include <iostream>
#include <vector>
#include <ranges>
#include "bdd.h"
#include "BDDHelper.hpp"
#include <gtest/gtest.h>

int main()
{
  using namespace bddHelper;
  bdd_init(1000, 100);
  bdd_setvarnum(9);
  ::testing::InitGoogleTest();
  auto res = RUN_ALL_TESTS();
  bdd_done();
  return res;
}
