/**\file
 * \brief Characters
 *
 * This file implements the basic "characters" that do things in the game.
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

#if !defined(METAQUEST_CHARACTER_H)
#define METAQUEST_CHARACTER_H

#include <metaquest/item.h>
#include <metaquest/action.h>

#include <vector>

namespace metaquest {
/**\brief A character
 *
 * Characters, here, are basically objects that can use skills. So that's
 * pretty much how they're defined.
 *
 * \tparam T Base type for attributes. Integers are probably a good choice,
 *           at least for J-RPGs and tabletops.
 */
template <typename T = long> class character : public object<T> {
 public:
  typedef object<T> parent;

  using parent::attribute;

  character(const T points = 1) : parent() {
    attribute["Points/Creation"] = points;
  }

  /**\brief Is the character alive?
   *
   * Characters are either alive or not. This function tells you which it is.
   *
   * \returns 'true' if the character counts as 'alive'.
   */
  virtual bool alive(void) const {
    const auto hp = attribute.find("HP/Current");
    if (hp != attribute.end()) {
      return hp->second > 0;
    }
    return false;
  }

  /**\brief Can the character act?
   *
   * Some characters might not currently be able to act. This function will tell
   * you whether that's the case. By default, all living characters can act.
   *
   * Some rule sets could override this, e.g. when the rules support states such
   * as 'sleeping'.
   *
   * \returns 'true' if the character can act.
   */
  virtual bool able(void) const { return alive(); }

  /**\brief Is the character defeated?
   *
   * By default, any character who is not alive counts as defeated; if all
   * characters in a party are defeated, that party is removed from play.
   *
   * Some rule sets could override this, e.g. such that characters with a
   * condition that makes them unable to act and that doesn't expire on its own
   * count as defeated. Examples of this are characters turned into stone in
   * some games.
   *
   * \returns 'true' if the character should count as defeated.
   */
  virtual bool defeated(void) const { return !alive(); }

  /**\brief Use global skill
   *
   * Uses a skill with a global effect.
   *
   * \param[in] skill The skill to use.
   *
   * \returns 'True' if the skill was used successfully, 'false' if it
   *          failed.
   */
  bool operator()(const std::string &skill) { return false; }

  /**\brief Use targeted skill
   *
   * Uses a skill that targets one or more other characters.
   *
   * \param[in] skill  The skill to use.
   * \param[in] target The skill's target.
   *
   * \returns 'True' if the skill was used successfully, 'false' if it
   *          failed.
   */
  std::string operator()(const std::string &skill,
                         std::vector<character *> &pTarget) {
    auto act = action.find(skill);
    if (act == action.end()) {
      return object<T>::name.display() + " looks bewildered";
    }

    auto &cost = act->second.cost;
    if (!cost.canApply(*this)) {
      return object<T>::name.display() + " can't use " + skill + " right now";
    }

    objects<T> source, target;
    source.push_back(this);

    for (auto &t : pTarget) {
      target.push_back(t);
    }

    cost.apply(*this);

    return act->second(source, target);
  }

  /**\brief List of equipped items
   *
   * The list of items that a character currently has equipped.
   */
  items<T> equipment;

  /**\brief List of inventory items
   *
   * The list of items that a character currently has in their
   * inventory. These items are not equipped and should not have any
   * effect on the character's stats.
   */
  std::vector<item<T> > inventory;

  metaquest::action<T> &bind(
      const std::string &name, bool isVisible,
      std::function<std::string(objects<T> &, std::vector<object<T> *> &)>
          pApply,
      const enum metaquest::action<T>::scope &pScope =
          metaquest::action<T>::enemy,
      const enum metaquest::action<T>::filter &pFilter =
          metaquest::action<T>::none,
      const resource::total<T> pCost = {
  }) {
    metaquest::action<T> act(isVisible, pApply, pScope, pFilter, pCost);
    act.name = metaquest::name::simple<>(name);
    action[name] = act;
    return action[name];
  }

  std::vector<std::string> visibleActions(void) {
    std::vector<std::string> actions;

    for (auto a : action) {
      if (a.second.visible) {
        actions.push_back(a.first);
      }
    }

    return actions;
  }

  const enum metaquest::action<T>::scope scope(const std::string &act) const {
    const auto it = action.find(act);
    if (it == action.end()) {
      return metaquest::action<T>::self;
    } else {
      return it->second.scope;
    }
  }

  const enum metaquest::action<T>::filter filter(const std::string &act) const {
    const auto it = action.find(act);
    if (it == action.end()) {
      return metaquest::action<T>::none;
    } else {
      return it->second.filter;
    }
  }

  virtual std::string getResourceLabel(const std::string &act) const {
    const auto it = action.find(act);
    if (it == action.end()) {
      return "";
    } else {
      return it->second.cost.label(*this);
    }
  }

 protected:
  std::map<std::string, action<T> > action;
};
}

#endif
