/**\file
 * \brief Generic game flow
 *
 * Contains a generic game flow object.
 *
 * \copyright
 * Copyright (c) 2015, Magnus Achim Deininger <magnus@ef.gy>
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
 * \see Documentation: https://ef.gy/documentation/metaquest
 * \see Source Code: https://github.com/jyujin/metaquest
 * \see Licence Terms: https://github.com/jyujin/metaquest/COPYING
 */

#if !defined(METAQUEST_FLOW_GENERIC_H)
#define METAQUEST_FLOW_GENERIC_H

namespace metaquest {
namespace flow {
template <typename interaction, typename logic> class generic {
public:
  generic(void) : interact(), game(interact) { interact.clear(); }

  ~generic(void) { interact.clear(); }

  virtual bool run(void) {
    while (true) {
      interact.drawUI(game);

      switch (game.state()) {
      case logic::menu:
        interact.log(log = game.doMenu());
        break;
      case logic::combat:
        interact.log(log = game.doCombat());
        break;
      case logic::victory:
        interact.log(log = game.doVictory());
        break;
      case logic::defeat:
        interact.log(log = game.doDefeat());
        return true;
      case logic::exit:
        return true;
      default:
        return false;
      }
    }

    return true;
  }

  interaction interact;
  logic game;
  std::string log;
};
}
}

#endif
