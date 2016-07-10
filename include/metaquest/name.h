/**\file
 * \brief Character, item, etc. names.
 *
 * Everything in a game needs a name. This is the base header that provides an
 * abstraction for these names.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#if !defined(METAQUEST_NAME_H)
#define METAQUEST_NAME_H

#include <ef.gy/markov.h>
#include <ef.gy/json.h>

#include <data/female.first.h>
#include <data/male.first.h>
#include <data/all.last.h>

#include <algorithm>
#include <cctype>

namespace metaquest {
/**\brief Names
 *
 * Names are an important aspect of pretty much every self-respecting game.
 * You need to have names for items, characters, skills, corporations and
 * the characters' favourite pet. This namespace encapsulates everything
 * related to naming things - including templates to generate names on the
 * fly.
 */
namespace name {
/**\brief Default seed
 *
 * Used to seed the PRNG when generating names.
 */
static const unsigned long seed = std::random_device()();

/**\brief A name
 *
 * Base class that holds a single portion of a name, along with a tag
 * that describes what kind of name it is.
 *
 * \tparam T         The type used for single characters in names.
 * \tparam generator A class that can generate random names, e.g. a
 *                   variant of efgy::markov::chain.
 */
template <typename T = char, typename generator = efgy::markov::chain<T, 3> >
class name {
 public:
  /**\brief Name type
   *
   * Names generally fall into different categories, depending on
   * what it is that they provide a name for. This enum is used to
   * classify names based on these categories, so they can be
   * displayed correctly in different contexts.
   */
  enum type {
    givenName,  /**< A given name, e.g. a first or middle name. */
    familyName, /**< A family name, e.g. a last name. */
    nickName, /**< A nickname, e.g. a shortened form of a
                   shortened form of a last name. */
    callSign, /**< A nickname which is not necessarily used in
                   a colloquial setting. */
    otherName   /**< Used with things like items, etc. */
  };

  /**\brief Construct with generator and type
   *
   * Initialises a new instance of a name by generating a random
   * name with the provided generator instance.
   *
   * \param[in] pGenerator An instance of a name generator.
   * \param[in] pType      The type to tag the name with.
   */
  name(generator &pGenerator, const enum type &pType = otherName)
      : type(pType) {
    pGenerator >> value;
  }

  /**\brief Construct with name and type
   *
   * Initialises a new instance of a name by using a provided name
   * string and tagging that with a type.
   *
   * \param[in] pValue The name to use.
   * \param[in] pType  The type of the provided name.
   */
  name(const std::basic_string<T> &pValue, const enum type &pType = otherName)
      : value(pValue), type(pType) {}

  name(efgy::json::json json) {
    if (json.isObject()) {
      value = json("name").asString();
      type = (enum type)json("type").asNumber();
    }
  }

  /**\brief The actual name
   *
   * The tectual representation of the name, either generated or
   * passed to the constructor.
   */
  std::basic_string<T> value;

  /**\brief Type of the name
   *
   * This stores the type of the name. Passed to the constructor
   * upon initialising a new instance of the class.
   */
  enum type type;

  efgy::json::json json(void) const {
    efgy::json::json rv;
    
    rv.toObject();
    rv("name") = value;
    rv("type") = efgy::json::json::numeric(type);

    return rv;
  }
};

/**\brief A proper name
 *
 * Represents a proper, or "full" name. For example, a person tends to
 * have both a given name and a family name, and may quite likely also
 * have a nickname. This class groups all of these together.
 *
 * \tparam T         The type used for single characters in names.
 * \tparam generator A class that can generate random names, e.g. a
 *                   variant of efgy::markov::chain.
 */
template <typename T = char, typename generator = efgy::markov::chain<T, 3> >
class proper : public std::vector<name<T, generator> > {
 public:
  /**\brief Query the full name
   *
   * This function provides access to a "full" name string, with
   * all the names in the vector appended with spaces inbetween.
   *
   * \returns A string with the proper, full name.
   */
  std::basic_string<T> full(void) const {
    std::basic_string<T> s = "";

    for (const name<T, generator> &n : *this) {
      if (s.size() > 0) {
        s += " ";
      }
      s += n.value;
    }

    return s;
  }

  /**\brief Query the display name
   *
   * Using the full name everywhere would be quite cumbersome in a
   * game, so this function can be used to get a shorter version.
   *
   * \returns A shorter version of the name, if that can be
   *          deduced easily.
   */
  std::basic_string<T> display(void) const {
    for (const name<T, generator> &n : *this) {
      switch (n.type) {
        case name<T, generator>::givenName:
          return n.value;
        case name<T, generator>::callSign:
          return n.value;
        default:
          break;
      }
    }

    return full();
  }

  bool load(efgy::json::json json) {
    this->clear();

    for (const auto n : json.asArray()) {
      this->push_back(name<T, generator>(n));
    }

    return true;
  }

  efgy::json::json json(void) const {
    efgy::json::json rv;
    
    rv.toArray();
    for (auto &n : *this) {
      rv.push(n.json());
    }

    return rv;
  }
};

template <typename T = char, typename generator = efgy::markov::chain<T, 3> >
class simple : public proper<T, generator> {
 public:
  typedef proper<T, generator> parent;

  simple(const std::string &pName) : parent() { parent::push_back(pName); }
};

/**\brief American names
 *
 * Provides an easier interface to automatically generate American names
 * based on historic census data.
 */
namespace american {
/**\brief Automatically-generated, American-sounding given name
 *
 * This template can be used to automatically generate American-ish
 * sounding given names based on census data.
 *
 * \tparam T         The type used for single characters in names.
 * \tparam generator A class that can generate random names, e.g. a
 *                   variant of efgy::markov::chain.
 */
template <typename T = char, typename generator = efgy::markov::chain<T, 3> >
class given : public name<T, generator> {
 public:
  /**\brief Base name type
   *
   * This is a rather convenient typedef so we don't have to
   * write the full type everywhere.
   */
  typedef name<T, generator> parent;

  using parent::value;

  /**\brief Construct with gender and maximum length
   *
   * Constructs a new instance of the name class. You'll need
   * to provide the gender of the name so the code can figure
   * out which part of the census data to use. Additionally,
   * the constructor imposes a limit on the length of the name
   * so the names don't get unwieldy but memorable instead.
   *
   * \param[in] female Whether the code should use the census
   *                   data for female names; Defaults to
   *                   'true'.
   * \param[in] length The maximum length of the name. If the
   *                   generated name is longer then a new one
   *                   is generated.
   */
  given(bool female = true, unsigned int length = 9)
      : parent("", parent::givenName) {
    static typename generator::random PRNG(seed);
    static generator femaleFirstNames(PRNG, data::female_first);
    static generator maleFirstNames(PRNG, data::male_first);

    while ((value.size() == 0) || (value.size() > length)) {
      switch (PRNG() % 10) {
        case 0:
          (female ? maleFirstNames : femaleFirstNames) >> value;
        default:
          (female ? femaleFirstNames : maleFirstNames) >> value;
      }
    }

    std::transform(value.begin() + 1, value.end(), value.begin() + 1,
                   [](char a)->char {
      return std::tolower(a);
    });
  }
};

/**\brief Automatically-generated, American-sounding family name
 *
 * Like the american::given template, but generates family names
 * instead.
 *
 * \tparam T         The type used for single characters in names.
 * \tparam generator A class that can generate random names, e.g. a
 *                   variant of efgy::markov::chain.
 */
template <typename T = char, typename generator = efgy::markov::chain<T, 3> >
class family : public name<T, generator> {
 public:
  /**\copydoc given::name */
  typedef name<T, generator> parent;

  using parent::value;

  /**\brief Construct with maximum length
   *
   * Initialises a new instance of the name::name class with a
   * randomly generated last name. The constructor imposes a
   * limit on the length of that name so as to make sure the
   * names don't get too unwieldy.
   *
   * \param[in] length The maximum length of the name. If the
   *                   generated name is longer then a new one
   *                   is generated.
   */
  family(unsigned int length = 9) : parent("", parent::familyName) {
    static typename generator::random PRNG(seed);
    static generator lastNames(PRNG, data::all_last);

    while ((value.size() == 0) || (value.size() > length)) {
      lastNames >> value;
    }

    std::transform(value.begin() + 1, value.end(), value.begin() + 1,
                   [](char a)->char {
      return std::tolower(a);
    });
  }
};

/**\brief Automatically-generated, American-sounding proper name
 *
 * Uses american::given and american::family to create random
 * proper, full names for characters.
 *
 * \tparam T         The type used for single characters in names.
 * \tparam generator A class that can generate random names, e.g. a
 *                   variant of efgy::markov::chain.
 */
template <typename T = char, typename generator = efgy::markov::chain<T, 3> >
class proper : public metaquest::name::proper<T, generator> {
 public:
  /**\copydoc given::name */
  typedef metaquest::name::proper<T, generator> parent;

  /**\brief Construct with gender and maximum length
   *
   * Constructs a proper, full name with a variable number of
   * first and last names using the american::given and
   * american::family templates. The parameters are passed to
   * these types' constructors as applicable.
   *
   * \param[in] female Whether the code should use the census
   *                   data for female names; Defaults to
   *                   'true'.
   * \param[in] length The maximum length of the name. If the
   *                   generated name is longer then a new one
   *                   is generated.
   */
  proper(bool female = true, unsigned int length = 9) {
    static typename generator::random PRNG(seed);

    do {
      given<T, generator> f(female, length);
      parent::push_back(f);
    } while ((PRNG() % 10) == 0);

    do {
      family<T, generator> l(length);
      parent::push_back(l);
    } while ((PRNG() % 10) == 0);
  }
};
}
}
}

#endif
