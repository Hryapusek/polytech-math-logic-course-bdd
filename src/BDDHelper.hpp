#ifndef BDD_HELPER_HPP
#define BDD_HELPER_HPP

#include <vector>
#include "bdd.h"
#include <type_traits>
#include <cassert>

enum class Object
{
  FIRST,
  SECOND,
  THIRD,
  FOURTH,
  FIFTH
};
enum class Property
{
  H_COLOR,
  NATION,
  PLANT,
  ANIMAL,
  CHOCO
};
enum class House
{
  RED,
  GREEN,
  BLUE,
  YELLOW,
  WHITE
};
enum class Nation
{
  UKRAINE,
  BELORUS,
  GRUZIN,
  HISPANE,
  CHINA
};
enum class Plant
{
  MALINA,
  SMOROD,
  KRIZH,
  KLUBN,
  VINOGR
};
enum class Animal
{
  DOG,
  CAT,
  PARROT,
  HOMYAK,
  FISH
};
enum class Treat
{
  TWIX,
  SNICKERS,
  MARS,
  BOUNTY,
  KITKAT
};

class BDDHelper
{
public:
  /**
   * @brief Construct a new BDDHelper object.
   *
   * @param vars contains o2 o1 o0 p2 p1 p0 v2 v1 v0
   */
  BDDHelper(std::vector< bdd > vars);
  
  /**
   * @return Combination of o0 o1 o2 with respect to given value
   * @note For example:
   * @note Object::FIRST return !o0 & !o1 & !o2
   * @note Object::THIRD return !o0 & o1 & 1o2
   * @note etc...
   */
  bdd getObj(Object obj);

  /**
   * @return Combination of p0 p1 p2 with respect to given value
   * @note For example:
   * @note Property::H_COLOR return !p0 & !p1 & !p2
   * @note Property::PLANT return !p0 & p1 & !p2
   * @note etc...
   */
  bdd getProp(Property prop);

  /**
   * @tparam V_t - must be one of House Nation Plant Animal Treat enum type
   * @param value 
   * @return Combination of v0 v1 v2 with respect to given value
   * @note For example:
   * @note House::RED return !v0 & !v1 & !v2
   * @note Nation::BELARUS return !v0 & !v1 & v2
   * @note etc...
   */
  template< class V_t,
    class = std::enable_if_t<
      std::is_same_v< V_t, House > ||
      std::is_same_v< V_t, Nation > ||
      std::is_same_v< V_t, Plant > ||
      std::is_same_v< V_t, Animal > ||
      std::is_same_v< V_t, Treat >
      > >
  bdd getVal(V_t value);

private:
  std::vector< bdd > o_;
  std::vector< bdd > p_;
  std::vector< bdd > v_;
  bdd fromNum(int num, std::vector< bdd > &var);
};

template < class V_t, class enable_if_t >
inline bdd BDDHelper::getVal(V_t value)
{
  auto val = static_cast< int >(value);
  return fromNum(val, v_);
}

#endif
