/**\file
 *
 * \copyright
 * Copyright (c) 2014, Magnus Achim Deininger <magnus@ef.gy>
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

#if !defined(METAQUEST_GAME_H)
#define METAQUEST_GAME_H

#include <metaquest/character.h>
#include <metaquest/party.h>
#include <random>

namespace metaquest
{
    namespace game
    {
        template<typename character>
        class base : public metaquest::object<typename character::base>
        {
            public:
                typedef metaquest::object<typename character::base> parent;

                using parent::attribute;
                using parent::function;

                base (long pParties = 2)
                    : parent()
                    {
                        attribute["parties"] = pParties;
                        function["generate-parties"] = doGenerateParties;
                        generateParties();
                    }

                std::vector<metaquest::party<character>> parties;

                typename parent::base generateParties (void)
                {
                    return (*this)["generate-parties"];
                }

                typename parent::base next (void)
                {
                    return (*this)["next"];
                }

            protected:
                static typename parent::base doGenerateParties (parent &pSelf)
                {
                    base &self = static_cast<base&>(pSelf);

                    typename parent::base generated = 0;

                    while (self.parties.size() < self["parties"])
                    {
                        self.parties.push_back(metaquest::generate<character>(4));
                        generated++;
                    }

                    return generated;
                }
        };
    }
}

#endif
