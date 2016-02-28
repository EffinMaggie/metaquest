/**\file
 * \brief Metaquest: Arena
 *
 * This is the 'arena' programme of the metaquest project. The idea of the game
 * is to have the combat portions of contemporary J-RPGs all mashed up into a
 * separate game, so you can pit your favourite characters of different games
 * against each other. Large amounts of win should ensue.
 *
 * The game is currently far from complete, so writing documentation for the
 * code feels kind of awkward.
 *
 * \copyright
 * This file is part of the Metaquest project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#include <metaquest/terminal.h>
#include <metaquest/party.h>
#include <metaquest/rules-simple.h>
#include <metaquest/flow-generic.h>
#include <ef.gy/stream-json.h>

using namespace efgy;

/**\brief Metaquest: Arena main function
 *
 * This is the main function for the 'arena' programme. It is currently far from
 * complete.
 *
 * \returns 0 on success, something else otherwise.
 */
int main(int, const char **) {
  efgy::json::value<> json;

  {
    metaquest::flow::generic<metaquest::interact::terminal::base<>,
                             metaquest::rules::simple::game<
                                 metaquest::interact::terminal::base<> > > game;

    game.run();

    json = game.json();
  }

  std::cout << efgy::json::tag() << json;
  std::cout << "\n";

  return 0;
}
