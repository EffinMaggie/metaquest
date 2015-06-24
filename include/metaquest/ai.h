/**\file
 *
 * \copyright
 * Copyright (c) 2014-2015, Magnus Achim Deininger <magnus@ef.gy>
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

#if !defined(METAQUEST_AI_H)
#define METAQUEST_AI_H

#include <metaquest/game.h>
#include <random>

namespace metaquest {
namespace ai {
template <typename inter> class random {
public:
  random(inter &pInteract) : interact(pInteract), rng(std::random_device()()) {}

  template <typename T, typename G>
  std::string query(const G &game, const metaquest::character<T> &source,
                    const std::vector<std::string> &list,
                    std::size_t indent = 4, std::string carry = "") {
    return carry + list[(rng() % list.size())];
  }

  template <typename T, typename G>
  std::vector<metaquest::character<T> *>
  query(G &game, const metaquest::character<T> &source,
        const std::vector<metaquest::character<T> *> &candidates,
        std::size_t indent = 4) {
    std::vector<metaquest::character<T> *> targets;
    targets.push_back(candidates[(rng() % candidates.size())]);
    return targets;
  }

protected:
  inter &interact;
  std::mt19937 rng;
};
}
}

#endif
