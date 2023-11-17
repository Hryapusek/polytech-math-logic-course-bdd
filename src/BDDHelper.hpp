#ifndef BDD_HELPER_HPP
#define BDD_HELPER_HPP

#include <vector>
#include <type_traits>
#include <cassert>
#include "bdd.h"

namespace bddHelper
{
  enum class House
  {
    FIRST,
    SECOND,
    THIRD,
    FOURTH,
    FIFTH
  };
  enum class Property
  {
    HOUSE,
    NATION,
    PLANT,
    ANIMAL,
    TREAT
  };
  enum class H_Color
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

  namespace traits_
  {
    template
    < class V_t, class = std::enable_if_t<
      std::is_same_v< V_t, H_Color > ||
      std::is_same_v< V_t, Nation > ||
      std::is_same_v< V_t, Plant > ||
      std::is_same_v< V_t, Animal > ||
      std::is_same_v< V_t, Treat > > >
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
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, H_Color > > >
    {
      static constexpr Property value = Property::HOUSE;
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
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Treat > > >
    {
      static constexpr Property value = Property::TREAT;
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
    /**
     * @brief Construct a new BDDHelper object.
     *
     * @param vars contains o2 o1 o0 p2 p1 p0 v2 v1 v0
     */
    BDDHelper(std::vector< bdd > vars);

    /**
     * @return Combination of o0 o1 o2 with respect to given value
     * @note For example:
     * @note House::FIRST return !o0 & !o1 & !o2
     * @note House::THIRD return !o0 & o1 & 1o2
     * @note etc...
     */
    bdd getHouse(House obj);

    /**
     * @return Combination of p0 p1 p2 with respect to given value
     * @note For example:
     * @note Property::HOUSE return !p0 & !p1 & !p2
     * @note Property::PLANT return !p0 & p1 & !p2
     * @note etc...
     */
    bdd getProp(Property prop);

    /**
     * @tparam V_t - must be one of H_Color Nation Plant Animal Treat enum type
     * @return Combination of v0 v1 v2 with respect to given value
     * @note For example:
     * @note H_Color::RED return !v0 & !v1 & !v2
     * @note Nation::BELARUS return !v0 & !v1 & v2
     * @note etc...
     */
    template< class V_t >
    bdd getVal(V_t value);

    /**
     * @tparam V_t - must be one of H_Color Nation Plant Animal Treat enum type
     * @return Combination of p0 p1 p2 v0 v1 v2 with respect to given value
     */
    template< class V_t >
    bdd getPropAndVal(V_t value);

    /**
     * @return Combination of o0 o1 o2 p0 p1 p2 v0 v1 v2 that describes
     * given house with property and value
     */
    template< class V_t >
    bdd getHouseAndVal(House obj, V_t value);

  private:
    std::vector< bdd > o_;
    std::vector< bdd > p_;
    std::vector< bdd > v_;
    bdd fromNum(int num, std::vector< bdd > &var);
  };

  template < class V_t >
  inline bdd BDDHelper::getVal(V_t value)
  {
    static_assert(traits_::IsValueType_v< V_t >, "Value must be one of properties type");
    auto val = static_cast< int >(value);
    return fromNum(val, v_);
  }

  template <class V_t>
  inline bdd BDDHelper::getPropAndVal(V_t value)
  {
    static_assert(traits_::IsValueType_v< V_t >, "Value must be one of properties types");
    Property prop = traits_::PropertyFromValueEnum_v< V_t >;
    auto val = static_cast< int >(value);
    return getProp(prop) & getVal(value);
  }

  template < class V_t >
  inline bdd BDDHelper::getHouseAndVal(House obj, V_t value)
  {
    static_assert(traits_::IsValueType_v< V_t >, "Value must be one of properties types");
    Property prop = traits_::PropertyFromValueEnum_v< V_t >;
    auto val = static_cast< int >(value);
    return getHouse(obj) & getProp(prop) & getVal(value);
  }

}
#endif
