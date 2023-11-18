#include <gtest/gtest.h>
#include "BDDHelper.hpp"

class VarsSetupFixture: public ::testing::Test
{
public:
  static constexpr int nObjs = bddHelper::BDDHelper::nObjs;
  static constexpr int nProps = bddHelper::BDDHelper::nProps;
  static constexpr int nVals = bddHelper::BDDHelper::nVals;
  static constexpr int nObjsVars = bddHelper::BDDHelper::nObjsVars;
  static constexpr int nPropsVars = bddHelper::BDDHelper::nPropsVars;
  static constexpr int nValuesVars = bddHelper::BDDHelper::nValuesVars;
  static constexpr int nTotalVars = bddHelper::BDDHelper::nTotalVars;
  static std::vector< bdd > vars;
  static std::vector< bdd > objs;
  static std::vector< bdd > props;
  static std::vector< bdd > vals;
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
    objs = std::vector< bdd > (vars.begin(), vars.begin() + nObjs);
    props = std::vector< bdd > (vars.begin() + nObjsVars, vars.begin() + nObjsVars + nPropsVars);
    vals = std::vector< bdd > (vars.begin() + nObjsVars + nPropsVars, vars.end());

    h = BDDHelper(objs, props, vals);
  }

  virtual void TearDown()
  { }
};
