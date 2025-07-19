#include <iostream>
#include <utility>
#include <stdlib.h>
#include <time.h>
#include "E:\\Documents\\CodeStuff\\C++\\Headers\\LateStageGraphicalism\\ConGraphicsFinal.h"
#include <windows.h>

using namespace std;

//Function declarations
void GetInput();
void (*KeyEventProc)(KEY_EVENT_RECORD);
void KbStartMenu(KEY_EVENT_RECORD);
void KbGame(KEY_EVENT_RECORD);

void Display();

bool CheckLost();
char CheckCollide(int,int);
void CheckForLines();
void ClearLine(int);
void EmplacePiece();
void MovePiece(int,int);
void RotatePiece(int);
void SpawnPiece(int);

//Global variables (Should perhaps change this around and use structs instead)
const int boardY = 30;
const int boardX = 10;
bool gameBoard[boardX][boardY];

int difficultySelect;
int frameRate;

int score = 0;
bool lost = false;
bool exitMenu;
bool quit;

int inputDelay = 1;

struct Piece
{
    pair<int,int> coords[4];

    Piece(int x,int y,int shape)
    {
        switch(shape)
        {
        case 0: //Square
            coords[0] = make_pair(x,y);
            coords[1] = make_pair(x+1,y);
            coords[2] = make_pair(x,y+1);
            coords[3] = make_pair(x+1,y+1);
        break;
        case 1: //Line
            coords[0] = make_pair(x,y);
            coords[1] = make_pair(x,y+1);
            coords[2] = make_pair(x,y+2);
            coords[3] = make_pair(x,y+3);
        break;
        case 2: //T
            coords[0] = make_pair(x,y);
            coords[1] = make_pair(x,y+1);
            coords[2] = make_pair(x+1,y+1);
            coords[3] = make_pair(x-1,y+1);
        break;
        case 3: //L
            coords[0] = make_pair(x,y);
            coords[1] = make_pair(x,y+1);
            coords[2] = make_pair(x,y+2);
            coords[3] = make_pair(x+1,y+2);
        break;

        }
    }
}*activePiece;

char CheckCollide(int x, int y)
{
    /*
    Checks if the active piece is touching a filled space or touching/beyond the edge of the board.
    The check will offset from the active piece's position by int x/y.
    Thus inputting 0,0 will check at the pieces current position. Inputting 1,0 or -1,0 would
    check to the right/left of the piece.
    */
    for(int i = 0; i < 4; i++)
    {
        if(activePiece->coords[i].first + x < 0) //Touching left-side of board.)
        {
            return 'L';
        }
        if(activePiece->coords[i].first + x >= boardX) //Touching right-side of board.
        {
            return 'R';
        }
        if(activePiece->coords[i].second + y >= boardY) //Touching bottom of board.)
        {
            return 'B';
        }
        /* Ensure checked coordinate lies inside the board before checking for filled spaces
        otherwise the program crashes on the if-statement after the one below. */
        if(activePiece->coords[i].second >= 0)
        {
            if(gameBoard[activePiece->coords[i].first + x][activePiece->coords[i].second + y]) //Touching filled board space.
            {
                return 'F';
            }
        }
    }
    return 0;
}

void MovePiece(int x = 0,int y = 0)
{
    /*
    Moves active piece by x/y distance. Generally
    you want to run collide checks with this
    */
    for(int i = 0; i < 4; i++)
    {
        activePiece->coords[i].first += x;
        activePiece->coords[i].second += y;
    }
}

void RotatePiece(int direction) //Add bounds-checking to this
{
    //Consider the origin to be the first coordinate in the piece
    int originX = activePiece->coords[0].first;
    int originY = activePiece->coords[0].second;

    for(int i = 0; i < 4; i++)//Loop through each point and calculate the rotated positions
    {
        //Temp variables for the sake of not polluting the actual coords during calculation
        int x = activePiece->coords[i].first;
        int y = activePiece->coords[i].second;

        //Calculate rotation with translated origins
        //Uses a dumbed down rotation formula for 90 degree intervals
        x = (activePiece->coords[i].second - originY) * (-1*direction) + originX;
        y = (activePiece->coords[i].first - originX) * (1*direction) + originY;

        //Assign post-calculated temp values to actual piece coordinates
        activePiece->coords[i].first = x;
        activePiece->coords[i].second = y;
    }
    //If the rotated piece extends out of bounds shunt it back into the board.
    while(CheckCollide(0,0) == 'L')
    {
        MovePiece(1);
    }
    while(CheckCollide(0,0) == 'R')
    {
        MovePiece(-1);
    }
}

void ClearLine(int level)
{
    for(int x = 0; x < boardX; x++) //Loop through x coords for input y-level and clear them
    {
        gameBoard[x][level] = false;
    }
    for(int y = level; y >= 0; y--) //Loop upwards from level of the line
    {
        for(int x = 0; x < boardX; x++) //Loop through each x-space
        {
            if(gameBoard[x][y]) //If there was a filled space move it down 1 unit
            {
                gameBoard[x][y] = false;
                gameBoard[x][y+1] = true;
            }
        }
    }
    score++;
}

void CheckForLines()
{
    bool lineFound;
    for(int y = 0; y < boardY; y++) //Loop through each y-level
    {
        lineFound = true; //Set flag to true, if no empty space is found on the following row it will stay set to true
        for(int x = 0; x < boardX; x++) //Loop through each x for current y-level
        {
            if(!gameBoard[x][y]) //If an empty space was found there isn't a line on that level
            {
                lineFound = false; //Flag that no line exists on that y-level
                break; //Terminate x checking loop and advance to next y-level
            }
        }
        if(lineFound)
        {
            ClearLine(y);
        }
    }
}

bool CheckLost()
{
    /*
    Check for lose conditions by seeing if a piece would be above the board. Generally would
    run this function at the time it is about to be emplaced to the board.
    */
    for(int i = 0; i < 4; i++)
    {
        if(activePiece->coords[i].second <= 0)
        {
            return true;
        }
    }
    return false;
}

void EmplacePiece()
{
    /*
    Push the active piece to the game board at it's current position.
    Generally would run this function after a vertical collision has occured.
    */
    for(int i = 0; i < 4; i++)
    {
        gameBoard[activePiece->coords[i].first][activePiece->coords[i].second] = true;
    }
}

void SpawnPiece(int shape)
{
    //Create a new game piece above the board at a random X-coordinate.
    activePiece = new Piece(rand() % boardX,-5,shape);

    //If the created piece extends out of bounds shunt it back into the board.
    while(CheckCollide(0,0) == 'L')
    {
        MovePiece(1);
    }
    while(CheckCollide(0,0) == 'R')
    {
        MovePiece(-1);
    }

}

void StartMenu()
{
    exitMenu = false;

    ClearConsole();
    KeyEventProc = KbStartMenu;
    cout << "Welcome to Tetris!\nPress number keys to select difficulty.\n";
    cout << "1. Easy\n2. Medium\n3. Hard";
    while(!exitMenu)
    {
        if(inputDelay > 2)
        {
            GetInput();
            inputDelay = 1;
        }

        inputDelay++;
        Sleep(200);
    }
    ClearConsole();
    frameRate = 500/difficultySelect;
    KeyEventProc = KbGame;
}

void LoseMenu()
{
    Sleep(1000);
    ClearConsole();
    cout << "Ya lost.\nPlay again? [Y/N]";
    char selection;
    cin >> selection;
    if(selection == 'Y' || selection == 'y')
    {
        cout << "Nice.";
        Sleep(1000);

        lost = false;//Reset lose condition flag.
        score = 0;//Reset score counter.

        //Clear board.
        for(int y = 0; y < boardY; y++) //Loop through each y-level.
        {
            for(int x = 0; x < boardX; x++) //Loop through each x-space.
            {
                gameBoard[x][y] = false;
            }
        }
    }
    else
    {
        cout << "\nK, bye!";
        Sleep(1000);
        quit = true;
    }
}

void Display()
{
    static Image* screen;
    static Image board(boardX,boardY);
    if(!screen)
    {
        screen = new Image(boardX+2,boardY+2);
        //Draw border around game board.
        screen->InsertPlot(GenerateSquare(0,0,boardX+1,boardY+1,BACKGROUND_GREEN));
    }
    //Draw game board to image.
    for(int y = 0; y < boardY; y++)
    {
        for(int x = 0; x < boardX; x++)
        {
            if(gameBoard[x][y])
            {
                board.Draw(x,y,BACKGROUND_WHITE);
            }
            else
            {
                board.Draw(x,y,0);
            }
        }
    }
    screen->InsertImage(board,1,1); //Push game board image to screen image.

    for(int i = 0; i < 4; i++) //Draw active piece on above-layer.
    {
        if(activePiece->coords[i].second >= 0)
        {
            screen->Draw(activePiece->coords[i].first+1,activePiece->coords[i].second+1,BACKGROUND_RED);
        }
    }

    screen->Display(2,1); //Display with 2x1 pixel size.

    SetConsoleCursorPosition(screen->consoleOut,{0,boardY+2});
    cout << "Score: " << score;
}

void KbStartMenu(KEY_EVENT_RECORD k)
{
    if(k.bKeyDown)
    {
        switch(k.wVirtualKeyCode)
        {
        case '1':
            difficultySelect = 1;
            exitMenu = true;
        break;
        case '2':
            difficultySelect = 3;
            exitMenu = true;
        break;
        case '3':
            difficultySelect = 5;
            exitMenu = true;
        break;
        }
    }
}

void KbGame(KEY_EVENT_RECORD k)
{
    if(k.bKeyDown)
    {
    switch(k.wVirtualKeyCode)
    {
    case 'A':
        //Move left if there is open space to do so.
        if(!CheckCollide(-1,0))
        {
            MovePiece(-1);
        }
    break;
    case 'D':
        //Move right if there is open space to do so.
        if(!CheckCollide(1,0))
        {
            MovePiece(1);
        }
    break;
    case 'W':
        //Rotate clockwise
        RotatePiece(1);
        //If rotated position intersects with a filled-space, undo it.
        if(CheckCollide(0,0))
        {
            RotatePiece(-1);
        }
    break;
    case 'S':
        //Rotate counter-clockwise
        RotatePiece(-1);
        //If rotated position intersects with a filled-space, undo it.
        if(CheckCollide(0,0))
        {
            RotatePiece(1);
        }
    break;
    default:
        return;
    break;
    }
    }
}

void GetInput(){ //Does what it says on the tin
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD inputBuffer[1];
    DWORD nRead;
    GetNumberOfConsoleInputEvents(hInput,&nRead); //Check if any input was entered
    if(nRead > 0) //If it was process that input
    {
        ReadConsoleInput(hInput,inputBuffer,1,&nRead); //Load buffer array from console buffer
        for(int a=0;a<(int)nRead;a++){ //Loop through input buffer
            switch(inputBuffer[a].EventType){ //Call appropriate event functions
                    case KEY_EVENT:
                        KeyEventProc(inputBuffer[a].Event.KeyEvent);
                    break;
                    default:
                        return;
                    break;
            }
        }
        FlushConsoleInputBuffer(hInput); //Clear console input buffer before next reading
    }
}

int main()
{
    HWND hWindow = GetConsoleWindow();
    SetWindowPos(hWindow,HWND_TOP,0,0,500,700,SWP_NOMOVE);

    while(!quit)
    {
        StartMenu();
        srand (time(NULL));
        SpawnPiece(rand() % 3);
        Display();
        while(!lost) //Game loop
        {
            /*
            Loop through coordinates of active piece and emplace that piece to the board if the bottom of
            the board or a filled space is below it.
            */
            if(CheckCollide(0,1)) //Check for collisions underneath the active piece
            {
                if(CheckLost()) //Check for lose conditions (Piece would be emplaced above board)
                {
                    lost = true;
                }
                else //If lose condition is false
                {
                    EmplacePiece(); //Emplace piece to board
                    CheckForLines(); //Check if newly emplaced piece completes a line
                    SpawnPiece(rand() % 3); //Spawn a new piece
                }
            }
            else //Space under piece is empty so move it down.
            {
                MovePiece(0,1);
            }

            //Get input only when enough time has elapsed since previous input
            if(inputDelay >= 2)
            {
                GetInput();
                inputDelay = 1; //Reset input delay counter
            }

            //Update screen
            Display();

            //Update input delay counter
            inputDelay++;

            //Wait and loop to next frame
            Sleep(frameRate);
        }
        LoseMenu();
    }
    return 0;
}
