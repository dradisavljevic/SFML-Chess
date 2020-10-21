#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

using namespace sf;

// Chess piece structure.
struct chessSprite {
  Sprite sprite;
  int value;
  bool isWhite;
  int moves = 0;
};

// Size of each square.
int size = 113;

// Array of pieces
chessSprite f[32];

// Pieces pawn can promote into.
chessSprite promotionPieces[4];

// Chess board with starting positions.
int board[8][8] = {-1, -2, -3, -4, -5, -3, -2, -1,
                   -6, -6, -6, -6, -6, -6, -6, -6,
                    0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,
                    6,  6,  6,  6,  6,  6,  6,  6,
                    1,  2,  3,  4,  5,  3,  2,  1};

// Pieces inside the promotion widnow. Depends on color.
int promoteWhite[4] = {1, 2, 3, 4};

int promoteBlack[4] = {-1, -2, -3, -4};

// Array of previously promoted pieces. Maximum 16 since there is 16 pawns.
int promotions[16];

// Counter of promoted pieces.
int promotionIndex = 0;

// Which player is on the move. true = white; false = black.
bool onMove = true;
// If checkmate has occurred.
bool checkmate = false;
// If stalemate has occured.
bool stalemate = false;

// All the moves played so far.
std::string position = "";

/**
 * Convert chess notation on the board to coordinates on the screen.
 *
 * Converts chess notaiton to numbers using ASCII code.
 *
 * @param a chess notation of the X-axis (a-h).
 * @param b chess notation of the Y-axis (1-8).
 * @return Vector2f coordinates on screen.
 */
Vector2f toCoord(char a, char b) {
  int x = int(a) - 97;
  int y = 7 - int(b) + 49;
  return Vector2f(x * size, y * size);
}

/**
 * Moves piece from one square to another, based on the string value.
 *
 *  Takes in chess notation string and converts it into position. Checks for en passant
 *  and castling along the way.
 *
 * @param str string value of the move in chess notation e.g. a1b2.
 * @param lastMove string value of the previous move. Used to check for en passant.
 */
void move(std::string str, std::string lastMove) {
  Vector2f oldPos = toCoord(str[0], str[1]);
  Vector2f newPos = toCoord(str[2], str[3]);

  // In case of capture move previous figure.
  for (int i = 0; i < 32; i++)
    if (f[i].sprite.getPosition() == newPos)
      f[i].sprite.setPosition(-100, -100);

  // Move piece.
  for (int i = 0; i < 32; i++)
    if (f[i].sprite.getPosition() == oldPos) {
      if ((f[i].value == 6 && str[3] == '8') ||
          (f[i].value == -6 && str[3] == '1')) {
        int x = abs(promotions[promotionIndex]) - 1;
        bool isWhite = promotions[promotionIndex] > 0;
        int y = isWhite ? 1 : 0;
        f[i].sprite.setTextureRect(IntRect(size * x, size * y, size, size));
        f[i].value = promotions[promotionIndex];
        promotionIndex += 1;
      }
      f[i].sprite.setPosition(newPos);
      // Make it so the other player is on the move.
      onMove = !f[i].isWhite;
      f[i].moves += 1;

      // Check for en passant if the piece on the move is Pawn.
      if (abs(f[i].value) == 6 && lastMove != "") {
        int pawnDirection;
        if (f[i].isWhite == true) {
          pawnDirection = 1;
        } else {
          pawnDirection = -1;
        }
        Vector2f enPassantPos =
            Vector2f(newPos.x, newPos.y + size * pawnDirection);
        std::string lastMoveString =
            lastMove.substr(lastMove.length() - 4);
        Vector2f startingPos = toCoord(lastMoveString[0], lastMoveString[1]);
        Vector2f lastPos = toCoord(lastMoveString[2], lastMoveString[3]);

        // If en passant occurred, remove opposing piece.
        if (enPassantPos == lastPos &&
            abs(int(startingPos.y - lastPos.y)) == 2 * size) {
          for (int j = 0; j < 32; j++)
            if (f[j].sprite.getPosition() == enPassantPos)
              f[j].sprite.setPosition(-100, -100);
        }
      }
    }

  // Check for castling.
  if (str == "e1g1")
    if (position.substr(0, position.find(str)).find("e1") == -1)
      move("h1f1", "");
  if (str == "e8g8")
    if (position.substr(0, position.find(str)).find("e8") == -1)
      move("h8f8", "");
  if (str == "e1c1")
    if (position.substr(0, position.find(str)).find("e1") == -1)
      move("a1d1", "");
  if (str == "e8c8")
    if (position.substr(0, position.find(str)).find("e8") == -1)
      move("a8d8", "");
}

/**
 * Convert  screen coordinates to chess notation.
 *
 * Creates a string of chess notation from the screen coordinates of pieces.
 *
 * @param p Vector2f coordinates of the piece on screen.
 * @return chess notation of the pieces movement.
 */
std::string toChessNote(Vector2f p) {
  std::string s = "";
  s += char(p.x / size + 97);
  s += char(7 - p.y / size + 49);
  return s;
}

/**
 * Check for collision for pieces moving in straight line (Queen or Rook).
 *
 * Prevents movement if there is any piece in between the starting and
 * end destination of the moving piece.
 *
 * @param oldCoord starting position of the piece on X or Y axis.
 * @param newCoord ending position of the piece on X or Y axis.
 * @param oldPos starting coordinates of the piece on screen.
 * @param axis integer value of axis on which movement occurs. 0 = X axis; 1 = Y axis.
 * @return whether or not collision during movement occurred.
 */
bool lineMovementCollision(int oldCoord, int newCoord, Vector2f oldPos,
                           int axis) {
  int squares = abs(int(oldCoord - newCoord)) / size;
  int multiplier = (oldCoord - newCoord) < 0 ? 1 : -1;
  Vector2f inBetweenPos = Vector2f(oldPos);
  for (int i = 1; i < squares; i++) {
    if (axis == 0)
      inBetweenPos.x = oldCoord + i * size * multiplier;
    else
      inBetweenPos.y = oldCoord + i * size * multiplier;
    for (int i = 0; i < 32; i++)
      if (f[i].sprite.getPosition() == inBetweenPos)
        return true;
  }
  return false;
}

/**
 * Check for collision for pieces moving in diagonal (Queen or Bishop).
 *
 * Prevents movement if there is any piece in between the starting and
 * end destination of the moving piece.
 *
 * @param oldPos starting coordinates of the piece on screen.
 * @param newPos destination coordinates of the piece on screen.
 * @return whether or not collision during movement occurred.
 */
bool diagonalMovementCollision(Vector2f oldPos, Vector2f newPos) {
  int squares = abs(int(oldPos.x - newPos.x)) / size;
  int multiplierX = (oldPos.x - newPos.x) < 0 ? 1 : -1;
  int multiplierY = (oldPos.y - newPos.y) < 0 ? 1 : -1;
  Vector2f inBetweenPos = Vector2f(newPos.x, oldPos.y);
  for (int i = 1; i < squares; i++) {
    inBetweenPos.y = oldPos.y + i * size * multiplierY;
    inBetweenPos.x = oldPos.x + i * size * multiplierX;
    for (int i = 0; i < 32; i++)
      if (f[i].sprite.getPosition() == inBetweenPos)
        return true;
  }
  return false;
}

/**
 * Check whether or not check has occurred with the current layout of pieces on board.
 *
 * Goes through all the opposing pieces and checks one by one if
 * if they are endangering the king piece.
 *
 * @param colour of the player for which check is performed. True = white; False = black.
 * @return whether or not check has occurred.
 */
bool check(bool colour) {
  chessSprite king;
  for (int i = 0; i < 32; i++) {
    if (abs(f[i].value) == 5 && f[i].isWhite == colour) {
      king = f[i];
    }
  }

  // Direction in which Pawns should move.
  int pawnDirection;
  if (colour == true) {
    pawnDirection = -1;
  } else {
    pawnDirection = 1;
  }

  chessSprite rivalPieces[16];

  int k = 0;
  // Array of all the opposing pieces. Excludes those that have been captured.
  for (int i = 0; i < 32; i++) {
    if (f[i].isWhite != king.isWhite &&
        f[i].sprite.getPosition() != Vector2f(-100, -100)) {
      rivalPieces[k] = f[i];
      k++;
    }
  }

  for (int i = 0; i < k; i++) {
    switch (abs(rivalPieces[i].value)) {
    case 1:
      // If there is no collision in line, the Rooks are putting opposing King in check.
      if ((!lineMovementCollision(king.sprite.getPosition().y,
                                  rivalPieces[i].sprite.getPosition().y,
                                  rivalPieces[i].sprite.getPosition(), 1) &&
           rivalPieces[i].sprite.getPosition().x ==
               king.sprite.getPosition().x) ||
          (!lineMovementCollision(king.sprite.getPosition().x,
                                  rivalPieces[i].sprite.getPosition().x,
                                  rivalPieces[i].sprite.getPosition(), 0) &&
           rivalPieces[i].sprite.getPosition().y ==
               king.sprite.getPosition().y))
        return true;
      break;
    case 2:
      // If King is with the "L" shape from Knight, check has occurred.
      if ((abs(int(rivalPieces[i].sprite.getPosition().x -
                   king.sprite.getPosition().x)) == size &&
           abs(int(rivalPieces[i].sprite.getPosition().y -
                   king.sprite.getPosition().y)) == 2 * size) ||
          (abs(int(rivalPieces[i].sprite.getPosition().x -
                   king.sprite.getPosition().x)) == 2 * size &&
           abs(int(rivalPieces[i].sprite.getPosition().y -
                   king.sprite.getPosition().y)) == size))
        return true;
      break;
    case 3:
      // If there is no collision in diagonal, Bishop is putting King in check.
      if (!diagonalMovementCollision(king.sprite.getPosition(),
                                     rivalPieces[i].sprite.getPosition()) &&
          abs(int(king.sprite.getPosition().x -
                  rivalPieces[i].sprite.getPosition().x)) ==
              abs(int(king.sprite.getPosition().y -
                      rivalPieces[i].sprite.getPosition().y)))
        return true;
      break;
    case 4:
      // If there is no collision in diagonal or in line, Queen is putting opposign King in check.
      if ((!lineMovementCollision(king.sprite.getPosition().y,
                                  rivalPieces[i].sprite.getPosition().y,
                                  rivalPieces[i].sprite.getPosition(), 1) &&
           rivalPieces[i].sprite.getPosition().x ==
               king.sprite.getPosition().x) ||
          (!lineMovementCollision(king.sprite.getPosition().x,
                                  rivalPieces[i].sprite.getPosition().x,
                                  rivalPieces[i].sprite.getPosition(), 0) &&
           rivalPieces[i].sprite.getPosition().y ==
               king.sprite.getPosition().y) ||
          (!diagonalMovementCollision(king.sprite.getPosition(),
                                      rivalPieces[i].sprite.getPosition()) &&
           abs(int(king.sprite.getPosition().x -
                   rivalPieces[i].sprite.getPosition().x)) ==
               abs(int(king.sprite.getPosition().y -
                       rivalPieces[i].sprite.getPosition().y))))
        return true;
      break;
    case 5:
      // King can't be bothered to put other King in check.
      break;
    case 6:
      // If King is diagonal of the Pawn, check has occurred.
      if (abs(int(rivalPieces[i].sprite.getPosition().x -
                  king.sprite.getPosition().x)) == size &&
          (rivalPieces[i].sprite.getPosition().y -
           king.sprite.getPosition().y) == size * pawnDirection)
        return true;
      break;
    }
  }
  return false;
}

/**
 * Check whether or not attempted move is valid.
 *
 * Checks for collision and overall validity of the move. E.g. you can't capture your own pieces.
 *
 * @param figure a piece that is being moved.
 * @param oldPos position of the piece before movement.
 * @param newPos destination of the piece.
 * @return whether or not the move is valid.
 */
bool isValidMove(chessSprite figure, Vector2f oldPos, Vector2f newPos) {
  bool isValid = false;
  int pawnDirection;

  // In which directions should pawn move.
  if (figure.isWhite == true) {
    pawnDirection = 1;
  } else {
    pawnDirection = -1;
  }

  chessSprite destinationFigure;

  for (int i = 0; i < 32; i++)
    if (f[i].sprite.getPosition() == newPos)
      destinationFigure = f[i];

  // If there is a piece and it is not opposing, prevent capture.
  // Same for both King pieces.
  if ((abs(destinationFigure.value) < 7 &&
       figure.isWhite == destinationFigure.isWhite) ||
      abs(destinationFigure.value) == 5) {
    return false;
  }

  if (oldPos==newPos)
    return false;

  switch (abs(figure.value)) {
  case 1:
    // If there is no collision in line, the move is valid for Rook.
    if (abs(int(oldPos.x - newPos.x)) == 0 &&
        abs(int(oldPos.y - newPos.y)) > 0) {
      if (lineMovementCollision(oldPos.y, newPos.y, oldPos, 1))
        return false;
      isValid = true;
    } else if (abs(int(oldPos.y - newPos.y)) == 0 &&
               abs(int(oldPos.x - newPos.x)) > 0) {
      if (lineMovementCollision(oldPos.x, newPos.x, oldPos, 0))
        return false;
      isValid = true;
    } else {
      isValid = false;
    }
    break;
  case 2:
    // If the movement is L shaped, it is a valid Knight move.
    if (abs(int(oldPos.x - newPos.x)) == size &&
        abs(int(oldPos.y - newPos.y)) == size * 2) {
      isValid = true;
    } else if (abs(int(oldPos.x - newPos.x)) == 2 * size &&
               abs(int(oldPos.y - newPos.y)) == size) {
      isValid = true;
    } else {
      isValid = false;
    }
    break;
  case 3:
    // If no collision occuts in diagonal movement, it is a valid Bishop move.
    if (abs(int(oldPos.x - newPos.x)) == abs(int(oldPos.y - newPos.y))) {
      if (diagonalMovementCollision(oldPos, newPos))
        return false;
      isValid = true;
    } else {
      isValid = false;
    }
    break;
  case 4:
    // If there is no collision while moving in line or diagonal, it makes for
    // a valid Queen move.
    if (abs(int(oldPos.x - newPos.x)) == 0 &&
        abs(int(oldPos.y - newPos.y)) > 0) {
      if (lineMovementCollision(oldPos.y, newPos.y, oldPos, 1))
        return false;
      isValid = true;
    } else if (abs(int(oldPos.y - newPos.y)) == 0 &&
               abs(int(oldPos.x - newPos.x)) > 0) {
      if (lineMovementCollision(oldPos.x, newPos.x, oldPos, 0))
        return false;
      isValid = true;
    } else if (abs(int(oldPos.x - newPos.x)) == abs(int(oldPos.y - newPos.y))) {
      if (diagonalMovementCollision(oldPos, newPos))
        return false;
      isValid = true;
    } else {
      isValid = false;
    }
    break;
  case 5:
    // Check in all 8 directions for the valid king move. Also check for castling.
    if (abs(int(oldPos.x - newPos.x)) <= size &&
        abs(int(oldPos.y - newPos.y)) <= size) {
      isValid = true;
    // If the King is in check, castling should be prevented.
    } else if (abs(int(oldPos.x - newPos.x)) == 2 * size &&
               abs(int(oldPos.y - newPos.y)) == 0 && figure.moves == 0 &&
               figure.sprite.getColor() != Color(100, 0, 0) &&
               figure.sprite.getColor() != Color(100, 0, 100)) {
      int multiplier = (oldPos.x - newPos.x) < 0 ? -1 : 1;
      if (lineMovementCollision(oldPos.x, newPos.x + 2 * size * multiplier,
                                oldPos, 0))
        return false;

      // Check if Rooks have moved before castling.
      char row;
      if (figure.isWhite) {
        row = 'a';
      } else {
        row = 'h';
      }

      char column;
      if ((oldPos.x - newPos.x) > 0) {
        column = '1';
      } else {
        column = '8';
      }

      Vector2f castleCandidate = toCoord(row, column);
      isValid = false;
      // If the Rook hasn't moved.
      for (int i = 0; i < 32; i++)
        if (f[i].sprite.getPosition() == castleCandidate && f[i].moves == 0)
          isValid = true;

    } else {
      isValid = false;
    }
    break;
  case 6:
    if ((oldPos.y - newPos.y) == size * pawnDirection ||
        ((oldPos.y - newPos.y) == size * pawnDirection * 2 &&
         figure.moves == 0)) {
      if (abs(int(oldPos.x - newPos.x)) > size) {
        isValid = false;
      } else if (abs(int(oldPos.x - newPos.x)) == size) {
        if (abs(destinationFigure.value) < 7 &&
            abs(destinationFigure.value) != 5)
          isValid = true;
        else {
          // Check for possibility of en passant capture. If opponent moved
          // Pawn two squares in the last move.
          Vector2f enPassantPos =
              Vector2f(newPos.x, newPos.y + size * pawnDirection);
          if (position.length() >= 5) {
            std::string lastPosString = position.substr(position.length() - 5);
            Vector2f startingPos = toCoord(lastPosString[0], lastPosString[1]);
            Vector2f lastPos = toCoord(lastPosString[2], lastPosString[3]);
            if (enPassantPos == lastPos &&
                abs(int(startingPos.y - lastPos.y)) == 2 * size) {
              for (int i = 0; i < 32; i++)
                if (f[i].sprite.getPosition() == enPassantPos)
                  f[i].sprite.setPosition(-100, -100);
              isValid = true;
            } else {
              isValid = false;
            }
          } else {
            isValid = false;
          }
        }
      } else {
        // Pawns can't capture in straight line, only diagonal.
        if (abs(destinationFigure.value) > 6)
          isValid = true;
        else
          isValid = false;
      }
    } else {
      isValid = false;
    }
    break;
  }
  return isValid;
}

/**
 * Check if moving a piece in line removes check.
 *
 * For a piece that can move in line, check for each square if moving to it
 * removes check state.
 *
 * @param piece that is being moved.
 * @param step change to current position.
 * @param dx multiplier for X axis. If 0, the piece moves on the Y axis.
 * @param dy multiplier for Y axis. If 0, the piece moves on the X axis.
 * @param colour of the piece. True if white, False if black.
 * @return whether or not check can be removed by moving the selected piece.
 */
bool isLineMovePossible(chessSprite piece, int step, int dx, int dy, bool colour) {
  int beginning;
  if (dx != 0) {
    beginning = piece.sprite.getPosition().x + step;
  } else {
    beginning = piece.sprite.getPosition().y + step;
  }
  for (int j = beginning; j < 8 * size && j >= 0; j += step) {
    Vector2f initialPosition = piece.sprite.getPosition();
    if (isValidMove(piece, initialPosition,
                  Vector2f(dy * initialPosition.x + (dx * j),
                           dx * initialPosition.y + (dy * j)))) {
      int movingPieceIndex = -2;
      for (int l = 0; l < 32; l++) {
        if (f[l].sprite.getPosition() == initialPosition)
          movingPieceIndex = l;
      }
      f[movingPieceIndex].sprite.setPosition(dy * initialPosition.x + (dx * j),
                                             dx * initialPosition.y + (dy * j));
      int destinationPieceIndex = -2;
      // If destination contains opposing piece capture it.
      bool isCaptured = false;
      for (int i = 0; i < 32; i++) {
        if (f[i].sprite.getPosition() ==
                Vector2f(dy * initialPosition.x + (dx * j),
                         dx * initialPosition.y + (dy * j)) &&
            f[i].isWhite != piece.isWhite) {
          isCaptured = true;
          destinationPieceIndex = i;
          f[destinationPieceIndex].sprite.setPosition(-100, -100);
        }
      }
      if (!check(colour)) {
        // Return board to pre-move state.
        f[movingPieceIndex].sprite.setPosition(initialPosition);
        if (isCaptured)
          f[destinationPieceIndex].sprite.setPosition(
              Vector2f(dy * initialPosition.x + (dx * j),
                       dx * initialPosition.y + (dy * j)));
        return true;
      }
      f[movingPieceIndex].sprite.setPosition(initialPosition);
      if (isCaptured)
        f[destinationPieceIndex].sprite.setPosition(
            Vector2f(dy * initialPosition.x + (dx * j),
                     dx * initialPosition.y + (dy * j)));
    } else
      break;
  }

  return false;
}

/**
 * Check if moving a piece in diagonal removes check.
 *
 * For a piece that can move in diagonal, check for each square if moving to it
 * removes check state.
 *
 * @param piece a piece that is being moved.
 * @param dx multiplier for X axis. Values can be either 1 or -1.
 * @param dy multiplier for Y axis. Values can be either 1 or -1.
 * @param colour of the piece. True if white, False if black.
 * @return whether or not check can be removed by moving the selected piece.
 */
bool isDiagonalMovePossible(chessSprite piece, int dx, int dy, bool colour) {
  int beginningX = piece.sprite.getPosition().x;
  int beginningY = piece.sprite.getPosition().y;
  Vector2f initialPosition = piece.sprite.getPosition();

  while (beginningX < 8 * size && beginningX >= 0 && beginningY < 8 * size &&
         beginningY >= 0) {
    beginningX += dx * size;
    beginningY += dy * size;
    if (isValidMove(piece, initialPosition, Vector2f(beginningX, beginningY))) {
      int movingPieceIndex = -2;
      for (int l = 0; l < 32; l++) {
        if (f[l].sprite.getPosition() == initialPosition)
          movingPieceIndex = l;
      }
      f[movingPieceIndex].sprite.setPosition(beginningX, beginningY);
      // If there is opposing piece on the destination square, simulate capture.
      int destinationPieceIndex = -2;
      bool isCaptured = false;
      for (int i = 0; i < 32; i++) {
        if (f[i].sprite.getPosition() == Vector2f(beginningX, beginningY) &&
            f[i].isWhite != piece.isWhite) {
          isCaptured = true;
          destinationPieceIndex = i;
          f[destinationPieceIndex].sprite.setPosition(-100, -100);
        }
      }
      if (!check(colour)) {
        // Return board to pre-move state.
        f[movingPieceIndex].sprite.setPosition(initialPosition);
        if (isCaptured)
          f[destinationPieceIndex].sprite.setPosition(
              Vector2f(beginningX, beginningY));
        return true;
      }
      f[movingPieceIndex].sprite.setPosition(initialPosition);
      if (isCaptured)
        f[destinationPieceIndex].sprite.setPosition(
            Vector2f(beginningX, beginningY));
    } else
      break;
  }

  return false;
}


/**
 * Check if there is any valid move left that gets player out of check.
 *
 * For each players piece, check if moving it will leave it in check or not.
 * First occurrence returns true value.
 *
 * @param colour of the piece. True if white, False if black.
 * @return whether or not player can make any move that gets him out of check.
 */
bool isMovementPossible(bool colour) {
  chessSprite king;
  for (int i = 0; i < 32; i++) {
    if (abs(f[i].value) == 5 && f[i].isWhite == colour) {
      king = f[i];
    }
  }

  chessSprite possiblePieces[16];

  int m = 0;
  for (int i = 0; i < 32; i++) {
    if (f[i].isWhite == king.isWhite &&
        f[i].sprite.getPosition() != Vector2f(-100, -100)) {
      possiblePieces[m] = f[i];
      m++;
    }
  }

  for (int i = 0; i < m; i++) {
    switch (abs(possiblePieces[i].value)) {
    case 1:
      // Check in all 4 directions for Rooks.
      if (isLineMovePossible(possiblePieces[i], size, 1, 0, colour))
        return true;
      if (isLineMovePossible(possiblePieces[i], size, 0, 1, colour))
        return true;
      if (isLineMovePossible(possiblePieces[i], -1 * size, 1, 0, colour))
        return true;
      if (isLineMovePossible(possiblePieces[i], -1 * size, 0, 1, colour))
        return true;
      break;
    case 2:
      for (int k = -2; k <= 2; k++) {
        for (int j = -2; j <= 2; j++) {
          // Make sure Knight moves only in L shape.
          if (k == 0 || j == 0 || abs(j) == abs(k))
            continue;
          Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
          if (initialPosition.x + k * size < 8 * size &&
              initialPosition.x + k * size >= 0 &&
              initialPosition.y + j * size < 8 * size &&
              initialPosition.y + j * size >= 0)
            if (isValidMove(possiblePieces[i], initialPosition,
                          Vector2f(initialPosition.x + k * size,
                                   initialPosition.y + j * size))) {
              int movingPieceIndex = -2;
              for (int l = 0; l < 32; l++) {
                if (f[l].sprite.getPosition() == initialPosition)
                  movingPieceIndex = l;
              }
              f[movingPieceIndex].sprite.setPosition(
                  initialPosition.x + k * size, initialPosition.y + j * size);
              if (!check(colour)) {
                f[movingPieceIndex].sprite.setPosition(initialPosition);
                return true;
              }
              f[movingPieceIndex].sprite.setPosition(initialPosition);
            }
        }
      }
      break;
    case 3:
      // Check for all 4 diagonal movements for Bishops.
      if (isDiagonalMovePossible(possiblePieces[i], 1, 1, colour))
        return true;
      if (isDiagonalMovePossible(possiblePieces[i], 1, -1, colour))
        return true;
      if (isDiagonalMovePossible(possiblePieces[i], -1, 1, colour))
        return true;
      if (isDiagonalMovePossible(possiblePieces[i], -1, -1, colour))
        return true;
      break;
    case 4:
      // Check for all 4 diagonal and all 4 line movements for Queen.
      if (isLineMovePossible(possiblePieces[i], size, 1, 0, colour))
        return true;
      if (isLineMovePossible(possiblePieces[i], size, 0, 1, colour))
        return true;
      if (isLineMovePossible(possiblePieces[i], -1 * size, 1, 0, colour))
        return true;
      if (isLineMovePossible(possiblePieces[i], -1 * size, 0, 1, colour))
        return true;
      if (isDiagonalMovePossible(possiblePieces[i], 1, 1, colour))
        return true;
      if (isDiagonalMovePossible(possiblePieces[i], 1, -1, colour))
        return true;
      if (isDiagonalMovePossible(possiblePieces[i], -1, 1, colour))
        return true;
      if (isDiagonalMovePossible(possiblePieces[i], -1, -1, colour))
        return true;
      break;
    case 5:
      // Check if King can be moved anywhere. Checking for castling is redundant
      // as King can't castle if he is in check.
      for (int k = -1; k <= 1; k++) {
        for (int j = -1; j <= 1; j++) {
          if (k == 0 && j == 0)
            continue;
          Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
          if (initialPosition.x + k * size < 8 * size &&
              initialPosition.x + k * size >= 0 &&
              initialPosition.y + j * size < 8 * size &&
              initialPosition.y + j * size >= 0)
            if (isValidMove(possiblePieces[i], initialPosition,
                          Vector2f(initialPosition.x + k * size,
                                   initialPosition.y + j * size))) {
              int movingPieceIndex = -2;
              for (int l = 0; l < 32; l++) {
                if (f[l].sprite.getPosition() == initialPosition)
                  movingPieceIndex = l;
              }
              int destinationPieceIndex = -2;
              bool isCaptured = false;
              for (int l = 0; l < 32; l++) {
                if (f[l].sprite.getPosition() ==
                        Vector2f(initialPosition.x + k * size,
                                 initialPosition.y + j * size) &&
                    f[l].isWhite != possiblePieces[i].isWhite) {
                  isCaptured = true;
                  destinationPieceIndex = l;
                  f[destinationPieceIndex].sprite.setPosition(-100, -100);
                }
              }
              f[movingPieceIndex].sprite.setPosition(
                  initialPosition.x + k * size, initialPosition.y + j * size);
              if (!check(colour)) {
                f[movingPieceIndex].sprite.setPosition(initialPosition);
                f[destinationPieceIndex].sprite.setPosition(
                    Vector2f(initialPosition.x + k * size,
                             initialPosition.y + j * size));
                return true;
              }
              f[movingPieceIndex].sprite.setPosition(initialPosition);
              if (isCaptured)
                f[destinationPieceIndex].sprite.setPosition(
                    Vector2f(initialPosition.x + k * size,
                             initialPosition.y + j * size));
            }
        }
      }
      break;
    case 6:
      // Check for pawn movement in a single direction.
      int pawnDirection;

      // In which directions should pawn move.
      if (possiblePieces[i].isWhite == false) {
        pawnDirection = 1;
      } else {
        pawnDirection = -1;
      }
      for (int j = 1; j <= 2; j++) {
        if (j == 1) {
          for (int k = -1; k <= 1; k++) {

            Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
            if (initialPosition.x + k * size < 8 * size &&
                initialPosition.x + k * size >= 0 &&
                initialPosition.y + j * size * pawnDirection < 8 * size)
              if (isValidMove(
                      possiblePieces[i], initialPosition,
                      Vector2f(initialPosition.x + k * size,
                               initialPosition.y + j * size * pawnDirection))) {
                int movingPieceIndex = -2;
                for (int l = 0; l < 32; l++) {
                  if (f[l].sprite.getPosition() == initialPosition)
                    movingPieceIndex = l;
                }
                f[movingPieceIndex].sprite.setPosition(
                    initialPosition.x + k * size,
                    initialPosition.y + j * size * pawnDirection);
                if (!check(colour)) {
                  f[movingPieceIndex].sprite.setPosition(initialPosition);
                  return true;
                }
                f[movingPieceIndex].sprite.setPosition(initialPosition);
              }
          }
        } else {
          Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
          if (initialPosition.y + j * size < 8 * size)
            if (isValidMove(
                    possiblePieces[i], initialPosition,
                    Vector2f(initialPosition.x,
                             initialPosition.y + j * size * pawnDirection))) {
              int movingPieceIndex = -2;
              for (int l = 0; l < 32; l++) {
                if (f[l].sprite.getPosition() == initialPosition)
                  movingPieceIndex = l;
              }
              f[movingPieceIndex].sprite.setPosition(
                  initialPosition.x,
                  initialPosition.y + j * size * pawnDirection);
              if (!check(colour)) {
                f[movingPieceIndex].sprite.setPosition(initialPosition);
                return true;
              }
              f[movingPieceIndex].sprite.setPosition(initialPosition);
            }
        }
      }
      break;
    }
  }
  return false;
}

/**
 * Load position of the pieces on the board based on the position string.
 *
 * Set up the board based on previous movements in case of undo function
 * or set up a new board if it is a new game.
 */
void loadPosition() {
  int k = 0;
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++) {
      int n = board[i][j];
      if (!n)
        continue;
      int x = abs(n) - 1;
      bool isWhite = n > 0;
      int y = isWhite ? 1 : 0;
      f[k].sprite.setTextureRect(IntRect(size * x, size * y, size, size));
      f[k].sprite.setPosition(size * j, size * i);
      f[k].value = n;
      f[k].isWhite = isWhite;
      f[k].moves = 0;
      k++;
    }

  // Last move is gonna be used to detect en passant capture.
  std::string lastMove = "";
  for (int i = 0; i < position.length(); i += 5) {
    move(position.substr(i, 4), lastMove);
    lastMove = position.substr(i, 4);
  }
  // Checkmate and stalemate can't occur in the middle of the game
  // so leaving the indicators on true makes no sense.
  checkmate = false;
  stalemate = false;

  for (int i = 0; i < 32; i++) {
    if (abs(f[i].value) == 5) {
      for (int j = 0; j < 32; j++) {
        if (abs(f[j].value) != 5 && f[j].isWhite == f[i].isWhite) {
          if (check(f[i].isWhite))
            f[i].sprite.setColor(Color(100, 0, 0));
          else
            f[i].sprite.setColor(f[j].sprite.getColor());
        }
      }
    }
  }
}

int main() {
  // Render SFML main window.
  RenderWindow window(VideoMode(908, 910), "The Chess!", Style::Close);

  // Setup chess board and pieces.
  Texture t1, t2;
  t1.loadFromFile("images/figures.png");
  t2.loadFromFile("images/board.png");

  Sprite s(t1);
  Sprite sBoard(t2);

  for (int i = 0; i < 32; i++)
    f[i].sprite.setTexture(t1);

  loadPosition();

  // If the figure is being dragged.
  bool dragging = false;
  // Delta of the figures movement.
  float dx = 0, dy = 0;
  Vector2f oldPos, newPos;
  std::string str;
  int n = 0;

  while (window.isOpen()) {
    Vector2i pos = Mouse::getPosition(window);
    Event e;
    while (window.pollEvent(e)) {
      if (e.type == Event::Closed)
        window.close();

      if (e.type == Event::KeyPressed) {
        // Undo function.
        if (e.key.code == Keyboard::BackSpace) {
          if (position.length() > 5) {
            position.erase(position.length() - 6, 5);
            promotionIndex = 0;
            loadPosition();
          }
        }
      }

      // Piece movement.
      if (e.type == Event::MouseButtonPressed)
        if (e.key.code == Mouse::Left)
          for (int i = 0; i < 32; i++)
            if (f[i].sprite.getGlobalBounds().contains(pos.x, pos.y) &&
                f[i].isWhite == onMove && stalemate == false &&
                checkmate == false) {
              dragging = true;
              n = i;
              dx = pos.x - f[i].sprite.getPosition().x;
              dy = pos.y - f[i].sprite.getPosition().y;
              oldPos = f[i].sprite.getPosition();
            }

      if (e.type == Event::MouseButtonReleased)
        if (e.key.code == Mouse::Left) {
          dragging = false;
          Vector2f p = f[n].sprite.getPosition() + Vector2f(size / 2, size / 2);
          Vector2f newPos =
              Vector2f(size * int(p.x / size), size *int(p.y / size));
          if (isValidMove(f[n], oldPos, newPos)) {
            str = toChessNote(oldPos) + toChessNote(newPos);

            // Should the pawn reach opposite side backline, open promotion window.
            if ((f[n].value == 6 && str[3] == '8') ||
                (f[n].value == -6 && str[3] == '1')) {
              RenderWindow window2(VideoMode(454, 120),
                                   "Promote pawn to:", Style::Titlebar);
              for (int i = 0; i < 4; i++) {
                int pieces;
                // Depending on pawns color, pick the pieces to display in promotion window.
                if (f[n].value < 0)
                  pieces = promoteBlack[i];
                else
                  pieces = promoteWhite[i];
                int x = abs(pieces) - 1;
                bool isWhite = pieces > 0;
                int y = isWhite ? 1 : 0;
                promotionPieces[i].sprite.setTexture(t1);
                promotionPieces[i].sprite.setTextureRect(
                    IntRect(size * x, size * y, size, size));
                promotionPieces[i].sprite.setPosition(size * i, 0);
                promotionPieces[i].value = pieces;
              }

              while (window2.isOpen()) {
                Event event;
                Vector2i pos2 = Mouse::getPosition(window2);
                while (window2.pollEvent(event)) {
                  // This has now been disabled by making the window unclosable
                  // to prevent any inconsistency.
                  if (event.type == Event::Closed)
                    window2.close();

                  if (event.type == Event::MouseButtonPressed)
                    if (event.key.code == Mouse::Left)
                      for (int i = 0; i < 4; i++)
                        if (promotionPieces[i]
                                .sprite.getGlobalBounds()
                                .contains(pos2.x, pos2.y)) {
                          int pieces;
                          if (f[n].value < 0)
                            pieces = promoteBlack[i];
                          else
                            pieces = promoteWhite[i];
                          int x = abs(pieces) - 1;
                          bool isWhite = pieces > 0;
                          int y = isWhite ? 1 : 0;
                          // Change pieces value and sprite based on promotion selection.
                          f[n].sprite.setTextureRect(
                              IntRect(size * x, size * y, size, size));
                          f[n].value = pieces;
                          promotions[promotionIndex] = pieces;
                          promotionIndex += 1;
                          window2.close();
                        }
                }
                // Black background so pieces are distinguishable.
                window2.clear(Color(13, 13, 13));
                for (int i = 0; i < 4; i++)
                  window2.draw(promotionPieces[i].sprite);
                window2.display();
              }
            }
            int destinationIndex = -2;
            for (int l = 0; l < 32; l++) {
              if (f[l].sprite.getPosition() == newPos)
                destinationIndex = l;
            }
            move(str, "");
            f[n].sprite.setPosition(newPos);
            if (!check(f[n].isWhite)) {
              f[n].moves += 1;
              position += str + " ";
              std::cout << str << std::endl;
              onMove = !onMove;
            } else {
              f[n].sprite.setPosition(oldPos);
              if (destinationIndex != -2) {
                f[destinationIndex].sprite.setPosition(newPos);
              }
            }
            // If check occurred indicate it by changing Kings color.
            if (check(!f[n].isWhite) && isMovementPossible(!f[n].isWhite)) {
              std::cout << "CHECK" << std::endl;
              for (int l = 0; l < 32; l++) {
                if (abs(f[l].value) == 5 && f[l].isWhite != f[n].isWhite) {
                  f[l].sprite.setColor(Color(100, 0, 0));
                }
              }
            } else {
              if (!check(f[n].isWhite)) {
                // Since movement that leaves King in check is disabled, color the King back
                // upon the movement that takes him out of check.
                Color usualColor = Color(100, 100, 100);
                for (int l = 0; l < 32; l++) {
                  if (abs(f[l].value) == 1 && f[l].isWhite == f[n].isWhite) {
                    usualColor = f[l].sprite.getColor();
                  }
                  if (abs(f[l].value) == 5 && f[l].isWhite == f[n].isWhite) {
                    f[l].sprite.setColor(usualColor);
                  }
                }
              }
            }
            // In case of a checkmate, color the King purple.
            if (check(!f[n].isWhite) && !isMovementPossible(!f[n].isWhite)) {
              std::cout << "CHECK MATE" << std::endl;
              checkmate = true;
              for (int l = 0; l < 32; l++) {
                if (abs(f[l].value) == 5 && f[l].isWhite != f[n].isWhite) {
                  f[l].sprite.setColor(Color(100, 0, 100));
                }
              }
            } else {
              if (!check(f[n].isWhite)) {
                Color usualColor = Color(100, 100, 100);
                for (int l = 0; l < 32; l++) {
                  if (abs(f[l].value) == 1 && f[l].isWhite == f[n].isWhite) {
                    usualColor = f[l].sprite.getColor();
                  }
                  if (abs(f[l].value) == 5 && f[l].isWhite == f[n].isWhite) {
                    f[l].sprite.setColor(usualColor);
                  }
                }
              }
            }
            // If stalemate occurred, color both Kings yellow.
            if (!check(!f[n].isWhite) && !isMovementPossible(!f[n].isWhite)) {
              std::cout << "STALEMATE" << std::endl;
              stalemate = true;
              for (int l = 0; l < 32; l++) {
                if (abs(f[l].value) == 5) {
                  f[l].sprite.setColor(Color(100, 100, 10));
                }
              }
            }
          } else {
            if (onMove == f[n].isWhite)
              f[n].sprite.setPosition(oldPos);
          }
        }

      // If piece is being dragged.
      if (dragging)
        f[n].sprite.setPosition(pos.x - dx, pos.y - dy);
    }

    // Re-rendering window on movement.
    window.clear();
    window.draw(sBoard);
    for (int i = 0; i < 32; i++)
      window.draw(f[i].sprite);
    window.draw(f[n].sprite);
    window.display();
  }

  return 0;
}
