/**\file
 * \brief Parties
 *
 * Groups of characters are called 'parties'. This file contains things related
 * to this concept.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#if !defined(METAQUEST_PARTY_H)
#define METAQUEST_PARTY_H

#include <metaquest/character.h>

namespace metaquest {
/**\brief A party
 *
 * This type represents a group of characters, referred to as a 'party'. The
 * type is based on std::vector as opposed to std::set because in some
 * contexts (menu, etc.) the order might actually be relevant.
 */
template <typename C>
class party : public std::vector<character<typename C::base> > {
public:
  using base = typename C::base;

  /**\brief Generate a party.
   *
   * Given the number of members you want the party to consist of, this will
   * randomly generate a party of that size.
   *
   * Actual character generation needs to be handled by the character class.
   *
   * \param[in] members The number of members the new party should have.
   *
   * \returns The generated party.
   */
  static party generate(unsigned int members, base points = 0) {
    static std::mt19937 rng = std::mt19937(std::random_device()());
    party p;

    for (unsigned int i = 0; i < members; i++) {
      base cpoints = points;
      if ((points > 0) && (i < (members - 1))) {
        cpoints = rng() % points;
        points -= cpoints;
      }
      p.push_back(C(cpoints));
    }

    return p;
  }

  /**\brief Is the party defeated?
   *
   * A party counts as defeated when all characters in that party count as
   * defeated. In the simplest case, that means all of the party members are
   * dead.
   *
   * \note Empty parties count as defeated.
   *
   * \returns 'true' if the party counts as defeated.
   */
  virtual bool defeated(void) const {
    bool ret = true;

    for (auto &c : *this) {
      ret &= c.defeated();
    }

    return ret;
  }

  static party load(efgy::json::json json) {
    party p;

    for (const auto o : json("member").asArray()) {
      C c;

      if (c.load(o)) {
        p.push_back(c);
      }
    }

    if (json("inventory").asArray().size() > 0) {
      p.inventory.load(json("inventory"));
    }

    return p;
  }

  virtual efgy::json::json json(void) const {
    efgy::json::json rv;

    auto &me = rv("member");
    for (auto &ch : *this) {
      me.push(ch.json());
    }

    rv("inventory") = inventory.json();

    return rv;
  }

  items<base> inventory;

protected:
  using std::vector<character<typename C::base> >::vector;
};
}

#endif
