/**\file
 * \brief Generic game flow
 *
 * Contains a generic game flow object.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#if !defined(METAQUEST_FLOW_GENERIC_H)
#define METAQUEST_FLOW_GENERIC_H

namespace metaquest {
namespace flow {
template <typename interaction, typename logic, typename L = long double>
class generic {
 public:
  generic(void) : interact(), game(interact) { interact.clear(); }

  ~generic(void) { interact.clear(); }

  virtual bool run(void) {
    while (true) {
      interact.drawUI(game);

      switch (game.state()) {
        case logic::menu:
          interact.log(log = game.doMenu());
          break;
        case logic::combat:
          interact.log(log = game.doCombat());
          break;
        case logic::victory:
          interact.log(log = game.doVictory());
          break;
        case logic::defeat:
          interact.log(log = game.doDefeat());
          return true;
        case logic::exit:
          return true;
        default:
          return false;
      }
    }

    return true;
  }

  virtual bool load(efgy::json::value<L> json) {
    game.load(json("game"));
    interact.load(json("interaction"));
    return true;
  }

  virtual efgy::json::value<L> json(void) const {
    efgy::json::value<L> rv;

    rv("game") = game.json();
    rv("interaction") = interact.json();

    return rv;
  }

  interaction interact;
  logic game;
  std::string log;
};
}
}

#endif
