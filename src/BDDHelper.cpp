#include "BDDHelper.hpp"
#include <expected>
#include <utility>
#include <algorithm>
#include <ranges>

namespace bddHelper
{
  /**
   * We have seen structedVars in main.cpp
   */
  BDDHelper::BDDHelper(vect< vect< vect< bdd > > > structedVars) :
    structVars_(std::move(structedVars))
  {
    // Check the sizes to be sure we haven't made a mistake yet...
    assert(("Incorrect size found",
            structVars_.size() == nObjs and
            structVars_[0].size() == nProps and
            structVars_[0][0].size() == nValueBits));
    /**
     * If we want to say, that second object's third property 
     * MUST have value 5 - we would say
     * ```
     * auto condition = not structVars_[1][2][0] & 
     *                  structVars_[1][2][1] &
     *                  not structVars_[1][2][2] & 
     *                  structVars_[1][2][3];
     * ```
     * because value 5 is 0101.
     * Our new array values_[1][2][5] will contain
     * exactly what 'condition' var contains.
     * It's like precomputed values optimization.
     */
    values_ = vect< vect< vect< bdd > > >(nObjs);
    for (auto objNum : std::views::iota(0, nObjs))
    {
      values_[objNum] = vect< vect< bdd > >(nProps);
      for (auto propNum : std::views::iota(0, nProps))
      {
        values_[objNum][propNum] = vect< bdd > (nVals);
        for (auto valNum : std::views::iota(0, nVals))
        {
          values_[objNum][propNum][valNum] = numToBin(valNum, structVars_[objNum][propNum]);
        }
      }
    }
  }

  std::vector< bdd > BDDHelper::getValueVars(Object obj, Property prop)
  {
    auto objNum = toNum(obj);
    auto propNum = toNum(prop);
    return structVars_[objNum][propNum];
  }

  bdd BDDHelper::numToBin(int num, vect< bdd > vars)
  {
    assert(vars.size() == 4);
    assert(num >= 0 and num <= 8);
    return numToBinUnsafe(num, vars);
  }

  bdd BDDHelper::numToBinUnsafe(int num, vect< bdd > vars)
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
  EXPECT_EQ(nTotalVars, 9 * 4 * 4);
  EXPECT_EQ(vars.size(), nTotalVars);
  EXPECT_EQ(v[0][0][0], vars[0]);
  EXPECT_EQ(v[0][1][0], vars[nValueBits]);
  EXPECT_EQ(h.getObjectVal(Object::THIRD, Color::BLUE),
    not v[2][0][0] & not v[2][0][1] & v[2][0][2] & not v[2][0][3]);
  EXPECT_EQ(h.getObjectVal(Object::SECOND, Animal::BIRD),
    not v[1][3][0] & v[1][3][1] & v[1][3][2] & not v[1][3][3]);
}

#endif
