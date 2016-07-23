/**\file
 * \brief Items
 *
 * Items, the spice of any game. This header defines the basic interface for the
 * lot of them.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#if !defined(METAQUEST_ITEM_H)
#define METAQUEST_ITEM_H

#include <metaquest/action.h>

namespace metaquest {
/**\brief An item
 *
 * Defines the basic interface that any item follows - turns out items are
 * your standard type of object. Who'da thunk?
 *
 * \tparam T Base type for attributes. Integers are probably a good choice,
 *           at least for J-RPGs and tabletops.
 */
template <typename T = long> class item : public object<T> {
public:
  using parent = metaquest::object<T>;

  std::string effect;

  virtual const slots<T> usedSlots(void) const { return targetSlots; }

  virtual bool load(efgy::json::json json) {
    parent::load(json);

    for (const auto data : json("target-slots").asObject()) {
      targetSlots[data.first] = data.second.asNumber();
    }

    effect = json("effect").asString();

    return true;
  }

  virtual efgy::json::json json(void) const {
    efgy::json::json rv = parent::json();

    auto &sl = rv("target-slots");
    for (auto &slot : targetSlots) {
      sl(slot.first) = efgy::json::json::numeric(slot.second);
    }

    rv("effect") = effect;

    return rv;
  }

protected:
  slots<T> targetSlots;
};

template <typename T> class items : public std::vector<item<T> > {
public:
  using std::vector<item<T> >::vector;

  virtual bool load(efgy::json::json json) {
    this->clear();

    for (const auto data : json.asArray()) {
      item<T> it;
      it.load(data);
      this->push_back(it);
    }

    return true;
  }

  virtual efgy::json::json json(void) const {
    efgy::json::json rv;
    rv.toArray();

    for (auto &it : *this) {
      rv.push(it.json());
    }

    return rv;
  }
};
}

#endif
