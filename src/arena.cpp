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

#include <metaquest/terminal.h>
#include <metaquest/party.h>
#include <metaquest/rules-simple.h>

using namespace efgy;

/**\brief Metaquest: Arena main function
 *
 * This is the main function for the 'arena' programme. It is currently far from
 * complete.
 *
 * \returns 0 on success, something else otherwise.
 */
int main(int, const char **)
{
    metaquest::interact::terminal<> inter;

    metaquest::rules::simple::game<metaquest::interact::terminal<>> game(inter);

    inter.out.to(0,0).clear();

    std::string r;

    while (true)
    {
        inter.drawUI(game);

        inter.flush();

        r = game.next();

        if (r.find("victorious") != r.npos)
        {
            break;
        }

        inter.out.to(0,5)
                 .write(r, 400);
    }

    inter.flush();

    std::cerr << "\u261e" << "\n";

    inter.out.to(0,0).clear()
             .to(0,1).write(r, 200)
             .to(0,3);

    inter.flush();

    return 0;
}
