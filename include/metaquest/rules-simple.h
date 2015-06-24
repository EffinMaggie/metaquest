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
static long getPoints(long level) { return level * 10 + (level % 2) * 5; }

static long getHPTotal(const object<long> &t) {
  return getPoints(t["Level"] + 1);
}

static long getMPTotal(const object<long> &t) {
  return getPoints((t["Level"] + 1) * 2);
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

        t.add("HP/Current", -admg);
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

      t.add("MP/Current", -2);

      os << s.name.display() << " heals " << t.name.display() << "\n";

      int amt = roll(s["Attack"]);

      os << s.name.display() << " heals " << amt << " points of damage\n";

      t.add("HP/Current", "HP/Total", amt);
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

    attribute["HP/Current"] = (*this)["HP/Total"];
    attribute["MP/Current"] = (*this)["MP/Total"];

    bind("Attack", true, attack, action::enemy, action::onlyUndefeated);
    bind("Skill/Heal", true, heal, action::ally, action::onlyUnhealthy, {
      resource::cost<long>(2, "MP")
    });
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
  using parent::generateParties;

  virtual std::string next(void) {
    bool retry = false;

    if (parties.size() == 1) {
      std::vector<std::string> visible;

      visible.push_back("Fight");
      visible.push_back("Quit/Yes");
      visible.push_back("Quit/No");

      do {
        retry = false;

        std::string s = interact.query(*this, parties[0][0], visible);

        if (s == "Cancel" || s == "Quit/No") {
          retry = true;
          continue;
        }

        if (s == "Quit/Yes") {
          return "Quit.";
        }

        if (s == "Fight") {
          attribute["parties"] = 2;
          generateParties();
          return "OFF WITH THEIR HEADS!\n";
        }
      } while (retry);

      return "Nothing happened.";
    }

    for (std::size_t pi = 0; pi < parties.size(); pi++) {
      auto &p = parties[pi];

      if (p.defeated()) {
        std::ostringstream os("");
        os << "Party #" << (parties.size() - pi - 1) << " was victorious!\n";
        return os.str();
      }
    }

    std::size_t p = rng() % parties.size();
    std::size_t n = 0;
    do {
      n = rng() % parties[p].size();
    } while (!parties[p][n].able());

    character &c = parties[p][n];

    auto visible = c.visibleActions();

    if (!useAI(c)) {
      visible.push_back("Inspect");
      visible.push_back("Quit/Yes");
      visible.push_back("Quit/No");
    }

    do {
      retry = false;

      std::string s = interact.query(*this, c, visible);

      if (s == "Cancel" || s == "Quit/No") {
        retry = true;
        continue;
      }

      if (s == "Inspect") {
        // display status here.
        auto attr = c.attributes();
        std::vector<std::string> at(attr.begin(), attr.end());

        interact.query(*this, c, at, 30);

        retry = true;
        continue;
      }

      if (s == "Quit/Yes") {
        return "Quit.";
      }

      auto targets = resolve(c, s);

      if (!targets || (targets.just.size() < 1)) {
        retry = true;
        continue;
      }

      interact.action(*this, s, c, targets.just);

      return c(s, targets.just);
    } while (retry);

    return "Nothing happened.";
  }

protected:
  using parent::attribute;
};
}
}
}

#endif
