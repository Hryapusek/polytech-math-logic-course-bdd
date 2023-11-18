#include "Conditions.hpp"
#include <ranges>
#include <tuple>

using namespace bddHelper;

namespace
{
  const int HOUSE_COUNT = 5;
  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto resultFormulaToAdd = bdd_false();
    for (auto i : std::views::iota(0, HOUSE_COUNT))
    {
      auto house = static_cast< House >(i);
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
    for (auto i : std::views::iota(0, HOUSE_COUNT))
    {
      auto houseNum = static_cast< House >(i);
      if (i != 0)
      {
        auto prevHouseNum = static_cast< House >(i - 1);
        resultFormulaToAdd |= (h.getHouseAndVal(houseNum, value1) & h.getHouseAndVal(prevHouseNum, value2));
      }
      if (i != HOUSE_COUNT-1)
      {
        auto nextHouseNum = static_cast< House >(i + 1);
        resultFormulaToAdd |= (h.getHouseAndVal(houseNum, value1) & h.getHouseAndVal(nextHouseNum, value2));
      }
    }
    builder.addCondition(resultFormulaToAdd);
  }

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addFifthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addSixthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addSeventhCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addEighthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addNinethCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addTenthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addEleventhCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addTwelvethCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addThirteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addFourteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Nation::UKRAINE, H_Color::RED), h, builder);
  }

  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Nation::BELORUS, Animal::DOG), h, builder);
  }

  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(H_Color::GREEN, Plant::MALINA), h, builder);
  }

  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Nation::GRUZIN, Plant::VINOGR), h, builder);
  }

  void addFifthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto resultFormulaToAdd = bdd_false();
    for (auto i : std::views::iota(0, HOUSE_COUNT - 1))
    {
      auto nextHouseNum = static_cast< House >(i + 1);
      auto currentHouseNum = static_cast< House >(i);
      resultFormulaToAdd |= (h.getHouseAndVal(currentHouseNum, H_Color::WHITE) & h.getHouseAndVal(nextHouseNum, H_Color::GREEN));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  void addSixthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Treat::TWIX, Animal::CAT), h, builder);
  }

  void addSeventhCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(H_Color::YELLOW, Treat::SNICKERS), h, builder);
  }

  void addEighthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    builder.addCondition(h.getHouseAndVal(House::THIRD, Plant::KRIZH));
  }

  void addNinethCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    builder.addCondition(h.getHouseAndVal(House::FIFTH, Nation::HISPANE));
    builder.addCondition(h.getHouseAndVal(House::SECOND, Treat::MARS));
    builder.addCondition(h.getHouseAndVal(House::FOURTH, Nation::BELORUS));
  }

  void addTenthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addNeighbors(Treat::MARS, Animal::PARROT, h, builder);
  }

  void addEleventhCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addNeighbors(Animal::HOMYAK, Treat::SNICKERS, h, builder);
  }

  void addTwelvethCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Treat::KITKAT, Plant::KLUBN), h, builder);
  }

  void addThirteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(std::make_tuple(Nation::CHINA, Treat::BOUNTY), h, builder);
  }

  void addFourteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addNeighbors(Nation::HISPANE, H_Color::BLUE, h, builder);
  }
}

#ifdef GTEST_TESTING
#include <gtest/gtest.h>
#include <ranges>
#include "TestFixture.hpp"


TEST_F(VarsSetupFixture, Conditions_LoopCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addLoopCondition(std::make_tuple(H_Color::RED), h, build);
  auto expectedResult = h.getHouseAndVal(House::FIRST, H_Color::RED);
  expectedResult |= h.getHouseAndVal(House::SECOND, H_Color::RED);
  expectedResult |= h.getHouseAndVal(House::THIRD, H_Color::RED);
  expectedResult |= h.getHouseAndVal(House::FOURTH, H_Color::RED);
  expectedResult |= h.getHouseAndVal(House::FIFTH, H_Color::RED);
  EXPECT_EQ(build.result(), expectedResult);
}


TEST_F(VarsSetupFixture, Conditions_NeighborsCondition)
{
  using namespace bddHelper;
  BDDFormulaBuilder build;
  addNeighbors(H_Color::RED, H_Color::GREEN, h, build);
  auto expectedResult = h.getHouseAndVal(House::FIRST, H_Color::RED) & h.getHouseAndVal(House::SECOND, H_Color::GREEN);
  expectedResult |= h.getHouseAndVal(House::SECOND, H_Color::RED) & h.getHouseAndVal(House::FIRST, H_Color::GREEN);
  expectedResult |= h.getHouseAndVal(House::SECOND, H_Color::RED) & h.getHouseAndVal(House::THIRD, H_Color::GREEN);
  expectedResult |= h.getHouseAndVal(House::THIRD, H_Color::RED) & h.getHouseAndVal(House::SECOND, H_Color::GREEN);
  expectedResult |= h.getHouseAndVal(House::THIRD, H_Color::RED) & h.getHouseAndVal(House::FOURTH, H_Color::GREEN);
  expectedResult |= h.getHouseAndVal(House::FOURTH, H_Color::RED) & h.getHouseAndVal(House::THIRD, H_Color::GREEN);
  expectedResult |= h.getHouseAndVal(House::FOURTH, H_Color::RED) & h.getHouseAndVal(House::FIFTH, H_Color::GREEN);
  expectedResult |= h.getHouseAndVal(House::FIFTH, H_Color::RED) & h.getHouseAndVal(House::FOURTH, H_Color::GREEN);
  EXPECT_EQ(build.result(), expectedResult);
}

#endif

namespace conditions
{
  void addConditions(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addFirstCondition(h, builder);
    addSecondCondition(h, builder);
    addThirdCondition(h, builder);
    addFourthCondition(h, builder);
    addFifthCondition(h, builder);
    addSixthCondition(h, builder);
    addSeventhCondition(h, builder);
    addEighthCondition(h, builder);
    addNinethCondition(h, builder);
    addTenthCondition(h, builder);
    addEleventhCondition(h, builder);
    addTwelvethCondition(h, builder);
    addThirteenthCondition(h, builder);
    addFourteenthCondition(h, builder);
  }
}
