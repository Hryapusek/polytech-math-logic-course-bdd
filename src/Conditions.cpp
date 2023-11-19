#include "Conditions.hpp"
#include <ranges>
#include <tuple>
#include <optional>
#include <algorithm>
#include <numeric>
#include <execution>
#include <functional>

using namespace bddHelper;

namespace std
{
  template< class T >
  bool between(T const &val, T const &lo, T const &hi)
  {
    return between( val, lo, hi, std::less_equal< T >() );
  }

  template< class T, class Compare >
  bool between(T const &val, T const &lo, T const &hi, Compare comp)
  {
    return comp(lo, val) && comp(val, hi);
  }
}

namespace
{
  // X 0 1 2
  //Y
  //0
  //1
  //2

  std::vector leftNeighbourXYOffset = { 0, -1 };
  std::vector rightNeighbourXYOffset = { 1, 1 };
  constexpr bool vertSkleika = false;
  constexpr bool horSkleika = false;
  constexpr bool useSkleika = vertSkleika || horSkleika;

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addLeftNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addRightNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  std::optional< Object > getNeighbour_(Object obj, std::vector< int > neighbourXYOffset);
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
      auto obj = static_cast< Object >(i);
      auto formulas = bdd_true();
      std::apply([&formulas, &h, &obj](auto &&... args) {
        ((formulas &= h.getObjectVal(obj, args)), ...);
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
    return getNeighbour_(obj, leftNeighbourXYOffset);
  }

  std::optional< Object > getRightNeighbour(Object obj)
  {
    auto res = getNeighbour_(obj, rightNeighbourXYOffset);
    return res;
  }

  std::optional< Object > getNeighbour_(Object obj, std::vector< int > neighbourXYOffset)
  {
    assert(neighbourXYOffset.size() == 2);
    struct Point
    {
      int x;
      int y;
      constexpr bool operator==(const Point &rhs) const
      {
        return x == rhs.x && y == rhs.y;
      }
    };
    auto pointToObj =
      [](Point p) -> Object {
        assert(std::between(p.x, 0, 2) and std::between(p.y, 0, 2));
        return static_cast< Object >(p.x + p.y * 3);
      };
    auto normX =
      [](Point p) -> Point {
        assert(!std::between(p.x, 0, 2));
        Point res = p;
        if (p.x < 0)
          res.x = 3 + p.x % 3;
        else
          res.x = p.x % 3;
        return res;
      };
    auto normY =
      [](Point p) -> Point {
        assert(!std::between(p.y, 0, 2));
        Point res = p;
        if (p.y < 0)
          res.y = 3 + p.y % 3;
        else
          res.y = p.y % 3;
        return res;
      };
    assert((normX({ 3, 0 }) == Point{ 0, 0 }));
    assert((normY({ 0, 3 }) == Point{ 0, 0 }));
    assert((normX({ -1, 0 }) == Point{ 2, 0 }));
    assert((normY({ 0, -1 }) == Point{ 0, 2 }));
    assert((normX(normY({ -1, -1 })) == Point{ 2, 2 }));
    assert((normX(normY({ 4, 3 })) == Point{ 1, 0 }));
    auto objNum = toNum(obj);
    Point objPos = { objNum % 3, objNum / 3 };
    Point neighbObjPos = { objPos.x + neighbourXYOffset[0], objPos.y + neighbourXYOffset[1] };
    if (!std::between(neighbObjPos.x, 0, 2) and !std::between(neighbObjPos.y, 0, 2))
    {
      if (!vertSkleika or !horSkleika)
        return std::nullopt;
      return pointToObj(normX(normY(neighbObjPos)));
    }
    if (!std::between(neighbObjPos.x, 0, 2))
    {
      if (!horSkleika)
        return std::nullopt;
      return pointToObj(normX(neighbObjPos));
    }
    if (!std::between(neighbObjPos.y, 0, 2))
    {
      if (!vertSkleika)
        return std::nullopt;
      return pointToObj(normY(neighbObjPos));
    }
    return pointToObj(neighbObjPos);
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
    auto propRange = std::views::iota(0, BDDHelper::nProps);
    std::for_each(std::execution::par, propRange.begin(), propRange.end(),
      [&](auto propNum) {
      auto prop = static_cast< Property >(propNum);
      auto obj1Range = std::views::iota(0, BDDHelper::nObjs);
      std::for_each(std::execution::par, obj1Range.begin(), obj1Range.end(),
        [&](auto objNum1) {
        auto obj1 = static_cast< Object >(objNum1);
        auto obj2Range = std::views::iota(objNum1 + 1, BDDHelper::nObjs);
        std::for_each(std::execution::par, obj2Range.begin(), obj2Range.end(),
          [&](auto objNum2) {
          auto obj2 = static_cast< Object >(objNum2);
          builder.addConditionTh(notEqual(h.getValueVars(obj1, prop), h.getValueVars(obj2, prop)));
        });
      });
    });
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
    if (not useSkleika)
    {

      builder.addCondition(h.getObjectVal(Object::FIRST, Nation::UKRAINE));
      builder.addCondition(h.getObjectVal(Object::SECOND, Nation::BELORUS));
      builder.addCondition(h.getObjectVal(Object::THIRD, Nation::GRUZIN));
      // builder.addCondition(h.getObjectVal(Object::FOURTH, Nation::HISPANE));
      // builder.addCondition(h.getObjectVal(Object::FIFTH, Nation::CHINA));
      // builder.addCondition(h.getObjectVal(Object::SIXTH, Nation::RUSSIAN));
      // builder.addCondition(h.getObjectVal(Object::SEVENTH, Nation::CHE4ENCI));
      // builder.addCondition(h.getObjectVal(Object::EIGTH, Nation::ARMENIAN));
      // builder.addCondition(h.getObjectVal(Object::NINETH, Nation::KAZAH));
    }
    else
    {
      // builder.addCondition(h.getObjectVal(Object::FIRST, Nation::UKRAINE));
      // builder.addCondition(h.getObjectVal(Object::SECOND, Nation::BELORUS));
      // builder.addCondition(h.getObjectVal(Object::THIRD, Nation::GRUZIN));
      builder.addCondition(h.getObjectVal(Object::FOURTH, Nation::HISPANE));
      builder.addCondition(h.getObjectVal(Object::FIFTH, Nation::CHINA));
      builder.addCondition(h.getObjectVal(Object::SIXTH, Nation::RUSSIAN));
      // builder.addCondition(h.getObjectVal(Object::SEVENTH, Nation::CHE4ENCI));
      // builder.addCondition(h.getObjectVal(Object::EIGTH, Nation::ARMENIAN));
      // builder.addCondition(h.getObjectVal(Object::NINETH, Nation::KAZAH));
    }
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

    addLoopCondition(std::make_tuple(Nation::UKRAINE, Color::RED), h, builder);
    addLoopCondition(std::make_tuple(Nation::BELORUS, Color::GREEN), h, builder);
    addLoopCondition(std::make_tuple(Nation::GRUZIN, Color::BLUE), h, builder);
    addLoopCondition(std::make_tuple(Nation::HISPANE, Color::YELLOW), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHINA, Color::WHITE), h, builder);
    addLoopCondition(std::make_tuple(Nation::RUSSIAN, Color::PURPLE), h, builder);
    addLoopCondition(std::make_tuple(Nation::CHE4ENCI, Color::BROWN), h, builder);
    addLoopCondition(std::make_tuple(Nation::ARMENIAN, Color::AQUA), h, builder);
    addLoopCondition(std::make_tuple(Nation::KAZAH, Color::BEIGE), h, builder);
  }

  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    // addLeftNeighbors(Color::RED, Color::GREEN, h, builder);
  }

  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    if (not useSkleika)
    {
      addNeighbors(Nation::CHINA, Nation::ARMENIAN, h, builder);
      addNeighbors(Nation::RUSSIAN, Nation::KAZAH, h, builder);
      addNeighbors(Nation::HISPANE, Nation::CHE4ENCI, h, builder);
    }
    else
    {
      addNeighbors(Nation::GRUZIN, Nation::ARMENIAN, h, builder);
      addNeighbors(Nation::UKRAINE, Nation::KAZAH, h, builder);
      addNeighbors(Nation::BELORUS, Nation::CHE4ENCI, h, builder);
    }
  }
}

namespace conditions
{
  void addConditions(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addFirstCondition(h, builder);
    addSecondCondition(h, builder);
    // addThirdCondition(h, builder);
    addFourthCondition(h, builder);
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

TEST(Neighbours, leftNeighbourCheckNoSkleika)
{
  if (useSkleika)
    GTEST_SKIP();
  auto res = getLeftNeighbour(Object::FIFTH);
  EXPECT_TRUE(res.has_value());
  EXPECT_TRUE(*res == Object::SECOND);
}

TEST(Neighbours, rightNeighbourCheckNoSkleika)
{
  if (useSkleika)
    GTEST_SKIP();
  auto res = getRightNeighbour(Object::SECOND);
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(*res, Object::SIXTH);
}

TEST(Neighbours, leftNeighbourCheckSkleika)
{
  if (!vertSkleika || !horSkleika)
    GTEST_SKIP();
  auto res = getLeftNeighbour(Object::FIRST);
  EXPECT_TRUE(res.has_value());
  EXPECT_TRUE(*res == Object::SEVENTH);
}

TEST(Neighbours, rightNeighbourCheckSkleika)
{
  if (!vertSkleika || !horSkleika)
    GTEST_SKIP();
  auto res = getRightNeighbour(Object::NINETH);
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(*res, Object::FIRST);
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

TEST_F(VarsSetupFixture, Conditions_NeighborsCondition_without_skleika)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  if (horSkleika || vertSkleika)
    GTEST_SKIP();
  addNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getObjectVal(Object::FIRST, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SECOND, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::EIGTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::NINETH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::FIRST, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::SECOND, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::SIXTH, Color::RED) & h.getObjectVal(Object::THIRD, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::SEVENTH, Color::RED) & h.getObjectVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::EIGTH, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::NINETH, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);

  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_LeftNeighborsCondition_without_skleika)
{
  if (horSkleika || vertSkleika)
    GTEST_SKIP();
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addLeftNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::FIRST, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::SECOND, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::SIXTH, Color::RED) & h.getObjectVal(Object::THIRD, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::SEVENTH, Color::RED) & h.getObjectVal(Object::FOURTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::EIGTH, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::NINETH, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

TEST_F(VarsSetupFixture, Conditions_RightNeighborsCondition_without_skleika)
{
  if (horSkleika || vertSkleika)
    GTEST_SKIP();
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addRightNeighbors(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getObjectVal(Object::FIRST, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SECOND, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::EIGTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::NINETH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

#endif
