#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>

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

int promoteWhite[4] = {1,2,3,4};

int promoteBlack[4] = {-1, -2, -3, -4};

int promotions[16];

int promotionIndex = 0;

std::string position = "";

Vector2f toCoord(char a, char b) {
    int x = int(a) - 97;
    int y = 7 - int(b) + 49;
    return Vector2f(x*size, y*size);
}

void move(std::string str) {
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
        }
    
    if (str=="e1g1") if (position.find("e1")==-1) move("h1f1");
    if (str=="e8g8") if (position.find("e8")==-1) move("h8f8");
    if (str=="e1c1") if (position.find("e1")==-1) move("a1d1");
    if (str=="e8c8") if (position.find("e8")==-1) move("a8d8");
}

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
    
    for (int i=0; i<position.length(); i+=5)
        move(position.substr(i,4));
        
}

std::string toChessNote(Vector2f p) {
    std::string s = "";
    s += char(p.x/size+97);
    s += char(7-p.y/size+49);
    return s;
}

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

bool validMove(chessSprite figure, Vector2f oldPos, Vector2f newPos) {
    bool isValid = false;
    int pawnDirection;
    
    if (figure.isWhite == true) {
        pawnDirection = 1;
    } else {
        pawnDirection = -1;
    }
    
    chessSprite destinationFigure;
    
    for (int i=0; i<32; i++)
        if (f[i].sprite.getPosition()==newPos) destinationFigure = f[i];
    
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
                if (lineMovementCollision(oldPos.x, newPos.x, oldPos, 0))
                    return false;
                isValid = true;
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
                    else
                        isValid = false;
                } else {
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

int main()
{
    RenderWindow window(VideoMode(908, 910), "The Chess!", Style::Close);

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
                         
                 
                         if ((f[n].value==6 && str[3]=='8') || (f[n].value==-6 && str[3]=='1')){
                             RenderWindow window2(VideoMode(454, 120), "Promote pawn to:", Style::Close);
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
                                                    std::cout<<promotionPieces[i].value<<std::endl;
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
                         move(str);
                         position += str + " ";
                         std::cout<<str<<std::endl;
                         f[n].sprite.setPosition(newPos);
                         f[n].moves += 1;
                     } else {
                         f[n].sprite.setPosition(oldPos);
                     }
                 }
            
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
