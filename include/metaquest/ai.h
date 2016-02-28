/**\file
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
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
    std::string r;
    do {
      r = carry + list[(rng() % list.size())];
    } while (r == "Pass");
    return r;
  }

  template <typename T, typename G>
  std::vector<metaquest::character<T> *> query(
      G &game, const metaquest::character<T> &source,
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
