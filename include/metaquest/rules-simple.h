/**\file
 * \brief S[ia]mple Rules
 *
 * Contains a very simple rule set that should serve as a template for more
 * complicated rule sets.
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

#if !defined(METAQUEST_RULES_SIMPLE_H)
#define METAQUEST_RULES_SIMPLE_H

#include <metaquest/character.h>
#include <metaquest/game.h>
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

            static std::string attack (std::vector<object<long>*> &source, std::vector<object<long>*> &target)
            {
                std::stringstream os("");
                for (auto &sp : source)
                {
                    auto &s = *sp;
                    for (auto &tp : target)
                    {
                        auto &t = *tp;

                        os << s.name.display() << " attacks " << t.name.display() << "\n";

                        int dmg = roll(s["Attack"]);
                        int def = roll(s["Defence"]);
                        int admg = dmg - def;

                        if (admg > 0)
                        {
                            os << s.name.display() << " hits for " << admg << " (" << dmg << ") points of damage\n";

                            t.attribute["HP/Current"] -= admg;
                            if (!t["Alive"])
                            {
                                s.attribute["Experience"] += t.attribute["Experience"]/2 + 1;
                            }
                        }
                        else
                        {
                            os << s.name.display() << " misses\n";
                        }
                    }
                }
                return os.str();
            }

            static std::string heal (std::vector<object<long>*> &source, std::vector<object<long>*> &target)
            {
                std::stringstream os("");
                for (auto &sp : source)
                {
                    auto &s = *sp;
                    for (auto &tp : target)
                    {
                        auto &t = *tp;

                        os << s.name.display() << " heals " << t.name.display() << "\n";

                        int amt = roll(s["Attack"]);

                        os << s.name.display() << " heals " << amt << " points of damage\n";

                        t.attribute["HP/Current"] += amt;
                        if (t["HP/Current"] > t["HP/Total"])
                        {
                            t.attribute["HP/Current"] -= t["HP/Current"] - t["HP/Total"];
                        }
                    }
                }
                return os.str();
            }

            static std::string pass (std::vector<object<long>*> &source, std::vector<object<long>*> &target)
            {
                std::stringstream os("");
                for (auto &sp : source)
                {
                    auto &s = *sp;
                    os << s.name.display() << " would rather be reading a book.\n";
                }
                return os.str();
            }

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

                            bind("Attack", true, attack);
                            bind("Heal", true, heal, metaquest::action<long>::ally);
                            bind("Pass", true, pass, metaquest::action<long>::self);
                        }
            };

            template<typename inter>
            class game : public metaquest::game::base<character, inter>
            {
                public:
                    typedef metaquest::game::base<character, inter> parent;

                    using parent::bind;

                    game(inter &pInteract)  
                        : parent(pInteract)
                        {
                            bind("next", doNext);
                        }

                protected:
                    static std::string doNext (typename parent::parent &pSelf)
                    {
                        game &self = static_cast<game&>(pSelf);

                        for (std::size_t pi = 0; pi < self.parties.size(); pi++)
                        {
                            auto &p = self.parties[pi];
                            long alive = 0;

                            for (auto &c : p)
                            {
                                if (c["Alive"])
                                {
                                    alive++;
                                }
                            }

                            if (alive == 0)
                            {
                                std::ostringstream os("");
                                os << "Party #" << (self.parties.size() - pi - 1) << " was victorious!\n";
                                return os.str();
                            }
                        }

                        static std::mt19937 rng;

                        std::size_t p = rng() % self.parties.size();
                        std::size_t n = 0;
                        do
                        {
                            n = rng() % self.parties[p].size();
                        }
                        while (!self.parties[p][n]["Alive"]);

                        character &c = self.parties[p][n];

                        auto visible = c.visibleActions();

                        std::string s = self.interact.query(self, p, c, visible);

                        std::vector<metaquest::character<>*> targets;
                        std::vector<metaquest::character<>*> candidates;

                        switch (c.scope(s))
                        {
                            case metaquest::action<long>::self:
                                candidates.push_back(&c);
                                break;
                            case metaquest::action<long>::ally:
                            case metaquest::action<long>::party:
                                for (auto &h : self.parties[p])
                                {
                                    if (h["Alive"])
                                    {
                                        candidates.push_back (&h);
                                    }
                                }
                                break;
                            case metaquest::action<long>::enemy:
                            case metaquest::action<long>::enemies:
                                for (auto &h : self.parties[(self.parties.size() - 1 - p)])
                                {
                                    if (h["Alive"])
                                    {
                                        candidates.push_back (&h);
                                    }
                                }
                                break;
                            case metaquest::action<long>::everyone:
                                for (auto &pa : self.parties)
                                {
                                    for (auto &h : pa)
                                    {
                                        if (h["Alive"])
                                        {
                                            candidates.push_back (&h);
                                        }
                                    }
                                }
                                break;
                        }

                        switch (c.scope(s))
                        {
                            case metaquest::action<long>::self:
                            case metaquest::action<long>::party:
                            case metaquest::action<long>::enemies:
                            case metaquest::action<long>::everyone:
                                targets = candidates;
                                break;
                            case metaquest::action<long>::ally:
                            case metaquest::action<long>::enemy:
                            {
                                std::string hc;
                                if (candidates.size() == 1)
                                {
                                    hc = candidates[0]->name.display();
                                }
                                else
                                {
                                    std::vector<std::string> l;
                                    for (auto h : candidates)
                                    {
                                        l.push_back (h->name.display());
                                    }
                                    hc = self.interact.query(self, p, c, l, 8);
                                }
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
                                break;
                            }
                        }

                        self.interact.clearQuery();

                        return c(s, targets);
                    }
            };
        }
    }
}

#endif
