/**\file
 * \brief Parties
 *
 * Groups of characters are called 'parties'. This file contains things related
 * to this concept.
 *
 * \copyright
 * Copyright (c) 2013-2015, Magnus Achim Deininger <magnus@ef.gy>
 * \copyright
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * \copyright
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * \copyright
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
template <typename C> class party : public std::vector<C> {
 public:
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
  static party generate(unsigned int members) {
    party p;

    for (unsigned int i = 0; i < members; i++) {
      C c;
      p.push_back(c);
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
  bool defeated(void) const {
    bool ret = true;

    for (auto &c : *this) {
      ret &= c.defeated();
    }

    return ret;
  }

 protected:
  using std::vector<C>::vector;
};
}

#endif
