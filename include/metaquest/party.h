/**\file
 * \brief Parties
 *
 * Groups of characters are called 'parties'. This file contains things related
 * to this concept.
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

#if !defined(METAQUEST_PARTY_H)
#define METAQUEST_PARTY_H

#include <metaquest/character.h>

namespace metaquest
{
    /**\brief A party
     *
     * This type represents a group of characters, referred to as a 'party'. The
     * type is based on std::vector as opposed to std::set because in some
     * contexts (menu, etc.) the order might actually be relevant.
     *
     * \tparam T Base type for attributes. Integers are probably a good choice,
     *           at least for J-RPGs and tabletops.
     */
    template<typename T = long>
    using party = std::vector<character<T> >;
};

#endif
