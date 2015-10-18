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

#if !defined(METAQUEST_GAME_H)
#define METAQUEST_GAME_H

#include <metaquest/character.h>
#include <metaquest/party.h>
#include <random>
#include <algorithm>
#include <iterator>
#include <functional>

namespace metaquest {
namespace game {
template <typename ch, typename inter> class base {
 public:
  using num = typename ch::base;
  using object = object<num>;
  using character = character<num>;
  using action = action<num>;

  base(inter &pInteract, num pParties = 1)
      : interact(pInteract),
        rng(std::random_device()()),
        willExit(false),
        nParties(pParties) {
    generateParties();
  }

  std::vector<metaquest::party<ch> > parties;

  enum state {
    menu,
    combat,
    victory,
    defeat,
    exit
  };

  virtual enum state state(void) const {
    if (willExit) {
      return exit;
    }

    if (parties.size() == 1) {
      return menu;
    }

    for (std::size_t pi = 0; pi < parties.size(); pi++) {
      auto &p = parties[pi];

      if (p.defeated()) {
        switch (parties.size() - pi - 1) {
          case 0:
            return victory;
          default:
            return defeat;
        }
      }
    }

    return combat;
  }

  virtual std::vector<character *> turnOrder(void) {
    std::vector<character *> candidates;

    for (auto &pa : parties) {
      for (auto &h : pa) {
        candidates.push_back(&h);
      }
    }

    std::vector<character *> filteredCandidates;

    std::copy_if(candidates.begin(), candidates.end(),
                 std::back_inserter(filteredCandidates),
                 [](character * cha)->bool {
      return cha->able();
    });

    std::random_shuffle(filteredCandidates.begin(), filteredCandidates.end());

    return filteredCandidates;
  }

  virtual character &nextCharacter(void) {
    character *next = 0;

    do {
      if (currentTurnOrder.size() == 0) {
        currentTurnOrder = turnOrder();
        doTurn();
      }

      next = currentTurnOrder.front();
      currentTurnOrder.erase(currentTurnOrder.begin());
    } while (!next->able());

    return *next;
  }

  virtual std::string doMenuAction(bool allowCharacterActions) {
    character &c = nextCharacter();

    auto act = actions(c);

    return resolve(c, act, allowCharacterActions);
  }

  virtual std::string doMenu(void) { return doMenuAction(false); }

  virtual std::string doCombat(void) { return doMenuAction(true); }

  virtual std::string doTurn(void) {
    turn += 1;
    return "Next turn";
  }

  virtual std::string doVictory(void) {
    currentTurnOrder.clear();
    parties.erase(parties.begin() + 1);
    interact.clear();
    return "The player party was victorious!";
  }

  virtual std::string doDefeat(void) {
    currentTurnOrder.clear();
    return "The player party was defeated!";
  }

  typedef std::map<std::string,
                   std::function<std::string(bool &, const character &)> >
      actionMap;

  std::string resolve(character &target, actionMap &actions,
                      bool allowCharacterActions = true) {
    std::vector<std::string> labels;
    bool retry = false;
    std::string res;

    if (allowCharacterActions) {
      labels = target.visibleActions(*this);
    }

    for (const auto &a : actions) {
      labels.push_back(a.first);
    }

    do {
      retry = false;

      std::string s = interact.query(*this, target, labels);

      if (s == "Cancel") {
        retry = true;
      } else if (actions.find(s) != actions.end()) {
        res = actions[s](retry, target);
      } else {
        auto resolution = apply(target, s);
        if (resolution.nothing) {
          retry = true;
        } else {
          return resolution.just;
        }
      }
    } while (retry);

    return res;
  }

  virtual efgy::maybe<std::string> apply(character &target,
                                         const std::string &s) {
    auto targets = resolve(target, s);

    if (targets.size() == 0) {
      return efgy::maybe<std::string>();
    }

    interact.action(*this, s, target, targets);

    return target(s, targets);
  }

  virtual actionMap actions(character &c) {
    using namespace std::placeholders;

    actionMap actions;

    if (!useAI(c)) {
      actions["Quit/Yes"] = std::bind(&base::quit, this, _1, _2);
      actions["Quit/No"] = ignore;
    }

    return actions;
  }

  static std::string ignore(bool &retry, const character &) {
    retry = true;
    return "Scratch that.";
  }

  std::string quit(bool &retry, const character &) {
    willExit = true;
    retry = false;
    return "Quit.";
  }

  std::string useItem(bool &retry, const character &o) {
    retry = false;

    return "...";
  }

  std::string equip(bool &retry, const character &o, const item<num> &i) {
    std::size_t pn = partyOf(o);
    std::size_t n = positionOf(o);

    auto &p = parties[pn];
    auto &c = p[n];

    std::set<std::string> se;

    se.insert(i.name.display());

    for (auto &slot : i.usedSlots()) {
      for (auto &item : p.inventory) {
        auto slots = item.usedSlots();
        if (slots[slot.first] > 0) {
          se.insert(item.name.display());
        }
      }
    }

    if (se.size() == 0) {
      return "No items to replace this with";
    }

    std::vector<std::string> sel(se.begin(), se.end());
    std::string sItem = interact.query(*this, o, sel, 12);

    for (std::size_t x = 0; x < c.equipment.size(); x++) {
      auto &item = c.equipment[x];
      if (item.name.display() == i.name.display()) {
        p.inventory.push_back(item);
        c.equipment.erase(c.equipment.begin() + x);
        break;
      }
    }

    for (std::size_t x = 0; x < p.inventory.size(); x++) {
      auto &item = p.inventory[x];
      if (item.name.display() == sItem) {
        c.equipment.push_back(item);
        p.inventory.erase(p.inventory.begin() + x);
        break;
      }
    }

    retry = false;

    return "Item swapped.";
  }

  std::string equip(bool &retry, const character &o, const std::string &s) {
    std::size_t pn = partyOf(o);
    std::size_t n = positionOf(o);

    auto &p = parties[pn];
    auto &c = p[n];

    std::set<std::string> se;

    for (auto &item : p.inventory) {
      auto slots = item.usedSlots();
      if (slots[s] > 0) {
        se.insert(item.name.display());
      }
    }

    if (se.size() == 0) {
      return "No items to equip in this slot.";
    }

    std::vector<std::string> sel(se.begin(), se.end());
    std::string sItem = interact.query(*this, o, sel, 12);

    for (std::size_t x = 0; x < p.inventory.size(); x++) {
      auto &item = p.inventory[x];
      if (item.name.display() == sItem) {
        c.equipment.push_back(item);
        p.inventory.erase(p.inventory.begin() + x);
        break;
      }
    }

    retry = false;

    return "Item equipped.";
  }

  std::string equipItem(bool &retry, const character &o) {
    retry = true;

    std::vector<std::string> slots;

    for (const auto &item : o.equipment) {
      for (const auto &slot : item.usedSlots()) {
        slots.push_back(slot.first + ": " + item.name.display());
      }
    }

    for (const auto &slot : o.freeSlots()) {
      if (slot.second > 0) {
        slots.push_back(slot.first);
      }
    }

    std::string sl = interact.query(*this, o, slots, 8);

    for (const auto &item : o.equipment) {
      for (const auto &slot : item.usedSlots()) {
        if ((slot.first + ": " + item.name.display()) == sl) {
          return equip(retry, o, item);
        }
      }
    }

    return equip(retry, o, sl);
  }

  std::string inspect(bool &retry, const character &o) {
    retry = true;

    auto cr = resolve(o, action::everyone, action::none);
    if (cr.size() == 0) {
      return "Maybe not?";
    }

    auto cs = interact.query(*this, o, cr);
    if (cs.nothing || (cs.just.size() == 0)) {
      return "Maybe not?";
    }

    auto &c = *(cs.just[0]);
    std::map<std::string, std::string> data;
    for (const auto &attr : c.attributes()) {
      std::ostringstream os("");
      os << c[attr];
      data[attr] = os.str();
    }

    for (const auto &slot : c.freeSlots()) {
      if (slot.second > 0) {
        std::ostringstream os("");
        os << slot.second;
        data[slot.first] = os.str();
      }
    }

    for (const auto &item : c.equipment) {
      for (const auto &slot : item.usedSlots()) {
        data[slot.first] +=
            (data[slot.first] != "" ? ", " : "") + item.name.display();
      }
    }

    interact.display("Status", data, 30);

    return "Let's see...";
  }

  inter &interact;

  size_t partyOf(const character &c) const {
    for (size_t pi = 0; pi < parties.size(); pi++) {
      for (const auto &ca : parties[pi]) {
        if (&ca == &c) {
          return pi;
        }
      }
    }

    return 0;
  }

  size_t positionOf(const character &c) const {
    const auto party = partyOf(c);

    for (auto pi = 0; pi < parties[party].size(); pi++) {
      const auto &ca = parties[party][pi];

      if (&ca == &c) {
        return pi;
      }
    }

    return 0;
  }

  /**\brief Is this character controlled by an AI?
   *
   * J-RPGs are usually single-player, so most characters in an
   * encounter are controlled by the computer. The 'default' way
   * to check whether this applies to any given character is to
   * see if they're in party 0 or not - party 0 being the default
   * player party.
   *
   * Different games may want to do this differently, so overrides
   * may be in order then.
   *
   * \param[in] c The character to look up.
   *
   * \returns 'true' when a character should be controlled by an AI.
   */
  bool useAI(const character &c) const {
    const auto party = partyOf(c);
    return party > 0;
  }

  std::vector<character *> resolve(const character &c, const std::string &s) {
    return resolve(c, c.scope(s), c.filter(s));
  }

  std::vector<character *> resolve(const character &c,
                                   const enum action::scope scope,
                                   const enum action::filter filter,
                                   bool query = true) {
    size_t p = partyOf(c);
    size_t m = positionOf(c);

    std::vector<character *> candidates;

    switch (scope) {
      case action::self:
        candidates.push_back(&(parties[p][m]));
        break;
      case action::ally:
      case action::party:
        for (auto &h : parties[p]) {
          candidates.push_back(&h);
        }
        break;
      case action::enemy:
      case action::enemies:
        for (size_t pi = 0; pi < parties.size(); pi++) {
          if (pi != p) {
            for (auto &h : parties[pi]) {
              candidates.push_back(&h);
            }
          }
        }
        break;
      case action::everyone:
        for (auto &pa : parties) {
          for (auto &h : pa) {
            candidates.push_back(&h);
          }
        }
        break;
    }

    std::vector<character *> filteredCandidates;

    switch (filter) {
      case action::none:
        filteredCandidates = candidates;
        break;
      case action::onlyHealthy:
        std::copy_if(candidates.begin(), candidates.end(),
                     std::back_inserter(filteredCandidates),
                     [](character * cha)->bool {
          return (*cha)["HP/Current"] == (*cha)["HP/Total"];
        });
        break;
      case action::onlyAlive:
        std::copy_if(candidates.begin(), candidates.end(),
                     std::back_inserter(filteredCandidates),
                     [](character * cha)->bool {
          return cha->alive();
        });
        break;
      case action::onlyUnhealthy:
        std::copy_if(candidates.begin(), candidates.end(),
                     std::back_inserter(filteredCandidates),
                     [](character * cha)->bool {
          return cha->alive() && (*cha)["HP/Current"] < (*cha)["HP/Total"];
        });
        break;
      case action::onlyDead:
        std::copy_if(candidates.begin(), candidates.end(),
                     std::back_inserter(filteredCandidates),
                     [](character * cha)->bool {
          return !cha->alive();
        });
        break;
      case action::onlyUndefeated:
        std::copy_if(candidates.begin(), candidates.end(),
                     std::back_inserter(filteredCandidates),
                     [](character * cha)->bool {
          return !cha->defeated();
        });
        break;
    }

    if (filteredCandidates.size() == 0) {
      return std::vector<character *>();
    }

    if (!query) {
      return filteredCandidates;
    }

    switch (scope) {
      case action::self:
      case action::party:
      case action::enemies:
      case action::everyone:
        return filteredCandidates;
      case action::ally:
      case action::enemy: {
        auto q = interact.query(*this, c, filteredCandidates, 8);
        if (q.nothing) {
          return std::vector<character *>();
        } else {
          return q.just;
        }
      }
    }
  }

  virtual std::string generateParty(void) {
    parties.push_back(metaquest::party<ch>::generate(4));
    return "a new party appeared!\n";
  }

  virtual std::string generateParties(void) {
    std::string out = "";

    while (parties.size() < nParties) {
      out += generateParty();
    }

    return out;
  }

 protected:
  std::mt19937 rng;
  std::vector<character *> currentTurnOrder;
  num nParties;
  num turn;

  bool willExit;
};
}
}

#endif
