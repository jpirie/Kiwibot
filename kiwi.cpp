/*
 * jpirie: WARNING: This is deprecated.
 *
 * Such functionality should now take place in the lua code.
 * This is just kept until it gets moved over or whatever
 */

/* constructor for Kiwi, sets bot attributes */

#include "kiwi.h"

/* constructor for the kiwi¸ initialise private class values */
Kiwi::Kiwi() {
  health = 100;
  hungerLevel = 5;
  level = 0;
}

/* feeds the kiwibot */
void Kiwi::feed(int value) {
  hungerLevel -= value;
}

/* an accessor for the hunger level */
int Kiwi::getHungerLevel() {
  return hungerLevel;
}

/* an accessor for the level variable */
int Kiwi::getLevel() {
  return level;
}

/* levels up the kiwi bot */
void Kiwi::levelUp() {
  level += 1;
}

/* gives the kiwibot some damage by decrementing the health */
void Kiwi::takeDamage(int value) {
  health -= value;
}
