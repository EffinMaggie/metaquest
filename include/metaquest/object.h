/**\file
 * \brief Objects
 *
 * Everything in the game is an object. This header defines the base class for
 * these objects.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#if !defined(METAQUEST_OBJECT_H)
#define METAQUEST_OBJECT_H

#include <ef.gy/maybe.h>
#include <ef.gy/json.h>

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
  using base = T;

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
        T m = (*this)[resource + "/Total"];
        if ((m != 0) && (n > m)) {
          n = m;
        }
      }
    }
    return attribute[s] = n;
  }

  virtual T add(const std::string &s, const T &b) {
    return set(s, attribute[s] + b);
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

  virtual const slots<T> allSlots(void) const { return slots; }

  virtual const slots<T> usedSlots(void) const { return {}; }

  virtual const slots<T> freeSlots(void) const { return allSlots(); }

  virtual bool load(efgy::json::json json) {
    name.load(json("name"));

    for (const auto data : json("attributes").asObject()) {
      attribute[data.first] = data.second.asNumber();
    }

    for (const auto data : json("slots").asObject()) {
      slots[data.first] = data.second.asNumber();
    }

    return true;
  }

  virtual efgy::json::json json(void) const {
    efgy::json::json rv;

    rv("name") = name.json();

    auto &at = rv("attributes");
    for (auto &attrib : attribute) {
      at(attrib.first) = efgy::json::json::numeric(attrib.second);
    }

    auto &sl = rv("slots");
    for (auto &slot : slots) {
      sl(slot.first) = efgy::json::json::numeric(slot.second);
    }

    return rv;
  }

  slots<T> slots;

  /**\brief Attribute generation functions
   *
   * Maps attribute names to thunks which can generate an attribute on
   * the fly, e.g. for derived attributes in RPGs.
   */
  std::map<std::string, std::function<T(const object &)>> function;

  /**\brief Basic attributes
   *
   * Maps basic attributes to their proper values.
   */
  std::map<std::string, T> attribute;
};

template <typename T> using objects = std::vector<object<T> *>;
}

#endif
