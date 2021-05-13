#ifndef GAME_PONG_H_
#define GAME_PONG_H_

#include "game/game.h"

void pong(const bool player1_move_left, const bool player1_move_right,
  const bool player2_move_left, const bool player2_move_right,
  unsigned int &player1_score, unsigned int &player2_score,
  AxiStreamRGB &outStream);

#endif // end GAME_PONG_H_