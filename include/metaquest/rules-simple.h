/**\file
 * \brief S[ia]mple Rules
 *
 * Contains a very simple rule set that should serve as a template for more
 * complicated rule sets.
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

#if !defined(METAQUEST_RULES_SIMPLE_H)
#define METAQUEST_RULES_SIMPLE_H

#include <metaquest/character.h>
#include <random>

namespace metaquest
{
    namespace rules
    {
        namespace simple
        {
            static long isAlive(object<long> &t)
            {
                return t.attribute["HP/Current"] > 0;
            }

            static long getHPTotal(object<long> &t)
            {
                return t.attribute["Experience"] * 2 + 5;
            }

            static int roll (int num, int sides = 6)
            {
                static std::mt19937 rng;

                int res = 0;
                for (int i = 0; i < num; i++)
                {
                    res += 1 + rng() % sides;
                }
                return res;
            }

            static bool attack (std::vector<object<long>*> &source, std::vector<object<long>*> &target)
            {
                for (auto &sp : source)
                {
                    auto &s = *sp;
                    for (auto &tp : target)
                    {
                        auto &t = *tp;

                        std::cout << s.name.display() << " attacks " << t.name.display() << "\n";

                        int dmg = roll(s["Attack"]);
                        int def = roll(s["Defence"]);
                        int admg = dmg - def;

                        if (admg > 0)
                        {
                            std::cout << s.name.display() << " hits for " << admg << " (" << dmg << ") points of damage\n";

                            t.attribute["HP/Current"] -= admg;
                            if (!t["Alive"])
                            {
                                s.attribute["Experience"] += t.attribute["Experience"]/2 + 1;
                            }
                        }
                        else
                        {
                            std::cout << s.name.display() << " misses\n";
                        }
                    }
                }
                return true;
            }

            class doAttack : public action<long>
            {
                public:
                    doAttack()
                        : action<long>(true, attack)
                        {}

            };

            class character : public metaquest::character<long>
            {
                public:
                    typedef metaquest::character<long> parent;

                    using parent::name;

                    character()
                        : parent()
                        {
                            metaquest::name::american::proper<> cname;
                            name = cname;

                            attribute["Attack"]     = 1;
                            attribute["Defence"]    = 1;
                            attribute["Experience"] = 0;

                            function["HP/Total"]    = getHPTotal;
                            function["Alive"]       = isAlive;

                            attribute["HP/Current"] = (*this)["HP/Total"];

                            actions["Attack"] = doAttack();
                        }
            };
        };
    };
};

#endif
