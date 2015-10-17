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

#include <ef.gy/maybe.h>

#include <metaquest/name.h>

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <set>
#include <regex>

namespace metaquest {
template <typename T> using slots = std::map<std::string, T>;

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

  virtual ~object(void) {}

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
  virtual T operator[](const std::string &s) const {
    auto fi = function.find(s);

    if (fi != function.end()) {
      const auto &f = fi->second;
      if (f != nullptr) {
        return f(*this);
      }
    }

    auto ai = attribute.find(s);

    if (ai != attribute.end()) {
      return ai->second;
    }

    return 0;
  }

  virtual T set(const std::string &s, const T &b) {
    T n = b;
    if (n < 0) {
      n = 0;
    }
    std::smatch matches;
    static std::regex resource("(.+)/(Current|Total)");
    if (std::regex_match(s, matches, resource)) {
      std::string resource = matches[1];
      if (matches[2] == "Current") {
        n = std::min(n, attribute[resource + "/Total"]);
      }
    }
    return attribute[s] = b;
  }

  virtual T add(const std::string &s, const T &b) {
    return set(s, attribute[s] + b);
  }

  virtual T add(const std::string &s, const std::string &m, const T &b) {
    attribute[s] += b;
    if ((*this)[s] > (*this)[m]) {
      return add(s, -((*this)[s] - (*this)[m]));
    }
    return attribute[s];
  }

  virtual bool have(const std::string &s) const {
    return (function.find(s) != function.end()) ||
           (attribute.find(s) != attribute.end());
  }

  virtual std::set<std::string> attributes(void) const {
    std::set<std::string> ret;

    for (const auto &m : function) {
      ret.insert(m.first);
    }

    for (const auto &m : attribute) {
      ret.insert(m.first);
    }

    return ret;
  }

  virtual std::set<std::string> resources(void) const {
    std::set<std::string> ret;

    std::smatch matches;

    for (const auto &a : attributes()) {
      static std::regex resource("(.+)/(Current|Total)");
      if (std::regex_match(a, matches, resource)) {
        ret.insert(matches[1]);
      }
    }

    return ret;
  }

  slots<T> slots;

 protected:
  /**\brief Attribute generation functions
   *
   * Maps attribute names to thunks which can generate an attribute on
   * the fly, e.g. for derived attributes in RPGs.
   */
  std::map<std::string, std::function<T(const object &)> > function;

  /**\brief Basic attributes
   *
   * Maps basic attributes to their proper values.
   */
  std::map<std::string, T> attribute;
};

template <typename T> using objects = std::vector<object<T> *>;
}

#endif
