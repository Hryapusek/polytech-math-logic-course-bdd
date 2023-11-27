#include "Conditions.hpp"
#include <ranges>
#include <tuple>
#include <optional>
#include <algorithm>
#include <numeric>
#include <execution>
#include <functional>
#include <type_traits>

using namespace bddHelper;

namespace // Dont read this...
{
  template < class ... Trest >
  struct unique_types;

  template < class T1, class T2, class ... Trest >
  struct unique_types< T1, T2, Trest ... >
    : unique_types< T1, T2 >, unique_types< T1, Trest ... >, unique_types< T2, Trest ... > { };

  template < class T1, class T2 >
  struct unique_types< T1, T2 >
  {
    static_assert(!std::is_same< T1, T2 >::value, "Types must be unique");
  };

  template < class T1 >
  struct unique_types< T1 >
  { };
}

namespace std
{
  // True if low >= val <= hi;
  // Other words, true if value between low and hi
  template< class T >
  bool between(T const &val, T const &lo, T const &hi)
  {
    return between( val, lo, hi, std::less_equal< T >() );
  }

  // Just ignore that
  template< class T, class Compare >
  bool between(T const &val, T const &lo, T const &hi, Compare comp)
  {
    return comp(lo, val) && comp(val, hi);
  }
}

namespace
{
  /**
   * Use these to set neighbours.
   * For example, {0, -1} means neighbour
   *    N * N
   *    N O N
   *    N N N
   *
   * {1, 1} means neighbour
   *    N N N
   *    N O N
   *    N N *
   *
   * X is horizontal
   * Y is vertical.
   *    X
   *    0 1 2
   * Y 0
   *   1
   *   2
   */
  std::vector leftNeighbourXYOffset = { 0, -1 };
  std::vector rightNeighbourXYOffset = { 1, 1 };
  // Use to enable disable any skleika
  // Read about this at 30 page.
  constexpr bool vertSkleika = false;
  constexpr bool horSkleika = false;

  // Don't touch it...
  constexpr bool useSkleika = vertSkleika || horSkleika;

  // See below
  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
  template < class V_t1, class V_t2 >
  void addNeighbours(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
  template < class V_t1, class V_t2 >
  void addLeftNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
  template < class V_t1, class V_t2 >
  void addRightNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
  std::optional< Object > getNeighbour_(Object obj, std::vector< int > neighbourXYOffset);
  // See below
  std::optional< Object > getLeftNeighbour(Object obj);
  // See below
  std::optional< Object > getRightNeighbour(Object obj);
  // See below
  std::vector< Object > getNeighbours(Object obj);

  // See below
  bdd equal(bdd a, bdd b);
  // See below
  bdd notEqual(std::vector< bdd > v1, std::vector< bdd > v2);

  // See below
  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  /**
   * Sets second type condition.
   *
   * Examples:
   * ```
   * addLoopCondition({Nation::UKRAINE, Animal::DOG}, ...);
   * ```
   * Says ONE of objects must have both UKRAINE with DOG.
   *
   * ```
   * addLoopCondition({Nation::HISPANE, Color::YELLOW}, ...);
   * ```
   * Says ONE of objects must have both HISPANE with YELLOW color house.
   *
   * ```
   * addLoopCondition({Nation::HISPANE, Nation::GRUZIN}, ...);
   * ```
   * Says ONE of objects must have both HISPANE and GRUZIN.
   * This is impossible condition, so called controversy, so result
   * function will always return false.
   */
  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    // Helps to avoid controversy in conditions.
    unique_types< V_ts... > check_uniquness;
    auto resultFormulaToAdd = bdd_false();
    // Here we loop through objects and say that
    // current object must have all given values.
    for (auto i : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(i);
      auto formulas = bdd_true();
      std::apply([&formulas, &h, &obj](auto &&... args) {
        ((formulas &= h.getObjectVal(obj, args)), ...); //Here we say current object must have all given values.
      }, values);
      // Here we say that there must be
      // first object with all values or
      // second object with all values or
      // third object with all values or...
      resultFormulaToAdd |= formulas;
    }
    // Add result condition
    builder.addCondition(resultFormulaToAdd);
  }

  // This function says that there must be any neighbours
  // so that first must have value1 and
  //          second must have value2
  // No matter who is left and who is right.
  // Things that relate to skleika and some other shit
  // handled in getNeighbours function
  template < class V_t1, class V_t2 >
  void addNeighbours(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    // So we loop though the objects and say that
    // for ANY object...
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      // ...current object must have value1 and
      // current object's any neighbour must have value2
      for (auto neighbObj : getNeighbours(obj)) // According to skleika we may have or not neighbours of current object
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(neighbObj, value2));
    }
    // Add result condition to formula
    builder.addCondition(resultFormulaToAdd);
  }

  // This function says that there must be any LEFT neighbours
  // so that first must have value1 and
  //          second must have value2
  // Here we say, that second IS LEFT neighbour of first
  // Things that relate to skleika and some other shit
  // handled in getLeftNeighbour function
  template < class V_t1, class V_t2 >
  void addLeftNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    // So we loop though the objects and say that
    // for ANY object...
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      // ...current object must have value1 and
      // current object's LEFT neighbour must have value2
      if (auto neighbObj = getLeftNeighbour(obj); neighbObj.has_value())
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(*neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  // Read about left neighbour if need
  template < class V_t1, class V_t2 >
  void addRightNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
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

  // This function searches neighbour for obj
  // If neighbour exists - return neighbour object.
  // Else return None
  // Neighbour may not exist if skleika not enabled
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
    /**
     * Execute horizontal skleika.
     * For example
     * Point p{-1, 0} will be converted to {2, 0}
     * Point p{-2, 0} will be converted to {1, 0}
     * Point p{3, 0} will be converted to {0, 0}
     * Point p{4, 0} will be converted to {1, 0}
     */
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
    /**
     * Execute vertical skleika.
     * For example
     * Point p{0, -1} will be converted to {0, 2}
     * Point p{0, -2} will be converted to {0, 1}
     * Point p{0, 3} will be converted to {0, 0}
     * Point p{0, 4} will be converted to {0, 1}
     */
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
    auto objNum = toNum(obj); // First we convert obj to int
    Point objPos = { objNum % 3, objNum / 3 }; // next we calculate obj coordinates
    Point neighbObjPos = { objPos.x + neighbourXYOffset[0], objPos.y + neighbourXYOffset[1] }; // calcuate neighbour coords
    // Now we have 4 situations.
    // First is when both horizontal and vertical limits are exceeded
    // It means we need both vertical and horizontal skleika
    if (!std::between(neighbObjPos.x, 0, 2) and !std::between(neighbObjPos.y, 0, 2))
    {
      if (!vertSkleika or !horSkleika)
        return std::nullopt;
      return pointToObj(normX(normY(neighbObjPos)));
    }
    // Second is when only horizontal limit is exceeded
    // It means we need horizontal skleika
    if (!std::between(neighbObjPos.x, 0, 2))
    {
      if (!horSkleika)
        return std::nullopt;
      return pointToObj(normX(neighbObjPos));
    }
    // Third is when only vertical limit is exceeded
    // It means we need vertical skleika
    if (!std::between(neighbObjPos.y, 0, 2))
    {
      if (!vertSkleika)
        return std::nullopt;
      return pointToObj(normY(neighbObjPos));
    }
    // Fourth is when no limits are exceeded
    // No skleika is needed
    return pointToObj(neighbObjPos);
  }

  // Just return left and right neighbours.
  // May return empty array or only one neighbour.
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

  // Speaks for itself
  bdd equal(bdd a, bdd b)
  {
    return (a & b) | ((not a) & (not b));
  }

  // a and b each contain 4 bdd variables
  // We say return condition
  // a[0] != b[0] or a[1] != b[1] or ... a[3] != b[3]
  // It's like comparing two binary numbers. Actually that is it.
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

  // Here we say, that each property value must be used exactly one time
  // For example
  // Object::FIRST have Color::RED. That means SECOND, THIRD... can not have Color::RED
  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto propRange = std::views::iota(0, BDDHelper::nProps);
    //We loop over properties
    std::for_each(std::execution::par, propRange.begin(), propRange.end(),
      [&](auto propNum) {
      auto prop = static_cast< Property >(propNum);
      auto obj1Range = std::views::iota(0, BDDHelper::nObjs);
      // Loop over objects
      std::for_each(std::execution::par, obj1Range.begin(), obj1Range.end(),
        [&](auto objNum1) {
        auto obj1 = static_cast< Object >(objNum1);
        auto obj2Range = std::views::iota(objNum1 + 1, BDDHelper::nObjs);
        // Loop over objects start with obj1+1
        std::for_each(std::execution::par, obj2Range.begin(), obj2Range.end(),
          [&](auto objNum2) {
          auto obj2 = static_cast< Object >(objNum2);
          // We say that for current property
          // obj1 property value must be not equal to obj2 property value
          builder.addConditionTh(notEqual(h.getObjPropertyVars(obj1, prop), h.getObjPropertyVars(obj2, prop)));
        });
      });
    });
  }

  // Here we simply state that each object's properties values must be less than 9
  // In other words, each object's properties values must be NOT 9 NOT 10 NOT 11... NOT 15
  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    // Loop over objects
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      // Loop over properties
      for (auto propNum : std::views::iota(0, BDDHelper::nProps))
      {
        auto prop = static_cast< Property >(propNum);
        // Loop over possible property value numbers
        for (auto valNum : std::views::iota(9, 16))
        {
          // Say that for object obj property prop can not have value equal to valNum
          builder.addCondition(not h.numToBinUnsafe(valNum, h.getObjPropertyVars(obj, prop)));
        }
      }
    }
  }

  /**
   * Here we add conditions of type 1
   * For example
   * builder.addCondition(h.getObjectVal(Object::FIRST, Nation::UKRAINE));
   * This means that First object MUST have Property Nation with value Nation::UKRAINE
   */
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

  /**
   * Here we add conditions of type 2
   * For example
   * addLoopCondition(std::make_tuple(Nation::UKRAINE, Animal::DOG), h, builder);
   * This says next.
   * There MUST exist object that has BOTH Nation::UKRAINE and Animal::DOG.
   * No matter if it Object::FIRST or Object::SECOND or...
   */
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

  /**
   * Here we add conditions of type 3
   * These are addLeftNeighbour and addRightNeighbour
   * For example
   * addLeftNeighbour(Color::RED, Color::GREEN, h, builder);
   * This says next.
   * There MUST exist object that are Neighbours and
   * one have Color::RED and
   * HIS LEFT NEIGHBOUR have Color::GREEN.
   */
  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    /**
     * I didn't add any conditions, because i preferred
     * to use 4th type condition.
     */

    // addLeftNeighbour(Color::RED, Color::GREEN, h, builder);
  }

  /**
   * Here we add conditions of type 4
   * For example
   * addNeighbours(Nation::CHINA, Nation::ARMENIAN, h, builder);
   * This says next.
   * There MUST exist object that are Neighbours and
   * one have Nation::CHINA and
   * second have Nation::ARMENIAN.
   * 
   * Actually it uses addLeftNeighbour and addLeftNeighbour
   */
  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    if (not useSkleika)
    {
      addNeighbours(Nation::CHINA, Nation::ARMENIAN, h, builder);
      addNeighbours(Nation::RUSSIAN, Nation::KAZAH, h, builder);
      addNeighbours(Nation::HISPANE, Nation::CHE4ENCI, h, builder);
    }
    else
    {
      addNeighbours(Nation::GRUZIN, Nation::ARMENIAN, h, builder);
      addNeighbours(Nation::UKRAINE, Nation::KAZAH, h, builder);
      addNeighbours(Nation::BELORUS, Nation::CHE4ENCI, h, builder);
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
  addNeighbours(Color::RED, Color::GREEN, h, build);
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
  addLeftNeighbour(Color::RED, Color::GREEN, h, build);
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
  addRightNeighbour(Color::RED, Color::GREEN, h, build);
  auto expectedResult = h.getObjectVal(Object::FIRST, Color::RED) & h.getObjectVal(Object::FIFTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::SECOND, Color::RED) & h.getObjectVal(Object::SIXTH, Color::GREEN);

  expectedResult |= h.getObjectVal(Object::FOURTH, Color::RED) & h.getObjectVal(Object::EIGTH, Color::GREEN);
  expectedResult |= h.getObjectVal(Object::FIFTH, Color::RED) & h.getObjectVal(Object::NINETH, Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

#endif
