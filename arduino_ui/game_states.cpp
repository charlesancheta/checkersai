#include "game_states.h"

extern sharedVars shared;

// not really important rn
bool menuScreen() {
  int off = 100;
  shared.tft->fillRect(98 + off,98 + off, 109, 18, TFT_WHITE);
  shared.tft->setCursor(100 + off, 100 + off);
  shared.tft->setTextColor(TFT_BLACK); 
  shared.tft->setTextSize(2);
  shared.tft->println("RESETTING");
  screenPos t = touch::process();
  while (t.x == touch::untch) {
    t = touch::process();
  }
  shared.tft->setTextSize(1);
  shared.tft->setTextColor(TFT_WHITE);
  shared.tft->setCursor(0,0);
  return true;
}

// initialize game
void gameInit(bool start) {
  // namespace is only used when not using 
  // would lead to lesser readability
  using namespace c;
  MCUFRIEND_kbv *tft = shared.tft;
  tft->fillScreen(TFT_BLACK);
  // draw checkers board
  tft->fillRect(off_x, off_y, board_w, board_w, board_dark);
  // print the light tiles
  for (int8_t i = 0; i < 8; i += 2) {
    for (int8_t j = 0; j < 8; j += 2) {
      tft->fillRect(off_x + (i*board_sq), 
                    off_y + (j*board_sq), 
                    board_sq, board_sq, board_light);
      tft->fillRect(off_x + ((i+1)*board_sq), 
                    off_y + ((j+1)*board_sq), 
                    board_sq, board_sq, board_light);
    }
  }
  // initialize empty spaces on board
  for (int8_t i = 12; i < 20; i++) {
    shared.board[i] = EMPTY;
  }
  
  // places pieces on board
  for (int8_t i = 0; i < num_pieces; i++) {
    // bot pieces
    shared.gamePieces[i] = {1, BOT, false, i};
    // player pieces
    shared.gamePieces[i + num_pieces] = {0, PLAYER, false, i + 20};
    
    // change colour if player chose black
    if (start) {
      shared.gamePieces[i].colour = 0;
      shared.gamePieces[i+ num_pieces].colour = 1;
    }
    // draw pieces on board
    draw::piece(shared.gamePieces[i]);
    draw::piece(shared.gamePieces[i + num_pieces]);
    // place pieces in board array
    shared.board[i] = BOT;
    shared.board[i + 20] = PLAYER;
  }
  // dummy piece
  shared.gamePieces[dummy] = {0, EMPTY, false, -1}; 
}

void doTurn() {
  // will skip moves if the player must capture
  if (mustCapture()) {return;}
  // else just move
  selected pieceSel = NO_PIECE;
  moveSt moves;
  while(pieceSel != DONE) {
    chooseMove(pieceSel, moves);
  }
}

bool noPieces(tile side) {
  int start = (side == BOT) ? 0 : c::num_pieces;
  for (int i = start; i < start + c::num_pieces; i++) {
    if (shared.gamePieces[i].pos != -1) {
      return false;
    }
  }
  return true;
}

bool noMoves() {
  // determine start index
  int start = shared.pTurn ? c::num_pieces : 0;

  for (int i = start; i < start + c::num_pieces; i++) {
    Piece *piece = &shared.gamePieces[i];
    moveSt moves = {NOT, NOT, NOT, NOT};
    // check for valid moves or captures
    check::move(*piece, moves);
    check::capture(*piece, moves);
    check::backwards(*piece, moves);
    if (has::moves(moves) || has::captures(moves)) {
      // there are moves that can still be made
      return false;
    }
  }
  // no more moves
  return true;
}

// check for endgame conditions
win endCheck() {
  if (noPieces(PLAYER)){return BOTW;}
  if (noPieces(BOT)) {return PLAYERW;}
  if (noMoves()) {return DRAW;}
  // game continues
  return NONE;
}

void game(bool start) {
  // true is player's turn, bot is false 
  shared.pTurn = start;
  win state = NONE;
  // goes on until the end
  while (state == NONE) {
    doTurn();
    shared.pTurn = !shared.pTurn;
    state = endCheck();
  }
}