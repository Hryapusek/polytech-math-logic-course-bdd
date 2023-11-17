#include <iostream>
#include <vector>
#include <ranges>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"

/*
   Украинец живет в доме красного цвета.
   Значит должен быть хотя бы один дом, где живет украинец и красный цвет
 */

using namespace bddHelper;

void addLoopCondition(bdd formula, BDDFormulaBuilder &builder, BDDHelper &h);
void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder);
void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder);
void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder);
void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
void addFifthCondition(BDDHelper &h, BDDFormulaBuilder &builder);

void addLoopCondition(bdd formula, BDDFormulaBuilder &builder, BDDHelper &h)
{
  auto formulaToAdd = bdd_true();
  for (auto i : std::views::iota(0, 5))
  {
    auto house = static_cast< House >(i);
    formulaToAdd |= (h.getHouse(house) & formula);
  }
  builder.addCondition(formulaToAdd);
}

void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder)
{
  auto ukraineNation = h.getPropVal(Nation::UKRAINE);
  auto redHouse = h.getPropVal(H_Color::RED);
  addLoopCondition(ukraineNation & redHouse, builder, h);
}

void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder)
{
  auto belarus = h.getPropVal(Nation::BELORUS);
  auto dog = h.getPropVal(Animal::DOG);
  addLoopCondition(belarus & dog, builder, h);
}

void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder)
{
  auto greenHouse = h.getPropVal(H_Color::GREEN);
  auto malina = h.getPropVal(Plant::MALINA);
  addLoopCondition(greenHouse & malina, builder, h);
}

void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
{
  auto gruzin = h.getPropVal(Nation::GRUZIN);
  auto vinograd = h.getPropVal(Plant::VINOGR);
  addLoopCondition(gruzin & vinograd, builder, h);
}

void addFifthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
{
  auto greenHouse = h.getPropVal(H_Color::GREEN);
  auto whiteHouse = h.getPropVal(H_Color::WHITE);
  auto formulaToAdd = bdd_true();
  for (auto i : std::views::iota(0, 4))
  {
    auto greenHouseNum = static_cast< House >(i + 1);
    auto whiteHouseNum = static_cast< House >(i);
    formulaToAdd |= (h.getHouse(whiteHouseNum) & whiteHouse
                     & h.getHouse(greenHouseNum) & greenHouse);
  }
  builder.addCondition(formulaToAdd);
}

/**
 * @return std::vector< bdd > with [ o2 o1 o0 p2 p1 p0 v2 v1 v0 ]
 * @note bdd_ithvar contains [o0 o1 o2 p0 p1 p2 v0 v1 v2]
 */
std::vector< bdd > createVariables()
{
  bdd_setvarnum(9);
  std::vector< bdd > vars(9);
  for (auto i : std::views::iota(0, 9))
  {
    auto varsIndex = i / 3 * 3 + (2 - i % 3);
    vars[i] = bdd_ithvar(varsIndex);
  }
  return vars;
}

int main()
{
  bdd_init(1000, 100);
  std::vector< bdd > vars = createVariables();
  BDDHelper h(vars);
  BDDFormulaBuilder builder;
  bdd_done();
  return 0;
}
