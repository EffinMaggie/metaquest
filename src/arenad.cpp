/**\file
 * \brief Metaquest: Arena
 *
 * This is the 'arenad' programme of the metaquest project. The idea of the game
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

#define ASIO_DISABLE_THREADS
#include <sstream>

#include <metaquest/terminal.h>
#include <metaquest/party.h>
#include <metaquest/rules-simple.h>
#include <metaquest/flow-generic.h>

#include <ef.gy/stream-json.h>
#include <ef.gy/httpd.h>

using namespace efgy;

static const char *arenaRx = "/arena";

template <class transport>
static bool arena(typename net::http::server<transport>::session &session,
                  std::smatch &) {
  efgy::json::value<> json;
  std::ostringstream oss("");

  // do game things here

  oss << efgy::json::tag() << json;

  session.reply(200, oss.str());
  return true;
}

namespace tcp {
using asio::ip::tcp;
static httpd::servlet<tcp> arenad(::arenaRx, ::arena<tcp>);
}

namespace unix {
using asio::local::stream_protocol;
static httpd::servlet<stream_protocol> arenad(::arenaRx,
                                              ::arena<stream_protocol>);
static httpd::servlet<stream_protocol> quit("/quit",
                                            httpd::quit<stream_protocol>);
}

int main(int argc, char *argv[]) { return io::main(argc, argv); }
