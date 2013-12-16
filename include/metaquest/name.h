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

#if !defined(METAQUEST_NAME_H)
#define METAQUEST_NAME_H

#include <ef.gy/markov.h>

#include <data/female.first.h>
#include <data/male.first.h>
#include <data/all.last.h>

#include <algorithm>
#include <cctype>

namespace metaquest
{
    namespace name
    {
        static const unsigned long seed = 1337;

        template<typename T = char, typename generator = efgy::markov::chain<T,3> >
        class name
        {
            public:
                enum type
                {
                    givenName,
                    familyName,
                    nickName,
                    callSign,
                    otherName
                };

                name(generator &pGenerator, const enum type &pType = otherName)
                    : type(pType)
                    {
                        pGenerator >> value;
                    }

                name(std::basic_string<T> pValue, const enum type &pType = otherName)
                    : value(pValue), type(pType)
                    {}

                std::basic_string<T> value;
                const enum type type;
        };

        template<typename T = char, typename generator = efgy::markov::chain<T,3> >
        class proper : public std::vector<name<T,generator> >
        {
            public:
                std::basic_string<T> full(void)
                {
                    std::basic_string<T> s = "";

                    for (const name<T,generator> &n : *this)
                    {
                        if (s.size() > 0)
                        {
                            s += " ";
                        }
                        s += n.value;
                    }

                    return s;
                }

                std::basic_string<T> display(void)
                {
                    for (const name<T,generator> &n : *this)
                    {
                        switch (n.type)
                        {
                            case name<T,generator>::givenName: return n.value;
                            case name<T,generator>::callSign:  return n.value;
                            default: break;
                        }
                    }

                    return full();
                }
        };

        namespace american
        {
            template<typename T = char, typename generator = efgy::markov::chain<T,3> >
            class given : public name<T,generator>
            {
                public:
                    typedef name<T,generator> parent;

                    using parent::value;

                    given (bool female = true, unsigned int length = 9)
                        : parent("", parent::givenName)
                        {
                            static typename generator::random PRNG(seed);
                            static generator femaleFirstNames(PRNG, data::female_first);
                            static generator maleFirstNames(PRNG, data::male_first);

                            while ((value.size() == 0) || (value.size() > length))
                            {
                                switch (PRNG() % 10)
                                {
                                    case 0:  (female ? maleFirstNames : femaleFirstNames) >> value;
                                    default: (female ? femaleFirstNames : maleFirstNames) >> value;
                                }
                            }

                            std::transform
                                (value.begin()+1, value.end(), value.begin()+1,
                                [] (char a) -> char
                                {
                                    return std::tolower(a);
                                });
                        }
            };

            template<typename T = char, typename generator = efgy::markov::chain<T,3> >
            class family : public name<T,generator>
            {
                public:
                    typedef name<T,generator> parent;

                    using parent::value;

                    family (unsigned int length = 9)
                        : parent("", parent::familyName)
                        {
                            static typename generator::random PRNG(seed);
                            static generator lastNames(PRNG, data::all_last);

                            while ((value.size() == 0) || (value.size() > length))
                            {
                                lastNames >> value;
                            }

                            std::transform
                                (value.begin()+1, value.end(), value.begin()+1,
                                [] (char a) -> char
                                {
                                    return std::tolower(a);
                                });
                        }
            };

            template<typename T = char, typename generator = efgy::markov::chain<T,3> >
            class proper : public metaquest::name::proper<T,generator>
            {
                public:
                    typedef metaquest::name::proper<T,generator> parent;

                    proper (bool female = true, unsigned int length = 9)
                        {
                            static typename generator::random PRNG(seed);

                            do
                            {
                                given<T,generator> f(female, length);
                                parent::push_back (f.value);
                            }
                            while ((PRNG() % 10) == 0);

                            do
                            {
                                family<T,generator> l(length);
                                parent::push_back (l.value);
                            }
                            while ((PRNG() % 10) == 0);
                        }
            };
        };
    };
};

#endif
