/**\file
 * \brief TTY-based interaction code for the game
 *
 * Contains all the code needed to use a generic TTY to play the game.
 *
 * \copyright
 * Copyright (c) 2015, Magnus Achim Deininger <magnus@ef.gy>
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

#if !defined(METAQUEST_TERMINAL_H)
#define METAQUEST_TERMINAL_H

#include <iostream>

#include <ef.gy/vt100.h>
#include <ef.gy/terminal-writer.h>
#include <ef.gy/maybe.h>
#include <metaquest/game.h>
#include <metaquest/ai.h>
#include <random>
#include <sstream>
#include <chrono>
#include <list>
#include <thread>
#include <mutex>
#include <functional>
#include <utility>
#include <algorithm>

namespace metaquest {
namespace interact {
namespace terminal {
namespace animator {
template <typename term, typename clock> class base {
public:
  base(const typename clock::duration &pSleepTime)
      : sleepTime(pSleepTime), validSince(clock::now()), validUntil() {}

  base(const typename clock::duration &pSleepTime,
       const std::chrono::milliseconds &ttl)
      : sleepTime(pSleepTime), validSince(clock::now()),
        validUntil(validSince + ttl) {}

  virtual ~base(void) {}

  bool expire(void) {
    validUntil = clock::now();
    return true;
  }

  bool valid(void) {
    return (bool) validUntil ? clock::now() < validUntil.just : true;
  }

  double progress(typename clock::duration until) {
    const auto el = std::chrono::duration_cast<std::chrono::milliseconds>(
        clock::now() - validSince).count();
    const auto ts =
        std::chrono::duration_cast<std::chrono::milliseconds>(until).count();

    return std::min((double) el / (double) ts, 1.0);
  }

  double progress(typename clock::time_point until) {
    const auto el = std::chrono::duration_cast<std::chrono::milliseconds>(
        clock::now() - validSince).count();
    const auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        until - validSince).count();

    return std::min((double) el / (double) ts, 1.0);
  }

  double progress(void) {
    if (!(bool) validUntil) {
      return .0;
    }

    return progress(validUntil.just);
  }

  virtual bool draw(typename term::base &terminal) = 0;
  virtual bool postProcess(const typename term::base &terminal,
                           const std::size_t &l, const std::size_t &c,
                           typename term::cell &cell) = 0;

  const typename clock::duration sleepTime;

protected:
  typename clock::time_point validSince;
  efgy::maybe<typename clock::time_point> validUntil;
};

template <typename term, typename clock>
class highlight : public base<term, clock> {
public:
  highlight(const std::size_t pColumn, const std::size_t pLine,
            const std::size_t pWidth, const std::size_t pHeight)
      : column(pColumn), line(pLine), width(pWidth), height(pHeight),
        base<term, clock>(std::chrono::milliseconds(50)) {}

  virtual bool draw(typename term::base &) { return false; }

  virtual bool postProcess(const typename term::base &terminal,
                           const std::size_t &l, const std::size_t &c,
                           typename term::cell &cell) {
    if ((l >= line) && (l < (line + height)) && (c >= column) &&
        (c < (column + width))) {
      std::swap(cell.foregroundColour, cell.backgroundColour);
      return true;
    }

    return false;
  }

  std::size_t column;
  std::size_t line;
  std::size_t width;
  std::size_t height;
};

template <typename term, typename clock> class glow : public base<term, clock> {
public:
  glow(const std::size_t pColumn, const std::size_t pLine,
       const std::size_t pWidth, const std::size_t pHeight)
      : column(pColumn), line(pLine), width(pWidth), height(pHeight),
        base<term, clock>(std::chrono::milliseconds(5),
                          std::chrono::seconds(1)) {}

  using base<term, clock>::progress;

  virtual bool draw(typename term::base &) { return false; }

  virtual bool postProcess(const typename term::base &terminal,
                           const std::size_t &l, const std::size_t &c,
                           typename term::cell &cell) {
    std::size_t h = column + width * progress();

    if ((l >= line) && (l < (line + height)) && (c >= column) && (c < h)) {
      std::swap(cell.foregroundColour, cell.backgroundColour);
      return true;
    }

    return false;
  }

  std::size_t column;
  std::size_t line;
  std::size_t width;
  std::size_t height;
};
}

template <typename term = efgy::terminal::vt100<>,
          template <typename> class AI = ai::random,
          typename clock = std::chrono::system_clock>
class base;

template <typename term, template <typename> class AI, typename clock>
class refresher {
public:
  refresher(base<term, AI> &pBase) : base(pBase) {}

  bool refresh() {
    std::lock_guard<std::mutex> lock(base.activeMutex);

    for (auto it = base.active.begin(); it != base.active.end();) {
      if (!(*it)->valid()) {
        auto *p = *it;
        base.active.erase(it);
        delete p;
        it = base.active.begin();
      } else {
        it++;
      }
    }

    bool ret = false;

    for (const auto &a : base.active) {
      if (a->valid()) {
        ret = a->draw(base.io) || ret;
      }
    }

    return ret;
  }

  typename term::cell postProcess(const typename term::base &terminal,
                                  const std::size_t &l, const std::size_t &c) {
    typename term::cell cell = terminal.target[l][c];

    for (const auto &a : base.active) {
      if (a->valid()) {
        (void) a->postProcess(terminal, l, c, cell);
      }
    }

    return cell;
  }

  void flush(void) {
    std::lock_guard<std::mutex> lock(base.activeMutex);

    while (base.io.flush([this](const typename term::base & terminal,
                                const std::size_t & l, const std::size_t & c)
                             ->typename term::cell {
      return postProcess(terminal, l, c);
    }))
      ;
  }

  typename clock::duration sleepTime(void) {
    std::lock_guard<std::mutex> lock(base.activeMutex);

    typename clock::duration sleepFor = std::chrono::milliseconds(50);

    for (const auto &a : base.active) {
      if (a->valid()) {
        if (a->sleepTime < sleepFor) {
          sleepFor = a->sleepTime;
        }
      }
    }

    return sleepFor;
  }

  static void run(base<term, AI, clock> &pBase) {
    refresher self(pBase);

    while (self.base.alive) {
      self.refresh();
      self.flush();
      std::this_thread::sleep_for(self.sleepTime());
    }

    self.flush();
  }

protected:
  base<term, AI, clock> &base;
};

template <typename term, template <typename> class AI, typename clock>
class base {
public:
  base()
      : io(), out(io), rng(std::random_device()()), ai(*this), logbook(""),
        alive(true),
        refresherThread(refresher<term, AI, clock>::run, std::ref(*this)) {
    io.resize(io.getOSDimensions());
  }

  ~base(void) {
    alive = false;
    refresherThread.join();
    for (auto &a : active) {
      delete a;
    }
  }

  typedef animator::highlight<term, clock> highlight;
  typedef animator::glow<term, clock> glow;

  term io;
  efgy::terminal::writer<> out;
  AI<base<term, AI> > ai;
  std::stringstream logbook;
  std::thread refresherThread;
  volatile bool alive;
  std::list<animator::base<term, clock> *> active;
  std::mutex activeMutex;

  void addAnimator(animator::base<term, clock> *anim) {
    std::lock_guard<std::mutex> lock(activeMutex);

    active.push_back(anim);
  }

  void clear(void) { out.to(0, 0).clear(); }

  void log(std::string log) {
    logbook << log;
    out.to(0, 5).write(log, 400);
  }

  template <typename G> void drawUI(G &game) {
    long in = 0, i = 0;

    clearQuery();

    for (auto &party : game.parties) {
      if (in == 0) {
        i = -party.size();
      } else if (in == 1) {
        i = 0;
      } else {
        i++;
      }
      in++;

      for (auto &p : party) {
        std::ostringstream hp("");
        std::ostringstream mp("");

        hp << p["HP/Current"];
        mp << p["MP/Current"];

        out.to(0, i).clear(-1, 1).to(0, i).write(p.name.full(), 30).x(-60)
            .write(hp.str(), 4, 1).x(-55).write(mp.str(), 4, 4).x(-50)
            .bar2c(p["HP/Current"], p["HP/Total"], p["MP/Current"],
                   p["MP/Total"], 50, 1, 4);
        i++;
      }
    }
  }

  void clearQuery(void) { out.to(0, 8).clear(-1, 10); }

  template <typename T, typename G>
  std::string query(const G &game, const metaquest::character<T> &source,
                    const std::vector<std::string> &pList,
                    std::size_t indent = 4) {
    std::size_t party = game.partyOf(source);

    if (game.useAI(source)) {
      out.to(0, 15);
      return ai.query(game, source, pList, indent);
    }

    std::vector<std::string> list;
    std::map<std::string, std::vector<std::string> > map;

    for (const auto &la : pList) {
      std::string l = la;

      const auto pos = la.find('/');
      if (pos != std::string::npos) {
        l = la.substr(0, pos);
        map[l].push_back(la.substr(pos + 1));
      }

      if (std::find(list.begin(), list.end(), l) == list.end()) {
        list.push_back(l);
      }
    }

    size_t left = indent, top = 8, width = source.name.display().size() + 8,
           height = 2 + list.size(), llen = 0;

    for (const auto &la : list) {
      width = la.size() + 4 > width ? la.size() + 4 : width;
      std::string label = game.getResourceLabel(source, la);
      llen = label.size() > llen ? label.size() : llen;
    }

    width += llen;

    out.foreground = 7;
    out.background = 0;

    out.to(left, top).box(width, height);

    out.to(left + 2, top).write(": " + source.name.display() + " :",
                                source.name.display().size() + 4);

    for (std::size_t i = 0; i < list.size(); i++) {
      out.to(left + 1, top + 1 + i).write(" " + list[i], width - 3);

      std::string label = game.getResourceLabel(source, list[i]);
      if (label.size() > 0) {
        out.to(left + width - llen - 2, top + 1 + i).write(label, llen);
      }
    }

    long selection = 0;
    bool didSelect = false;
    bool didCancel = false;

    auto selector = new highlight(left + 1, top + 1, width - 2, 1);
    addAnimator(selector);

    do {
      selector->line = top + 1 + selection;

      io.read(
          [&selection, &didSelect, &didCancel](const typename term::command & c)
              ->bool {
        switch (c.code) {
        case 'A': // up
          selection--;
          break;
        case 'B': // down
          selection++;
          break;
        case 'C': // right: select
          didSelect = true;
          break;
        case 'D': // left: cancel
          didCancel = true;
          break;
        }
        return false;
      },
          [&didSelect](const T & l)->bool {
        if (l == '\n') {
          didSelect = true;
        }
        return false;
      });

      didSelect |= didCancel;

      if (selection >= (long) list.size()) {
        selection = list.size() - 1;
      }

      if (selection < 0) {
        selection = 0;
      }
    } while (!didSelect);

    selector->expire();

    out.to(0, 15);

    if (didCancel) {
      return "Cancel";
    }

    const auto &sel = list[selection];

    if (map[sel].size() > 0) {
      const auto sub = query(game, source, map[sel], indent + 4);

      if (sub == "Cancel") {
        return query(game, source, pList, indent);
      }

      return sel + '/' + sub;
    }

    return list[selection];
  }

  template <typename T, typename G>
  efgy::maybe<std::vector<metaquest::character<T> *> >
  query(G &game, const metaquest::character<T> &source,
        const std::vector<metaquest::character<T> *> &candidates,
        std::size_t indent = 4) {
    std::size_t party = game.partyOf(source);

    if (game.useAI(source)) {
      out.to(0, 15);
      return ai.query(game, source, candidates, indent);
    }

    std::string hc;
    if (candidates.size() == 1) {
      return candidates;
    }

    std::vector<metaquest::character<T> *> targets;
    long selection = 0;
    bool didSelect = false;
    bool didCancel = false;

    auto selector = new highlight(0, 0, io.size()[0], 1);
    addAnimator(selector);

    do {
      const auto &c = *(candidates[selection]);
      const auto &pa = game.partyOf(c);
      const auto &pp = game.positionOf(c);

      drawUI(game);

      selector->line =
          pp + (pa == 0 ? io.size()[1] - game.parties[pa].size() : 0);

      io.read(
          [&selection, &didSelect, &didCancel](const typename term::command & c)
              ->bool {
        switch (c.code) {
        case 'A': // up
          selection--;
          break;
        case 'B': // down
          selection++;
          break;
        case 'C': // right: select
          didSelect = true;
          break;
        case 'D': // left: cancel
          didCancel = true;
          break;
        }
        return false;
      },
          [&didSelect](const T & l)->bool {
        if (l == '\n') {
          didSelect = true;
        }
        return false;
      });

      didSelect |= didCancel;

      if (selection >= (long) candidates.size()) {
        selection = candidates.size() - 1;
      }

      if (selection < 0) {
        selection = 0;
      }
    } while (!didSelect);

    selector->expire();

    if (didCancel) {
      return efgy::maybe<std::vector<metaquest::character<T> *> >();
    }

    targets.push_back(candidates[selection]);
    return targets;
  }

protected:
  std::mt19937 rng;
};
}
}
}

#endif
