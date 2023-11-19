#include "BDDHelper.hpp"
#include <expected>
#include <utility>
#include <algorithm>
#include <ranges>

namespace bddHelper
{
  BDDHelper::BDDHelper(vect< bdd > objects, vect< vect< bdd > > props, vect< vect< vect< bdd > > > values) :
    o_(std::move(objects)),
    p_(std::move(props)),
    v_(std::move(values))
  {
    assert(("Incorrect size found",
            o_.size() == nObjs and
            p_.size() == nObjs and
            p_[0].size() == nProps and
            p_.size() * p_[0].size() == nPropsVars and
            v_.size() == nObjs and
            v_[0].size() == nProps and
            v_.size() * v_[0].size() == nPropsVars and
            v_.size() * v_[0].size() * v_[0][0].size() == nValuesVars));
  }

  std::vector<bdd> BDDHelper::getValueVars(Object obj, Property prop)
  {
    auto objNum = toNum(obj);
    auto propNum = toNum(prop);
    return v_[objNum][propNum];
  }

  bdd BDDHelper::getObj_(Object obj)
  {
    auto val = toNum(obj);
    return o_[val];
  }

  bdd BDDHelper::getProp_(Object obj, Property prop)
  {
    auto objNum = toNum(obj);
    auto propNum = toNum(prop);
    return p_[objNum][propNum];
  }

  bdd BDDHelper::numToBin(int num, vect< bdd > vars)
  {
    assert(vars.size() == 4);
    assert(num >= 0 and num <= 8);
    return numToBinUnsafe(num, vars);
  }

  bdd BDDHelper::numToBinUnsafe(int num, vect<bdd> vars)
  {
    assert(vars.size() == 4);
    assert(num >= 0 and num <= 15);
    auto resFormula = bdd_true();
    auto currentNum = num;
    for (auto var : std::views::reverse(vars))
    {
      auto bit = currentNum % 2;
      currentNum /= 2;
      if (bit == 1)
        resFormula &= var;
      else
        resFormula &= not var;
    }
    return resFormula;
  }

  int toNum(Property value)
  {
    auto val = static_cast< int >(value);
    assert(("Bad value", val >= 0 and val <= 3));
    return val;
  }
}

#ifdef GTEST_TESTING

#include <gtest/gtest.h>
#include <ranges>
#include "TestFixture.hpp"

bddHelper::BDDHelper::BDDHelper()
{ }

TEST_F(VarsSetupFixture, BDDHelperbasic)
{
  using namespace bddHelper;
  EXPECT_EQ(nTotalVars, 9 * 4 * 4 + 9 * 4 + 9);
  EXPECT_EQ(vars.size(), nTotalVars);
  EXPECT_EQ(o[4], vars[4]);
  EXPECT_EQ(p[0][0], vars[9]);
  EXPECT_EQ(p[0][1], vars[10]);
  EXPECT_EQ(v[0][0][0], vars[9 * 4 + 9]);
  EXPECT_EQ(h.getProp_(Object::FIRST, Property::ANIMAL), p[0][3]);
  EXPECT_EQ(h.getObjectAndVal(Object::THIRD, Color::BLUE),
    o[2] & p[2][0] & not v[2][0][0] & not v[2][0][1] & v[2][0][2] & not v[2][0][3]);
  EXPECT_EQ(h.getObjectAndVal(Object::SECOND, Animal::BIRD),
    o[1] & p[1][3] & not v[1][3][0] & v[1][3][1] & v[1][3][2] & not v[1][3][3]);
}

#endif
