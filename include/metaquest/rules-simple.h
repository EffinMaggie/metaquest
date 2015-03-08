/**\file
 * \brief S[ia]mple Rules
 *
 * Contains a very simple rule set that should serve as a template for more
 * complicated rule sets.
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

#if !defined(METAQUEST_RULES_SIMPLE_H)
#define METAQUEST_RULES_SIMPLE_H

#include <metaquest/character.h>
#include <metaquest/game.h>
#include <random>

namespace metaquest {
namespace rules {
namespace simple {
static long isAlive(object<long> &t) { return t.attribute["HP/Current"] > 0; }

static long getPoints(long level) { return level * 10 + (level % 2) * 5; }

static long getHPTotal(object<long> &t) {
  return getPoints(t.attribute["Level"] + 1);
}

static long getMPTotal(object<long> &t) {
  return getPoints((t.attribute["Level"] + 1) * 2);
}

static int roll(int num, int sides = 6) {
  static std::mt19937 rng = std::mt19937(std::random_device()());

  int res = 0;
  for (int i = 0; i < num; i++) {
    res += 1 + rng() % sides;
  }
  return res;
}

static std::string attack(objects<long> &source, objects<long> &target) {
  std::stringstream os("");
  for (auto &sp : source) {
    auto &s = *sp;
    for (auto &tp : target) {
      auto &t = *tp;

      os << s.name.display() << " attacks " << t.name.display() << "\n";

      int dmg = roll(s["Attack"]);
      int def = roll(s["Defence"]);
      int admg = dmg - def;

      if (admg > 0) {
        os << s.name.display() << " hits for " << admg << " (" << dmg
           << ") points of damage\n";

        t.attribute["HP/Current"] -= admg;
        if (!t["Alive"]) {
          s.attribute["Experience"] += t.attribute["Experience"] / 2 + 1;
        }
      } else {
        os << s.name.display() << " misses\n";
      }
    }
  }
  return os.str();
}

static std::string heal(objects<long> &source, objects<long> &target) {
  std::stringstream os("");
  for (auto &sp : source) {
    auto &s = *sp;
    for (auto &tp : target) {
      auto &t = *tp;

      if (t["MP/Current"] < 2) {
        os << s.name.display() << " does not have enough MP!\n";
        continue;
      }

      t.attribute["MP/Current"] -= 2;

      os << s.name.display() << " heals " << t.name.display() << "\n";

      int amt = roll(s["Attack"]);

      os << s.name.display() << " heals " << amt << " points of damage\n";

      t.attribute["HP/Current"] += amt;
      if (t["HP/Current"] > t["HP/Total"]) {
        t.attribute["HP/Current"] -= t["HP/Current"] - t["HP/Total"];
      }
    }
  }
  return os.str();
}

static std::string pass(objects<long> &source, objects<long> &target) {
  std::stringstream os("");
  for (auto &sp : source) {
    auto &s = *sp;
    os << s.name.display() << " would rather be reading a book.\n";
  }
  return os.str();
}

class character : public metaquest::character<long> {
public:
  typedef metaquest::character<long> parent;
  typedef metaquest::action<long> action;

  using parent::name;

  character(long points = 1) : parent(points) {
    metaquest::name::american::proper<> cname(roll(1, 10) > 5);
    name = cname;

    attribute["Attack"] = 1;
    attribute["Defence"] = 1;
    attribute["Experience"] = 0;

    function["HP/Total"] = getHPTotal;
    function["MP/Total"] = getMPTotal;
    function["Alive"] = isAlive;

    attribute["HP/Current"] = (*this)["HP/Total"];
    attribute["MP/Current"] = (*this)["MP/Total"];

    bind("Attack", true, attack);
    bind("Skill/Heal", true, heal, action::ally);
    bind("Pass", true, pass, action::self);
  }
};

template <typename inter>
class game : public metaquest::game::base<character, inter> {
public:
  typedef metaquest::game::base<character, inter> parent;

  game(inter &pInteract) : parent(pInteract) {}

  using parent::parties;
  using parent::rng;
  using parent::useAI;
  using parent::interact;
  using parent::resolve;
  using parent::alive;

  virtual std::string next(void) {
    for (std::size_t pi = 0; pi < parties.size(); pi++) {
      auto &p = parties[pi];

      if (!alive(p)) {
        std::ostringstream os("");
        os << "Party #" << (parties.size() - pi - 1) << " was victorious!\n";
        return os.str();
      }
    }

    std::size_t p = rng() % parties.size();
    std::size_t n = 0;
    do {
      n = rng() % parties[p].size();
    } while (!alive(parties[p][n]));

    character &c = parties[p][n];

    auto visible = c.visibleActions();

    if (!useAI(c)) {
      visible.push_back("Status");
      visible.push_back("Quit/Yes");
      visible.push_back("Quit/No");
    }

    bool retry = false;

    do {
      retry = false;

      std::string s = interact.query(*this, c, visible);

      if (s == "Cancel" || s == "Quit/No") {
        retry = true;
        continue;
      }

      if (s == "Status") {
        // display status here.
        retry = true;
        continue;
      }

      if (s == "Quit/Yes") {
        return "Quit.";
      }

      auto targets = resolve(c, s);

      if (!targets) {
        retry = true;
        continue;
      }

      return c(s, targets.just);
    } while (retry);

    return "Nothing happened.";
  }
};
}
}
}

#endif
