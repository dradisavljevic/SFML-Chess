#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <string>

using namespace sf;

struct chessSprite {
    Sprite sprite;
    int value;
    bool isWhite;
    int moves = 0;
};

int size = 113;

chessSprite f[32];

chessSprite promotionPieces[4];

int board[8][8] =
    {-1, -2, -3, -4, -5, -3, -2, -1,
     -6, -6, -6, -6, -6, -6, -6, -6,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     6, 6, 6, 6, 6, 6, 6, 6,
     1, 2, 3, 4, 5, 3, 2, 1};

// Figures inside the promotion widnow
int promoteWhite[4] = {1,2,3,4};

int promoteBlack[4] = {-1, -2, -3, -4};

int promotions[16];

int promotionIndex = 0;

// All the moves played so far
std::string position = "";

// Convert chess notation to coordinates on the chess board
Vector2f toCoord(char a, char b) {
    int x = int(a) - 97;
    int y = 7 - int(b) + 49;
    return Vector2f(x*size, y*size);
}

// move figure to position
void move(std::string str, std::string lastPosition) {
    Vector2f oldPos = toCoord(str[0], str[1]);
    Vector2f newPos = toCoord(str[2], str[3]);
    
    for (int i=0; i<32; i++)
        if (f[i].sprite.getPosition()==newPos) f[i].sprite.setPosition(-100, -100);
    
    for (int i=0; i<32; i++)
        if (f[i].sprite.getPosition()==oldPos)
        {
            if ((f[i].value==6 && str[3]=='8') || (f[i].value==-6 && str[3]=='1')){
                int x = abs(promotions[promotionIndex]) - 1;
                bool isWhite = promotions[promotionIndex] > 0;
                int y = isWhite ? 1 : 0;
                f[i].sprite.setTextureRect(IntRect(size*x, size*y, size, size));
                f[i].value = promotions[promotionIndex];
                promotionIndex+=1;
            }
            f[i].sprite.setPosition(newPos);
            f[i].moves += 1;
            
            if (abs(f[i].value)==6 && lastPosition!="") {
                int pawnDirection;
                if (f[i].isWhite == true) {
                    pawnDirection = 1;
                } else {
                    pawnDirection = -1;
                }
                Vector2f enPassantPos = Vector2f(newPos.x, newPos.y+size*pawnDirection);
                std::string lastPosString = lastPosition.substr( lastPosition.length() - 4 );
                Vector2f startingPos = toCoord(lastPosString[0], lastPosString[1]);
                Vector2f lastPos = toCoord(lastPosString[2], lastPosString[3]);
                
                if (enPassantPos == lastPos && abs(int(startingPos.y - lastPos.y))==2*size) {
                    for (int j=0; j<32; j++)
                        if (f[j].sprite.getPosition()==enPassantPos) f[j].sprite.setPosition(-100, -100);
                }
            }
        }
    
    
    if (str=="e1g1") if (position.substr(0,position.find(str)).find("e1")==-1) move("h1f1", "");
    if (str=="e8g8") if (position.substr(0,position.find(str)).find("e8")==-1) move("h8f8", "");
    if (str=="e1c1") if (position.substr(0,position.find(str)).find("e1")==-1) move("a1d1", "");
    if (str=="e8c8") if (position.substr(0,position.find(str)).find("e8")==-1) move("a8d8", "");
}

// Load initial position, or backtrack all the moves after an undo
void loadPosition() {
    int k=0;
    for (int i=0; i<8; i++)
        for (int j=0; j<8; j++)
        {
            int n = board[i][j];
            if (!n) continue;
            int x = abs(n) - 1;
            bool isWhite = n > 0;
            int y = isWhite ? 1 : 0;
            f[k].sprite.setTextureRect(IntRect(size*x, size*y, size, size));
            f[k].sprite.setPosition(size*j, size*i);
            f[k].value = n;
            f[k].isWhite = isWhite;
            f[k].moves = 0;
            k++;
        }
    
    std::string lastPosition = "";
    for (int i=0; i<position.length(); i+=5) {
        move(position.substr(i,4), lastPosition);
        lastPosition = position.substr(i,4);
    }
        
}

// Convert position to chess notation
std::string toChessNote(Vector2f p) {
    std::string s = "";
    s += char(p.x/size+97);
    s += char(7-p.y/size+49);
    return s;
}

// If there is any piece in line between figure and destination, prevent movement
bool lineMovementCollision(int oldCoord, int newCoord, Vector2f oldPos, int axis) {
    int squares = abs(int(oldCoord - newCoord))/size;
    int multiplier = (oldCoord - newCoord) < 0 ? 1 : -1;
    Vector2f inBetweenPos = Vector2f(oldPos);
    for (int i=1; i<squares; i++){
        if (axis == 0)
            inBetweenPos.x = oldCoord + i*size*multiplier;
        else
            inBetweenPos.y = oldCoord + i*size*multiplier;
        for (int i=0; i<32; i++)
            if (f[i].sprite.getPosition()==inBetweenPos) return true;
    }
    return false;
}

// If there is any piece in diagonal, prevent movement
bool diagonalMovementCollision(Vector2f oldPos, Vector2f newPos) {
    int squares = abs(int(oldPos.x - newPos.x))/size;
    int multiplierX = (oldPos.x - newPos.x) < 0 ? 1 : -1;
    int multiplierY = (oldPos.y - newPos.y) < 0 ? 1 : -1;
    Vector2f inBetweenPos = Vector2f(newPos.x, oldPos.y);
    for (int i=1; i<squares; i++){
        inBetweenPos.y = oldPos.y + i*size*multiplierY;
        inBetweenPos.x = oldPos.x + i*size*multiplierX;
        for (int i=0; i<32; i++)
            if (f[i].sprite.getPosition()==inBetweenPos) return true;
    }
    return false;
}

// Check if check occured
bool check(bool colour) {
    chessSprite king;
    for (int i=0; i<32; i++) {
        if (abs(f[i].value) == 5 && f[i].isWhite==colour) {
            king = f[i];
        }
    }
    
    int pawnDirection;
    if (colour == true) {
        pawnDirection = -1;
    } else {
        pawnDirection = 1;
    }
    
    chessSprite rivalPieces[16];
    
    int k=0;
    for (int i=0; i<32; i++){
        if (f[i].isWhite!=king.isWhite && f[i].sprite.getPosition()!=Vector2f(-100, -100)) {
            rivalPieces[k] = f[i];
            k++;
        }
    }
    
    for (int i=0; i<k; i++){
        switch(abs(rivalPieces[i].value)) {
            case 1:
                if ((!lineMovementCollision(king.sprite.getPosition().y, rivalPieces[i].sprite.getPosition().y, rivalPieces[i].sprite.getPosition(), 1) && rivalPieces[i].sprite.getPosition().x==king.sprite.getPosition().x) || (!lineMovementCollision(king.sprite.getPosition().x, rivalPieces[i].sprite.getPosition().x, rivalPieces[i].sprite.getPosition(), 0) && rivalPieces[i].sprite.getPosition().y==king.sprite.getPosition().y))
                    return true;
                break;
            case 2:
                if ((abs(int(rivalPieces[i].sprite.getPosition().x - king.sprite.getPosition().x))==size && abs(int(rivalPieces[i].sprite.getPosition().y - king.sprite.getPosition().y))==2*size) || (abs(int(rivalPieces[i].sprite.getPosition().x - king.sprite.getPosition().x))==2*size && abs(int(rivalPieces[i].sprite.getPosition().y - king.sprite.getPosition().y))==size))
                    return true;
                break;
            case 3:
                if (!diagonalMovementCollision(king.sprite.getPosition(), rivalPieces[i].sprite.getPosition()) && abs(int(king.sprite.getPosition().x - rivalPieces[i].sprite.getPosition().x))==abs(int(king.sprite.getPosition().y - rivalPieces[i].sprite.getPosition().y)))
                    return true;
                break;
            case 4:
                if ((!lineMovementCollision(king.sprite.getPosition().y, rivalPieces[i].sprite.getPosition().y, rivalPieces[i].sprite.getPosition(), 1) && rivalPieces[i].sprite.getPosition().x==king.sprite.getPosition().x) || (!lineMovementCollision(king.sprite.getPosition().x, rivalPieces[i].sprite.getPosition().x, rivalPieces[i].sprite.getPosition(), 0) && rivalPieces[i].sprite.getPosition().y==king.sprite.getPosition().y) || (!diagonalMovementCollision(king.sprite.getPosition(), rivalPieces[i].sprite.getPosition())  && abs(int(king.sprite.getPosition().x - rivalPieces[i].sprite.getPosition().x))==abs(int(king.sprite.getPosition().y - rivalPieces[i].sprite.getPosition().y))))
                    return true;
                break;
            case 5:
                // King can't be bothered to put other king in check
                break;
            case 6:
                if (abs(int(rivalPieces[i].sprite.getPosition().x-king.sprite.getPosition().x))==size && (rivalPieces[i].sprite.getPosition().y-king.sprite.getPosition().y)==size*pawnDirection)
                    return true;
                break;
        }
    }
    return false;
}

// Check validity of a move. Function takes in figure, starting position and it's ending position
bool validMove(chessSprite figure, Vector2f oldPos, Vector2f newPos) {
    bool isValid = false;
    int pawnDirection;
    
    // In which directions should pawn move
    if (figure.isWhite == true) {
        pawnDirection = 1;
    } else {
        pawnDirection = -1;
    }
    
    chessSprite destinationFigure;
    
    for (int i=0; i<32; i++)
        if (f[i].sprite.getPosition()==newPos) destinationFigure = f[i];
    
    // If there is a figure and it is not opposing, prevent capture. Same for any king piece.
    if ((abs(destinationFigure.value) < 7 && figure.isWhite == destinationFigure.isWhite) || abs(destinationFigure.value)==5)
    {
        return false;
    }
    
    switch(abs(figure.value)) {
        case 1:
            if (abs(int(oldPos.x - newPos.x))==0 && abs(int(oldPos.y - newPos.y))>0) {
                if (lineMovementCollision(oldPos.y, newPos.y, oldPos, 1))
                    return false;
                isValid = true;
            } else if (abs(int(oldPos.y - newPos.y))==0 && abs(int(oldPos.x - newPos.x))>0) {
                if (lineMovementCollision(oldPos.x, newPos.x, oldPos, 0))
                    return false;
                isValid = true;
            } else {
                isValid = false;
            }
            break;
        case 2:
            if (abs(int(oldPos.x - newPos.x))==size && abs(int(oldPos.y - newPos.y))==size*2) {
                 isValid = true;
            } else if (abs(int(oldPos.x - newPos.x))==2*size && abs(int(oldPos.y - newPos.y))==size) {
                 isValid = true;
            } else {
                isValid = false;
            }
            break;
        case 3:
            if (abs(int(oldPos.x - newPos.x))==abs(int(oldPos.y - newPos.y))) {
                if (diagonalMovementCollision(oldPos, newPos))
                    return false;
                isValid = true;
            } else {
                isValid = false;
            }
            break;
        case 4:
            if (abs(int(oldPos.x - newPos.x))==0 && abs(int(oldPos.y - newPos.y))>0) {
                if (lineMovementCollision(oldPos.y, newPos.y, oldPos, 1))
                    return false;
                isValid = true;
            } else if (abs(int(oldPos.y - newPos.y))==0 && abs(int(oldPos.x - newPos.x))>0) {
                if (lineMovementCollision(oldPos.x, newPos.x, oldPos, 0))
                    return false;
                isValid = true;
            } else if (abs(int(oldPos.x - newPos.x))==abs(int(oldPos.y - newPos.y))) {
                if (diagonalMovementCollision(oldPos, newPos))
                    return false;
                isValid = true;
            } else {
                isValid = false;
            }
            break;
        case 5:
            if (abs(int(oldPos.x - newPos.x))<=size && abs(int(oldPos.y - newPos.y))<=size) {
                isValid = true;
            } else if (abs(int(oldPos.x - newPos.x))==2*size && abs(int(oldPos.y - newPos.y))==0 && figure.moves==0) {
                int multiplier = (oldPos.x - newPos.x) < 0 ? -1 : 1;
                if (lineMovementCollision(oldPos.x, newPos.x+2*size*multiplier, oldPos, 0))
                    return false;
                
                
                // Check for possibility of castling
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
                // If the rook hasn't moved
                for (int i=0; i<32; i++)
                    if (f[i].sprite.getPosition()==castleCandidate && f[i].moves == 0) isValid = true;
                
            } else {
                isValid = false;
            }
            break;
        case 6:
            if ((oldPos.y - newPos.y)==size*pawnDirection || ((oldPos.y - newPos.y)==size*pawnDirection*2 && figure.moves==0)) {
                if (abs(int(oldPos.x - newPos.x))>size)
                {
                    isValid = false;
                } else if (abs(int(oldPos.x - newPos.x))==size) {
                    if (abs(destinationFigure.value) < 7 && abs(destinationFigure.value)!=5)
                        isValid = true;
                    else {
                        // Check for possibility of en passant. If it is the last move opponent played.
                        Vector2f enPassantPos = Vector2f(newPos.x, newPos.y+size*pawnDirection);
                        std::string lastPosString = position.substr( position.length() - 5 );
                        Vector2f startingPos = toCoord(lastPosString[0], lastPosString[1]);
                        Vector2f lastPos = toCoord(lastPosString[2], lastPosString[3]);
                        
                        if (enPassantPos == lastPos && abs(int(startingPos.y - lastPos.y))==2*size) {
                            for (int i=0; i<32; i++)
                                if (f[i].sprite.getPosition()==enPassantPos) f[i].sprite.setPosition(-100, -100);
                            isValid = true;
                        } else {
                            isValid = false;
                        }
                    }
                } else {
                    // Pawns can't eat in straight line, only diagonal
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

bool moveInLine(chessSprite piece, int step, int dx, int dy, bool colour) {
    int beginning;
    if (dx!=0) {
        beginning = piece.sprite.getPosition().x+step;
    } else {
        beginning = piece.sprite.getPosition().y+step;
    }
    for (int j=beginning; j<8*size && j>=0; j+=step){
        Vector2f initialPosition = piece.sprite.getPosition();
        if (validMove(piece, initialPosition, Vector2f(dy*initialPosition.x+(dx*j), dx*initialPosition.y+(dy*j)))) {
            int movingPieceIndex=-2;
            for (int l=0; l<32; l++) {
                if(f[l].sprite.getPosition()==initialPosition)
                    movingPieceIndex = l;
            }
            f[movingPieceIndex].sprite.setPosition(dy*initialPosition.x+(dx*j), dx*initialPosition.y+(dy*j));
            int destinationPieceIndex=-2;
            bool isCaptured=false;
            for (int i=0; i<32; i++) {
                if (f[i].sprite.getPosition()==Vector2f(dy*initialPosition.x+(dx*j), dx*initialPosition.y+(dy*j)) && f[i].isWhite!=piece.isWhite){
                    isCaptured=true;
                    destinationPieceIndex=i;
                    f[destinationPieceIndex].sprite.setPosition(-100, -100);
                }
            }
            if (!check(colour)){
                f[movingPieceIndex].sprite.setPosition(initialPosition);
                if (isCaptured)
                    f[destinationPieceIndex].sprite.setPosition(Vector2f(dy*initialPosition.x+(dx*j), dx*initialPosition.y+(dy*j)));
                return true;
            }
            f[movingPieceIndex].sprite.setPosition(initialPosition);
            if (isCaptured)
                f[destinationPieceIndex].sprite.setPosition(Vector2f(dy*initialPosition.x+(dx*j), dx*initialPosition.y+(dy*j)));
        }
        else
            break;
    }
    
    return false;
}

bool moveDiagonal(chessSprite piece, int dx, int dy, bool colour) {
    int beginningX = piece.sprite.getPosition().x;
    int beginningY = piece.sprite.getPosition().y;
    Vector2f initialPosition = piece.sprite.getPosition();
    
    while (beginningX<8*size && beginningX>=0 && beginningY<8*size && beginningY>=0) {
        beginningX+=dx*size;
        beginningY+=dy*size;
        if (validMove(piece, initialPosition, Vector2f(beginningX, beginningY))) {
                int movingPieceIndex=-2;
                for (int l=0; l<32; l++) {
                    if(f[l].sprite.getPosition()==initialPosition)
                        movingPieceIndex = l;
                }
                f[movingPieceIndex].sprite.setPosition(beginningX, beginningY);
                int destinationPieceIndex=-2;
                bool isCaptured=false;
                for (int i=0; i<32; i++) {
                    if (f[i].sprite.getPosition()==Vector2f(beginningX, beginningY) && f[i].isWhite!=piece.isWhite){
                        isCaptured=true;
                        destinationPieceIndex=i;
                        f[destinationPieceIndex].sprite.setPosition(-100, -100);
                    }
                }
                if (!check(colour)){
                    f[movingPieceIndex].sprite.setPosition(initialPosition);
                    if (isCaptured)
                        f[destinationPieceIndex].sprite.setPosition(Vector2f(beginningX, beginningY));
                    return true;
                }
                f[movingPieceIndex].sprite.setPosition(initialPosition);
                if (isCaptured)
                    f[destinationPieceIndex].sprite.setPosition(Vector2f(beginningX , beginningY));
            }
            else
                break;
        }
    
    return false;
}

bool getValidMoves(bool colour) {
    chessSprite king;
    for (int i=0; i<32; i++) {
        if (abs(f[i].value) == 5 && f[i].isWhite==colour) {
            king = f[i];
        }
    }
    
    chessSprite possiblePieces[16];
    
    int m=0;
    for (int i=0; i<32; i++){
        if (f[i].isWhite==king.isWhite && f[i].sprite.getPosition()!=Vector2f(-100, -100)) {
            possiblePieces[m] = f[i];
            m++;
        }
    }
    
    for (int i=0; i<m; i++){
        switch(abs(possiblePieces[i].value)) {
            case 1:
                if (moveInLine(possiblePieces[i], size, 1, 0, colour))
                    return true;
                if (moveInLine(possiblePieces[i], size, 0, 1, colour))
                    return true;
                if (moveInLine(possiblePieces[i], -1*size, 1, 0, colour))
                    return true;
                if (moveInLine(possiblePieces[i], -1*size, 0, 1, colour))
                    return true;
                break;
            case 2:
                for (int k = -2; k<=2; k++){
                    for (int j = -2; j<=2; j++) {
                        if (k==0 || j==0 || abs(j)==abs(k))
                            continue;
                        Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
                        if (initialPosition.x+k*size<8*size && initialPosition.x+k*size>=0 && initialPosition.y+j*size<8*size && initialPosition.y+j*size>=0)
                            if (validMove(possiblePieces[i], initialPosition, Vector2f(initialPosition.x+k*size, initialPosition.y+j*size))) {
                                int movingPieceIndex=-2;
                                for (int l=0; l<32; l++) {
                                    if(f[l].sprite.getPosition()==initialPosition)
                                        movingPieceIndex = l;
                                }
                                f[movingPieceIndex].sprite.setPosition(initialPosition.x+k*size, initialPosition.y+j*size);
                                if (!check(colour)){
                                    f[movingPieceIndex].sprite.setPosition(initialPosition);
                                    return true;
                                }
                                f[movingPieceIndex].sprite.setPosition(initialPosition);
                            }
                    }
                }
                break;
            case 3:
                if (moveDiagonal(possiblePieces[i], 1, 1, colour))
                    return true;
                if (moveDiagonal(possiblePieces[i], 1, -1, colour))
                    return true;
                if (moveDiagonal(possiblePieces[i], -1, 1, colour))
                    return true;
                if (moveDiagonal(possiblePieces[i], -1, -1, colour))
                    return true;
                break;
            case 4:
                if (moveInLine(possiblePieces[i], size, 1, 0, colour))
                    return true;
                if (moveInLine(possiblePieces[i], size, 0, 1, colour))
                    return true;
                if (moveInLine(possiblePieces[i], -1*size, 1, 0, colour))
                    return true;
                if (moveInLine(possiblePieces[i], -1*size, 0, 1, colour))
                    return true;
                if (moveDiagonal(possiblePieces[i], 1, 1, colour))
                    return true;
                if (moveDiagonal(possiblePieces[i], 1, -1, colour))
                    return true;
                if (moveDiagonal(possiblePieces[i], -1, 1, colour))
                    return true;
                if (moveDiagonal(possiblePieces[i], -1, -1, colour))
                    return true;
                break;
            case 5:
                for (int k = -1; k<=1; k++){
                    for (int j = -1; j<=1; j++) {
                        if (k==0 && j==0)
                            continue;
                        Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
                        if (initialPosition.x+k*size<8*size && initialPosition.x+k*size>=0 && initialPosition.y+j*size<8*size && initialPosition.y+j*size>=0)
                            if (validMove(possiblePieces[i], initialPosition, Vector2f(initialPosition.x+k*size, initialPosition.y+j*size))) {
                                int movingPieceIndex=-2;
                                for (int l=0; l<32; l++) {
                                    if(f[l].sprite.getPosition()==initialPosition)
                                        movingPieceIndex = l;
                                }
                                f[movingPieceIndex].sprite.setPosition(initialPosition.x+k*size, initialPosition.y+j*size);
                                if (!check(colour)){
                                    f[movingPieceIndex].sprite.setPosition(initialPosition);
                                    return true;
                                }
                                f[movingPieceIndex].sprite.setPosition(initialPosition);
                            }
                    }
                }
                break;
            case 6:
                int pawnDirection;
                
                // In which directions should pawn move
                if (possiblePieces[i].isWhite == false) {
                    pawnDirection = 1;
                } else {
                    pawnDirection = -1;
                }
                for (int j=1; j<=2; j++) {
                    if (j==1) {
                        for (int k=-1; k<=1; k++) {
                            
                            Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
                            if (initialPosition.x+k*size<8*size && initialPosition.x+k*size>=0 && initialPosition.y+j*size*pawnDirection<8*size)
                                if (validMove(possiblePieces[i], initialPosition, Vector2f(initialPosition.x+k*size, initialPosition.y+j*size*pawnDirection))) {
                                    int movingPieceIndex=-2;
                                    for (int l=0; l<32; l++) {
                                        if(f[l].sprite.getPosition()==initialPosition)
                                            movingPieceIndex = l;
                                    }
                                    f[movingPieceIndex].sprite.setPosition(initialPosition.x+k*size, initialPosition.y+j*size*pawnDirection);
                                    if (!check(colour)){
                                        f[movingPieceIndex].sprite.setPosition(initialPosition);
                                        return true;
                                    }
                                    f[movingPieceIndex].sprite.setPosition(initialPosition);
                                    
                                }
                        }
                    } else {
                        Vector2f initialPosition = possiblePieces[i].sprite.getPosition();
                        if (initialPosition.y+j*size<8*size)
                            if (validMove(possiblePieces[i], initialPosition, Vector2f(initialPosition.x, initialPosition.y+j*size*pawnDirection))) {
                                int movingPieceIndex=-2;
                                for (int l=0; l<32; l++) {
                                    if(f[l].sprite.getPosition()==initialPosition)
                                        movingPieceIndex = l;
                                }
                                f[movingPieceIndex].sprite.setPosition(initialPosition.x, initialPosition.y+j*size*pawnDirection);
                                if (!check(colour)){
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

int main()
{
    // Render SFML main window
    RenderWindow window(VideoMode(908, 910), "The Chess!", Style::Close);

    // Setup chess board and pieces
    Texture t1, t2;
    t1.loadFromFile("images/figures.png");
    t2.loadFromFile("images/board.png");
    
    Sprite s(t1);
    Sprite sBoard(t2);
    
    for (int i=0; i<32; i++) f[i].sprite.setTexture(t1);
    
    loadPosition();
    
    bool isMove = false;
    float dx=0, dy=0;
    Vector2f oldPos, newPos;
    std::string str;
    int n=0;

    while (window.isOpen()) {
        Vector2i pos = Mouse::getPosition(window);
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();
            
            if (e.type == Event::KeyPressed) {
                // Undo function
                if (e.key.code == Keyboard::BackSpace)
                {
                    if (position.length() > 5 )
                    {
                        position.erase(position.length()-6,5);
                        promotionIndex = 0;
                        loadPosition();
                    }
                }
            }
            
            // Piece movement
            if (e.type == Event::MouseButtonPressed)
                if (e.key.code == Mouse::Left)
                    for (int i=0; i<32; i++)
                        if (f[i].sprite.getGlobalBounds().contains(pos.x, pos.y))
                        {
                            isMove = true;
                            n = i;
                            dx = pos.x - f[i].sprite.getPosition().x;
                            dy = pos.y - f[i].sprite.getPosition().y;
                            oldPos = f[i].sprite.getPosition();
                        }
            
            if (e.type == Event::MouseButtonReleased)
                if (e.key.code == Mouse::Left)
                 {
                     isMove = false;
                     Vector2f p = f[n].sprite.getPosition() + Vector2f(size/2, size/2);
                     Vector2f newPos = Vector2f( size*int(p.x/size), size*int(p.y/size) );
                     if (validMove(f[n], oldPos, newPos)) {
                         str = toChessNote(oldPos)+toChessNote(newPos);
                         
                         
                         // Should the pawn reach opposite side backline, open promotion window
                         if ((f[n].value==6 && str[3]=='8') || (f[n].value==-6 && str[3]=='1')){
                             RenderWindow window2(VideoMode(454, 120), "Promote pawn to:", Style::Titlebar);
                             for(int i=0; i<4; i++){
                                 int pieces;
                                 if (f[n].value < 0)
                                     pieces = promoteBlack[i];
                                 else
                                     pieces = promoteWhite[i];
                                 int x = abs(pieces) - 1;
                                 bool isWhite = pieces > 0;
                                 int y = isWhite ? 1 : 0;
                                 promotionPieces[i].sprite.setTexture(t1);
                                 promotionPieces[i].sprite.setTextureRect(IntRect(size*x, size*y, size, size));
                                 promotionPieces[i].sprite.setPosition(size*i, 0);
                                 promotionPieces[i].value = pieces;
                             }
                               
                             while (window2.isOpen())
                             {
                                 Event event;
                                 Vector2i pos2 = Mouse::getPosition(window2);
                                 while (window2.pollEvent(event))
                                 {
                                     if (event.type == Event::Closed)
                                         window2.close();
                                     
                                     if (event.type == Event::MouseButtonPressed)
                                         if (event.key.code == Mouse::Left)
                                             for (int i=0; i<4; i++)
                                                 if (promotionPieces[i].sprite.getGlobalBounds().contains(pos2.x, pos2.y))
                                                 {
                                                     int pieces;
                                                     if (f[n].value < 0)
                                                         pieces = promoteBlack[i];
                                                     else
                                                         pieces = promoteWhite[i];
                                                     int x = abs(pieces) - 1;
                                                     bool isWhite = pieces > 0;
                                                     int y = isWhite ? 1 : 0;
                                                     f[n].sprite.setTextureRect(IntRect(size*x, size*y, size, size));
                                                     f[n].value = pieces;
                                                     promotions[promotionIndex] = pieces;
                                                     promotionIndex += 1;
                                                     window2.close();
                                                 }
                                 }
                                     window2.clear(Color(13, 13, 13));
                                     for (int i=0; i<4; i++)
                                         window2.draw(promotionPieces[i].sprite);
                                     window2.display();
                             }
                               
                         }
                         int destinationIndex = -2;
                         for (int l=0; l<32; l++) {
                             if (f[l].sprite.getPosition()==newPos)
                                 destinationIndex = l;
                         }
                         move(str, "");
                         f[n].sprite.setPosition(newPos);
                         if(!check(f[n].isWhite)) {
                             f[n].moves += 1;
                             position += str + " ";
                             std::cout<<str<<std::endl;
                         } else {
                             f[n].sprite.setPosition(oldPos);
                             if (destinationIndex!= -2){
                                 f[destinationIndex].sprite.setPosition(newPos);
                             }
                         }
                         if (check(!f[n].isWhite) && getValidMoves(!f[n].isWhite)) {
                             std::cout<<"CHECK"<<std::endl;
                             for (int l=0; l<32;l++) {
                                 if (abs(f[l].value)==5 && f[l].isWhite!=f[n].isWhite) {
                                     f[l].sprite.setColor(Color(100,0,0));
                                 }
                             }
                         } else {
                             if (!check(f[n].isWhite)) {
                                 Color usualColor = Color(100,100,100);
                                 for (int l=0; l<32;l++) {
                                     if (abs(f[l].value)==1 && f[l].isWhite==f[n].isWhite) {
                                         usualColor = f[l].sprite.getColor();
                                     }
                                     if (abs(f[l].value)==5 && f[l].isWhite==f[n].isWhite) {
                                         f[l].sprite.setColor(usualColor);
                                     }
                                 }
                             }
                         }
                         if (check(!f[n].isWhite) && !getValidMoves(!f[n].isWhite)) {
                             std::cout<<"CHECK MATE"<<std::endl;
                             for (int l=0; l<32;l++) {
                                 if (abs(f[l].value)==5 && f[l].isWhite!=f[n].isWhite) {
                                     f[l].sprite.setColor(Color(100,0,100));
                                 }
                             }
                         } else {
                             if (!check(f[n].isWhite)) {
                                 Color usualColor = Color(100,100,100);
                                 for (int l=0; l<32;l++) {
                                     if (abs(f[l].value)==1 && f[l].isWhite==f[n].isWhite) {
                                         usualColor = f[l].sprite.getColor();
                                     }
                                     if (abs(f[l].value)==5 && f[l].isWhite==f[n].isWhite) {
                                         f[l].sprite.setColor(usualColor);
                                     }
                                 }
                             }
                         }
                         if (!check(!f[n].isWhite) && !getValidMoves(!f[n].isWhite)) {
                             std::cout<<"STALEMATE"<<std::endl;
                             for (int l=0; l<32;l++) {
                                 if (abs(f[l].value)==5) {
                                     f[l].sprite.setColor(Color(100,100,10));
                                 }
                             }
                         }
                     } else {
                         f[n].sprite.setPosition(oldPos);
                     }
                 }
            
            // dragging
            if (isMove) f[n].sprite.setPosition(pos.x - dx, pos.y - dy);
        }

        window.clear();
        window.draw(sBoard);
        for (int i=0; i<32; i++) window.draw(f[i].sprite);
        window.draw(f[n].sprite);
        window.display();
    }
    
    return 0;
}
