/**\file
 *
 * \copyright
 * Copyright (c) 2013, Magnus Achim Deininger <magnus@ef.gy>
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
#include <ef.gy/markov.h>
#include <metaquest/party.h>

#include <cctype>

using namespace efgy;

int main(int argc, const char **argv)
{
    terminal::vt100<> output;
    typename markov::chain<char,3>::random PRNG(1337);
    markov::chain<char,3> mcFemale(PRNG, data::female_first);
    markov::chain<char,3> mcMale(PRNG, data::male_first);
    markov::chain<char,3> mcLast(PRNG, data::all_last);

    output.resize(output.getOSDimensions());

    for (unsigned int i = 0; i < 100; i++)
    {
        metaquest::name::american::proper<> name;
        std::cerr << name.full() << "\n";
        /*
        std::string first, last;
        switch (PRNG() % 2)
        {
            case 0: mcFemale >> first; break;
            case 1: mcMale   >> first; break;
        }
        mcLast >> last;
        output.write(first, 0, i, 37);
        */
        /*
        std::transform
            (first.begin()+1, first.end(), first.begin()+1,
             [] (char a) -> char
             {
                return std::tolower(a);
             });
        std::transform
            (last.begin()+1,  last.end(),  last.begin()+1,
             [] (char a) -> char
             {
                return std::tolower(a);
             });
        std::cerr << first << " " << last << "\n";
        */
    }

    output.flush();

    metaquest::character<> C;

    std::cerr << C["frob"] << "\n";
    std::cerr << "\u261e" << "\n";

    return 0;
}
