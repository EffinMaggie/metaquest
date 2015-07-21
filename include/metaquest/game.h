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

namespace metaquest {
namespace game {
template <typename character, typename inter>
class base : public metaquest::object<typename character::base> {
public:
  typedef metaquest::object<typename character::base> parent;

  using parent::attribute;
  using parent::function;

  base(inter &pInteract, long pParties = 1)
      : parent(), interact(pInteract), rng(std::random_device()()) {
    attribute["parties"] = pParties;
    generateParties();
  }

  std::vector<metaquest::party<character> > parties;

  virtual std::string next(void) = 0;

  typedef std::map<std::string,
      std::function<std::string(bool &, const character &)> > actionMap;

  std::string
  resolve(character &target,
          actionMap &actions, bool allowCharacterActions = true) {
    std::vector<std::string> labels;
    bool retry = false;
    std::string res;

    if (allowCharacterActions) {
      labels = target.visibleActions();
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
        auto targets = resolve(target, s);

        if (!targets || (targets.just.size() < 1)) {
          retry = true;
          continue;
        }

        interact.action(*this, s, target, targets.just);

        return target(s, targets.just);
      }
    } while (retry);

    return res;
  }

  static std::string ignore(bool &retry, const character &) {
    retry = true;
    return "Scratch that.";
  }

  static std::string quit(bool &retry, const character &) {
    retry = false;
    return "Quit.";
  }

  std::string inspect(bool &retry, const character &c) {
    std::map<std::string, std::string> data;
    for (const auto &attr : c.attributes()) {
      std::ostringstream os("");
      os << c[attr];
      data[attr] = os.str();
    }

    interact.display("Status", data, 30);

    retry = true;
    return "Let's see...";
  }

  std::string operator()(const std::string &command) {
    auto act = action.find(command);
    if (act != action.end()) {
      return act->second(*this);
    }

    return command + " is not something that came up while writing this game\n";
  }

  base &bind(const std::string &name,
             std::function<std::string(parent &)> apply) {
    action[name] = apply;
    return *this;
  }

  inter &interact;

  template <typename C> size_t partyOf(const C &c) const {
    for (size_t pi = 0; pi < parties.size(); pi++) {
      for (const auto &ca : parties[pi]) {
        if (&ca == &c) {
          return pi;
        }
      }
    }

    return 0;
  }

  template <typename C> size_t positionOf(const C &c) const {
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
   * \tparam C The type of character to look up.
   *
   * \param[in] c The character to look up.
   *
   * \returns 'true' when a character should be controlled by an AI.
   */
  template <typename C> bool useAI(const C &c) const {
    const auto party = partyOf(c);
    return party > 0;
  }

  template <typename C>
  efgy::maybe<std::vector<metaquest::character<typename character::base> *> >
  resolve(C &c, const std::string &s) {
    size_t p = partyOf(c);

    std::vector<metaquest::character<typename character::base> *> candidates;

    switch (c.scope(s)) {
    case metaquest::action<typename character::base>::self:
      candidates.push_back(&c);
      break;
    case metaquest::action<typename character::base>::ally:
    case metaquest::action<typename character::base>::party:
      for (auto &h : parties[p]) {
        candidates.push_back(&h);
      }
      break;
    case metaquest::action<typename character::base>::enemy:
    case metaquest::action<typename character::base>::enemies:
      for (size_t pi = 0; pi < parties.size(); pi++) {
        if (pi != p) {
          for (auto &h : parties[pi]) {
            candidates.push_back(&h);
          }
        }
      }
      break;
    case metaquest::action<typename character::base>::everyone:
      for (auto &pa : parties) {
        for (auto &h : pa) {
          candidates.push_back(&h);
        }
      }
      break;
    }

    std::vector<metaquest::character<typename character::base> *>
        filteredCandidates;

    switch (c.filter(s)) {
    case metaquest::action<typename character::base>::none:
      filteredCandidates = candidates;
      break;
    case metaquest::action<typename character::base>::onlyHealthy:
      std::copy_if(
          candidates.begin(), candidates.end(),
          std::back_inserter(filteredCandidates),
          [](metaquest::character<typename character::base> * cha)->bool {
        return (*cha)["HP/Current"] == (*cha)["HP/Total"];
      });
      break;
    case metaquest::action<typename character::base>::onlyAlive:
      std::copy_if(
          candidates.begin(), candidates.end(),
          std::back_inserter(filteredCandidates),
          [](metaquest::character<typename character::base> * cha)->bool {
        return cha->alive();
      });
      break;
    case metaquest::action<typename character::base>::onlyUnhealthy:
      std::copy_if(
          candidates.begin(), candidates.end(),
          std::back_inserter(filteredCandidates),
          [](metaquest::character<typename character::base> * cha)->bool {
        return (*cha)["HP/Current"] < (*cha)["HP/Total"];
      });
      break;
    case metaquest::action<typename character::base>::onlyDead:
      std::copy_if(
          candidates.begin(), candidates.end(),
          std::back_inserter(filteredCandidates),
          [](metaquest::character<typename character::base> * cha)->bool {
        return !cha->alive();
      });
      break;
    case metaquest::action<typename character::base>::onlyUndefeated:
      std::copy_if(
          candidates.begin(), candidates.end(),
          std::back_inserter(filteredCandidates),
          [](metaquest::character<typename character::base> * cha)->bool {
        return !cha->defeated();
      });
      break;
    }

    if (filteredCandidates.size() == 0) {
      return efgy::maybe<
          std::vector<metaquest::character<typename character::base> *> >();
    }

    switch (c.scope(s)) {
    case metaquest::action<typename character::base>::self:
    case metaquest::action<typename character::base>::party:
    case metaquest::action<typename character::base>::enemies:
    case metaquest::action<typename character::base>::everyone:
      return filteredCandidates;
      break;
    case metaquest::action<typename character::base>::ally:
    case metaquest::action<typename character::base>::enemy:
      return interact.query(*this, c, filteredCandidates, 8);
      break;
    }
  }

  virtual std::string generateParties(void) {
    base &self = *this;

    std::string out = "";

    while (parties.size() < self["parties"]) {
      parties.push_back(metaquest::party<character>::generate(4));
      out += "a new party appeared!\n";
    }

    return out;
  }

protected:
  std::map<std::string, std::function<std::string(parent &)> > action;
  std::mt19937 rng;
};
}
}

#endif
