#include <gtest/gtest.h>
#include "BDDHelper.hpp"
#include <algorithm>
#include <ranges>

// Ignore this whole file.
// Used for gtest

class VarsSetupFixture: public ::testing::Test
{
public:
  template < class T > using vect = std::vector< T >;
  static constexpr int nObjs = bddHelper::BDDHelper::nObjs;
  static constexpr int nProps = bddHelper::BDDHelper::nProps;
  static constexpr int nVals = bddHelper::BDDHelper::nVals;
  static constexpr int nValueBits = bddHelper::BDDHelper::nValueBits;
  static constexpr int nValuesVars = bddHelper::BDDHelper::nValuesVars;
  static constexpr int nTotalVars = bddHelper::BDDHelper::nTotalVars;
  static std::vector< bdd > vars;
  static vect< vect< vect< bdd > > > v;
  static bddHelper::BDDHelper h;

protected:
  virtual void SetUp()
  {
    using namespace bddHelper;
    vars = std::vector< bdd > (nTotalVars);
    {
      int i = 0;
      std::generate(std::begin(vars), std::end(vars),
        [&i]() {
        return bdd_ithvar(i++);
      });
    }
    v = vect< vect< vect< bdd > > >(nObjs);
    for (auto objNum : std::views::iota(0, nObjs))
    {
      v[objNum] = vect< vect< bdd > >(nProps);
      for (auto propNum : std::views::iota(0, nProps))
      {
        auto baseIndex = objNum * nProps * nValueBits + propNum * nValueBits;
        v[objNum][propNum] = vect< bdd >{
          bdd_ithvar(baseIndex + 0), 
          bdd_ithvar(baseIndex + 1),
          bdd_ithvar(baseIndex + 2),
          bdd_ithvar(baseIndex + 3)};
      }
    }
    h = BDDHelper(v);
  }

  virtual void TearDown()
  { }
};
