#include "raylib.h"
#include "raymath.h"
#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_TUBES 100
#define FLOPPY_RADIUS 24
#define TUBES_WIDTH 80

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct SusBird {
    Vector2 position;
    int radius;
    Color color;
    double speed;
} SusBird;

typedef struct Tubes {
    Rectangle rec;
    Color color;
    bool active;
} Tubes;

typedef struct Ground {
    Rectangle rec;
    Color color;
} Ground;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 1600;
static const int screenHeight = 900;

float scrollingBack = 0.0f;
float scrollingMid = 0.0f;
float scrollingFore = 0.0f;

static Texture2D background;
static Texture2D midground;
static Texture2D foreground;
static Texture2D tubeT;
static Texture2D tubeTbot;

static Rectangle ground;
static Rectangle ceiling;

static bool gameOver = false;
static bool pause = false;
static int score = 0;
static int hiScore = 0;

static SusBird susBird = { 0 };
static Tubes tubes[MAX_TUBES*2] = { 0 };
static Vector2 tubesPos[MAX_TUBES] = { 0 };
static int tubesSpeedX = 0;
static bool superfx = false;
//static Ground ground1 = { 0 };
//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Sus Bird Game");
    background = LoadTexture("resources/cyberpunk_street_background.png"); // hope and pray this is optimal
    midground = LoadTexture("resources/cyberpunk_street_midground.png");
    foreground = LoadTexture("resources/cyberpunk_street_foreground.png");
    tubeT = LoadTexture("resources/billboardA.png");
    tubeTbot = LoadTexture("resources/billboardA.png"); //whatever pic you want
    
    InitGame();
    InitAudioDevice();
    Music music = LoadMusicStream("resources/ps.mp3");

    PlayMusicStream(music);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(144);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        
        
        UpdateMusicStream(music);
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    //UnloadTexture(background);  // Unload background texture
    //UnloadTexture(midground);   // Unload midground texture
    //UnloadTexture(foreground);  // Unload foreground texture

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void)
{
    //ground stats (0, 900, screenWidth, 150)
    ground.width = screenWidth;
    ground.height = 150;
    ground.x = 0;
    ground.y = 900;
    ceiling.width = screenWidth;
    ceiling.height = 150;
    ceiling.x = 0;
    ceiling.y = -150;
    susBird.radius = FLOPPY_RADIUS;
    susBird.speed = 0.0;
    susBird.position = (Vector2){80, (screenHeight/2 - susBird.radius)-50};
    tubesSpeedX = 2.7;

    for (int i = 0; i < MAX_TUBES; i++)
    {
        tubesPos[i].x = 1000 + 280*i;
        tubesPos[i].y = GetRandomValue(-120, 240);// was 0, 120
    }

    for (int i = 0; i < MAX_TUBES*2; i += 2)
    {
        tubes[i].rec.x = tubesPos[i/2].x;
        tubes[i].rec.y = tubesPos[i/2].y - 255;
        tubes[i].rec.width = 1.5*TUBES_WIDTH;
        tubes[i].rec.height = 555;

        tubes[i+1].rec.x = tubesPos[i/2].x+30;
        tubes[i+1].rec.y = 700 + tubesPos[i/2].y - 255;
        tubes[i+1].rec.width = 1.5*TUBES_WIDTH;
        tubes[i+1].rec.height = 1555;// was 255

        tubes[i/2].active = true;
    }

    score = 0;

    gameOver = false;
    superfx = false;
    pause = false;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            for (int i = 0; i < MAX_TUBES; i++) tubesPos[i].x -= tubesSpeedX;

            for (int i = 0; i < MAX_TUBES*2; i += 2)
            {
                tubes[i].rec.x = tubesPos[i/2].x;
                tubes[i+1].rec.x = tubesPos[i/2].x;
            }

            if (IsKeyDown(KEY_SPACE) && !gameOver) {
                susBird.speed = 0;
                susBird.position.y -= 3;} // movement of sus ball
            else {
                susBird.speed+= 0.05;
                susBird.position.y += susBird.speed;
                } // speed used instead of static number (1)

            // Check Collisions
            for (int i = 0; i < MAX_TUBES*2; i++)
            {
                if (CheckCollisionCircleRec(susBird.position, susBird.radius, tubes[i].rec))
                {
                    gameOver = true;
                    pause = false;
                }
                /*else if (CheckCollisionCircleRec(susBird.position, susBird.radius, ground)) // use only if you want a game over here
                {
                    gameOver = true;
                    pause = false;
                }*/
                else if (CheckCollisionCircleRec(susBird.position, susBird.radius, ground)) // use if you want the ground to be solid.
                {
                    //susBird.speed = 0;
                    
                    susBird.position.y = screenHeight-susBird.radius;
                    susBird.speed = -susBird.speed;
                    pause = false;
                }
                else if (CheckCollisionCircleRec(susBird.position, susBird.radius, ceiling))
                {
                    gameOver = true;
                    pause = false;
                }
                else if ((tubesPos[i/2].x < susBird.position.x) && tubes[i/2].active && !gameOver)
                {
                    score += 100;
                    tubes[i/2].active = false;

                    superfx = true;

                    if (score > hiScore) hiScore = score;
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RED);
        
        
        
        
        
        if (!gameOver)
        {
            scrollingBack -= 0.1f;
            scrollingMid -= 0.5f;
            scrollingFore -= 1.0f;
            
            if (scrollingBack <= -background.width*2) scrollingBack = 0;
            if (scrollingMid <= -midground.width*2) scrollingMid = 0;
            if (scrollingFore <= -foreground.width*2) scrollingFore = 0;
            
                // Draw background image twice for smooth scrolling ofc
                DrawTextureEx(background, (Vector2){ scrollingBack, 0 }, 0.0f, 5.0f, RED);
                DrawTextureEx(background, (Vector2){ background.width*2 + scrollingBack, 0 }, 0.0f, 5.0f, RED);

                // Draw midground image twice
                DrawTextureEx(midground, (Vector2){ scrollingMid, 20 }, 0.0f, 5.0f, RED);
                DrawTextureEx(midground, (Vector2){ midground.width*2 + scrollingMid, 20 }, 0.0f, 5.0f, RED);

                // Draw foreground image twice
                DrawTextureEx(foreground, (Vector2){ scrollingFore, 70 }, 0.0f, 5.0f, WHITE);
                DrawTextureEx(foreground, (Vector2){ foreground.width*2 + scrollingFore, 70 }, 0.0f, 5.0f, WHITE);
                DrawCircle(susBird.position.x, susBird.position.y, susBird.radius, ORANGE);

            // Draw tubes   
            for (int i = 0; i < MAX_TUBES; i++)
            {
                DrawRectangle(tubes[i*2].rec.x, tubes[i*2].rec.y, tubes[i*2].rec.width, tubes[i*2].rec.height, GRAY);
                DrawTextureRec(tubeT, tubes[i*2].rec, (Vector2){tubes[i*2].rec.x, tubes[i*2].rec.y}, WHITE);
                DrawRectangle(tubes[i*2 + 1].rec.x, tubes[i*2 + 1].rec.y, tubes[i*2 + 1].rec.width, tubes[i*2 + 1].rec.height, GRAY);
                DrawTextureRec(tubeTbot, tubes[i*2+1].rec, (Vector2){tubes[i*2+1].rec.x, tubes[i*2+1].rec.y}, WHITE);
            }

            // Draw flashing fx (one frame only)
            
            if (superfx)
            {
                DrawRectangle(0, 0, screenWidth, screenHeight, RED);
                superfx = false;
            }

            DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);
            DrawText(TextFormat("HI-SCORE: %04i", hiScore), 20, 70, 20, LIGHTGRAY);
            
            //DrawRectangle(0, 900, screenWidth, 150, BLACK); //GROUND x, y, width, height, color
            DrawRectangleRec(ground, BLACK); //GROUND x, y, width, height, color
            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else {
            
            
            DrawText("YOU SUCK, PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("YOU SUCK, PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
        }

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}