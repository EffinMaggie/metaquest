/**\file
 * \brief Objects
 *
 * Everything in the game is an object. This header defines the base class for
 * these objects.
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

#if !defined(METAQUEST_OBJECT_H)
#define METAQUEST_OBJECT_H

#include <metaquest/name.h>

#include <string>
#include <map>
#include <functional>
#include <vector>

namespace metaquest {
/**\brief A game object
 *
 * The base class for items, characters, etc. Provides common properties,
 * such as a name, rules and attributes.
 *
 * \tparam T Base type for attributes. Integers are probably a good choice,
 *           at least for J-RPGs and tabletops.
 */
template <typename T = long, typename C = char> class object {
public:
  typedef T base;

  /**\brief Object name
   *
   * Everything needs a name. Since everything in the game is an
   * object, this is that name.
   */
  name::proper<C> name;

  /**\brief Access attribute
   *
   * Provides a simple way to access attributes. Attributes may either
   * be members of the 'attribute' map, or they can be calculated on
   * the fly using a member of the 'function' map.
   *
   * \param[in] s The attribute you'd like to access.
   *
   * \returns The attribute you tried to access.
   */
  T operator[](const std::string &s) {
    std::function<T(object &)> &f = function[s];
    if (f == nullptr) {
      return attribute[s];
    } else {
      return f(*this);
    }
  }

  bool have(const std::string &s) {
    return (function.find(s) != function.end()) ||
           (attribute.find(s) != attribute.end());
  }

  /**\brief Attribute generation functions
   *
   * Maps attribute names to thunks which can generate an attribute on
   * the fly, e.g. for derived attributes in RPGs.
   */
  std::map<std::string, std::function<T(object &)> > function;

  /**\brief Basic attributes
   *
   * Maps basic attributes to their proper values.
   */
  std::map<std::string, T> attribute;
};

template <typename T> using objects = std::vector<object<T> *>;
}

#endif
