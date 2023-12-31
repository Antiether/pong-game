#include <raylib.h>
#include <cstdio>
#include <algorithm>

using namespace std;

//struktur bola
struct Ball
{
    float x, y;
    float speedX, speedY;
    float radius;

    void Draw()
    {
        DrawCircle((int)x, (int)y, radius, WHITE);
    }
};

//struktur paddle
struct Paddle
{
    float x, y;
    float speed;
    float width, height;

    Rectangle GetRect()
    {
        return Rectangle{ x - width / 2, y - height / 2, 10, 100 };
    }

    void Draw()
    {
        DrawRectangleRec(GetRect(), WHITE);
    }
};

//gambar dari bayangan paddle
void DrawPaddleWithShadow(Paddle paddle)
{
    DrawRectangle(paddle.x - paddle.width / 2 + 4.5, paddle.y - paddle.height / 2 + 4.5, paddle.width, paddle.height, BLACK);
    DrawRectangleRec(paddle.GetRect(), BLACK);
}

//gambar dari bayangan bola (sedikit tidak berguna gegara shadow nya ketutupan trail atau ekor dari paddle nya)
void DrawBallWithShadow(Ball ball)
{
    DrawCircle(ball.x + 5, ball.y + 5, ball.radius, BLACK);
    DrawCircle(ball.x + 5, ball.y + 5, ball.radius, BLACK);
}

//ekor dari bola atau ball trail
void DrawBallWithTrail(Ball ball, Vector2 trailPositions[], int trailLength)
{
    const Color trailColor = { 255, 255, 255, 200 };

    for (int i = trailLength - 1; i > 0; --i)
    {
        trailPositions[i] = trailPositions[i - 1];
    }

    trailPositions[0] = { ball.x, ball.y };

    for (int i = 0; i < trailLength; ++i)
    {
        float alpha = (float)(trailLength - i) / trailLength;
        DrawCircleV(trailPositions[i], ball.radius, Fade(trailColor, alpha));
    }

    DrawCircle(ball.x, ball.y, ball.radius, WHITE);
}

//untuk menghasilkan format teks
const char* SimpleTextFormat(const char* text, int value)
{
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), text, value);
    return buffer;
}

//menu untuk keadaan di permainan
enum GameState
{
    StartMenu,
    Playing,
};

//menu untuk batasan skor
enum ScoreLimit
{
    ScoreLimit5 = 5,
    ScoreLimit10 = 10,
    ScoreLimit15 = 15,
    ScoreLimit20 = 20,
};

int main(void)
{
    //inisialisasi window dan batasan FPS
    InitWindow(800, 600, "Pong");
    SetWindowState(FLAG_VSYNC_HINT);

    //inisialisasi dari bola
    Ball ball;
    ball.x = GetScreenWidth() / 2.0f;
    ball.y = GetScreenHeight() / 2.0f;
    ball.radius = 5;
    ball.speedX = 300;
    ball.speedY = 300;

    //inisialisasi paddle kiri
    Paddle leftPaddle;
    leftPaddle.x = 50;
    leftPaddle.y = GetScreenHeight() / 2;
    leftPaddle.width = 10;
    leftPaddle.height = 100;
    leftPaddle.speed = 500;

    //inisialisasi paddle kanan
    Paddle rightPaddle;
    rightPaddle.x = GetScreenWidth() - 50;
    rightPaddle.y = GetScreenHeight() / 2;
    rightPaddle.width = 10;
    rightPaddle.height = 100;
    rightPaddle.speed = 500;

    //base skor dari kedua pemain
    int player1Score = 0;
    int player2Score = 0;

    //teks pemenang dan restart game
    const char* winnerText = nullptr;
    const char* restartGame = nullptr;

    //panjang ekor bola
    const int trailLength = 10;
    Vector2 trailPositions[trailLength] = { 0 };

    //buffer atau delay teks
    char buffer[256];

    //keadaan awal permainan sekaligus batasan skor
    GameState gameState = StartMenu;
    ScoreLimit scoreLimit = ScoreLimit5;

    //init
    InitAudioDevice();

    //memuat resource
    Music backgroundMusic = LoadMusicStream("C:/Games/gameprj/pong game/sound/whistle.mp3");
    bool isMusicPlaying = false; //variabel buat track kalau musik dimainkan
    Sound hitSound = LoadSound("C:/Games/gameprj/pong game/sound/hit_sound.wav");
    Sound swapSound = LoadSound("C:/Games/gameprj/pong game/sound/gamestateswap_sound.wav");

    while (!WindowShouldClose())
    {
        switch (gameState)
        {
        //case untuk awal menu
        case StartMenu:

            BeginDrawing();
            ClearBackground(DARKPURPLE);

            //ngegambar menu di awal program
            DrawText("Pong - Start Menu", GetScreenWidth() / 2 - MeasureText("Pong - Start Menu", 30) / 2, GetScreenHeight() / 4, 30, WHITE);
            DrawText("Choose Score Limit Using Z,X,C,V Button", GetScreenWidth() / 2 - MeasureText("Choose Score Limit Using Z,X,C,V Button", 20) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
            DrawText("Score Limit:", 10, GetScreenHeight() - 50, 20, WHITE);
            DrawText("5 ", 160, GetScreenHeight() - 50, (scoreLimit == ScoreLimit5) ? 30 : 20, (scoreLimit == ScoreLimit5) ? YELLOW : WHITE);
            DrawText("10", 220, GetScreenHeight() - 50, (scoreLimit == ScoreLimit10) ? 30 : 20, (scoreLimit == ScoreLimit10) ? YELLOW : WHITE);
            DrawText("15", 280, GetScreenHeight() - 50, (scoreLimit == ScoreLimit15) ? 30 : 20, (scoreLimit == ScoreLimit15) ? YELLOW : WHITE);
            DrawText("20", 340, GetScreenHeight() - 50, (scoreLimit == ScoreLimit20) ? 30 : 20, (scoreLimit == ScoreLimit20) ? YELLOW : WHITE);

            EndDrawing();

            //untuk menginput pemilihan skor
            if (IsKeyPressed(KEY_Z)) scoreLimit = ScoreLimit5, PlaySound(swapSound);
            else if (IsKeyPressed(KEY_X)) scoreLimit = ScoreLimit10, PlaySound(swapSound);
            else if (IsKeyPressed(KEY_C)) scoreLimit = ScoreLimit15, PlaySound(swapSound);
            else if (IsKeyPressed(KEY_V)) scoreLimit = ScoreLimit20, PlaySound(swapSound);

            //untuk memulai permainan
            if (IsKeyPressed(KEY_ENTER))
            {
                PlaySound(swapSound);
                gameState = Playing;
                player1Score = 0;
                player2Score = 0;
                winnerText = nullptr;
                restartGame = nullptr;
            }
            //untuk memulai musik kalau musik belum menyala
            PlayMusicStream(backgroundMusic);
            if (!isMusicPlaying)
            {
                PlayMusicStream(backgroundMusic);
                isMusicPlaying = true;
            }
            break;

            //case ketika permainan dimulai
        case Playing:
            //update posisi bola dengan kecepatan
            ball.x += ball.speedX * GetFrameTime();
            ball.y += ball.speedY * GetFrameTime();
            if (ball.y < 0)
            {
                PlaySound(hitSound); //start sound effect kalau bola hit border
                ball.y = 0;
                ball.speedY *= -1;
            }
            if (ball.y > GetScreenHeight())
            {
                PlaySound(hitSound); //start sound effect kalau bola hit border
                ball.y = GetScreenHeight();
                ball.speedY *= -1;
            }

            //kontrol paddle kiri
            if (IsKeyDown(KEY_W) && leftPaddle.y - leftPaddle.speed * GetFrameTime() - leftPaddle.height / 2 > 0)
            {
                leftPaddle.y -= leftPaddle.speed * GetFrameTime();
            }
            if (IsKeyDown(KEY_S) && leftPaddle.y + leftPaddle.speed * GetFrameTime() + leftPaddle.height / 2 < GetScreenHeight())
            {
                leftPaddle.y += leftPaddle.speed * GetFrameTime();
            }
            //kontrol paddle kanan
            if (IsKeyDown(KEY_UP) && rightPaddle.y - rightPaddle.speed * GetFrameTime() - rightPaddle.height / 2 > 0)
            {
                rightPaddle.y -= rightPaddle.speed * GetFrameTime();
            }
            if (IsKeyDown(KEY_DOWN) && rightPaddle.y + rightPaddle.speed * GetFrameTime() + rightPaddle.height / 2 < GetScreenHeight())
            {
                rightPaddle.y += rightPaddle.speed * GetFrameTime();
            }

            //deteksi tabrakan dari bola dengan paddle kiri
            if (CheckCollisionCircleRec(Vector2{ ball.x, ball.y }, ball.radius, leftPaddle.GetRect()))
            {
                PlaySound(hitSound); //start sound effect kalau bola hit paddle
                if (ball.speedX < 0)
                {
                    //mengubah arah gerak bola
                    ball.speedX *= -1.1f;
                    ball.speedY = (ball.y - leftPaddle.y) / (leftPaddle.height / 2) * ball.speedX;
                }
            }
            //deteksi tabrakan dari bola dengan paddle kanan
            if (CheckCollisionCircleRec(Vector2{ ball.x, ball.y }, ball.radius, rightPaddle.GetRect()))
            {
                PlaySound(hitSound); //start sound effect kalau bola hit paddle
                if (ball.speedX > 0)
                {
                    //mengubah arah gerak bola
                    ball.speedX *= -1.1f;
                    ball.speedY = (ball.y - rightPaddle.y) / (rightPaddle.height / 2) * ball.speedX;
                }
            }

            //pengecekan jika bola keluar dari layar
            if (ball.x < 0)
            {
                if (!winnerText)
                {
                    PlaySound(swapSound);
                    //meningkatkan skor pemain 2
                    player2Score++;
                    //pengecekan jumlah skor
                    if (player2Score == scoreLimit)
                    {
                        winnerText = "Player 2 Wins!";
                        restartGame = "Press Enter to Return to Start Menu";
                    }
                    else
                    {
                        //reset posisi bola
                        ball.x = GetScreenWidth() / 2;
                        ball.y = GetScreenHeight() / 2;
                        ball.speedX = 300;
                        ball.speedY = 300;
                    }
                }
            }
            //pengecekan jika bola keluar dari layar
            if (ball.x > GetScreenWidth())
            {
                if (!winnerText)
                {
                    PlaySound(swapSound);
                    //penambahan skor player 1
                    player1Score++;
                    //pengecekan jumlah skor
                    if (player1Score == scoreLimit)
                    {
                        winnerText = "Player 1 Wins!";
                        restartGame = "Press Enter to Return to Start Menu";
                    }
                    else
                    {
                        //reset posisi bola
                        ball.x = GetScreenWidth() / 2;
                        ball.y = GetScreenHeight() / 2;
                        ball.speedX = 300;
                        ball.speedY = 300;
                    }
                }
            }
            //input untuk kembali ke menu
            if (winnerText && IsKeyPressed(KEY_ENTER))
            {
                PlaySound(swapSound);
                winnerText = nullptr;
                restartGame = nullptr;
                player1Score = 0;
                player2Score = 0;
                gameState = StartMenu;

                StopMusicStream(backgroundMusic);
                isMusicPlaying = false;
            }

            //mengupdate musik
            UpdateMusicStream(backgroundMusic);

            //gambar frame game
            BeginDrawing();
            ClearBackground(DARKPURPLE);

            //jejak bola
            DrawBallWithTrail(ball, trailPositions, trailLength);
            //bayangan bola
            DrawPaddleWithShadow(leftPaddle);
            DrawPaddleWithShadow(rightPaddle);

            //menggambar elemen game
            ball.Draw();
            leftPaddle.Draw();
            rightPaddle.Draw();

            //menggambar teks permainan
            if (winnerText)
            {
                int textWidth = MeasureText(winnerText, 60);
                DrawText(winnerText, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 30, 60, YELLOW);
            }
            if (restartGame)
            {
                int textWidth = MeasureText(restartGame, 30);
                DrawText(restartGame, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 + 80, 30, LIGHTGRAY);
            }

            //menampilkan fps
            DrawFPS(10, 10);

            //papan skor
            snprintf(buffer, sizeof(buffer), "%d - %d", player1Score, player2Score);
            DrawText(buffer, GetScreenWidth() / 2 - MeasureText("0 - 0", 20) / 2, 10, 20, WHITE);

            //menampilkan batasan skor
            DrawText("Score Limit:", 10, GetScreenHeight() - 50, 20, WHITE);
            DrawText("5 ", 130, GetScreenHeight() - 50, (scoreLimit == ScoreLimit5) ? 30 : 20, (scoreLimit == ScoreLimit5) ? YELLOW : WHITE);
            DrawText("10", 190, GetScreenHeight() - 50, (scoreLimit == ScoreLimit10) ? 30 : 20, (scoreLimit == ScoreLimit10) ? YELLOW : WHITE);
            DrawText("15", 250, GetScreenHeight() - 50, (scoreLimit == ScoreLimit15) ? 30 : 20, (scoreLimit == ScoreLimit15) ? YELLOW : WHITE);
            DrawText("20", 310, GetScreenHeight() - 50, (scoreLimit == ScoreLimit20) ? 30 : 20, (scoreLimit == ScoreLimit20) ? YELLOW : WHITE);

            EndDrawing();
            break;
        }
    }
    //untuk meng unload stream musik
    UnloadMusicStream(backgroundMusic);

    //menutup audio
    CloseAudioDevice();

    //menutup window
    CloseWindow();

    return 0;
}
