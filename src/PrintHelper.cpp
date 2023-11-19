#include "PrintHelper.hpp"

std::string to_string(bddHelper::Object obj)
{
  using namespace bddHelper;
  switch (obj)
  {
    case Object::FIRST:   return "Object #1";
    case Object::SECOND:  return "Object #2";
    case Object::THIRD:   return "Object #3";
    case Object::FOURTH:  return "Object #4";
    case Object::FIFTH:   return "Object #5";
    case Object::SIXTH:   return "Object #6";
    case Object::SEVENTH: return "Object #7";
    case Object::EIGTH:   return "Object #8";
    case Object::NINETH:  return "Object #9";
  }
}

std::string to_string(bddHelper::Property prop)
{
  using namespace bddHelper;
  switch (prop)
  {
    case Property::ANIMAL:   return "Animal";
    case Property::NATION:   return "Nation";
    case Property::COLOR:    return "Color";
    case Property::PLANT:    return "Plant";
  }
}

std::string to_string(bddHelper::Color col)
{
  using namespace bddHelper;
  switch (col)
  {
    case Color::AQUA: return "AQUA";
    case Color::BEIGE: return "BEIGE";
    case Color::BLUE: return "BLUE";
    case Color::BROWN: return "BROWN";
    case Color::GREEN: return "GREEN";
    case Color::PURPLE: return "PURPLE";
    case Color::RED: return "RED";
    case Color::WHITE: return "WHITE";
    case Color::YELLOW: return "YELLOW";
  }
}

std::string to_string(bddHelper::Nation nat)
{
  using namespace bddHelper;
  switch (nat)
  {
    case Nation::ARMENIAN: return "ARMENIAN";
    case Nation::BELORUS: return "BELORUS";
    case Nation::CHE4ENCI: return "CHE4ENCI";
    case Nation::CHINA: return "CHINA";
    case Nation::GRUZIN: return "GRUZIN";
    case Nation::HISPANE: return "HISPANE";
    case Nation::KAZAH: return "KAZAH";
    case Nation::RUSSIAN: return "RUSSIAN";
    case Nation::UKRAINE: return "UKRAINE";
  }
}

std::string to_string(bddHelper::Plant plant)
{
  using namespace bddHelper;
  switch (plant)
  {
    case Plant::APPLE: return "APPLE";
    case Plant::CHERRY: return "CHERRY";
    case Plant::GRUSHA: return "GRUSHA";
    case Plant::KLUBN: return "KLUBN";
    case Plant::KRIZH: return "KRIZH";
    case Plant::MALINA: return "MALINA";
    case Plant::PINEAPPLE: return "PINEAPPLE";
    case Plant::SLIVA: return "SLIVA";
    case Plant::VINOGR: return "VINOGR";
  }
}

std::string to_string(bddHelper::Animal animal)
{
  using namespace bddHelper;
  switch (animal)
  {
    case Animal::BIRD: return "BIRD";
    case Animal::CAT: return "CAT";
    case Animal::DOG: return "DOG";
    case Animal::ELEPHANT: return "ELEPHANT";
    case Animal::FISH: return "FISH";
    case Animal::HOMYAK: return "HOMYAK";
    case Animal::HORSE: return "HORSE";
    case Animal::LION: return "LION";
    case Animal::REPTILIES: return "REPTILIES";
  }
}
