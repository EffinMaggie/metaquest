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

  cost(enum operation pOperation, T pValue, const std::string &pResource,
       bool pVisible)
      : operation(pOperation), value(pValue), resource(pResource),
        visible(pVisible) {}

  std::string resource;
  bool visible;
  T value;

  virtual T resolve(const object<T> &c) const { return 0; }

  virtual bool apply(object<T> &c) { return true; }

  virtual std::string label(const object<T> &c) const {
    std::ostringstream os("");
    os << resolve(c) << " " << resource;
    return os.str();
  }
};

template <typename T> class total : public std::vector<cost<T> > {
public:
  virtual std::string label(const object<T> &c) {
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
};
}

template <typename T> class action : public object<T> {
public:
  typedef metaquest::object<T> parent;

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
         const enum scope &pScope = self, const enum filter &pFilter = none)
      : parent(), visible(pVisible), apply(pApply), scope(pScope),
        filter(pFilter) {}

  std::string operator()(objects<T> &source, objects<T> &target) {
    if (apply != nullptr) {
      return apply(source, target);
    }

    return "";
  }

  bool visible;

  std::function<std::string(objects<T> &source, objects<T> &target)> apply;
};
}

#endif
