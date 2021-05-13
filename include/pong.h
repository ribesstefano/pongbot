#ifndef PONG_H_
#define PONG_H_

#include "game/game.h"

void pong(const bool player1_move_left, const bool player1_move_right,
  const bool player2_move_left, const bool player2_move_right,
  unsigned int &player1_score, unsigned int &player2_score,
  AxiStreamRGB &outStream);

#endif // end PONG_H_