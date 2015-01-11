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
 * \see Project Documentation: http://ef.gy/documentation/metaquest
 * \see Project Source Code: http://git.becquerel.org/jyujin/metaquest.git
 */

#if !defined(METAQUEST_TERMINAL_H)
#define METAQUEST_TERMINAL_H

#include <iostream>

#include <ef.gy/vt100.h>
#include <ef.gy/terminal-writer.h>
#include <metaquest/game.h>
#include <random>

namespace metaquest
{
    namespace interact
    {
        template<typename term = efgy::terminal::vt100<>>
        class terminal
        {
            public:
                terminal()
                    : io(),
                      out(io),
                      rng(std::random_device()())
                    {
                        io.resize(io.getOSDimensions());
                    }

                efgy::terminal::writer<> out;

                void clear(void)
                {
                    out.to(0,0).clear();
                }

                void flush(void)
                {
                    while(io.flush());
                }

                void log(std::string log)
                {
                    out.to(0,5)
                       .write(log, 400);
                }

                template<typename G>
                void drawUI
                    (G &game)
                {
                    auto &party    = game.parties[0];
                    auto &hostiles = game.parties[1];

                    long i = 0;

                    clearQuery();

                    for (auto &h : hostiles)
                    {
                        out.to(-50, i)
                           .bar(h["HP/Current"], h["HP/Total"], 50)
                           .x(0)
                           .write(h.name.full(), 30);
                        i++;
                    }

                    i = -1;
                    for (auto &p : party)
                    {
                        out.to(-50, i)
                           .bar(p["HP/Current"], p["HP/Total"], 50)
                           .x(0)
                           .write(p.name.full(), 30);
                        i--;
                    }
                }

                void clearQuery (void)
                {
                    out.to(0, 8).clear(-1, 10);
                }

                template<typename T, typename G>
                std::string query
                    (const G &game,
                     const metaquest::character<T> &source,
                     const std::vector<std::string> &list,
                     std::size_t indent = 4)
                {
                    std::size_t party = game.partyOf (source);

                    if (party > 0)
                    {
                        return list[(rng() % list.size())];
                    }

                    size_t left = indent, top = 8,
                           width = 20, height = 2 + list.size();

                    out.foreground = 7;
                    out.background = 0;

                    out.to(left, top).box(width, height);
                    flush();

                    out.to(left + 2, top)
                       .write(": " + source.name.display() + " :", source.name.display().size() + 4);

                    for (std::size_t i = 0; i < list.size(); i++)
                    {
                        out.to(left + 1, top + 1 + i).write(" " + list[i], width - 3);
                    }

                    long selection = 0;
                    bool didSelect = false;

                    do
                    {
                        out.to(left + 1, top + 1)
                           .colour(width - 2, height - 2);

                        out.foreground = 0;
                        out.background = 7;

                        out.to(left + 1, top + 1 + selection)
                           .colour(width - 2, 1);

                        out.foreground = 7;
                        out.background = 0;

                        flush();

                        io.read([&selection] (const typename term::command &c) -> bool
                            {
                                switch (c.code)
                                {
                                    case 'A': // up
                                        selection--;
                                        break;
                                    case 'B': // down
                                        selection++;
                                        break;
                                }
                                return false;
                            }, [&didSelect] (const T &l) -> bool
                            {
                                if (l == '\n')
                                {
                                    didSelect = true;
                                }
                                return false;
                            });

                        if (selection >= (long)list.size())
                        {
                            selection = list.size() - 1;
                        }

                        if (selection < 0)
                        {
                            selection = 0;
                        }
                    }
                    while (!didSelect);

                    out.to(0,15);

                    return list[selection];
                }

                template<typename T, typename G>
                std::vector<metaquest::character<T>*> query
                    (const G &game,
                     const metaquest::character<T> &source,
                     const std::vector<metaquest::character<T>*> &candidates,
                     std::size_t indent = 4)
                {
                    std::vector<metaquest::character<T>*> targets;

                    std::string hc;
                    if (candidates.size() == 1)
                    {
                        return candidates;
                    }

                    std::vector<std::string> l;
                    for (auto h : candidates)
                    {
                        l.push_back (h->name.display());
                    }
                    hc = query(game, source, l, indent);
                    for (auto h : candidates)
                    {
                        if (h->name.display() == hc)
                        {
                            targets.push_back(h);
                        }
                    }

                    while (targets.size() > 1)
                    {
                        targets.erase(targets.begin() + (rng() % targets.size()));
                    }

                    return targets;
                }

            protected:
                term io;
                std::mt19937 rng;
        };
    }
}

#endif
