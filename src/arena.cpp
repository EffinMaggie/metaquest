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
 * Copyright (c) 2013-2014, Magnus Achim Deininger <magnus@ef.gy>
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
#include <metaquest/party.h>
#include <metaquest/rules-simple.h>

using namespace efgy;

template<typename T, typename U>
void drawBar(terminal::terminal<U> &term,
             const T &x, const T &y, const T &width, const T &min, const T &max,
             const T &full = '#', const T &left = '[', const T &right = ']',
             const int &foregroundColour = 7,
             const int &backgroundColour = 0)
{
    const auto dim = term.size();
    const double perc = double(min)/double(max);
    const T fullchars = perc > 0 ? (width-2) * perc : 0;

    const T ry = (y >= 0) ? y : (dim[1] + y);
    T rx = (x >= 0) ? x : (dim[0] + x);

    term.target[ry][rx].content = left;
    term.target[ry][rx].foregroundColour = foregroundColour;
    term.target[ry][rx].backgroundColour = backgroundColour;
    rx++;
    for (T i = 0; i < (width - 2); i++, rx++)
    {
        term.target[ry][rx].content = (i < fullchars) ? full : ' ';
        term.target[ry][rx].foregroundColour = foregroundColour;
        term.target[ry][rx].backgroundColour = backgroundColour;
    }
    term.target[ry][rx].content = right;
    term.target[ry][rx].foregroundColour = foregroundColour;
    term.target[ry][rx].backgroundColour = backgroundColour;
}

/**\brief Metaquest: Arena main function
 *
 * This is the main function for the 'arena' programme. It is currently far from
 * complete.
 *
 * \returns 0 on success, something else otherwise.
 */
int main(int, const char **)
{
    terminal::vt100<> output;

    output.resize(output.getOSDimensions());

    auto party = metaquest::generate<metaquest::rules::simple::character>(4);

    std::cerr << party[0].name.full() << " [" << party[0].name.display() << "]\n";

    for (unsigned int i = 0; i < 100; i++)
    {
        auto hostiles = metaquest::generate<metaquest::rules::simple::character>(4);

        //std::cerr << hostiles[0].name.full() << " [" << hostiles[0].name.display() << "]\n";

        //std::cerr << hostiles[0]["Attack"] << "\n";
        //std::cerr << hostiles[0]["HP/Current"] << "\n";

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

                drawBar<long>(output, -50, i, 50, h["HP/Current"], h["HP/Total"]);
                i++;
            }

            i = -1;
            for (auto &p : party)
            {
                drawBar<long>(output, -50, i, 50, p["HP/Current"], p["HP/Total"]);
                i--;
            }

            while(output.flush());

            if (targets.size() == 0)
            {
                break;
            }

            //std::cerr << party[0]("Attack", targets) << "\n";
            party[0]("Attack", targets);
        }
    }

    output.flush();

    std::cerr << "\u261e" << "\n";

    return 0;
}
