#ifndef BDD_HELPER_HPP
#define BDD_HELPER_HPP

#include <vector>
#include <type_traits>
#include <cassert>
#include <utility>
#include <cmath>
#include "bdd.h"

/**
 * Wherever in prog you see GTEST_TESTING or gtest or anything
 * like this - just ignore that. I used it for my variant, to test
 * if functions work correctly. After you change variant - test wont work
 * properly. So just \b ignore \b them.
 */

#ifdef GTEST_TESTING // ignore

#include <gtest/gtest.h>

class VarsSetupFixture_BDDHelperbasic_Test;
class VarsSetupFixture;

#endif

namespace bddHelper
{
  /**
   * Here we have enum, that represents objects.
   */
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

  /**
   * This enum represents each object properties
   */
  enum class Property
  {
    COLOR,
    NATION,
    PLANT,
    ANIMAL
  };

  /**
   * Enums below represent each property values
   */

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

  /**
   * Funciton used to transform all but Property enums values
   * into int with size checking. Of course we can do static_cast
   * instead of toNum, but for example this
   * ```c++
   *    auto color = static_cast<Color>(10000); // Color must be from 0 to 8...
   *    auto colorNum = static_cast<int>(color); // Logic error, but no one knows...
   * ```
   * does NOT cause an error and leads to undefined behavour.
   * While this
   * ```c++
   *    auto color = static_cast<Color>(10000); // Color must be from 0 to 8...
   *    auto colorNum = toNum(color); // Error, program will terminate
   * ```
   * Will throw an assertion error and terminate program.
   * Defending from ourselves.
   */
  template< class Enum_Val_t >
  int toNum(Enum_Val_t value);

  /**
   * @overload
   * Same as previous one, but works exactly with Property enum.
   */
  int toNum(Property value);

  // Just ignore that. Purely C plus plus shit
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
    /// See all these in \b main.cpp

    static constexpr int nObjs = 9;
    static constexpr int nProps = 4;
    static constexpr int nVals = 9;
    static constexpr int nValueBits = 4;
    static constexpr int nValuesVars = nObjs * nProps * nValueBits;
    static constexpr int nTotalVars = nValuesVars;

    // See BDDHelper.cpp file
    BDDHelper(vect< vect< vect< bdd > > > structedVars);

    // See below
    template< class V_t >
    bdd getObjectVal(Object obj, V_t value);

    std::vector< bdd > getValueVars(Object obj, Property prop);

    bdd numToBin(int num, vect< bdd > vars);

    bdd numToBinUnsafe(int num, vect< bdd > vars);

  private:
  #ifdef GTEST_TESTING
    friend class ::VarsSetupFixture_BDDHelperbasic_Test;
    friend class ::VarsSetupFixture;
  #endif
    vect< vect< vect< bdd > > > structVars_;
    vect< vect< vect< bdd > > > values_;

    BDDHelper();
  };

  /**
   * See values_ array description in constructor comments.
   */
  template < class V_t >
  inline bdd BDDHelper::getObjectVal(Object obj, V_t value)
  {
    static_assert(traits_::IsValueType_v< V_t >, "Value must be one of properties type");
    auto objNum = toNum(obj);
    /**
     * PropertyFromValueEnum_v
     * For example, this will convert
     * - Color::RED to Property::Color.
     * - Animal::Bird to Property::Animal.
     * 
     * It's way better than passing Property as additional parameter,
     * because we can by mistake pass Property::Animal and Nation::4e4enec.
     * This will lead to undefined behaviour, and no one can check this error...
     */
    auto propNum = toNum(traits_::PropertyFromValueEnum_v< V_t >);
    auto valNum = toNum(value);
    return values_[objNum][propNum][valNum];
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
