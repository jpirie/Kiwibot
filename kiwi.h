#ifndef KIWI_H_
#define KIWI_H_

class Kiwi {
 public:
  Kiwi();

  void feed(int);
  void levelUp();
  void takeDamage(int);

  /* accessorrs to private data */
  int getHungerLevel();
  int getLevel();

 private:
  int level;
  int health;
  int hungerLevel;

};

#endif /* KIWIBOT_H */
