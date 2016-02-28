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

  item(const action<T> &pApply, const slots<T> &pUsedSlots)
      : parent(), apply(pApply), targetSlots(pUsedSlots) {}

  item(const action<T> &pApply) : parent(), apply(pApply), targetSlots() {}

  item(const slots<T> &pUsedSlots)
      : parent(), apply(), targetSlots(pUsedSlots) {}

  item(void) : parent(), apply(), targetSlots() {}

  action<T> apply;

  virtual const slots<T> usedSlots(void) const { return targetSlots; }

 protected:
  slots<T> targetSlots;
};

template <typename T> class items : public std::vector<item<T>> {
 public:
  using std::vector<item<T>>::vector;
};
}

#endif
