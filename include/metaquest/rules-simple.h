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
static long solveDamage(double a, double b, double c) {
  static std::mt19937 rng = std::mt19937(std::random_device()());
  return 5 * std::sqrt(a * b / c) * (0.95 + (rng() % 100) / 1000.0);
}

static long getLevel(const object<long> &t) {
  const double x = std::max<long>(t["Experience"], 1);
  return std::floor(1 + std::log(x * x));
}

static long getHPTotal(const object<long> &t) {
  return std::floor(70.0 + t["Level"] * (double(t["Endurance"]) / 10.0));
}

static long getMPTotal(const object<long> &t) {
  return std::floor(40.0 + t["Level"] * (double(t["Magic"]) / 10.0));
}

static std::string attack(objects<long> &source, objects<long> &target) {
  std::stringstream os("");
  for (auto &sp : source) {
    auto &s = *sp;
    for (auto &tp : target) {
      auto &t = *tp;

      os << s.name.display() << " attacks " << t.name.display() << "\n";

      long admg = solveDamage(s["Attack"], s["Damage"], t["Defence"]);

      os << s.name.display() << " hits for " << admg << " points of damage";

      t.add("HP/Current", -admg);
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

      os << s.name.display() << " heals " << t.name.display() << "\n";

      long amt = solveDamage(s["Attack"], t["Endurance"], 1);

      os << s.name.display() << " heals " << amt << " points of damage";

      t.add("HP/Current", amt);
    }
  }
  return os.str();
}

static std::string pass(objects<long> &source, objects<long> &target) {
  std::stringstream os("");
  for (auto &sp : source) {
    auto &s = *sp;
    os << s.name.display() << " would rather be reading a book.";
  }
  return os.str();
}

class weapon : public metaquest::item<long> {
 public:
  using parent = metaquest::item<long>;

  weapon(const std::string &name)
      : parent({
    { "Weapon", 1 }
  }) {
    static std::mt19937 rng = std::mt19937(std::random_device()());

    attribute["Damage"] = 5 + rng() % 10;

    parent::name = name::simple<>(name);
    parent::name.push_back("+" + std::to_string(attribute["Damage"]));
  }
};

class character : public metaquest::character<long> {
 public:
  using parent = metaquest::character<long>;
  using action = metaquest::action<long>;

  character(long points = 0) : parent(points) {
    static std::mt19937 rng = std::mt19937(std::random_device()());

    metaquest::name::american::proper<> cname(rng() % 2);
    parent::name = cname;

    parent::equipment.push_back(weapon("Sword"));

    parent::slots = { { "Weapon", 1 }, { "Trinket", 1 } };

    attribute["Experience"] = points;

    attribute["Endurance"] = 1 + rng() % 100;
    attribute["Magic"] = 100 - attribute["Endurance"];

    attribute["Attack"] = 6;
    attribute["Defence"] = 3;

    function["Level"] = getLevel;
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
  using parent = metaquest::game::base<character, inter>;

  game(inter &pInteract) : parent(pInteract) {
    if (parent::parties.size() > 0) {
      auto &p = parent::parties[0];
      p.inventory.push_back(weapon("Dagger"));
    }
  }

  std::string fight(bool &retry, const typename parent::character &) {
    nParties = 2;
    parent::generateParties();
    return "OFF WITH THEIR HEADS!";
  }

  virtual typename parent::actionMap actions(typename parent::character &c) {
    using namespace std::placeholders;

    auto actions = parent::actions(c);

    switch (parent::state()) {
      case parent::menu:
        actions["Fight"] = std::bind(&game::fight, this, _1, _2);
        actions["Equipment"] = std::bind(&game::equipItem, this, _1, _2);
        break;
      default:
        break;
    }

    if (!parent::useAI(c)) {
      actions["Inspect"] = std::bind(&parent::inspect, this, _1, _2);
    }

    return actions;
  }

 protected:
  using parent::nParties;
};
}
}
}

#endif
