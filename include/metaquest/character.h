/**\file
 * \brief Characters
 *
 * This file implements the basic "characters" that do things in the game.
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

#if !defined(METAQUEST_CHARACTER_H)
#define METAQUEST_CHARACTER_H

#include <metaquest/item.h>
#include <metaquest/action.h>

#include <vector>

namespace metaquest
{
    /**\brief A character
     *
     * Characters, here, are basically objects that can use skills. So that's
     * pretty much how they're defined.
     *
     * \tparam T Base type for attributes. Integers are probably a good choice,
     *           at least for J-RPGs and tabletops.
     */
    template<typename T = long>
    class character : public object<T>
    {
        public:
            /**\brief Use global skill
             *
             * Uses a skill with a global effect.
             *
             * \param[in] skill The skill to use.
             *
             * \returns 'True' if the skill was used successfully, 'false' if it
             *          failed.
             */
            bool operator () (const std::string &skill)
            {
                return false;
            }

            /**\brief Use targeted skill
             *
             * Uses a skill that targets one or more other characters.
             *
             * \param[in] skill  The skill to use.
             * \param[in] target The skill's target.
             *
             * \returns 'True' if the skill was used successfully, 'false' if it
             *          failed.
             */
            std::string operator () (const std::string &skill, std::vector<character*> &pTarget)
            {
                auto act = action.find(skill);
                if (act != action.end())
                {
                    std::vector<object<T>*> source, target;
                    source.push_back(this);

                    for (auto &t : pTarget)
                    {
                        target.push_back(t);
                    }

                    return act->second(source, target);
                }
                
                return object<T>::name.display() + " looks bewildered\n";
            }

            /**\brief List of equipped items
             *
             * The list of items that a character currently has equipped.
             */
            std::vector<item<T>> equipment;

            /**\brief List of inventory items
             *
             * The list of items that a character currently has in their
             * inventory. These items are not equipped and should not have any
             * effect on the character's stats.
             */
            std::vector<item<T>> inventory;

            metaquest::action<T> &bind
                (const std::string &name,
                 bool isVisible,
                 std::function<std::string(std::vector<object<T>*>&,
                 std::vector<object<T>*>&)> pApply,
                 const enum metaquest::action<T>::scope &pScope = metaquest::action<T>::enemy)
            {
                metaquest::action<T> act(isVisible, pApply);
                act.name = metaquest::name::simple<>(name);
                act.scope = pScope;
                action[name] = act;
                return action[name];
            }

            std::vector<std::string> visibleActions (void)
            {
                std::vector<std::string> actions;

                for(auto a : action)
                {
                    if (a.second.visible)
                    {
                        actions.push_back(a.first);
                    }
                }

                return actions;
            }

            const enum metaquest::action<T>::scope scope (const std::string &act)
            {
                if (action.find(act) == action.end())
                {
                    return metaquest::action<T>::self;
                }
                else
                {
                    return action[act].scope;
                }
            }

        protected:
            std::map<std::string,action<T>> action;
    };
}

#endif
