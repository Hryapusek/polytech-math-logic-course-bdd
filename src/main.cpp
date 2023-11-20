#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include "Conditions.hpp"
#include "PrintHelper.hpp"

/**
 * The key idea of is next. We have some objects that have some
 * properties. We need to choose combinations of objects properties
 * according to the conditions.
 * Conditions, For example, red house is near white.
 * Or 4e4enec lives with dog.
 *
 * We will create function, that gets combination of objects properties
 * and return true if all the conditions are satisfied, and false if not.
 *
 * > Just feel the idea. Don't try to understand river from single bucket with water...
 * 
 * So we say, let's create nValuesVars, so that
 * var[0] var[1] var[2] var[3] will set first object's FIRST property value.
 * We can set var[0] var[1] var[2] var[3] as 0001 - first object's first property
 * value is second. In my program this means first object's Color is Green.
 *
 * var[4] var[5] var[6] var[7] will set first object's SECOND property value.
 * We can set var[4] var[5] var[6] var[7] as 0001 - first object's SECOND property
 * value is second. In my program this means first object's Nation is Belarus.
 *
 * Objects in my program like houses, which has Color, where lives Nation person,
 * that has some Plant and has some Animal. Yet i'm not high btw...
 */

using namespace bddHelper;

template < class T > using vect = std::vector< T >;

// We have nObjs objects
constexpr int nObjs = bddHelper::BDDHelper::nObjs;

// Each object has nProps properties, for example
// Color, Animal, Plant
constexpr int nProps = bddHelper::BDDHelper::nProps;

/**Each property has nVals variants, for example
   property Color has Green Red Blue...,
   property Animal has Bird Cat Dog...*/
constexpr int nVals = bddHelper::BDDHelper::nVals;

/**We have 9 values of each property. To code 9 values
   we need 4 bits. So first value is 0000, next is 0001...
   The last will be 1000, so we will need to remove trash
   combinations like 1001, 1010... because these do not
   make sense for us*/
constexpr int nValueBits = bddHelper::BDDHelper::nValueBits;

/** Each value of property is 4 bits(variables), we have nObjs that
   have nProps, that means we need nObjs * nProps * nValueBits
   to code each objects property value.*/
constexpr int nValuesVars = bddHelper::BDDHelper::nValuesVars;

// Actually it's same as nValuesVars.
constexpr int nTotalVars = bddHelper::BDDHelper::nTotalVars;

// We will use it to call function exactly one time over program life.
std::once_flag once;
// Here we will save one suitable combination of variables values
std::string varset;

/** This function is used to visit all the suitable
   variables values combination.
   @note See pattern Visitor.
 */
void fun(char *varset_, int size)
{
  std::call_once(once, [&]() {
    varset = std::string(varset_, size);
  });
}

// Nothing interesting, just printing results
void printProp(Property prop, int valNum)
{
  switch (prop)
  {
    case Property::ANIMAL:
      std::cout << to_string(static_cast< Animal >(valNum)) << '\n';
      break;

    case Property::COLOR:
      std::cout << to_string(static_cast< Color >(valNum)) << '\n';
      break;

    case Property::NATION:
      std::cout << to_string(static_cast< Nation >(valNum)) << '\n';
      break;

    case Property::PLANT:
      std::cout << to_string(static_cast< Plant >(valNum)) << '\n';
      break;
  }
}

// Nothing interesting, just printing results
void printObjects()
{
  if (varset.empty())
  {
    std::cout << "No suitable object property value combination was found.\n";
    return;
  }
  if (varset.size() != nTotalVars)
  {
    std::cout << "Array varset must contain 144 values.\
                  Otherwise there is an error in calculations.\n";
    return;
  }
  for (auto objNum : std::views::iota(0, nObjs))
  {
    auto obj = static_cast< Object >(objNum);
    std::cout << to_string(obj) << " {\n";
    for (auto propNum : std::views::iota(0, nProps))
    {
      auto prop = static_cast< Property >(propNum);
      std::cout << '\t' << to_string(prop) << ": ";
      auto baseIndex = objNum * nProps * nValueBits + propNum * nValueBits;
      int valNum = (varset.at(baseIndex) << 3) +
                   (varset.at(baseIndex + 1) << 2) +
                   (varset.at(baseIndex + 2) << 1) +
                   (varset.at(baseIndex + 3) << 0);
      printProp(prop, valNum);
    }
    std::cout << "}\n";
  }
}

int main()
{
  // Let's give bdd some memory. You can change it according to your needs.
  bdd_init(3000000, 100000);
  // Let's create bdd variables. They described in the up.
  bdd_setvarnum(BDDHelper::nTotalVars);
  // Array to save all these variables.
  // vars[0] will contain first var
  // vars[1] will contain second var
  // vars[2]...
  std::vector< bdd > vars(nTotalVars);
  { // Here we just put all these variables in array.
    int i = 0;
    std::generate(std::begin(vars), std::end(vars),
      [&i]() {
      return bdd_ithvar(i++);
    });
  }
  /**
   * Of course vars contains each objects property variables.
     And we can access for example second objects second property vars by
     vars[nProps*nVaueBits*1 + nValueBits],
     vars[nProps*nVaueBits*1 + nValueBits + 1]
     vars[nProps*nVaueBits*1 + nValueBits + 2]
     vars[nProps*nVaueBits*1 + nValueBits + 3].
     But it's just \b fucking \b disgusting.
     I create that array v, so that we can access
     second objects second property by structedVars[1][1].
     Dont forget that arrays indexes start with \b 0, not \b 1.
     */
  auto structedVars = vect< vect< vect< bdd > > >(nObjs);
  for (auto objNum : std::views::iota(0, nObjs))
  {
    structedVars[objNum] = vect< vect< bdd > >(nProps);
    for (auto propNum : std::views::iota(0, nProps))
    {
      auto baseIndex = objNum * nProps * nValueBits + propNum * nValueBits;
      structedVars[objNum][propNum] = vect< bdd >{
        bdd_ithvar(baseIndex + 0),
        bdd_ithvar(baseIndex + 1),
        bdd_ithvar(baseIndex + 2),
        bdd_ithvar(baseIndex + 3) };
    }
  }
  // Let's explore what is BDDHelper
  bddHelper::BDDHelper h(structedVars);
  BDDFormulaBuilder builder;
  conditions::addConditions(h, builder);
  std::cout << "Bdd formula created. Starting counting sets...\n";
  std::cout << "Count of true variables values combinations: " << bdd_satcount(builder.result()) << '\n';
  std::cout << "Objects are...\n";
  bdd_allsat(builder.result(), fun);
  printObjects();
  bdd_done();
  return 0;
}
