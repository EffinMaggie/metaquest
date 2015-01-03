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
 * \see Project Documentation: http://ef.gy/documentation/metaquest
 * \see Project Source Code: http://git.becquerel.org/jyujin/metaquest.git
 */

#include <iostream>

#include <ef.gy/vt100.h>
#include <ef.gy/terminal-writer.h>
#include <metaquest/party.h>
#include <metaquest/rules-simple.h>

using namespace efgy;

template<typename term = terminal::vt100<>>
class interact
{
    public:
        interact()
            : io(),
              out(io),
              rng()
            {
                io.resize(io.getOSDimensions());
            }
              
        terminal::writer<> out;

        void flush(void)
        {
            while(io.flush());
        }

        template<typename T, typename G>
        std::string query (const G &game, const metaquest::character<T> &source, const std::vector<std::string> &list)
        {
            out.to(6, 6).box(20,2 + list.size());
            flush();

            for (std::size_t i = 0; i < list.size(); i++)
            {
                out.to(8, 7+i).write(list[i], 17);
            }

            long selection = 0;
            bool didSelect = false;

            do
            {
                flush();

                io.read([&selection] (const typename term::command &c) -> bool
                    {
                        return false;
                    }, [&didSelect] (const T &l) -> bool
                    {
                        if (l == '\n')
                        {
                            didSelect = true;
                        }
                        return false;
                    });

                flush();
            }
            while (!didSelect);

            out.to(0,15);

            return list[selection];
        }

    protected:
        term io;
        std::mt19937 rng;
};

/**\brief Metaquest: Arena main function
 *
 * This is the main function for the 'arena' programme. It is currently far from
 * complete.
 *
 * \returns 0 on success, something else otherwise.
 */
int main(int, const char **)
{
    interact<> inter;

    metaquest::rules::simple::game<interact<>> game(inter);

    auto &party = game.parties[0];
    auto &hostiles = game.parties[1];

    while (true)
    {
        std::vector<metaquest::character<>*> targets;
        int i = 0;

        for (auto &h : hostiles)
        {
            if (h["Alive"])
            {
                targets.push_back(&h);
            }

            inter.out.to(-50, i)
                     .bar(h["HP/Current"], h["HP/Total"], 50)
                     .x(0)
                     .write(h.name.full(), 30);
            i++;
        }

        i = -1;
        for (auto &p : party)
        {
            inter.out.to(-50, i)
                     .bar(p["HP/Current"], p["HP/Total"], 50)
                     .x(0)
                     .write(p.name.full(), 30);
            i--;
        }

        inter.flush();

        if (targets.size() == 0)
        {
            break;
        }

        inter.out.to(0,5)
                 .write(game.next(), 400);

        // party[0]("Attack", targets);
    }

    inter.flush();

    std::cerr << "\u261e" << "\n";

    return 0;
}
