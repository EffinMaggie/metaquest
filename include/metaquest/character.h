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

#if !defined(METAQUEST_CHARACTER_H)
#define METAQUEST_CHARACTER_H

#include <metaquest/item.h>

#include <vector>

namespace metaquest
{
    template<typename T = long>
    class character : public object<T>
    {
        public:
            using object<T>::name;
            using object<T>::operator[];

            std::string type;

            bool operator () (const std::string &skill, const T &modifier)
            {
                return false;
            }

            bool operator () (const std::string &skill, character &target)
            {
                return false;
            }

            bool operator () (const std::string &skill, std::vector<character> &target)
            {
                return false;
            }

        protected:
            std::vector<item<T> > equipment;
            std::vector<item<T> > inventory;
    };
};

#endif
