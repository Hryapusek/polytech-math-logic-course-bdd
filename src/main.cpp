#include <iostream>
#include <vector>
#include <ranges>
#include "bdd.h"
#include "BDDHelper.hpp"

int main()
{
  bdd_init(1000, 100);
  bdd_setvarnum(9);
  std::vector<bdd> vars;
  for (auto i : std::views::iota(0, 9))
    vars.push_back(bdd_ithvar(i));
  BDDHelper h(vars);
  h.getProp(Property::ANIMAL);
  bdd_done();
  return 0;
}
