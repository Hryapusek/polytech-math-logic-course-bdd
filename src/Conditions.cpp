#include "Conditions.hpp"
#include <vector>
#include <ranges>

using namespace bddHelper;

namespace
{
  const int HOUSE_COUNT = 5;
  void addLoopCondition(bdd formula, BDDHelper &h, BDDFormulaBuilder &builder);
  
  template < class V_t1, class V_t2 >
  void addNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  void addLoopCondition(bdd formula, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto formulaToAdd = bdd_false();
    for (auto i : std::views::iota(0, HOUSE_COUNT))
    {
      auto house = static_cast< House >(i);
      formulaToAdd |= (h.getHouse(house) & formula);
    }
    builder.addCondition(formulaToAdd);
  }

  template < class V_t1, class V_t2 >
  void addNeighbors(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto formulaToAdd = bdd_false();
    for (auto i : std::views::iota(0, HOUSE_COUNT - 1))
    {
      auto houseNum = static_cast< House >(i);
      if (i != 0)
      {
        auto prevHouseNum = static_cast< House >(i - 1);
        formulaToAdd |= (h.getHouseAndVal(houseNum, value1) & h.getHouseAndVal(prevHouseNum, value2));
      }
      if (i != HOUSE_COUNT - 1)
      {
        auto nextHouseNum = static_cast< House >(i + 1);
        formulaToAdd |= (h.getHouseAndVal(houseNum, value1) & h.getHouseAndVal(nextHouseNum, value2));
      }
    }
    builder.addCondition(formulaToAdd);
  }

  void addFirstCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addSecondCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addThirdCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addFourthCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addFifthCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addSixthCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addSeventhCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addEighthCondition(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder);
  void addNinethCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addTenthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addEleventhCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addTwelvethCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addThirteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  void addFourteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto ukraineNation = h.getPropAndVal(Nation::UKRAINE);
    auto redHouse = h.getPropAndVal(H_Color::RED);
    addLoopCondition(ukraineNation & redHouse, h, builder);
  }

  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto belarus = h.getPropAndVal(Nation::BELORUS);
    auto dog = h.getPropAndVal(Animal::DOG);
    addLoopCondition(belarus & dog, h, builder);
  }

  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto greenHouse = h.getPropAndVal(H_Color::GREEN);
    auto malina = h.getPropAndVal(Plant::MALINA);
    addLoopCondition(greenHouse & malina, h, builder);
  }

  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto gruzin = h.getPropAndVal(Nation::GRUZIN);
    auto vinograd = h.getPropAndVal(Plant::VINOGR);
    addLoopCondition(gruzin & vinograd, h, builder);
  }

  void addFifthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto greenHouse = h.getPropAndVal(H_Color::GREEN);
    auto whiteHouse = h.getPropAndVal(H_Color::WHITE);
    auto formulaToAdd = bdd_false();
    for (auto i : std::views::iota(0, HOUSE_COUNT - 1))
    {
      auto greenHouseNum = static_cast< House >(i + 1);
      auto whiteHouseNum = static_cast< House >(i);
      formulaToAdd |= (h.getHouse(whiteHouseNum) & whiteHouse
                       & h.getHouse(greenHouseNum) & greenHouse);
    }
    builder.addCondition(formulaToAdd);
  }

  void addSixthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto twix = h.getPropAndVal(Treat::TWIX);
    auto cat = h.getPropAndVal(Animal::CAT);
    addLoopCondition(cat & twix, h, builder);
  }

  void addSeventhCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto yellow = h.getPropAndVal(H_Color::YELLOW);
    auto sneackers = h.getPropAndVal(Treat::SNICKERS);
    addLoopCondition(yellow & sneackers, h, builder);
  }

  void addEighthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    builder.addCondition(h.getHouseAndVal(House::THIRD, Plant::KRIZH));
  }

  void addNinethCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    builder.addCondition(h.getHouseAndVal(House::FIFTH, Nation::HISPANE));
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
    addLoopCondition(h.getPropAndVal(Treat::KITKAT) & h.getPropAndVal(Plant::KLUBN), h, builder);
  }

  void addThirteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addLoopCondition(h.getPropAndVal(Nation::CHINA) & h.getPropAndVal(Treat::BOUNTY), h, builder);
  }

  void addFourteenthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    addNeighbors(Nation::HISPANE, H_Color::BLUE, h, builder);
  }
}

namespace conditions
{
  void addConditions(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder)
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
