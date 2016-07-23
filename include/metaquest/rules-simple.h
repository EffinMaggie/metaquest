/**\file
 * \brief S[ia]mple Rules
 *
 * Contains a very simple rule set that should serve as a template for more
 * complicated rule sets.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
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
static long solve(double a, double b, double c) {
  static std::mt19937 rng = std::mt19937(std::random_device()());
  return 5 * std::sqrt(a * b / c) * (0.95 + (rng() % 100) / 1000.0);
}

static long getLevel(const object<long> &t) {
  const double x = std::max<long>(t["Experience"], 1);
  return std::floor(1 + std::log(x * x));
}

static long calculate(double b, const std::string &a, const object<long> &t) {
  return std::floor(b + t["Level"] * (double(t[a]) / 10.0));
}

static long getAttack(const object<long> &t) {
  return calculate(10.0, "Endurance", t);
}

static long getDefence(const object<long> &t) {
  return calculate(5.0, "Endurance", t);
}

static long getHPTotal(const object<long> &t) {
  return calculate(70.0, "Endurance", t);
}

static long getMPTotal(const object<long> &t) {
  return calculate(40.0, "Magic", t);
}

static std::string attack(objects<long> &source, objects<long> &target) {
  std::stringstream os("");
  for (auto &sp : source) {
    auto &s = *sp;
    for (auto &tp : target) {
      auto &t = *tp;

      long admg = solve(s["Attack"], s["Damage"], t["Defence"]);

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

      long amt = solve(s["Magic"], t["Endurance"], 1);

      os << s.name.display() << " heals " << amt << " points of damage";

      t.add("HP/Current", amt);
    }
  }
  return os.str();
}

static std::string pass(objects<long> &source, objects<long> &target) {
  return "";
}

class weapon : public metaquest::item<long> {
public:
  using parent = metaquest::item<long>;

  weapon(const std::string &name) : parent() {
    static std::mt19937 rng = std::mt19937(std::random_device()());

    targetSlots["Weapon"] = 1;
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

    function["Level"] = getLevel;
    function["HP/Total"] = getHPTotal;
    function["MP/Total"] = getMPTotal;

    function["Attack"] = getAttack;
    function["Defence"] = getDefence;

    attribute["HP/Current"] = (*this)["HP/Total"];
    attribute["MP/Current"] = (*this)["MP/Total"];

    attribute.erase("Points/Creation");

    parent::actions = { "Attack", "Skill/Heal", "Pass" };
  }
};

template <typename inter>
class game : public metaquest::game::base<character, inter> {
public:
  using parent = metaquest::game::base<character, inter>;
  using action = metaquest::action<long>;

  game(inter &pInteract) : parent(pInteract) {
    parent::bind("Attack", true, attack, action::enemy, action::onlyUndefeated);
    parent::bind("Skill/Heal", true, heal, action::ally, action::onlyUnhealthy,
                 { resource::cost<long>(2, "MP") });
    parent::bind("Pass", true, pass, action::self);
  }

  std::string fight(bool &retry, const typename parent::character &) {
    parent::currentTurnOrder.clear();
    parent::nParties = 2;
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

  virtual std::string generateParty(void) {
    if (parent::parties.size() == 0) {
      parent::parties.push_back(metaquest::party<character>::generate(4));
    } else {
      long xp = 0;
      for (auto &c : parent::parties[0]) {
        xp += c["Experience"];
      }
      parent::parties.push_back(metaquest::party<character>::generate(3, xp));
    }
    return "a new party appeared!\n";
  }

  virtual std::string doVictory(void) {
    if (parent::parties.size() > 1) {
      auto &p = parent::parties[0];
      auto &d = parent::parties[1];

      p.inventory.insert(p.inventory.end(), d.inventory.begin(),
                         d.inventory.end());

      long xp = 0;
      for (auto &c : d) {
        p.inventory.insert(p.inventory.end(), c.equipment.begin(),
                           c.equipment.end());
        xp += c["Experience"];
      }

      xp /= p.size();
      if (xp == 0) {
        xp = 1;
      }

      for (auto &c : p) {
        c.add("Experience", xp);
      }
    }

    return parent::doVictory();
  }
};
}
}
}

#endif
