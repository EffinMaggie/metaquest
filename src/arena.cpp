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

#include <fstream>
#include <sstream>

#include <metaquest/terminal.h>
#include <metaquest/party.h>
#include <metaquest/rules-simple.h>
#include <metaquest/flow-generic.h>
#include <ef.gy/stream-json.h>
#include <ef.gy/cli.h>

using namespace efgy;

static cli::flag<std::string> saveFile("save-file",
                                       "where to store/load game data to/from");

/**\brief Metaquest: Arena main function
 *
 * This is the main function for the 'arena' programme. It is currently far from
 * complete.
 *
 * \returns 0 on success, something else otherwise.
 */
int main(int argc, char **argv) {
  int rv = cli::options<>::common().apply(argc, argv);

  const std::string file = saveFile;
  efgy::json::value<> json;

  {
    metaquest::flow::generic<metaquest::interact::terminal::base<>,
                             metaquest::rules::simple::game<
                                 metaquest::interact::terminal::base<>>> game;

    if (file != "") {
      std::ifstream save(file);
      std::istreambuf_iterator<char> eos;
      std::string s(std::istreambuf_iterator<char>(save), eos);

      s >> json;

      game.load(json);
    }

    game.run();

    json = game.json();
  }

  if (file != "") {
    std::ofstream save(file);
    std::ostringstream oss("");

    oss << efgy::json::tag() << json;
    save << oss.str();
  }

  return 0;
}
