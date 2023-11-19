#include "Conditions.hpp"
#include <ranges>
#include <tuple>
#include <optional>
#include <algorithm>
#include <numeric>
#include <execution>

// TODO fix neighbours
// TODO add left right skleika

using namespace bddHelper;

namespace
{
  constexpr int leftNeighbour = -2;
  constexpr int rightNeighbour = -1;
  constexpr bool useSkleika = false;

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addLeftNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addRightNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  std::optional< Object > getNeighbour_(Object obj, int neighbourOffset);
  std::optional< Object > getLeftNeighbour(Object obj);
  std::optional< Object > getRightNeighbour(Object obj);
  std::vector< Object > getNeighbours(Object obj);

  bdd equal(bdd a, bdd b);
  bdd notEqual(std::vector< bdd > v1, std::vector< bdd > v2);

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto resultFormulaToAdd = bdd_false();
    for (auto i : std::views::iota(0, BDDHelper::nObjs))
    {
      auto house = static_cast< Object >(i);
      auto formulas = bdd_true();
      std::apply([&formulas, &h, &house](auto &&... args) {
        ((formulas &= h.getObjectVal(house, args)), ...);
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
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(neighbObj, value2));
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
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(*neighbObj, value2));
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
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(*neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  std::optional< Object > getLeftNeighbour(Object obj)
  {
    return getNeighbour_(obj, leftNeighbour);
  }

  std::optional< Object > getRightNeighbour(Object obj)
  {
    return getNeighbour_(obj, rightNeighbour);
  }

  std::optional< Object > getNeighbour_(Object obj, int neighbourOffset)
  {
    auto objNum = toNum(obj);
    auto neighbObjNum = objNum + neighbourOffset;
    if (neighbObjNum >= 0 and neighbObjNum < BDDHelper::nObjs)
      return static_cast< Object >(neighbObjNum);
    if (useSkleika)
    {
      if (neighbObjNum > 0)
        return static_cast< Object >(neighbObjNum % BDDHelper::nObjs);
      else
        return static_cast< Object >(BDDHelper::nObjs + neighbObjNum % BDDHelper::nObjs);
    }
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

  bdd equal(bdd a, bdd b)
  {
    return (a & b) | ((not a) & (not b));
  }

  bdd notEqual(std::vector< bdd > a, std::vector< bdd > b)
  {
    assert(a.size() == 4 && a.size() == b.size());
    return std::inner_product(
      a.begin(),
      a.end(),
      b.begin(),
      bdd_false(),
      std::bit_or< bdd >(),
      [](bdd a, bdd b) {
      return not equal(a, b);
    });
  }

  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    for (auto propNum : std::views::iota(0, BDDHelper::nProps))
    {
      auto prop = static_cast< Property >(propNum);
      for (auto objNum1 : std::views::iota(0, BDDHelper::nObjs))
      {
        auto obj1 = static_cast< Object >(objNum1);
        for (auto objNum2 : std::views::iota(objNum1 + 1, BDDHelper::nObjs))
        {
          auto obj2 = static_cast< Object >(objNum2);
          auto range = std::views::iota(0, BDDHelper::nVals);
          std::for_each(std::execution::par, range.begin(), range.end(),
            [&](auto valNum) {
            builder.addConditionTh(notEqual(h.getValueVars(obj1, prop), h.getValueVars(obj2, prop)));
          });
        }
      }
    }
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

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    // builder.addCondition(h.getObjectVal(Object::FIRST, Nation::BELORUS));
  }

  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Nation::UKRAINE, Animal::DOG), h, builder);
    addLoopCondition(std::make_tuple(Nation::BELORUS, Animal::CAT), h, builder);
    addLoopCondition(std::make_tuple(Nation::GRUZIN, Animal::REPTILIES), h, builder);
    addLoopCondition(std::make_tuple(Nation::HISPANE, Animal::HOMYAK), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHINA, Animal::FISH), h, builder);
    addLoopCondition(std::make_tuple(Nation::RUSSIAN, Animal::HORSE), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHE4ENCI, Animal::BIRD), h, builder);
    addLoopCondition(std::make_tuple(Nation::ARMENIAN, Animal::LION), h, builder);
    addLoopCondition(std::make_tuple(Nation::KAZAH, Animal::ELEPHANT), h, builder);

    addLoopCondition(std::make_tuple(Nation::UKRAINE, Plant::MALINA), h, builder);
    addLoopCondition(std::make_tuple(Nation::BELORUS, Plant::CHERRY), h, builder);
    addLoopCondition(std::make_tuple(Nation::GRUZIN, Plant::KRIZH), h, builder);
    addLoopCondition(std::make_tuple(Nation::HISPANE, Plant::KLUBN), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHINA, Plant::VINOGR), h, builder);
    addLoopCondition(std::make_tuple(Nation::RUSSIAN, Plant::SLIVA), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHE4ENCI, Plant::GRUSHA), h, builder);
    addLoopCondition(std::make_tuple(Nation::ARMENIAN, Plant::APPLE), h, builder);
    addLoopCondition(std::make_tuple(Nation::KAZAH, Plant::PINEAPPLE), h, builder);

    addLoopCondition(std::make_tuple(Nation::UKRAINE, static_cast< Color >(0)), h, builder);
    addLoopCondition(std::make_tuple(Nation::BELORUS, static_cast< Color >(1)), h, builder);
    addLoopCondition(std::make_tuple(Nation::GRUZIN, static_cast< Color >(2)), h, builder);
    addLoopCondition(std::make_tuple(Nation::HISPANE, static_cast< Color >(3)), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHINA, static_cast< Color >(4)), h, builder);
    addLoopCondition(std::make_tuple(Nation::RUSSIAN, static_cast< Color >(5)), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHE4ENCI, static_cast< Color >(6)), h, builder);
    addLoopCondition(std::make_tuple(Nation::ARMENIAN, static_cast< Color >(7)), h, builder);
    addLoopCondition(std::make_tuple(Nation::KAZAH, static_cast< Color >(8)), h, builder);
  }

  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLeftNeighbors(Color::RED, Color::GREEN, h, builder);
  }

  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addNeighbors(Color::RED, Color::GREEN, h, builder);
  }
}

namespace conditions
{
  void addConditions(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addFirstCondition(h, builder);
    addSecondCondition(h, builder);
    // addThirdCondition(h, builder);
    // addFourthCondition(h, builder);
    addUniqueCondition(h, builder);
    addValuesUpperBoundCondition(h, builder);
  }
}

#ifdef GTEST_TESTING
#include <gtest/gtest.h>
#include <ranges>
#include "TestFixture.hpp"
//TODO remove o and p vectors

TEST_F(VarsSetupFixture, Conditions_Equality)
{
  using namespace bddHelper;
  EXPECT_EQ(equal(v[0][0][0], v[0][0][0]), bdd_true());
  EXPECT_EQ(equal(v[0][0][0], not v[0][0][0]), bdd_false());
  EXPECT_EQ(notEqual(v[0][0], { v[0][0][0], not v[0][0][1], v[0][0][2], v[0][0][3] }), bdd_true());
}

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
  EXPECT_TRUE(*res == Object::FIRST);

  res = getRightNeighbour(Object::NINETH);
  EXPECT_TRUE(!useSkleika || res.has_value());
  EXPECT_TRUE(!useSkleika || *res == Object::EIGTH);
}

TEST_F(VarsSetupFixture, Conditions_LoopCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addLoopCondition(std::make_tuple(Color::RED), h, build);
  auto expectedResult = h.getObjectVal(Object::FIRST, Color::RED);
  expectedResult |= h.getObjectVal(Object::SECOND, Color::RED);
  expectedResult |= h.getObjectVal(Object::THIRD, Color::RED);
  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED);
  expectedResult |= h.getObjectVal(Object::SIXTH, Color::RED);
  expectedResult |= h.getObjectVal(Object::SEVENTH, Color::RED);
  expectedResult |= h.getObjectVal(Object::EIGTH, Color::RED);
  expectedResult |= h.getObjectVal(Object::NINETH, Color::RED);
  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_NeighborsCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getObjectVal(Object::SECOND, Color::RED) & h.getObjectVal(Object::FIRST, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::THIRD, Color::RED) & h.getObjectVal(Object::FIRST, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::THIRD, Color::RED) & h.getObjectVal(Object::SECOND, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::SECOND, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::THIRD, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::THIRD, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::FOURTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::SIXTH, Color::RED) & h.getObjectVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SIXTH, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::SEVENTH, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SEVENTH, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::EIGTH, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::EIGTH, Color::RED) & h.getObjectVal(Object::SEVENTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::NINETH, Color::RED) & h.getObjectVal(Object::SEVENTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::NINETH, Color::RED) & h.getObjectVal(Object::EIGTH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_LeftNeighborsCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addLeftNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getObjectVal(Object::THIRD, Color::RED) & h.getObjectVal(Object::FIRST, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::SECOND, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::THIRD, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SIXTH, Color::RED) & h.getObjectVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SEVENTH, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::EIGTH, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::NINETH, Color::RED) & h.getObjectVal(Object::SEVENTH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_RightNeighborsCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addRightNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getObjectVal(Object::SECOND, Color::RED) & h.getObjectVal(Object::FIRST, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::THIRD, Color::RED) & h.getObjectVal(Object::SECOND, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::THIRD, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SIXTH, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SEVENTH, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::EIGTH, Color::RED) & h.getObjectVal(Object::SEVENTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::NINETH, Color::RED) & h.getObjectVal(Object::EIGTH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

#endif
