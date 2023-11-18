#include "BDDHelper.hpp"
#include <expected>
#include <utility>

namespace bddHelper
{
  BDDHelper::BDDHelper(std::vector< bdd > objects, std::vector< bdd > props, std::vector< bdd > values) :
    o_(std::move(objects)),
    p_(std::move(props)),
    v_(std::move(values))
  {
    assert(("Incorrect size found",
            o_.size() == nObjs and
            p_.size() == nObjs * nProps and
            v_.size() == nObjs * nProps * nVals));
  }

  bdd BDDHelper::getObj_(House obj)
  {
    auto val = toNum(obj);
    return o_[val];
  }

  bdd BDDHelper::getProp_(House obj, Property prop)
  {
    auto objNum = toNum(obj);
    auto propNum = toNum(prop);
    auto index = objNum * nObjs + propNum;
    return p_[index];
  }
}

#ifdef GTEST_TESTING

#include <gtest/gtest.h>
#include <ranges>
#include "TestFixture.hpp"

bddHelper::BDDHelper::BDDHelper()
{}

TEST_F(VarsSetupFixture, BDDHelperbasic)
{
  using namespace bddHelper;
  EXPECT_EQ(nTotalVars, 5*5*5+5*5+5);
  EXPECT_EQ(vars.size(), nTotalVars);
  EXPECT_EQ(objs.size(), 5);
  EXPECT_EQ(props.size(), 5*5);
  EXPECT_EQ(vals.size(), 5*5*5);
  EXPECT_EQ(objs[4], vars[4]);
  EXPECT_EQ(props[0], vars[5]);
  EXPECT_EQ(props[1], vars[6]);
  EXPECT_EQ(vals[0], vars[5*5 + 5]);
  EXPECT_EQ(h.getProp_(House::FIRST, Property::ANIMAL), props[3]);
  EXPECT_EQ(h.getHouseAndVal(House::THIRD, H_Color::BLUE),
    objs[2] & props[2*5] & vals[2*5*5 + 2]);
  EXPECT_EQ(h.getHouseAndVal(House::SECOND, Treat::BOUNTY),
    objs[1] & props[1*5+4] & vals[1*5*5 + 4*5 + 3]);
}

#endif
