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

#include <data/female.first.h>
#include <data/male.first.h>
#include <data/all.last.h>

using namespace efgy;

int main(int argc, const char **argv)
{
    typename markov::chain<char,3>::random PRNG(1);
    markov::chain<char,3> mc(PRNG);

    for (const char *s : data::female_first)
    {
        std::string str = s;
        mc << typename markov::chain<char,3>::input(str.begin(), str.end());
    }

    typename markov::chain<char,3>::output out;
    for (unsigned int i = 0; i < 100; i++)
    {
        mc >> out;
        std::cerr << std::string(out.begin(), out.end()) << "\n";
    }

    metaquest::character<> C;

    std::cerr << C["frob"] << "\n";

    return 0;
}
