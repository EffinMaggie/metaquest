/**\file
 * \brief Characters
 *
 * This file implements the basic "characters" that do things in the game.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
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
  virtual bool operator()(const std::string &skill) { return false; }

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
  items<T> inventory;

  std::vector<std::string> visibleActions(void) const { return actions; }

  virtual std::set<std::string> attributes(void) const {
    auto rv = parent::attributes();
    for (const auto &item : equipment) {
      auto attr = item.attributes();
      rv.insert(attr.begin(), attr.end());
    }
    return rv;
  }

  virtual T operator[](const std::string &s) const {
    T rv = parent::operator[](s);
    for (const auto &item : equipment) {
      rv += item[s];
    }
    return rv;
  }

  virtual const slots<T> allSlots(void) const {
    auto s = parent::allSlots();
    for (auto &item : equipment) {
      for (auto &sl : item.allSlots()) {
        s[sl.first] += sl.second;
      }
    }
    return s;
  }

  virtual const slots<T> usedSlots(void) const {
    slots<T> s = {};
    for (auto &item : equipment) {
      for (auto &sl : item.usedSlots) {
        s[sl.first] += sl.second;
      }
    }
    return s;
  }

  virtual const slots<T> freeSlots(void) const {
    auto s = allSlots();
    for (auto &sl : usedSlots()) {
      s[sl.first] -= sl.second;
    }
    return s;
  }

  virtual bool load(efgy::json::json json) {
    parent::load(json);

    equipment.load(json("equipment"));
    inventory.load(json("inventory"));

    return true;
  }

  virtual efgy::json::json json(void) const {
    efgy::json::json rv = parent::json();

    rv("equipment") = equipment.json();
    rv("inventory") = inventory.json();

    return rv;
  }

  std::vector<std::string> actions;
};
}

#endif
