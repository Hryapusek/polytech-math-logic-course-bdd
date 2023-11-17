#include "BDDHelper.hpp"

namespace bddHelper
{
  bdd BDDHelper::fromNum(int num, std::vector< bdd > &var)
  {
    switch (num)
    {
      case 0:
        return not var[2] & not var[1] & not var[0];

      case 1:
        return not var[2] & not var[1] & var[0];

      case 2:
        return not var[2] & var[1] & not var[0];

      case 3:
        return not var[2] & var[1] & var[0];

      case 4:
        return var[2] & not var[1] & not var[0];

      default:
        assert(("Value must be between 0 and 4", false));
    }
  }

  BDDHelper::BDDHelper(std::vector< bdd > vars)
  {
    assert(("Size of vars must be 9.", vars.size() == 9));
    v_ = std::vector(vars.rbegin(), vars.rbegin() + 3);
    p_ = std::vector(vars.rbegin() + 3, vars.rbegin() + 6);
    o_ = std::vector(vars.rbegin() + 6, vars.rend());
    assert(("Size of all the vectors v p and o must be 3",
            v_.size() == p_.size() and
            v_.size() == o_.size() and
            v_.size() == 3));
  }

  bdd BDDHelper::getObj(Object obj)
  {
    auto val = static_cast< int >(obj);
    return fromNum(val, o_);
  }

  bdd BDDHelper::getProp(Property prop)
  {
    auto val = static_cast< int >(prop);
    return fromNum(val, p_);
  }
}


#ifdef GTEST_TESTING

#include <gtest/gtest.h>
#include <ranges>

TEST(BDDHelper, basic)
{
  using namespace bddHelper;
  std::vector< bdd > vars(9);
  for (auto i : std::views::iota(0, 9))
  {
    auto varsIndex = i / 3 * 3 + (2 - i % 3);
    EXPECT_TRUE(i != 0 or varsIndex == 2);
    EXPECT_TRUE(i != 1 or varsIndex == 1);
    EXPECT_TRUE(i != 2 or varsIndex == 0);
    EXPECT_TRUE(i != 3 or varsIndex == 5);
    EXPECT_TRUE(i != 4 or varsIndex == 4);
    vars[i] = bdd_ithvar(varsIndex);
  }
  auto v = std::vector(vars.rbegin(), vars.rbegin() + 3);
  auto p = std::vector(vars.rbegin() + 3, vars.rbegin() + 6);
  auto o = std::vector(vars.rbegin() + 6, vars.rend());
  BDDHelper h(vars);
  EXPECT_EQ(h.getProp(Property::ANIMAL), p[0] & p[1] & not p[2]);
  EXPECT_EQ(h.getObjVal(Object::FIRST, House::BLUE),
    not o[0] & not o[1] & not o[2] &
    not p[0] & not p[1] & not p[2] &
    not v[0] & v[1] & not v[2]);
  EXPECT_EQ(h.getObjVal(Object::SECOND, Treat::BOUNTY),
    o[0] & not o[1] & not o[2] &
    not p[0] & not p[1] & p[2] &
    v[0] & v[1] & not v[2]);
  EXPECT_EQ(h.getPropVal(Treat::BOUNTY),
    not p[0] & not p[1] & p[2] &
    v[0] & v[1] & not v[2]);
}

#endif
