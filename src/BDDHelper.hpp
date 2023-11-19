#ifndef BDD_HELPER_HPP
#define BDD_HELPER_HPP

#include <vector>
#include <type_traits>
#include <cassert>
#include <utility>
#include <cmath>
#include "bdd.h"

#ifdef GTEST_TESTING

#include <gtest/gtest.h>

class VarsSetupFixture_BDDHelperbasic_Test;
class VarsSetupFixture;

#endif

namespace bddHelper
{
  enum class Object
  {
    FIRST,
    SECOND,
    THIRD,
    FOURTH,
    FIFTH,
    SIXTH,
    SEVENTH,
    EIGTH,
    NINETH
  };
  enum class Property
  {
    COLOR,
    NATION,
    PLANT,
    ANIMAL
  };
  enum class Color
  {
    RED,
    GREEN,
    BLUE,
    YELLOW,
    WHITE,
    PURPLE,
    BROWN,
    AQUA,
    BEIGE
  };
  enum class Nation
  {
    UKRAINE,
    BELORUS,
    GRUZIN,
    HISPANE,
    CHINA,
    RUSSIAN,
    CHE4ENCI,
    ARMENIAN,
    KAZAH
  };
  enum class Plant
  {
    MALINA,
    CHERRY,
    KRIZH,
    KLUBN,
    VINOGR,
    SLIVA,
    GRUSHA,
    APPLE,
    PINEAPPLE
  };
  enum class Animal
  {
    DOG,
    CAT,
    REPTILIES,
    HOMYAK,
    FISH,
    HORSE,
    BIRD,
    LION,
    ELEPHANT
  };

  template< class Enum_Val_t >
  int toNum(Enum_Val_t value);

  int toNum(Property value);

  namespace traits_
  {
    template
    < class V_t, class = std::enable_if_t<
      std::is_same_v< V_t, Color > ||
      std::is_same_v< V_t, Nation > ||
      std::is_same_v< V_t, Plant > ||
      std::is_same_v< V_t, Animal > > >
    struct IsValueType: std::true_type
    { };

    template < class V_t >
    struct IsValueType< V_t, int >: std::false_type
    { };

    template < class V_t, class = void >
    struct PropertyFromValueEnum
    {
      static_assert(false, "Incorrect value passed to research property type");
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Color > > >
    {
      static constexpr Property value = Property::COLOR;
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Nation > > >
    {
      static constexpr Property value = Property::NATION;
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Plant > > >
    {
      static constexpr Property value = Property::PLANT;
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Animal > > >
    {
      static constexpr Property value = Property::ANIMAL;
    };

    template < class V_t >
    constexpr Property PropertyFromValueEnum_v = PropertyFromValueEnum< V_t >::value;

    template < class V_t >
    constexpr bool IsValueType_v = IsValueType< V_t >::type::value;
  }

  class BDDHelper
  {
  public:
    template < class T > using vect = std::vector< T >;
    static constexpr int nObjs = 9;
    static constexpr int nProps = 4;
    static constexpr int nVals = 9;
    static constexpr int nValueBits = 4;
    static constexpr int nValuesVars = nObjs * nProps * nValueBits;
    static constexpr int nTotalVars = nValuesVars;

    BDDHelper(vect< vect< vect< bdd > > > values);

    template< class V_t >
    bdd getObjectVal(Object obj, V_t value);

    template< class V_t >
    bdd calcObjectVal_(Object obj, V_t value);

    std::vector< bdd > getValueVars(Object obj, Property prop);

    bdd numToBin(int num, vect< bdd > vars);

    bdd numToBinUnsafe(int num, vect< bdd > vars);

  private:
  #ifdef GTEST_TESTING
    friend class ::VarsSetupFixture_BDDHelperbasic_Test;
    friend class ::VarsSetupFixture;
  #endif
    vect< vect< vect< bdd > > > vars_;
    vect< vect< vect< bdd > > > values_;

    BDDHelper();
  };

  template < class V_t >
  inline bdd BDDHelper::getObjectVal(Object obj, V_t value)
  {
    static_assert(traits_::IsValueType_v< V_t >, "Value must be one of properties type");
    auto objNum = toNum(obj);
    auto propNum = toNum(traits_::PropertyFromValueEnum_v< V_t >);
    auto valNum = toNum(value);
    return values_[objNum][propNum][valNum];
  }

  template < class V_t >
  inline bdd BDDHelper::calcObjectVal_(Object obj, V_t value)
  {
    static_assert(traits_::IsValueType_v< V_t >, "Value must be one of properties type");
    auto objNum = toNum(obj);
    auto propNum = toNum(traits_::PropertyFromValueEnum_v< V_t >);
    auto valNum = toNum(value);
    return numToBin(valNum, vars_[objNum][propNum]);
  }

  template < class Enum_Val_t >
  int toNum(Enum_Val_t value)
  {
    static_assert(std::is_enum_v< Enum_Val_t >, "Value must be enum");
    int num = static_cast< int >(value);
    assert(("Invalid enum value found", num >= 0 and num <= 8));
    return num;
  }
}
#endif
