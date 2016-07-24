/**\file
 * \brief Actions
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#if !defined(METAQUEST_ACTION_H)
#define METAQUEST_ACTION_H

#include <metaquest/object.h>

#include <string>
#include <sstream>

namespace metaquest {
namespace resource {
template <typename T> class cost {
public:
  enum operation { subtract, add } operation;

  cost(T pValue, const std::string &pResource,
       enum operation pOperation = subtract, bool pVisible = true)
      : operation(pOperation), value(pValue), resource(pResource),
        visible(pVisible) {}

  std::string resource;
  bool visible;
  T value;

  virtual T resolve(const object<T> &c) const { return value; }

  virtual bool canApply(const object<T> &c) const {
    auto attr = c.attributes();
    T val = resolve(c);
    if (attr.find(resource) != attr.end()) {
      if (operation == subtract) {
        if (c[resource] >= val) {
          return true;
        }
      } else {
        return true;
      }
    } else if (attr.find(resource + "/Current") != attr.end()) {
      if (operation == subtract) {
        if (c[resource + "/Current"] >= val) {
          return true;
        }
      } else {
        return true;
      }
    }

    return false;
  }

  virtual bool apply(object<T> &c) {
    if (!canApply(c)) {
      return false;
    }

    auto attr = c.attributes();
    T val = operation == subtract ? -resolve(c) : resolve(c);
    if (attr.find(resource) != attr.end()) {
      c.add(resource, val);
      return true;
    } else if (attr.find(resource + "/Current") != attr.end()) {
      c.add(resource + "/Current", val);
      return true;
    }

    return false;
  }

  virtual std::string label(const object<T> &c) const {
    std::ostringstream os("");
    os << resolve(c) << " " << resource;
    return os.str();
  }
};

template <typename T> class total : public std::vector<cost<T>> {
public:
  using std::vector<cost<T>>::vector;

  virtual std::string label(const object<T> &c) const {
    std::string res = "";
    for (const auto &cost : *this) {
      if (res == "") {
        res = cost.label(c);
      } else {
        res += " " + cost.label(c);
      }
    }
    return res;
  }

  virtual bool canApply(const object<T> &c) const {
    bool res = true;
    for (const auto &cost : *this) {
      res &= cost.canApply(c);
    }
    return res;
  }

  virtual bool apply(object<T> &c) {
    if (canApply(c)) {
      bool res = true;
      for (auto &cost : *this) {
        res &= cost.apply(c);
      }
      return res;
    }
    return false;
  }
};
}

template <typename T> class action : public object<T> {
public:
  using parent = metaquest::object<T>;

  enum scope { self, ally, enemy, party, enemies, everyone } scope;

  enum filter {
    none,
    onlyHealthy,
    onlyAlive,
    onlyUnhealthy,
    onlyDead,
    onlyUndefeated
  } filter;

  action(bool pVisible = false,
         std::function<std::string(objects<T> &source, objects<T> &target)>
             pApply = nullptr,
         const enum scope &pScope = self, const enum filter &pFilter = none,
         const resource::total<T> pCost = {})
      : parent(), visible(pVisible), apply(pApply), scope(pScope),
        filter(pFilter), cost(pCost) {}

  std::string operator()(objects<T> &source, objects<T> &target) {
    if (apply != nullptr) {
      return apply(source, target);
    }

    return "";
  }

  template <typename C> bool usable(const C &character) const {
    if (character.defeated()) {
      return false;
    }

    if (!cost.canApply(character)) {
      return false;
    }

    return true;
  }

  template <typename G, typename C>
  bool usable(G &game, const C &character) const {
    if (!usable(character)) {
      return false;
    }

    auto potentialTargets = game.resolve(character, scope, filter, false);
    if (potentialTargets.size() == 0) {
      return false;
    }

    return true;
  }

  bool visible;
  resource::total<T> cost;

  std::function<std::string(objects<T> &source, objects<T> &target)> apply;
};
}

#endif
