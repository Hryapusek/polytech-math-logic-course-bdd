#include "Conditions.hpp"
#include <ranges>
#include <tuple>
#include <optional>

using namespace bddHelper;

namespace
{
  constexpr int leftNeighbour = -2;
  constexpr int rightNeighbour = 2;
  constexpr bool useSkleika = false;

  std::optional< Object > getLeftNeighbour(Object obj)
  {
    auto objNum = toNum(obj);
    auto leftObjNum = objNum + leftNeighbour;
    if (leftObjNum >= 0)
      return static_cast< Object >(leftObjNum);
    if (useSkleika)
      return static_cast< Object >(BDDHelper::nObjs + leftObjNum);
    return std::nullopt;
  }

  std::optional< Object > getRightNeighbour(Object obj)
  {
    auto objNum = toNum(obj);
    auto rightObjNum = objNum + rightNeighbour;
    if (rightObjNum < BDDHelper::nObjs)
      return static_cast< Object >(rightObjNum);
    if (useSkleika)
      return static_cast< Object >(rightObjNum % BDDHelper::nObjs);
    return std::nullopt;
  }

  std::vector< Object > getNeighbours(Object obj)
  {
    auto left = getLeftNeighbour(obj);
    auto right = getRightNeighbour(obj);
    std::vector< Object > resArr;
    if (left)
      resArr.push_back(*left);
    if (right)
      resArr.push_back(*right);
    return resArr;
  }

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addLeftNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addRightNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto resultFormulaToAdd = bdd_false();
    for (auto i : std::views::iota(0, BDDHelper::nObjs))
    {
      auto house = static_cast< Object >(i);
      auto formulas = bdd_true();
      std::apply([&formulas, &h, &house](auto &&... args) {
        ((formulas &= h.getHouseAndVal(house, args)), ...);
      }, values);
      resultFormulaToAdd |= formulas;
    }
    builder.addCondition(resultFormulaToAdd);
  }

  template < class V_t1, class V_t2 >
  void addNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      for (auto neighbObj : getNeighbours(obj))
        resultFormulaToAdd |= (h.getHouseAndVal(obj, value1) & h.getHouseAndVal(neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  template < class V_t1, class V_t2 >
  void addLeftNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      if (auto neighbObj = getLeftNeighbour(obj); neighbObj.has_value())
        resultFormulaToAdd |= (h.getHouseAndVal(obj, value1) & h.getHouseAndVal(*neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  template < class V_t1, class V_t2 >
  void addRightNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      if (auto neighbObj = getRightNeighbour(obj); neighbObj.has_value())
        resultFormulaToAdd |= (h.getHouseAndVal(obj, value1) & h.getHouseAndVal(*neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  bdd notEqual(std::vector< bdd > v1, std::vector< bdd > v2);

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  bdd equal(bdd a, bdd b)
  {
    return (a & b) | ((not a) & (not b));
  }

  bdd notEqual(std::vector< bdd > a, std::vector< bdd > b)
  {
    assert(a.size() == 4 && a.size() == b.size());
    return (not equal(a[0], b[0])) bitor
           (not equal(a[1], b[1])) bitor
           (not equal(a[2], b[2])) bitor
           (not equal(a[3], b[3]));
  }

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Nation::UKRAINE, Color::RED), h, builder);
    addLoopCondition(std::make_tuple(Nation::UKRAINE, Animal::REPTILIES), h, builder);
    addLoopCondition(std::make_tuple(Nation::UKRAINE, Plant::PINEAPPLE), h, builder);
  }

  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto resFormulaToAdd = bdd_true();
    for (auto propNum : std::views::iota(0, BDDHelper::nProps))
    {
      auto prop = static_cast< Property >(propNum);
      for (auto objNum1 : std::views::iota(0, BDDHelper::nObjs))
      {
        auto obj1 = static_cast< Object >(objNum1);
        for (auto objNum2 : std::views::iota(objNum1 + 1, BDDHelper::nObjs))
        {
          auto obj2 = static_cast< Object >(objNum2);
          resFormulaToAdd &= notEqual(h.getValueVars(obj1, prop), h.getValueVars(obj2, prop));
        }
      }
    }
    builder.addCondition(resFormulaToAdd);
  }

  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto resFormulaToAdd = bdd_true();
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      for (auto propNum : std::views::iota(0, BDDHelper::nProps))
      {
        auto prop = static_cast< Property >(propNum);
        for (auto valNum : std::views::iota(9, 16))
        {
          resFormulaToAdd &= not h.numToBinUnsafe(valNum, h.getValueVars(obj, prop));
        }
      }
    }
    builder.addCondition(resFormulaToAdd);
  }

  // void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  // {
  //   addLoopCondition(std::make_tuple(Nation::BELORUS, Animal::DOG), h, builder);
  // }

  // void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  // {
  //   addLoopCondition(std::make_tuple(Color::GREEN, Plant::MALINA), h, builder);
  // }

  // void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  // {
  //   addLoopCondition(std::make_tuple(Nation::GRUZIN, Plant::VINOGR), h, builder);
  // }
}

namespace conditions
{
  void addConditions(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    // addFirstCondition(h, builder);
    addUniqueCondition(h, builder);
    addValuesUpperBoundCondition(h, builder);
    // TODO add distinct values condition
    // For each property:
    //   For each object:
    //     For each object1:
    //        prop(obj1) != prop(obj2)
    // addSecondCondition(h, builder);
    // addThirdCondition(h, builder);
    // addFourthCondition(h, builder);
  }
}

#ifdef GTEST_TESTING
#include <gtest/gtest.h>
#include <ranges>
#include "TestFixture.hpp"

TEST(Neighbours, leftNeighbourCheck)
{
  auto res = getLeftNeighbour(Object::THIRD);
  EXPECT_TRUE(res.has_value());
  EXPECT_TRUE(*res == Object::FIRST);

  res = getLeftNeighbour(Object::FIRST);
  EXPECT_TRUE(!useSkleika || res.has_value());
  EXPECT_TRUE(!useSkleika || *res == Object::EIGTH);
}

TEST(Neighbours, rightNeighbourCheck)
{
  auto res = getRightNeighbour(Object::SECOND);
  EXPECT_TRUE(res.has_value());
  EXPECT_TRUE(*res == Object::FOURTH);

  res = getRightNeighbour(Object::NINETH);
  EXPECT_TRUE(!useSkleika || res.has_value());
  EXPECT_TRUE(!useSkleika || *res == Object::SECOND);
}

TEST_F(VarsSetupFixture, Conditions_LoopCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addLoopCondition(std::make_tuple(Color::RED), h, build);
  auto expectedResult = h.getHouseAndVal(Object::FIRST, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::SECOND, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::THIRD, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::FOURTH, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::FIFTH, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::SIXTH, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::SEVENTH, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::EIGTH, Color::RED);
  expectedResult |= h.getHouseAndVal(Object::NINETH, Color::RED);
  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_NeighborsCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getHouseAndVal(Object::FIRST, Color::RED) & h.getHouseAndVal(Object::THIRD, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SECOND, Color::RED) & h.getHouseAndVal(Object::FOURTH, Color::GREEN);

  expectedResult |= h.getHouseAndVal(Object::THIRD, Color::RED) & h.getHouseAndVal(Object::FIRST, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::THIRD, Color::RED) & h.getHouseAndVal(Object::FIFTH, Color::GREEN);

  expectedResult |= h.getHouseAndVal(Object::FOURTH, Color::RED) & h.getHouseAndVal(Object::SECOND, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::FOURTH, Color::RED) & h.getHouseAndVal(Object::SIXTH, Color::GREEN);

  expectedResult |= h.getHouseAndVal(Object::FIFTH, Color::RED) & h.getHouseAndVal(Object::THIRD, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::FIFTH, Color::RED) & h.getHouseAndVal(Object::SEVENTH, Color::GREEN);

  expectedResult |= h.getHouseAndVal(Object::SIXTH, Color::RED) & h.getHouseAndVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SIXTH, Color::RED) & h.getHouseAndVal(Object::EIGTH, Color::GREEN);

  expectedResult |= h.getHouseAndVal(Object::SEVENTH, Color::RED) & h.getHouseAndVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SEVENTH, Color::RED) & h.getHouseAndVal(Object::NINETH, Color::GREEN);

  expectedResult |= h.getHouseAndVal(Object::EIGTH, Color::RED) & h.getHouseAndVal(Object::SIXTH, Color::GREEN);

  expectedResult |= h.getHouseAndVal(Object::NINETH, Color::RED) & h.getHouseAndVal(Object::SEVENTH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_LeftNeighborsCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addLeftNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getHouseAndVal(Object::THIRD, Color::RED) & h.getHouseAndVal(Object::FIRST, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::FOURTH, Color::RED) & h.getHouseAndVal(Object::SECOND, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::FIFTH, Color::RED) & h.getHouseAndVal(Object::THIRD, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SIXTH, Color::RED) & h.getHouseAndVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SEVENTH, Color::RED) & h.getHouseAndVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::EIGTH, Color::RED) & h.getHouseAndVal(Object::SIXTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::NINETH, Color::RED) & h.getHouseAndVal(Object::SEVENTH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_RightNeighborsCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addRightNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getHouseAndVal(Object::FIRST, Color::RED) & h.getHouseAndVal(Object::THIRD, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SECOND, Color::RED) & h.getHouseAndVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::THIRD, Color::RED) & h.getHouseAndVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::FOURTH, Color::RED) & h.getHouseAndVal(Object::SIXTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::FIFTH, Color::RED) & h.getHouseAndVal(Object::SEVENTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SIXTH, Color::RED) & h.getHouseAndVal(Object::EIGTH, Color::GREEN);
  expectedResult |= h.getHouseAndVal(Object::SEVENTH, Color::RED) & h.getHouseAndVal(Object::NINETH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

#endif
