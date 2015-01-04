/**\file
 *
 * \copyright
 * Copyright (c) 2014-2015, Magnus Achim Deininger <magnus@ef.gy>
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
        template<typename character, typename inter>
        class base : public metaquest::object<typename character::base>
        {
            public:
                typedef metaquest::object<typename character::base> parent;

                using parent::attribute;
                using parent::function;

                base (inter &pInteract, long pParties = 2)
                    : parent(),
                      interact(pInteract)
                    {
                        attribute["parties"] = pParties;
                        bind("generate-parties", doGenerateParties);
                        generateParties();
                    }

                std::vector<metaquest::party<character>> parties;

                std::string generateParties (void)
                {
                    return (*this)("generate-parties");
                }

                std::string next (void)
                {
                    return (*this)("next");
                }

                std::string operator () (const std::string &command)
                {
                    auto act = action.find(command);
                    if (act != action.end())
                    {
                        return act->second(*this);
                    }

                    return command + " is not something that came up while writing this game\n";
                }

                base &bind (const std::string &name, std::function<std::string(parent&)> apply)
                {
                    action[name] = apply;
                    return *this;
                }

                inter &interact;

                size_t getPartyIDOf (const character &c)
                {
                    for (size_t pi = 0; pi < parties.size(); pi++)
                    {
                        for (const auto &ca : parties[pi])
                        {
                            if (&ca == &c)
                            {
                                return pi;
                            }
                        }
                    }

                    return 0;
                }

                std::vector<metaquest::character<typename character::base>*> resolve
                    (character &c,
                     const std::string &s)
                {
                    size_t p = getPartyIDOf(c);

                    std::vector<metaquest::character<typename character::base>*> targets;
                    std::vector<metaquest::character<typename character::base>*> candidates;

                    switch (c.scope(s))
                    {
                        case metaquest::action<typename character::base>::self:
                            candidates.push_back(&c);
                            break;
                        case metaquest::action<typename character::base>::ally:
                        case metaquest::action<typename character::base>::party:
                            for (auto &h : parties[p])
                            {
                                if (h["Alive"])
                                {
                                    candidates.push_back (&h);
                                }
                            }
                            break;
                        case metaquest::action<typename character::base>::enemy:
                        case metaquest::action<typename character::base>::enemies:
                            for (size_t pi = 0; pi < parties.size(); pi++)
                            {
                                if (pi != p)
                                {
                                    for (auto &h : parties[pi])
                                    {
                                        if (h["Alive"])
                                        {
                                            candidates.push_back (&h);
                                        }
                                    }
                                }
                            }
                            break;
                        case metaquest::action<typename character::base>::everyone:
                            for (auto &pa : parties)
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
                        case metaquest::action<typename character::base>::self:
                        case metaquest::action<typename character::base>::party:
                        case metaquest::action<typename character::base>::enemies:
                        case metaquest::action<typename character::base>::everyone:
                            targets = candidates;
                            break;
                        case metaquest::action<typename character::base>::ally:
                        case metaquest::action<typename character::base>::enemy:
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
                                hc = interact.query(*this, p, c, l, 8);
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

                    return targets;
                }

            protected:
                static std::string doGenerateParties (parent &pSelf)
                {
                    base &self = static_cast<base&>(pSelf);

                    std::string out = "";

                    while (self.parties.size() < self["parties"])
                    {
                        self.parties.push_back(metaquest::generate<character>(4));
                        out += "a new party appeared!\n";
                    }

                    return out;
                }

                std::map<std::string,std::function<std::string(parent&)>> action;
                std::mt19937 rng;
        };
    }
}

#endif
