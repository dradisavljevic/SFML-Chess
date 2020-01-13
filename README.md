# SFML Chess

Simple chess game written in C++ with the help of SFML library. Game has no AI behind it and is inspired by the videos uploaded by [FamTrinli](https://www.youtube.com/user/FamTrinli) on YouTube. Assets also come from FamTrinli's project, while most of the code is original and is based on enforicing the rules of Chess.

## Overview
Chess is a two-player strategy board game played on a checkered board with 64 squares arranged in an 8×8 grid. It is probably the first board game I got in touch with as a kid and something that I really enjoyed playing with my grandfather. This version of chess can be played as a two player game, as it has no AI behind the opposing pieces. There are restrictions that prevent player from making non-valid moves and that enforces players to play in turns, so it might be a good exercise. If the mistake has been made during the game or suboptimal move has been played, it is possible to Undo a move using the Backspace key. It is currently impossible to undo the opening move.

Game is finished when either a checkmate (indicated by the king piece being colored purple) or stalemate (both king pieces change color to greenish yellow) occur, after which you should manually close the game or backtrack couple of moves using the undo button. Restarting the whole game requires program to be run again.

## Installation
Before running the code, make sure you have the [SFML](https://github.com/SFML/SFML) library installed. Instructions regarding the installation can be found [here](https://www.sfml-dev.org/tutorials/2.5/).

After installing the library, you can compile code using G++ with the following command on MacOS or Linux:

```
g++ main.cpp -o sfml-chess -lsfml-graphics -lsfml-window -lsfml-system
```

And then running it with:

```
./sfml-chess
```

And voila, you can play chess. Have fun.

## Future improvemnts
The most obvious one would be adding some AI to control the opposing pieces, as well as giving the user option of picking a side should the AI exist. Other improvements can be adding something like Main Menu and making the in-game messages more informative to the users and not rely simply on color and window titlebar. One other improvement, possibly the easiest one, is allowing the opening move to be undone.
