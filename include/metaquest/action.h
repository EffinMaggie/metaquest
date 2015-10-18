/**\file
 * \brief Actions
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

#if !defined(METAQUEST_ACTION_H)
#define METAQUEST_ACTION_H

#include <metaquest/object.h>

#include <string>
#include <sstream>

namespace metaquest {
namespace resource {
template <typename T> class cost {
 public:
  enum operation {
    subtract,
    add
  } operation;

  cost(T pValue, const std::string &pResource,
       enum operation pOperation = subtract, bool pVisible = true)
      : operation(pOperation),
        value(pValue),
        resource(pResource),
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

template <typename T> class total : public std::vector<cost<T> > {
 public:
  using std::vector<cost<T> >::vector;

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

  enum scope {
    self,
    ally,
    enemy,
    party,
    enemies,
    everyone
  } scope;

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
         const resource::total<T> pCost = {
  })
      : parent(),
        visible(pVisible),
        apply(pApply),
        scope(pScope),
        filter(pFilter),
        cost(pCost) {}

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
    if (potentialTargets.nothing) {
      return false;
    }

    if (potentialTargets.just.size() == 0) {
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
