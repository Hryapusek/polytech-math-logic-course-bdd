#include "BDDHelper.hpp"

bdd BDDHelper::fromNum(int num, std::vector< bdd > &var)
{
  switch (num)
  {
    case 0:
      return not var[2] & not var[1] & not var[0];

    case 1:
      return not var[2] & not var[1] & var[0];

    case 2:
      return not var[2] & var[1] & not var[0];

    case 3:
      return not var[2] & var[1] & var[0];

    case 4:
      return var[2] & not var[1] & not var[0];

    default:
      assert(("Value must be between 0 and 4", false));
  }
}

BDDHelper::BDDHelper(std::vector< bdd > vars)
{
  assert(("Size of vars must be 9.", vars.size() == 9));
  v_ = std::vector(vars.rbegin(), vars.rbegin() + 3);
  p_ = std::vector(vars.rbegin() + 3, vars.rbegin() + 6);
  o_ = std::vector(vars.rbegin() + 6, vars.rend());
  assert(("Size of all the vectors v p and o must be 3",
          v_.size() == p_.size() and
          v_.size() == o_.size() and
          v_.size() == 3));
}

bdd BDDHelper::getObj(Object obj)
{
  auto val = static_cast< int >(obj);
  return fromNum(val, o_);
}

bdd BDDHelper::getProp(Property prop)
{
  auto val = static_cast< int >(prop);
  return fromNum(val, p_);
}
