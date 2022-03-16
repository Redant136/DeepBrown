#pragma once
#ifdef NDEBUG
#undef NDEBUG
#endif
// if the player can do illegal moves, uncomment
// #define noMoveCheck
#define utils_VectorObjects
// my own engine I built, is used here to render stuff.
#include <kNgine/kNgine.hpp>

#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdlib.h>

// im lazy so i have macros to print stuff
#define print(x) \
  std::cout << x << std::endl;
#define printv2(a) \
  std::cout << a.x << " " << a.y << std::endl;
#define printv3(a) \
  std::cout << a.x << " " << a.y << " " << a.z << std::endl;
#define printv4(a) \
  std::cout << a.x << " " << a.y << " " << a.z << " " << a.w << std::endl;

// foward declaration of objects
class board;
class EngineChessPiece;
class chessPiece;

static std::vector<iv2> pawnMoveFunc(board *bd, chessPiece *piece);
static std::vector<iv2> bishopMoveFunc(board *bd, chessPiece *piece);
static std::vector<iv2> knightMoveFunc(board *bd, chessPiece *piece);
static std::vector<iv2> rookMoveFunc(board *bd, chessPiece *piece);
static std::vector<iv2> queenMoveFunc(board *bd, chessPiece *piece);
static std::vector<iv2> kingMoveFunc(board *bd, chessPiece *piece);

// left to main to initialize
extern kNgine::SpriteMap *spriteMap;
extern board *chessBoard;
extern std::vector<EngineChessPiece *> enginePieces;

// piece type(pawn,bishop,etc)
enum pieceType
{
  P, //pawn
  B, //bishop
  N, //knight
  R, //rook
  Q, //queen
  K  //king
};
char getPieceCode(pieceType p)
{
  switch (p)
  {
  case pieceType::P:
    return 'p';
  case pieceType::B:
    return 'b';
  case pieceType::N:
    return 'n';
  case pieceType::R:
    return 'r';
  case pieceType::Q:
    return 'q';
  case pieceType::K:
    return 'k';
  default:
    return 0;
  }
}
// the score of each piece
unsigned int getPieceScore(pieceType p)
{
  switch (p)
  {
  case pieceType::P:
    return 1;
  case pieceType::B:
    return 3;
  case pieceType::N:
    return 3;
  case pieceType::R:
    return 5;
  case pieceType::Q:
    return 9;
  case pieceType::K:
    return 99999;
  default:
    return 0;
  }
}
// takes a string of 2 characters and returns a position
iv2 positionFromNotation(std::string n)
{
  assert(n.length() == 2);
  u8 x = 0;
  switch (n[0])
  {
  case 'a':
    x = 0;
    break;
  case 'b':
    x = 1;
    break;
  case 'c':
    x = 2;
    break;
  case 'd':
    x = 3;
    break;
  case 'e':
    x = 4;
    break;
  case 'f':
    x = 5;
    break;
  case 'g':
    x = 6;
    break;
  case 'h':
    x = 7;
    break;
  default:
    break;
  }
  return iv2(x, (n[1] - 48) - 1);
}

/**
 * a class representing a chess piece and contains all relevant information
 * contains all relevant data for it. 
*/
class chessPiece
{
public:
  // function pointer to get possible moves, allows to not have to extend class as c++ is more picky about it than java
  std::vector<iv2> (*possMoves)(board *, chessPiece *);
  // the type of piece this is
  pieceType type;
  // if the piece is white or not
  bool isWhite;
  // the position of the piece, a iv2 is a 2 element vector of integers
  iv2 pos = iv2(0, 0);
  // only used for castling
  bool hasMoved = false;
  // if the piece has been eaten
  bool eaten = false;
  chessPiece(pieceType type, bool isWhite, iv2 pos, std::vector<iv2> (*possMoves)(board *, chessPiece *))
  {
    this->possMoves = possMoves;
    this->type = type;
    this->isWhite = isWhite;
    this->pos = pos;
  }
  chessPiece(pieceType type, bool isWhite, iv2 pos, std::vector<iv2> (*possMoves)(board *, chessPiece *), bool hasMoved, bool eaten)
  {
    this->possMoves = possMoves;
    this->type = type;
    this->isWhite = isWhite;
    this->pos = pos;
    this->hasMoved = hasMoved;
    this->eaten = eaten;
  }
  std::vector<iv2> possibleMoves(board *b); // only declared and not defined because board has yet to be defined
};

/**
 * object passed to engine to render stuff on screen
 * 
*/
class EngineChessPiece : public kNgine::ComponentGameObject
{
public:
  // the piece it represents
  chessPiece *piece;
  // if the piece has been grabed
  bool grabed = false;
  EngineChessPiece(chessPiece *piece) : ComponentGameObject()
  {
    this->piece = piece;
    addComponent(new kNgine::SpriteReferenceComponent(this, spriteMap, piece->type + (piece->isWhite ? 0 : pieceType::K + 1)));
  }
  EngineChessPiece(const EngineChessPiece &base) : ComponentGameObject(base)
  {
    this->piece = base.piece;
  }
  // funtion to be called at each frame update, is defined later
  void update(std::vector<kNgine::msg> msgs);
};

// easy way to pass movement data
struct move_command
{
  enum
  {
    pPtr,
    pos_ini,
    pIndex
  } type;
  union
  {
    u32 index;
    chessPiece *p;
    iv2 iniPos;
  };
  iv2 nPos;
  bool operator==(const move_command &m)
  {
    return type == m.type && p == m.p && nPos == m.nPos;
  }
};

// board object that stores references to all pieces
class board
{
public:
  // the number of turns that have passed, count one turn each time a player move and not each 2 moves
  unsigned int turn = 0;
  // the pieces on the board
  std::vector<chessPiece *> pieces;
  // an array of the board, empty spots are 255 and when a piece is present, it's represented by it's index in the pieces array
  u8 boardPieceLocationIndex[64];
  // the player turn
  bool turnWhite = true;
  // variable to show green squares on the previous moves
  iv2 lastMovePos_ini = iv2(0, 0), lastMovePos_fin = iv2(0, 0);
  board()
  {
  }
  board(std::vector<chessPiece *> pieces)
  {
    this->pieces = pieces;
    for (u32 i = 0; i < 64; i++)
    {
      boardPieceLocationIndex[i] = 255;
    }
    for (u32 i = 0; i < pieces.size(); i++)
    {
      boardPieceLocationIndex[pieces[i]->pos.y * 8 + pieces[i]->pos.x] = i;
    }
  }
  board(std::string fenNotation)
  {
    for (int i = 0; i < 2; i++)
    {
      bool isWhite = i == 0;
      for (int i = 0; i < 8; i++) // pawn
      {
        chessPiece *piece = new chessPiece(pieceType::P, isWhite, iv2(-1, -1), pawnMoveFunc);
        pieces.push_back(piece);
      }
      { // bishop
        chessPiece *piece1 = new chessPiece(pieceType::B, isWhite, iv2(-1, -1), bishopMoveFunc);
        chessPiece *piece2 = new chessPiece(pieceType::B, isWhite, iv2(-1, -1), bishopMoveFunc);
        pieces.push_back(piece1);
        pieces.push_back(piece2);
      }
      { // knight
        chessPiece *piece1 = new chessPiece(pieceType::N, isWhite, iv2(-1, -1), knightMoveFunc);
        chessPiece *piece2 = new chessPiece(pieceType::N, isWhite, iv2(-1, -1), knightMoveFunc);
        pieces.push_back(piece1);
        pieces.push_back(piece2);
      }
      { //rook
        chessPiece *piece1 = new chessPiece(pieceType::R, isWhite, iv2(-1, -1), rookMoveFunc);
        chessPiece *piece2 = new chessPiece(pieceType::R, isWhite, iv2(-1, -1), rookMoveFunc);
        pieces.push_back(piece1);
        pieces.push_back(piece2);
      }
      { // queen
        chessPiece *piece = new chessPiece(pieceType::Q, isWhite, iv2(-1, -1), queenMoveFunc);
        pieces.push_back(piece);
      }
      { // king
        chessPiece *piece = new chessPiece(pieceType::K, isWhite, iv2(-1, -1), kingMoveFunc);
        pieces.push_back(piece);
      }
    }
    for (u32 i = 0; i < 64; i++)
    {
      boardPieceLocationIndex[i] = 255;
    }

    std::string fenCommands[6];
    u8 currentCommandIndex = 0;
    for (char c : fenNotation)
    {
      if (c == ' ')
      {
        currentCommandIndex++;
      }
      else
      {
        fenCommands[currentCommandIndex] += c;
      }
    }
    // fen arg 0
    {
      u32 numUsedPieces[2][7] = {{0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}};
      u32 currentX = 0;
      u32 currentY = 7;
      for (char c : fenCommands[0])
      {
        for (u32 i = 0; i <= pieceType::K; i++)
        {
          u32 pIndex = 0;
          bool isPiece = false;
          if (c == getPieceCode((pieceType)i))
          { // black
            isPiece = true;
            pIndex += 16;
            pIndex += numUsedPieces[1][i];
            numUsedPieces[1][i]++;
          }
          else if (c == getPieceCode((pieceType)i) - 32)
          { // white
            isPiece = true;
            pIndex += numUsedPieces[0][i];
            numUsedPieces[0][i]++;
          }
          if (isPiece)
          {
            if (i > pieceType::P)
            {
              pIndex += 8;
            }
            if (i > pieceType::B)
            {
              pIndex += 2;
            }
            if (i > pieceType::N)
            {
              pIndex += 2;
            }
            if (i > pieceType::R)
            {
              pIndex += 2;
            }
            if (i > pieceType::Q)
            {
              pIndex += 1;
            }
            boardPieceLocationIndex[currentY * 8 + currentX] = pIndex;
            pieces[pIndex]->pos = iv2(currentX, currentY);
            currentX++;
          }
        }
        if (c == '/')
        {
          currentY--;
          currentX = 0;
        }
        else if (c > 48 && c < 57)
        {
          currentX += c - 48;
        }
      }
    }
    for (u32 i = 0; i < pieces.size(); i++)
    {
      if (pieces[i]->pos == iv2(-1, -1))
      {
        pieces[i]->eaten = true;
      }
    }
    // arg 1
    turnWhite = fenCommands[1].c_str()[0] == 'w';
    // arg 2
    pieces[12]->hasMoved = true;      // rook q
    pieces[13]->hasMoved = true;      // rook k
    pieces[15]->hasMoved = true;      // king
    pieces[12 + 16]->hasMoved = true; // rook q
    pieces[13 + 16]->hasMoved = true; // rook k
    pieces[15 + 16]->hasMoved = true; // king
    for (char c : fenCommands[2])
    {
      if (c == 'K')
      {
        pieces[boardPieceLocationIndex[7 * 8 + 7]]->hasMoved = false;
        pieces[boardPieceLocationIndex[7 * 8 + 4]]->hasMoved = false;
      }
      else if (c == 'Q')
      {
        pieces[boardPieceLocationIndex[7 * 8 + 0]]->hasMoved = false;
        pieces[boardPieceLocationIndex[7 * 8 + 4]]->hasMoved = false;
      }
      else if (c == 'k')
      {
        pieces[boardPieceLocationIndex[0 * 8 + 7]]->hasMoved = false;
        pieces[boardPieceLocationIndex[0 * 8 + 4]]->hasMoved = false;
      }
      else if (c == 'q')
      {
        pieces[boardPieceLocationIndex[0 * 8 + 0]]->hasMoved = false;
        pieces[boardPieceLocationIndex[0 * 8 + 4]]->hasMoved = false;
      }
    }

    // arg 3
    // screw en passant

    // arg 4
    // halfmove advance clock not implemented

    // arg 5
    turn = (((u32)fenCommands[5].c_str()[0] - 48) - 1) * 2;
  }
  board(const board &base)
  {
    this->pieces = base.pieces;
    memcpy(this->boardPieceLocationIndex, base.boardPieceLocationIndex, sizeof(u8) * 64);
  }
  ~board()
  {
    for (int i = 0; i < pieces.size(); i++)
    {
      delete pieces[i];
    }
  }
  // return the piece at that position, if none, return NULL
  chessPiece *pieceAtPos(iv2 pos)
  {
    if (pos.x < 0 || pos.x > 7)
    {
      return NULL;
    }
    if (pos.y < 0 || pos.y > 7)
    {
      return NULL;
    }
    if (boardPieceLocationIndex[pos.y * 8 + pos.x] == 255)
    {
      return NULL;
    }

    return pieces[boardPieceLocationIndex[pos.y * 8 + pos.x]];
  }
  // returns the score of that board state
  int boardScore(bool isWhite)
  {
    int score = 0;
    for (u32 i = 0; i < pieces.size(); i++)
    {
      if (!pieces[i]->eaten)
      {
        if (pieces[i]->isWhite == isWhite)
        {
          score += getPieceScore(pieces[i]->type) * 1;
        }
        else
        {
          score += getPieceScore(pieces[i]->type) * -1;
        }
      }
      // if (pieces[i]->type == K && pieces[i]->eaten && pieces[i]->isWhite == isWhite)
      // {
      //   score += getPieceScore(pieces[i]->type) * -10;
      // }
    }
    return score;
  }
  // move piece at the new position
  void move(move_command c)
  {
    switch (c.type)
    {
    case move_command::pIndex:
      move(c.index, c.nPos);
      break;
    case move_command::pPtr:
      move(c.p, c.nPos);
      break;
    case move_command::pos_ini:
      move(pieceAtPos(c.iniPos), c.nPos);
      break;
    default:
      break;
    }
  }
  void move(unsigned int index, iv2 movement)
  {
    move(pieces[index], movement);
  }
  void move(chessPiece *piece, iv2 movement)
  {
    if (toIV2(piece->pos) != movement)
    {
      std::vector<iv2> moves = piece->possibleMoves(this);
      bool correct = false;
#ifndef noMoveCheck
      for (int i = 0; i < moves.size(); i++)
      {
        if (moves[i] == movement)
        {
          correct = true;
        }
      }
#else
      correct = true;
#endif
      if (correct)
      {
        turn++;
        piece->hasMoved = true;
        chessPiece *eat = pieceAtPos(movement);
        if (piece->type == pieceType::K && floor(piece->pos.x) == 4)
        {
          if (floor(movement.x) == 2)
          {
            chessPiece *rook = pieceAtPos(iv2(0, piece->pos.y));
            if (rook && rook->type == pieceType::R)
            {
              boardPieceLocationIndex[rook->pos.y * 8 + 3] = boardPieceLocationIndex[rook->pos.y * 8 + rook->pos.x];
              boardPieceLocationIndex[rook->pos.y * 8 + rook->pos.x] = 255;
              rook->pos.x = 3;
            }
            else
            {
              print("error");
              printBoard();
              throw "";
            }
          }
          else if (floor(movement.x) == 6)
          {
            chessPiece *rook = pieceAtPos(iv2(7, piece->pos.y));
            if (rook && rook->type == pieceType::R)
            {
              boardPieceLocationIndex[rook->pos.y * 8 + 5] = boardPieceLocationIndex[rook->pos.y * 8 + rook->pos.x];
              boardPieceLocationIndex[rook->pos.y * 8 + rook->pos.x] = 255;
              rook->pos.x = 5;
            }
            else
            {
              print("error");
              printBoard();
              throw "";
            }
          }
        }
        else if (piece->type == pieceType::P)
        {
          int lastLine = piece->isWhite ? 7 : 0;
          if (movement.y == lastLine)
          {
            piece->type = pieceType::Q;
            piece->possMoves = queenMoveFunc;
          }
          // check if en passant
          else if ((movement - piece->pos).x != 0)
          { // check if pawn eat
            chessPiece *atPos = pieceAtPos(lastMovePos_fin);
            if (atPos && atPos->type == pieceType::P &&
                (std::abs((lastMovePos_fin - lastMovePos_ini).y) == 2) &&
                (std::abs((lastMovePos_fin - piece->pos).x) == 1))
            {
              eat = atPos;
            }
          }
        }
        lastMovePos_ini = piece->pos;
        lastMovePos_fin = movement;
        if (eat)
        {
          eat->eaten = true;
          boardPieceLocationIndex[eat->pos.y * 8 + eat->pos.x] = 255;
        }
        boardPieceLocationIndex[movement.y * 8 + movement.x] = boardPieceLocationIndex[piece->pos.y * 8 + piece->pos.x];
        boardPieceLocationIndex[piece->pos.y * 8 + piece->pos.x] = 255;
        piece->pos = movement;
        turnWhite = !turnWhite;
      }
    }
  }
  // clones the board passed to it
  void clone(const board &base)
  {
    pieces = std::vector<chessPiece *>(base.pieces.size());
    for (int i = 0; i < base.pieces.size(); i++)
    {
      pieces[i] = new chessPiece(base.pieces[i]->type, base.pieces[i]->isWhite, base.pieces[i]->pos, base.pieces[i]->possMoves, base.pieces[i]->hasMoved, base.pieces[i]->eaten);
    }
    memcpy(this->boardPieceLocationIndex, base.boardPieceLocationIndex, sizeof(u8) * 64);
    this->turn = base.turn;
    this->turnWhite = base.turnWhite;
    this->lastMovePos_ini = base.lastMovePos_ini;
    this->lastMovePos_fin = base.lastMovePos_fin;
  }
  // print the board for debug purpose
  void printBoard()
  {
    for (int y = 7; y >= 0; y--)
    {
      for (int x = 0; x < 8; x++)
      {
        chessPiece *atPos = pieceAtPos(iv2(x, y));
        if (atPos)
        {
          if (atPos->isWhite)
          {
            std::cout << (char)(getPieceCode(atPos->type) - 32);
          }
          else
          {
            std::cout << getPieceCode(atPos->type);
          }
        }
        else
        {
          std::cout << " ";
        }
      }
      std::cout << std::endl;
    }
  }
  // similar to .equals() in java but better
  bool operator==(const board &b)
  {
    if (b.pieces.size() != pieces.size())
      return false;
    for (int i = 0; i < pieces.size(); i++)
    {
      if (pieces[i]->type != b.pieces[i]->type ||
          pieces[i]->isWhite != b.pieces[i]->isWhite ||
          pieces[i]->pos != b.pieces[i]->pos ||
          pieces[i]->hasMoved != b.pieces[i]->hasMoved ||
          pieces[i]->eaten != b.pieces[i]->eaten)
      {
        return false;
      }
    }
    return true;
  }
};

// object used by engine to render pieces and board
class chessCam : public kNgine::Camera
{
public:
  chessCam(float fov, int width, int height) : Camera(fov, width, height)
  {
  }
  // draw board
  void update(std::vector<kNgine::msg> msgs)
  {
    // call super function
    kNgine::Camera::update(msgs);
    // here is voodoo black magic stuff to make stuff fit the screen and not stretch
    u32 minWindowWidHei = std::min(kRenderer_getWindowWidth(), kRenderer_getWindowHeight());
    // the size of the board in range of -1 to 1
    v2 board_size = v2(minWindowWidHei, minWindowWidHei);
    board_size.x /= kRenderer_getWindowWidth();
    board_size.y /= kRenderer_getWindowHeight();
    // clear the screen and fill it with this color
    kRenderer_clear(v4(232.0 / 255, 204.0 / 255, 163.0 / 255, 1));
    for (int x = 0; x < 4; x++)
    {
      for (int y = 0; y < 8; y++)
      {
        // set color
        kRenderer_setDrawColor(v4(180.0 / 255, 136.0 / 255, 98.0 / 255, 1));
        // draw dark brown squares, if y is odd, offset x pos by one square
        kRenderer_drawRectV4(v4(
            x * board_size.x * 0.5 - 1 + board_size.x * 0.25 * ((y) % 2),
            y * board_size.y * 0.25 - (2 * board_size.y - 1),
            board_size.x * 0.25,
            board_size.y * 0.25));
      }
    }
    // set draw color to transparent green
    kRenderer_setDrawColor(v4(0, 150 / 255.0, 0, 75 / 255.0));
    // draw the last moves
    kRenderer_drawRectV4(v4(
                             chessBoard->lastMovePos_ini.x * board_size.x,
                             chessBoard->lastMovePos_ini.y * board_size.y,
                             board_size.x,
                             board_size.y) *
                             0.25 -
                         v4(1, 2 * board_size.y - 1, 0, 0));
    kRenderer_drawRectV4(v4(
                             chessBoard->lastMovePos_fin.x * board_size.x,
                             chessBoard->lastMovePos_fin.y * board_size.y,
                             board_size.x,
                             board_size.y) *
                             0.25 -
                         v4(1, 2 * board_size.y - 1, 0, 0));

    // make sure the camera is always in place during window resize
    v2 p = {posMapper.min.x + fov / 2, posMapper.max.y - fov / 2};
    p *= -1;
    p += v2(3.5, 3.5);
    position = p;
  }
};

// definition of possible moves
std::vector<iv2> chessPiece::possibleMoves(board *b)
{
  // had to define this latter as board was not defined
  std::vector<iv2> res = possMoves(b, this);
  return res;
}

// definition of update. takes care of grabbing piece and displaying possible moves
void EngineChessPiece::update(std::vector<kNgine::msg> msgs)
{
  // set the sprite the correct one even after promotion
  findComponent<kNgine::SpriteReferenceComponent>("[sprite]")->mapIndex = piece->type + (piece->isWhite ? 0 : pieceType::K + 1);
  // if the piece got eaten, disable the object
  if (piece->eaten)
  {
    kNgine::disableObject(this);
  }
  // if object is disabled, set it's position to -1,-1 just in case smt bad happens
  if (!this->enabled)
    this->position = {-1, -1};
  if (grabed)
  {
    // if the piece got grabbed, draw red squares showing all possible moves
    std::vector<iv2> moves = piece->possibleMoves(chessBoard);
    u32 minWindowWidHei = std::min(kRenderer_getWindowWidth(), kRenderer_getWindowHeight());
    v2 board_size = v2(minWindowWidHei, minWindowWidHei);
    board_size.x /= kRenderer_getWindowWidth();
    board_size.y /= kRenderer_getWindowHeight();
    kRenderer_setDrawColor(v4(255 / 255.0, 0, 0, 75 / 255.0));
    // another example of black magic being used here
    for (int i = 0; i < moves.size(); i++)
    {
      kRenderer_drawRectV4(v4(moves[i].x * board_size.x * 0.25 - 1, moves[i].y * board_size.y * 0.25 - (2 * board_size.y - 1), board_size.x * 0.25, board_size.y * 0.25));
    }
  }
  else
  {
    // if the piece isn't grabbed, reset it's position to the original one of the piece
    this->position.x = piece->pos.x;
    this->position.y = piece->pos.y;
  }
}

// user input and player
class phys_player : public kNgine::EngineObject
{
private:
  // if the user is grabbing a piece
  bool grabed = false;

public:
  board *b;
  bool color;
  phys_player(bool color, board *b)
  {
    this->color = color;
    this->b = b;
  }
  void update(std::vector<kNgine::msg> msgs)
  {
    // if it's your turn, only then do stuff
    if (b->turnWhite == color)
    {
      u32 minWindowWidHei = std::min(kRenderer_getWindowWidth(), kRenderer_getWindowHeight());
      v2 cursor;
      bool hasClick = false;
      for (u32 i = 0; i < msgs.size(); i++)
      {
        // get the cursor position
        if (msgs[i].msgType == kNgine::msg::CURSOR)
        {
          cursor = msgs[i].cursorPos;
        }
        else if (msgs[i].msgType == kNgine::msg::NONASCII_KEY)
        {
          // if click
          if (msgs[i].key & KeyBitmap(Key::MOUSE_LEFT))
          {
            hasClick = true;
            // this took me too much time than I am willing to admit
            iv2 newPos = iv2(
                std::floor((cursor.x + 1.0) * (kRenderer_getWindowWidth() / (f32)minWindowWidHei) * 4.0),
                std::floor((cursor.y - 1.0) * (kRenderer_getWindowHeight() / (f32)minWindowWidHei) * 4.0 + 8));
            // go through all the pieces and check position vs newPos
            for (int j = 0; j < b->pieces.size(); j++)
            {
              // if the user click on one of the same color, grab it if he hasn't grabbed one yet
              if (b->pieces[j]->isWhite == color && !b->pieces[j]->eaten && !grabed)
              {
                if (newPos == b->pieces[j]->pos)
                {
                  grabed = true;
                  enginePieces[j]->grabed = true;
                }
              }
              // if the piece is grabbed, set it's temporary position to the cursor's
              if (enginePieces[j]->grabed)
              {
                enginePieces[j]->position.x = newPos.x;
                enginePieces[j]->position.y = newPos.y;
              }
            }
          }
        }
      }
      if (!hasClick)
      {
        // if the user has stopped holding left click, drop any pieces that were grabbed
        for (int i = 0; i < enginePieces.size(); i++)
        {
          if (enginePieces[i]->grabed)
          {
            drop(i);
          }
          grabed = false;
        }
      }
    }
  }

  void drop(unsigned int index)
  {
    // ungrab piece
    enginePieces[index]->grabed = false;
    // try to move the piece to it's temporary position
    b->move(index, toIV2(enginePieces[index]->position));
    // play the chess tock sound
    kNgine::callEvent("chess_tock");
  }
};

// a namespace is simply used to avoid name clashing
namespace Book_Builder
{
  // an object containing move info
  // since a move is only 0-8 for x and y, I can multiply the y coodinate by 8 to hold it into one variable
  struct book_moves final
  {
    // a u8 is a single byte (less space in file) has more than enough values (7+7*8 < 255)
    u8 move_ini = 255;
    u8 move_fin = 255;
    // the number of times this move was present in the scanned games
    u8 weight = 0;
    // an array of the following moves, book_moves determines the type of the array and u8 the type of var used to store length
    Array<book_moves, u8> next;

    // frees the entire move tree
    void recursive_free()
    {
      if (next.arr)
      {
        for (u32 i = 0; i < next.length; i++)
        {
          next[i].recursive_free();
        }
        delete[] next.arr;
        next.arr = NULL;
        next.length = 0;
      }
    }

    // creates a perfect deep copy of the move tree
    book_moves clone()
    {
      book_moves b = {move_ini, move_fin, weight, {next.length, new book_moves[next.length]}};
      if (next.arr)
      {
        for (u32 i = 0; i < next.length; i++)
        {
          b.next[i] = next[i].clone();
        }
      }
      return b;
    }

    // returns a array of bytes
    std::vector<u8> getBytes()
    {
      u8 memberOffset = 3;
      std::vector<u8> r = std::vector<u8>(memberOffset + sizeof(next.length));
      r[0] = move_ini;
      r[1] = move_fin;
      r[2] = weight;
      u8 d[sizeof(next.length)];
      memcpy(d, &next.length, sizeof(next.length));
      for (u32 i = 0; i < sizeof(next.length); i++)
      {
        r[memberOffset + i] = d[i];
      }

      for (u32 i = 0; i < next.length; i++)
      {
        std::vector<u8> n = next[i].getBytes();
        for (u32 j = 0; j < n.size(); j++)
        {
          r.push_back(n[j]);
        }
      }
      return r;
    }
  }; // when writing to file: move+response+weight+length+arr (total size: 1+1+1+(1+size of the array))

  // got through a pointer to an array of bytes and convert it to a book_move tree
  static book_moves parseBook_moves(u8 **data)
  {
    book_moves r;
    // consume the first three bytes and store them into r
    r.move_ini = (*data)[0];
    r.move_fin = (*data)[1];
    r.weight = (*data)[2];
    *data += 3;
    // consume bytes equal to the length of the size in bytes of length
    memcpy(&r.next.length, *data, sizeof(r.next.length));
    *data += sizeof(r.next.length);
    // if the array's length was more than 0, create the array and parse its
    if (r.next.length > 0)
    {
      r.next.arr = new book_moves[r.next.length];
      for (u32 i = 0; i < r.next.length; i++)
      {
        // calling recursively as the already used bytes have already been removed
        r.next[i] = parseBook_moves(data);
      }
    }
    return r;
  }
  // read data from the book file and parse it
  static book_moves initBookMoves()
  {
    // data pointer
    char *data;
    // open the file in binary reading format as to not stop on /n i.e. 10
    std::ifstream bookFile("opening.book", std::ios_base::binary);
    if (bookFile.is_open())
    {
      // get the length of the file
      bookFile.seekg(0, std::ios::end);
      size_t length = bookFile.tellg();
      bookFile.seekg(0, std::ios::beg);

      // create an array of the length of the file
      data = new char[length];

      //read file
      bookFile.read(data, length);

      bookFile.close();
    }

    // make a copy of the pointer such that parseBook_moves does not modify it
    char *data_copy = data;
    book_moves book_root = parseBook_moves((u8 **)&data_copy);
    // delete the data
    delete[] data;
    return book_root;
  }
  // write move tree to the book file
  static void writeBookToFile(book_moves book_root)
  {
    std::ofstream book;
    // open the book
    book.open("opening.book", std::ios_base::binary);
    // get the data from the book_root
    std::vector<u8> data = book_root.getBytes();
    // write data
    book.write((char *)data.data(), data.size());
    book.close();
  }
  // parse pgn (portable game notation) to book move
  static book_moves parse_pgn(std::string moves, u32 maxDepth)
  {
    // function pointer to be called recursively
    std::function<void(book_moves *, u32)> setupGameBook_moves = [&setupGameBook_moves](book_moves *bm, u32 depth)
    {
      // set the values to default
      bm->move_ini = 255;
      bm->move_fin = 255;
      // if we go deeper, call this again but decrease depth by one
      if (depth > 1)
      {
        bm->next.length = 1;
        bm->next.arr = new book_moves[1];
        setupGameBook_moves(bm->next.arr, depth - 1);
      }
      else
      {
        bm->next.length = 0;
        bm->next.arr = NULL;
      }
    };
    // function to return the previous position of piece depending on pgn move format
    void (*parsePGNMove)(std::string, board *, bool, iv2 *) = [](std::string m, board *b, bool color, iv2 *storage)
    {
      // for some reason, some files have this weird non standard @ that not even lichess understands, if so just crash...
      assert(m.find("@") == -1);
      // if comment, crash
      assert(m.find("{") == -1);

      u8 x = 255, y = 255;
      assert(m[0] > 64); // see if first character is valid as number is invalid
      pieceType pieceCode = P;
      // if the first character dictates the piece, if ommited, it's a pawn
      if (m[0] < 91) // piece other than pawn
      {
        if (m[0] == 'O')
        {
          // castling
          iv2 startPos = iv2(4, color ? 0 : 7);
          storage[0] = startPos;
          if (m.length() > 3)
          {
            storage[1] = startPos + iv2(-2, 0);
          }
          else
          {
            storage[1] = startPos + iv2(2, 0);
          }
          return;
        }
        if (m[0] == 'N')
        {
          pieceCode = N;
        }
        else if (m[0] == 'B')
        {
          pieceCode = B;
        }
        else if (m[0] == 'R')
        {
          pieceCode = R;
        }
        else if (m[0] == 'Q')
        {
          pieceCode = Q;
        }
        else if (m[0] == 'K')
        {
          pieceCode = K;
        }
        m = m.substr(1);
      }

      // the index of the y coordinate
      u32 numberIndex = 1;
      for (u32 i = numberIndex; i < m.length(); i++)
      {
        if (m[i] < 58 && m[i] > 47)
        {
          numberIndex = i;
          break;
        }
      }

      // for some reason substr is different from java and takes pos then length, not start then end
      storage[1] = positionFromNotation(m.substr(numberIndex - 1, 2));
      m = m.substr(0, numberIndex - 1);

      // I despise exceptions, if a same piece can move to the same location they either put x or y coordinate
      if (m.length() > 0 && m[0] != 'x')
      {
        if (m[0] < 58)
        {
          std::string str = "a";
          str += m[0];
          y = positionFromNotation(str).y;
        }
        else
        {
          std::string str = "";
          str += m[0];
          x = positionFromNotation(str + "0").x;
        }
      }

      // finished parsing
      storage[0] = iv2(-1, -1);
      for (u32 i = 0; i < b->pieces.size(); i++)
      {
        if (b->pieces[i]->isWhite == color && b->pieces[i]->type == pieceCode && !b->pieces[i]->eaten && b->pieces[i]->pos != iv2(-1, -1))
        {
          for (iv2 v : b->pieces[i]->possibleMoves(b))
          {
            if (v == storage[1])
            {
              if (x == 255 && y == 255)
              {
                storage[0] = b->pieces[i]->pos;
              }
              else
              {
                if (b->pieces[i]->pos.x == x)
                {
                  storage[0] = b->pieces[i]->pos;
                }
                else if (b->pieces[i]->pos.y == y)
                {
                  storage[0] = b->pieces[i]->pos;
                }
              }
            }
          }
        }
      }
    };

    // the game where the pgn will be stored
    book_moves game;
    // setup the game depth and everything
    setupGameBook_moves(&game, maxDepth);

    // setup starting board
    board gameBoardState = board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // the current move, can be changed to point to always point to the next/current move. is similar to reference
    book_moves *current = &game;

    bool color = true;
    bool comment = false;
    while (moves.length() > 0)
    {
      // the the move str
      std::string m = moves.substr(0, moves.find(" "));
      // remove the move from str
      moves = moves.substr(moves.find(" ") + 1);
      if (!comment)
      {
        // if we are not inside a comment and there is a '{', turn on comments
        if (m.find("{") != -1)
        {
          comment = m.find("}") == -1;
        }
      }
      // if the first character is not a number ie(1. or 2. but e5) and is not a comment
      if (m[0] > 57 && !comment)
      {
        // storage for the initial position of the piece and the position after the move
        iv2 storage[2];
        parsePGNMove(m, &gameBoardState, color, storage);
        // if the game is finished/error, return
        if (storage[0] == iv2(-1, -1))
        {
          return game;
        }
        // store the stuff
        current->move_ini = storage[0].x + storage[0].y * 8;
        current->move_fin = storage[1].x + storage[1].y * 8;
        // update the board
        gameBoardState.move(gameBoardState.pieceAtPos(storage[0]), storage[1]);
        // loop until the array is not NULL
        if (current->next.arr)
        {
          // make current the next move
          current = current->next.arr;
          color = !color;
        }
        else
        {
          break;
        }
      }
      // if comments enabled and '}', disable comments
      if (comment)
      {
        if (m.find("}") != -1)
        {
          comment = false;
        }
      }
    }
    // reset the game pointer
    current = &game;
    // if a game has ended before the set length, delete the rest after it
    while (current->next.length > 0 && current->next.arr)
    {
      if (current->move_ini == 255 || current->move_fin == 255)
      {
        current->recursive_free();
        break;
      }
      current = &current->next[0];
    }
    return game;
  }
  // setup the oppening file
  static void setupOpening(u32 maxDepth = 16)
  {
    // just helper function, not from stack overflow
    auto ReplaceString = [](std::string subject, const std::string &search,
                            const std::string &replace)
    {
      size_t pos = 0;
      while ((pos = subject.find(search, pos)) != std::string::npos)
      {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
      }
      return subject;
    };

    // list of all files contaning games
    std::string database[] = {
        "chess_games_dataset/master_games.pgn",
        "chess_games_dataset/master_games (1).pgn",
        "chess_games_dataset/master_games (2).pgn",
        "chess_games_dataset/master_games (3).pgn",
        "chess_games_dataset/master_games (4).pgn",
        "chess_games_dataset/master_games (5).pgn",
        "chess_games_dataset/Modern.pgn",

        // this dataset has illegal moves in it, wtf
        // "chess_games_dataset/ficsgamesdb_search_198080/ficsgamesdb_search_198080.pgn",

        "chess_games_dataset/ficsgamesdb_search_197741/ficsgamesdb_search_197741.pgn"};
    std::vector<book_moves> allGames = std::vector<book_moves>();
    // loop through all files in database
    for (u32 i = 0; i < sizeof(database) / sizeof(database[i]); i++)
    {
      std::string line;
      std::ifstream bookFile(database[i]);
      if (bookFile.is_open())
      {
        // a game from the file
        std::string game = "";
        bool lastNewLine = false;
        while (getline(bookFile, line))
        {
          // because windows and mac have different line endings, I have to cover both
          if (!(line[0] == '[' || line[0] == '\n' || line[0] == '\r' || line == ""))
          {
            game += line + " ";
          }
          if (line == "" || line[0] == '\n' || line[0] == '\r')
          {
            // pgn format has two new lines between each game
            if (lastNewLine)
            {
              game = ReplaceString(game, ". ", ".");
              game = ReplaceString(game, ".", ". ");
              // if no weird @
              if (game.find("@") == -1)
              {
                book_moves g = parse_pgn(game, maxDepth);
                // if game has not completly screwed up and crashed after the first move
                if (g.move_ini != 255 && g.move_fin != 255)
                {
                  allGames.push_back(g);
                }
              }
              game = "";
            }
            lastNewLine = true;
          }
          else
          {
            lastNewLine = false;
          }
        }
        bookFile.close();
      }
      else
      {
        print("could not open file" << database[i]);
      }
    }
    // root of the whole book
    book_moves root = {0, 0, 0, {0, NULL}};

    // array
    Array<book_moves> gameTurn;
    gameTurn.length = allGames.size();
    gameTurn.arr = allGames.data();

    // setup book
    std::function<void(book_moves *, Array<book_moves>)> setupBook = [&setupBook](book_moves *move, Array<book_moves> gameList)
    {
      // array of all moves that are the same
      std::vector<book_moves> similarMoves = std::vector<book_moves>(0);
      for (u32 i = 0; i < gameList.length; i++)
      {
        bool included = false;
        for (u32 j = 0; j < similarMoves.size(); j++)
        {
          if (gameList[i].move_ini == similarMoves[j].move_ini && gameList[i].move_fin == similarMoves[j].move_fin)
          {
            included = true;
            similarMoves[j].weight++;
            break;
          }
        }
        if (!included)
        {
          similarMoves.push_back(gameList[i]);
          similarMoves[similarMoves.size() - 1].weight = 1;
        }
      }
      // setup the next moves array
      move->next.length = similarMoves.size();
      move->next.arr = new book_moves[similarMoves.size()];

      for (u32 i = 0; i < similarMoves.size(); i++)
      {
        move->next[i] = similarMoves[i];
        // setup next turn array of all the games
        Array<book_moves> nextTurn = {0, new book_moves[gameList.length]};
        for (u32 j = 0; j < gameList.length; j++)
        {
          // if similar move, add it to the next turn array
          if (similarMoves[i].move_ini == gameList[j].move_ini && similarMoves[i].move_fin == gameList[j].move_fin)
          {
            if (gameList[j].next.length > 0)
            {
              nextTurn.length++;
              nextTurn[nextTurn.length - 1] = gameList[j].next[0];
            }
          }
        }
        // recursive fun
        setupBook(&move->next[i], nextTurn);
        nextTurn.free();
      }
    };
    // setup book
    setupBook(&root, gameTurn);

    {
      // just to free memory as I allocate more than necessary while combining all games
      book_moves temp = root.clone();
      root.recursive_free();
      root = temp;
    }

    print("finished parsing, now printing to file");

    // write book to file, yes very useful comment I know
    writeBookToFile(root);

    // free the array
    root.recursive_free();
    // ------------------

    // had idea bot generated moves itself, but too many logistical prob with it.
    // Such as the algorithm discarding moves that were good but not as good as another one
    // also, moves that only rewarded you late game weren't considered

    // --------------------

    // bot_player pl1 = bot_player(*this);
    // bot_player pl2 = bot_player(*this);
    // pl1.precision += 3;
    // pl2.precision += 3;
    // board b = board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // while (b.turn < maxDepth && abs(b.boardScore(true) < 100))
    // {
    //   pl1.update(std::vector<kNgine::msg>());
    //   pl2.update(std::vector<kNgine::msg>());
    // }
  }
};

// bot player
class bot_player : public kNgine::EngineObject
{
protected:
  // Chess opening vars
  // --------------------------------------
  // yes, im very lazy and use a typedef here to be able to ommit out the "Book_builder::"
  typedef Book_Builder::book_moves book_moves;
  book_moves book_root;
  book_moves *currentMove;
  book_moves *getOpeningMove(iv2 lastMove_ini, iv2 lastMove_fin)
  {
    // the weight of weight, yes
    const f32 weightW = 1.0;
    // the weight of the number of moves afterwards
    const f32 lengthW = -0.3;
    // if first ever move of the game
    if (currentMove->move_ini == 0 && currentMove->move_fin == 0 && lastMove_ini == lastMove_fin)
    {
      // weighted random code
      f32 totalOptions = 0;
      for (u32 i = 0; i < currentMove->next[i].next.length; i++)
      {
        totalOptions += currentMove->next[i].weight * weightW;
        totalOptions += currentMove->next[i].next.length * lengthW;
      }
      i32 index = -1;
      f32 randIndex = randf();
      f32 currentIndex = 0;
      for (u32 i = 0; i < currentMove->next.length; i++)
      {
        currentIndex += (currentMove->next[i].weight * weightW + currentMove->next[i].next.length * lengthW) / (f32)totalOptions;
        if (randIndex < currentIndex)
        {
          index = i;
          break;
        }
      }
      return &currentMove->next[index];
    }
    for (u32 i = 0; i < currentMove->next.length; i++)
    {
      if (currentMove->next[i].move_ini == lastMove_ini.x + lastMove_ini.y * 8 && currentMove->next[i].move_fin == lastMove_fin.x + lastMove_fin.y * 8)
      {
        if (currentMove->next[i].next.length > 0)
        {
          // if the move has a next, do a weighted random selection
          f32 totalOptions = 0;
          for (u32 j = 0; j < currentMove->next[i].next.length; j++)
          {
            totalOptions += currentMove->next[i].next[j].weight * weightW;
            totalOptions += currentMove->next[i].next[j].next.length * lengthW;
          }
          i32 index = -1;
          f32 randIndex = randf();
          f32 currentIndex = 0;
          for (u32 j = 0; j < currentMove->next[i].next.length; j++)
          {
            currentIndex += (currentMove->next[i].next[j].weight * weightW + currentMove->next[i].next[j].next.length * lengthW) / (f32)totalOptions;
            if (randIndex < currentIndex)
            {
              index = j;
              break;
            }
          }
          assert(index != -1);
          return &currentMove->next[i].next[index];
        }
        else
        {
          return NULL;
        }
      }
    }
    return NULL;
  }
  // ------------------------------------------

  // DEBUG
  // -------------------------------------------
  // is used to count the number of possible board states that have been searched
  u64 totalMoves;
  // ------------------------------------------

  // ASYNC
  // -----------------------------------------
  // there are 3 states while it's the bot turn (board.turn==this->color):
  //        start search(done=true,threadMove={-1,-1}),
  //        searching(done=false,threadMove!={any,any}),
  //        finished(done=true,threadMove!={-1,-1})
  f64 chronoElapsedTime = 0;
  move_command threadMove = {move_command::pIndex, 0, iv2(-1, -1)};
  // if the thread has finished and reached a result
  bool done = true;
  move_command lastMove = {move_command::pIndex, 0, iv2(0, 0)};
  u32 repeatedMoves = 0;
  // recusivly called func to decide best move. uses minimax algorithm with alpha beta prunning
  virtual f32 recusiveMove(bool color, int iterationsLeft, board *b1, move_command *storage,
                           int alpha = INT_MIN, int beta = INT_MAX)
  {
    if (done)
    {
      return 0;
    }
    if (currentMove && iterationsLeft == precision)
    {
      currentMove = getOpeningMove(b1->lastMovePos_ini, b1->lastMovePos_fin);
      if (currentMove)
      {
        iv2 iniPos = iv2(currentMove->move_ini % 8, currentMove->move_ini / 8);
        iv2 finPos = iv2(currentMove->move_fin % 8, currentMove->move_fin / 8);
        storage->type = move_command::pos_ini;
        storage->iniPos = iniPos;
        storage->nPos = finPos;
        // b1->move(b1->pieceAtPos(iniPos), finPos);
        return 0;
      }
      else
      {
        b1->printBoard();
        print("out of openings");
      }
    }

    float numMovesW = 0.f;
    if (iterationsLeft <= 0)
    {
      totalMoves++;
      return boardScore(b1);
    }
    std::vector<iv3> moves;
    for (int i = 0; i < b1->pieces.size(); i++)
    {
      if (b1->pieces[i]->isWhite == color && !b1->pieces[i]->eaten)
      {
        std::vector<iv2> pMoves = b1->pieces[i]->possibleMoves(b1);
        for (iv2 m : pMoves)
        {
          moves.push_back(iv3(m.x, m.y, i));
        }
      }
    }

    int res = (color == this->color) ? INT_MIN : INT_MAX;
    int index = -1;
    for (int i = 0; i < moves.size(); i++)
    {
      f32 score;
      board nBoard = board();
      nBoard.clone(*b1);
      move_command move = {move_command::pIndex, (u32)moves[i].z, toIV2(moves[i])};
      nBoard.move(move);
      bool kingDead = false;
      score = recusiveMove(!color, iterationsLeft - 1, &nBoard, storage, alpha, beta);
      // to disuade repetitive moves
      if (move == lastMove && iterationsLeft == precision)
      {
        score -= 2 * (++repeatedMoves) * 2;
      }
      if (color == this->color)
      {
        if (res < score)
        {
          res = score;
          index = i;
        }
        alpha = std::max(alpha, res);
      }
      else
      {
        if (res > score)
        {
          res = score;
          index = i;
        }
        beta = std::min(beta, res);
      }

      if (beta <= alpha)
      {
        break;
      }
    }
    if (iterationsLeft == precision && !done)
    {
      if (index == -1)
      {
        print("index -1");
        b1->printBoard();
        // b1->pieces[15+((this->color)?0:16)]->eaten = true;
        // b1->turnWhite=!b1->turnWhite;
        assert(index != -1);
        assert(0);
      }
      storage->type = move_command::pIndex;
      storage->index = moves[index].z;
      storage->nPos = toIV2(moves[index]);
      // b1->move(moves[index].z, iv2(moves[index].x, moves[index].y));
      //this->positionScores[b1->pieces[moves[index].z]->type][(u32)(moves[index].y * 8 + moves[index].x)] += (this->positionScores[b1->pieces[moves[index].z]->type][(u32)(moves[index].y * 8 + moves[index].x)] * -0.2 - 0.01);
    }
    return res + moves.size() * numMovesW;
  }
  // --------------------------------------------

public:
  // the score each piece has on it's position on the board
  f32 positionScores[6][64];
  // if I start new thread
  bool asyncMove = true;
  // the board
  board *b;
  // color
  bool color;
  // set time each move should take
  u32 secPerMove = 10;

  // the depth the search will go up to, initial value not important any more
  u32 precision = 4;
  bot_player(bool color, board *b)
  {
    this->color = color;
    this->b = b;
    book_root = Book_Builder::initBookMoves();
    currentMove = &book_root;

    f32 pawn_pS[] = {
        7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5,
        0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0,
        0.5, -0.5, -0.5, 0.0, 0.0, -0.5, -0.5, 0.5,
        0.5, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 0.5,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    f32 bishop_pS[] = {
        -2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0,
        -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0,
        -1.0, 0.0, 0.5, 1.0, 1.0, 0.5, 0.0, -1.0,
        -1.0, 0.5, 0.5, 1.0, 1.0, 0.5, 0.5, -1.0,
        -1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, -1.0,
        -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0,
        -1.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, -1.0,
        -2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0};
    f32 knight_pS[] = {
        -4.9, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -4.9,
        -4.0, -2.0, 0.0, 0.0, 0.0, 0.0, -2.0, -4.0,
        -3.0, 0.0, 1.0, 1.5, 1.5, 1.0, 0.0, -3.0,
        -3.0, 0.5, 1.5, 2.0, 2.0, 1.5, 0.5, -3.0,
        -3.0, 0.0, 1.5, 2.0, 2.0, 1.5, 0.0, -3.0,
        -2.0, 0.5, 1.5, 1.5, 1.5, 1.0, 0.5, -2.0,
        -4.0, -2.0, 0.0, 0.0, 0.0, 0.0, -2.0, -4.0,
        -4.9, -4.0, -3.0, -3.0, -3.0, -3.0, -3.0, -4.9};
    f32 rook_pS[] = {
        0.5, 0.5, 0.5, 0.0, 0.0, 0.5, 0.5, 0.5,
        0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5,
        -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
        -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
        -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
        -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
        -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
        0.4, -1.5, 0.0, 0.5, 0.5, 0.5, -1.5, 0.4};
    f32 queen_pS[] = {
        -2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0,
        -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0,
        -1.0, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -1.0,
        -0.5, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -0.5,
        0.0, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -0.5,
        -1.0, 0.5, 0.5, 0.5, 0.5, 0.0, 0.0, -1.0,
        -1.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, -1.0,
        -2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0};
    f32 king_pS[] = {
        -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
        -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
        -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
        -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
        -2.0, -3.0, -3.0, -4.0, -4.0, -3.0, -3.0, -2.0,
        -1.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -1.0,
        2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 2.0,
        2.0, 2.0, 2.5, -1.0, 0.0, -3.0, 3.0, 2.0};

    if (!color)
    {
      memcpy(positionScores[0], pawn_pS, sizeof(f32) * 64);
      memcpy(positionScores[1], bishop_pS, sizeof(f32) * 64);
      memcpy(positionScores[2], knight_pS, sizeof(f32) * 64);
      memcpy(positionScores[3], rook_pS, sizeof(f32) * 64);
      memcpy(positionScores[4], queen_pS, sizeof(f32) * 64);
      memcpy(positionScores[5], king_pS, sizeof(f32) * 64);
    }
    else
    {
      for (u32 y = 0; y < 8; y++)
      {
        for (u32 x = 0; x < 8; x++)
        {
          positionScores[0][y * 8 + x] = pawn_pS[(7 - y) * 8 + x];
          positionScores[1][y * 8 + x] = bishop_pS[(7 - y) * 8 + x];
          positionScores[2][y * 8 + x] = knight_pS[(7 - y) * 8 + x];
          positionScores[3][y * 8 + x] = rook_pS[(7 - y) * 8 + x];
          positionScores[4][y * 8 + x] = queen_pS[(7 - y) * 8 + x];
          positionScores[5][y * 8 + x] = king_pS[(7 - y) * 8 + x];
        }
      }
    }

    for (u32 i = 0; i < 6; i++)
    {
      for (u32 j = 0; j < 64; j++)
      {
        if (positionScores[i][j] < -(1 << 8) || positionScores[i][j] > 1 << 8)
        {
          print(color);
          print(i << " " << j);
          print(positionScores[i][j]);
          assert(0);
        }
      }
    }
  }
  void update(std::vector<kNgine::msg> msgs)
  {
    if (b->turnWhite == color)
    {
      if (asyncMove)
      {
        if (done && threadMove.nPos == iv2(-1, -1)) // start search
        {
          done = false;
          u32 seed = rand();
          // starting new thread resets the seed, why. took me like 5h to figure out this problem
          kThreadDetach([this, seed](void *arg)
                        {
                          srand(seed);
                          totalMoves = 0;
                          // keep deeper searching until time runs out
                          for (u32 d = 3; d < 10 && !done && (!currentMove || d == 3); d++)
                          {
                            board bClone = board();
                            bClone.clone(*b);
                            precision = d;
                            recusiveMove(color, precision, &bClone, &threadMove);
                          }
                          done = true;
                        },
                        NULL);
        }
        else if (!done) // searching
        {
          for (u32 i = 0; i < msgs.size(); i++)
          {
            if (msgs[i].msgType == kNgine::msg::TIME_ELAPSED)
            {
              // add time to time elapsed
              chronoElapsedTime += msgs[i].time;
            }
          }
          // if the time limit was reached, end the search
          if (chronoElapsedTime > secPerMove || currentMove && threadMove.nPos != iv2(-1, -1))
          {
            done = true;
            chronoElapsedTime = 0;
          }
        }
        else if (done) // completed search
        {
          chronoElapsedTime = 0;
          b->move(threadMove);
          lastMove = threadMove;
          kNgine::callEvent("chess_tock");
          threadMove = {move_command::pIndex, 0, iv2(-1, -1)};
        }
      }
      else
      {
        move_command m;
        recusiveMove(color, precision, b, &m);
        b->move(m);
        done = true;
      }
    }
  }
  // score funtion the bot uses to rate a state of the board
  virtual float boardScore(board *b)
  {
    return b->boardScore(this->color) + positionScore(b);
  }
  virtual float positionScore(board *b)
  {
    f64 total = 0;
    for (u32 i = 0; i < b->pieces.size(); i++)
    {
      if (!b->pieces[i]->eaten && isnormal(this->positionScores[b->pieces[i]->type][b->pieces[i]->pos.y * 8 + b->pieces[i]->pos.x]))
      {
        if (b->pieces[i]->isWhite == color)
        {
          total += this->positionScores[b->pieces[i]->type][b->pieces[i]->pos.y * 8 + b->pieces[i]->pos.x];
          // this->positionScores[b->pieces[i]->type][b->pieces[i]->pos.y * 8 + b->pieces[i]->pos.x]*=0.9;
        }
      }
    }
    return total;
  }
};

// declaration of all pieces move functions
static std::vector<iv2> pawnMoveFunc(board *bd, chessPiece *piece)
{
  // if the piece has been eaten
  if (piece->eaten || piece->pos.x < 0 || piece->pos.y < 0)
  {
    return std::vector<iv2>();
  }
  // direction of foward
  int dir = piece->isWhite ? 1 : -1;
  // list of all moves
  std::vector<iv2> moves;
  // check if theres a piece in front
  chessPiece *atPos = bd->pieceAtPos(piece->pos + iv2(0, dir));
  if (!atPos)
  {
    moves.push_back(piece->pos + iv2(0, dir));
    // if the piece can move 2 squares
    if (!piece->hasMoved)
    {
      // if theres a piece 2 squares ahead
      atPos = bd->pieceAtPos(piece->pos + iv2(0, dir * 2));
      if (!atPos)
      {
        moves.push_back(piece->pos + iv2(0, dir * 2));
      }
    }
  }
  // en passant
  if (piece->pos.y + dir * 3 == 0 || piece->pos.y + dir * 3 == 7)
  {
    // check if last move was pawn passing beside this with double move foward
    // yes, this is very ugly
    if (bd->lastMovePos_fin.y == piece->pos.y && bd->pieceAtPos(bd->lastMovePos_fin) &&
        (bd->pieceAtPos(bd->lastMovePos_fin)->type == pieceType::P) &&
        (bd->lastMovePos_ini.y == piece->pos.y + dir * 2))
    {
      if (bd->lastMovePos_ini.x == piece->pos.x + 1)
      {
        moves.push_back(piece->pos + iv2(1, dir));
      }
      if (bd->lastMovePos_ini.x == piece->pos.x - 1)
      {
        moves.push_back(piece->pos + iv2(-1, dir));
      }
    }
  }
  // eating move right
  atPos = bd->pieceAtPos(piece->pos + iv2(1, dir));
  if (atPos && atPos->isWhite != piece->isWhite)
  {
    moves.push_back(piece->pos + iv2(1, dir));
  }
  // eating move left
  atPos = bd->pieceAtPos(piece->pos + iv2(-1, dir));
  if (atPos && atPos->isWhite != piece->isWhite)
  {
    moves.push_back(piece->pos + iv2(-1, dir));
  }
  return moves;
}
// all the other moves work in a similar way as bishop where you check vel and if there's a piece in your path
static std::vector<iv2> bishopMoveFunc(board *bd, chessPiece *piece)
{
  // if the piece has been eaten
  if (piece->eaten || piece->pos.x < 0 || piece->pos.y < 0)
  {
    return std::vector<iv2>();
  }

  std::vector<iv2> move;
  // check all 4 directions
  for (int dir = 0; dir < 4; dir++)
  {
    // check at each distance
    for (int distance = 1; distance < 8; distance++)
    {
      // the vel of piece, ie pos+(1,1), pos+(2,2), pos+(-1,-1)...
      iv2 vel = iv2(0, 0);
      switch (dir)
      {
      case 0:
        vel.x = 1;
        vel.y = 1;
        break;
      case 1:
        vel.x = 1;
        vel.y = -1;
        break;
      case 2:
        vel.x = -1;
        vel.y = -1;
        break;
      case 3:
        vel.x = -1;
        vel.y = 1;
        break;
      default:
        break;
      }
      vel *= distance;
      // if the position is out of bounds, break from direction loop
      if ((piece->pos + vel).x < 0 || (piece->pos + vel).y < 0 || (piece->pos + vel).x > 7 || (piece->pos + vel).y > 7)
      {
        break;
      }
      // if piece blocks path, check if you can eat it then break direction loop
      chessPiece *atPos = bd->pieceAtPos(piece->pos + vel);
      if (atPos)
      {
        if (atPos->isWhite != piece->isWhite)
        {
          move.push_back(piece->pos + vel);
          break;
        }
        else
        {
          break;
        }
      }
      else
      {
        move.push_back(piece->pos + vel);
      }
    }
  }
  return move;
}
static std::vector<iv2> knightMoveFunc(board *bd, chessPiece *piece)
{
  // if the piece has been eaten
  if (piece->eaten || piece->pos.x < 0 || piece->pos.y < 0)
  {
    return std::vector<iv2>();
  }

  std::vector<iv2> move;
  for (int dir = 0; dir < 8; dir++)
  {
    iv2 vel = iv2(0, 0);
    switch (dir)
    {
    case 0:
      vel = {2, 1};
      break;
    case 1:
      vel = {2, -1};
      break;
    case 2:
      vel = {1, 2};
      break;
    case 3:
      vel = {1, -2};
      break;
    case 4:
      vel = {-2, 1};
      break;
    case 5:
      vel = {-2, -1};
      break;
    case 6:
      vel = {-1, 2};
      break;
    case 7:
      vel = {-1, -2};
      break;
    default:
      break;
    }
    if (!((piece->pos + vel).x < 0 || (piece->pos + vel).y < 0 || (piece->pos + vel).x > 7 || (piece->pos + vel).y > 7))
    {
      chessPiece *atPos = bd->pieceAtPos(piece->pos + vel);
      if (atPos)
      {
        if (atPos->isWhite != piece->isWhite)
        {
          move.push_back(piece->pos + vel);
        }
      }
      else
      {
        move.push_back(piece->pos + vel);
      }
    }
  }
  return move;
}
static std::vector<iv2> rookMoveFunc(board *bd, chessPiece *piece)
{
  if (piece->eaten || piece->pos.x < 0 || piece->pos.y < 0)
  {
    return std::vector<iv2>();
  }

  std::vector<iv2> move;
  for (int dir = 0; dir < 4; dir++)
  {
    for (int distance = 1; distance < 8; distance++)
    {
      iv2 vel = iv2(0, 0);
      switch (dir)
      {
      case 0:
        vel.x = 1;
        break;
      case 1:
        vel.x = -1;
        break;
      case 2:
        vel.y = 1;
        break;
      case 3:
        vel.y = -1;
        break;
      default:
        break;
      }
      vel *= distance;
      if ((piece->pos + vel).x < 0 || (piece->pos + vel).y < 0 || (piece->pos + vel).x > 7 || (piece->pos + vel).y > 7)
      {
        break;
      }
      chessPiece *atPos = bd->pieceAtPos(piece->pos + vel);
      if (atPos)
      {
        if (atPos->isWhite != piece->isWhite)
        {
          move.push_back(piece->pos + vel);
          break;
        }
        else
        {
          break;
        }
      }
      else
      {
        move.push_back(piece->pos + vel);
      }
    }
  }
  return move;
}
static std::vector<iv2> queenMoveFunc(board *bd, chessPiece *piece)
{
  if (piece->eaten || piece->pos.x < 0 || piece->pos.y < 0)
  {
    return std::vector<iv2>();
  }

  std::vector<iv2> move;
  for (int dir = 0; dir < 8; dir++)
  {
    for (int distance = 1; distance < 8; distance++)
    {
      iv2 vel = iv2(0, 0);
      switch (dir)
      {
      case 0:
        vel = {1, 0};
        break;
      case 1:
        vel = {1, 1};
        break;
      case 2:
        vel = {0, 1};
        break;
      case 3:
        vel = {-1, 1};
        break;
      case 4:
        vel = {-1, 0};
        break;
      case 5:
        vel = {-1, -1};
        break;
      case 6:
        vel = {0, -1};
        break;
      case 7:
        vel = {1, -1};
        break;
      default:
        break;
      }
      vel *= distance;
      if ((piece->pos + vel).x < 0 || (piece->pos + vel).y < 0 || (piece->pos + vel).x > 7 || (piece->pos + vel).y > 7)
      {
        break;
      }
      chessPiece *atPos = bd->pieceAtPos(piece->pos + vel);
      if (atPos)
      {
        if (atPos->isWhite != piece->isWhite)
        {
          move.push_back(piece->pos + vel);
          break;
        }
        else
        {
          break;
        }
      }
      else
      {
        move.push_back(piece->pos + vel);
      }
    }
  }
  return move;
}
// castling was very hard
static std::vector<iv2> kingMoveFunc(board *bd, chessPiece *piece)
{
  if (piece->eaten || piece->pos.x < 0 || piece->pos.y < 0)
  {
    return std::vector<iv2>();
  }

  std::vector<iv2> move;
  for (int dir = 0; dir < 8; dir++)
  {
    for (int distance = 1; distance < 2; distance++)
    {
      iv2 vel = iv2(0, 0);
      switch (dir)
      {
      case 0:
        vel = {1, 0};
        break;
      case 1:
        vel = {1, 1};
        break;
      case 2:
        vel = {0, 1};
        break;
      case 3:
        vel = {-1, 1};
        break;
      case 4:
        vel = {-1, 0};
        break;
      case 5:
        vel = {-1, -1};
        break;
      case 6:
        vel = {0, -1};
        break;
      case 7:
        vel = {1, -1};
        break;
      default:
        break;
      }
      vel *= distance;
      if ((piece->pos + vel).x < 0 || (piece->pos + vel).y < 0 || (piece->pos + vel).x > 7 || (piece->pos + vel).y > 7)
      {
        break;
      }
      chessPiece *atPos = bd->pieceAtPos(piece->pos + vel);
      if (atPos)
      {
        if (atPos->isWhite != piece->isWhite)
        {
          move.push_back(piece->pos + vel);
          break;
        }
        else
        {
          break;
        }
      }
      else
      {
        move.push_back(piece->pos + vel);
      }
    }
  }
  // if castling is possible
  if (!piece->hasMoved)
  {
    // if rook is there check if it moved
    chessPiece *lRook = bd->pieceAtPos(iv2(0, piece->pos.y));
    if (lRook && !lRook->hasMoved && !lRook->eaten)
    {
      bool correct = true;
      int targetX = 2;
      // check if there are any pieces in the way of castling or if there are any threats
      for (int x = targetX; x <= piece->pos.x; x++)
      {
        for (int i = 0; i < bd->pieces.size(); i++)
        {
          if (x != lRook->pos.x && x != piece->pos.x && iv2(x, piece->pos.y) == toIV2(bd->pieces[i]->pos))
          {
            correct = false;
            break;
          }
          else
          {
            if (bd->pieces[i]->isWhite != piece->isWhite && bd->pieces[i]->type != pieceType::K)
            {
              for (iv2 move : bd->pieces[i]->possibleMoves(bd))
              {
                if (iv2(x, piece->pos.y) == move)
                {
                  correct = false;
                  break;
                }
              }
            }
          }
        }
      }
      if (correct)
      {
        move.push_back(iv2(targetX, piece->pos.y));
      }
    }
    // same as left side but right
    chessPiece *rRook = bd->pieceAtPos(iv2(7, piece->pos.y));
    if (rRook && !rRook->hasMoved && !rRook->eaten)
    {
      bool correct = true;
      int targetX = 6;
      for (int x = piece->pos.x; x <= targetX; x++)
      {
        for (int i = 0; i < bd->pieces.size(); i++)
        {
          if (x != rRook->pos.x && x != piece->pos.x && iv2(x, piece->pos.y) == toIV2(bd->pieces[i]->pos))
          {
            correct = false;
            break;
          }
          else
          {
            if (bd->pieces[i]->isWhite != piece->isWhite && bd->pieces[i]->type != pieceType::K)
            {
              for (iv2 move : bd->pieces[i]->possibleMoves(bd))
              {
                if (iv2(x, piece->pos.y) == move || iv2(x, piece->pos.y) == toIV2(bd->pieces[i]->pos))
                {
                  correct = false;
                  break;
                }
              }
            }
          }
        }
      }
      if (correct)
      {
        move.push_back(iv2(targetX, piece->pos.y));
      }
    }
  }
  return move;
}
