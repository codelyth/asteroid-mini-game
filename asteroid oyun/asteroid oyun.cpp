#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>

enum GameState {
    MENU,
    GAMEPLAY,
    GAME_OVER
};

struct Meteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
};

struct Bullet {
    Vector2 position;
    Vector2 speed;
    bool active;
};

struct Player {
    Vector2 position;
    Vector2 previousPosition;
    float rotation;
    float speed;
    int health;
    bool isInvincible;
    float invincibleTimer;
    int score;
};

bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2) {
    return (Vector2Distance(center1, center2) <= (radius1 + radius2));  
}

int main() {
    InitWindow(970, 555, "Asteroids Game");

    GameState currentState = MENU;

    Player player = {
        { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 },
        { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 },
        0.0f,
        200.0f,
        3,
        false,
        0.0f,
        0
    };

    //Mermi sayýsý 8
    const int maxBullets = 8;
    std::vector<Bullet> bullets(maxBullets);

    // Meteor sayýsý 8
    const int meteorCount = 8;
    std::vector<Meteor> meteors(meteorCount);

    // Meteorlarý baþlat - Daha dengeli daðýlým için ekraný bölgelere ayýrdýk
    for (int i = 0; i < meteorCount; i++) {
        // Ekraný 4x2'lik bir ýzgaraya böl ve her meteoru farklý bir bölgeye yerleþtir
        int gridX = i % 4;
        int gridY = i / 4;

        float regionWidth = GetScreenWidth() / 4.0f;
        float regionHeight = GetScreenHeight() / 2.0f;

        meteors[i].position.x = (gridX * regionWidth) + (float)(rand() % (int)regionWidth);
        meteors[i].position.y = (gridY * regionHeight) + (float)(rand() % (int)regionHeight);

        // Hýzlarý dengeli hale getirdik
        meteors[i].speed.x = (float)((rand() % 150) - 75);
        meteors[i].speed.y = (float)((rand() % 150) - 75);

        // Yarýçaplarý dengeli daðýttýk
        meteors[i].radius = (float)(25 + (rand() % 20));
        meteors[i].active = true;
    }

    SetTargetFPS(60);

    //Ana oyun döngüsü
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime(); //Bir karede geçen süreyi hesaplar

        switch (currentState) {
        case MENU: {
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = GAMEPLAY;
                player.health = 3;
                player.score = 0;
                player.position = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
                player.previousPosition = player.position;
                player.rotation = 0.0f;
                for (auto& bullet : bullets) bullet.active = false;

                // Oyun yeniden baþladýðýnda meteorlarý yeniden daðýt
                for (int i = 0; i < meteorCount; i++) {
                    int gridX = i % 4;
                    int gridY = i / 4;
                    float regionWidth = GetScreenWidth() / 4.0f;
                    float regionHeight = GetScreenHeight() / 2.0f;

                    meteors[i].position.x = (gridX * regionWidth) + (float)(rand() % (int)regionWidth);
                    meteors[i].position.y = (gridY * regionHeight) + (float)(rand() % (int)regionHeight);
                    meteors[i].speed.x = (float)((rand() % 150) - 75);
                    meteors[i].speed.y = (float)((rand() % 150) - 75);
                    meteors[i].active = true;
                }
            }
            //giriþ ekraný çizdirme
            BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawText("ASTEROIDS", GetScreenWidth() / 2 - MeasureText("ASTEROIDS", 60) / 2, GetScreenHeight() / 3, 60, WHITE);
            DrawText("Press ENTER to Start", GetScreenWidth() / 2 - MeasureText("Press ENTER to Start", 30) / 2, GetScreenHeight() / 2, 30, LIGHTGRAY);
            DrawText("Controls:", GetScreenWidth() / 2 - MeasureText("Controls:", 20) / 2, GetScreenHeight() / 2 + 60, 20, LIGHTGRAY);
            DrawText("W - Move Forward\nA/D - Rotate\nSPACE - Shoot", GetScreenWidth() / 2 - MeasureText("W - Move Forward", 20) / 2, GetScreenHeight() / 2 + 90, 20, LIGHTGRAY);
            EndDrawing();
        } break;

        case GAMEPLAY: {
            if (player.isInvincible) {
                player.invincibleTimer -= deltaTime;
                if (player.invincibleTimer <= 0) {
                    player.isInvincible = false;
                }
            }

            player.previousPosition = player.position;

            if (IsKeyDown(KEY_W)) {
                player.position.x += sinf(player.rotation * DEG2RAD) * player.speed * deltaTime;
                player.position.y -= cosf(player.rotation * DEG2RAD) * player.speed * deltaTime;
            }
            if (IsKeyDown(KEY_A)) player.rotation -= 180 * deltaTime;
            if (IsKeyDown(KEY_D)) player.rotation += 180 * deltaTime;

            //ekran sarmalama
            if (player.position.x < 0) player.position.x = GetScreenWidth();
            if (player.position.x > GetScreenWidth()) player.position.x = 0;
            if (player.position.y < 0) player.position.y = GetScreenHeight();
            if (player.position.y > GetScreenHeight()) player.position.y = 0;

            bool collisionDetected = false;
            for (auto& meteor : meteors) {
                if (meteor.active && CheckCollisionCircles(player.position, 20, meteor.position, meteor.radius)) {
                    if (!player.isInvincible) {
                        player.health--;
                        player.isInvincible = true;
                        player.invincibleTimer = 2.0f;

                        meteor.active = false;

                        // Yeni asteroid oluþturma mantýðý
                        meteor.position = { (float)(rand() % GetScreenWidth()), (float)(rand() % GetScreenHeight()) };
                        meteor.speed = { (float)((rand() % 150) - 75), (float)((rand() % 150) - 75) };
                        meteor.radius = (float)(25 + (rand() % 20));
                        meteor.active = true;

                        if (player.health <= 0) {
                            currentState = GAME_OVER;
                            break;
                        }
                    }
                    player.position = player.previousPosition;
                    collisionDetected = true;
                    break;
                }
            }

            if (IsKeyPressed(KEY_SPACE)) {
                for (auto& bullet : bullets) {
                    if (!bullet.active) {
                        bullet.position = player.position;
                        bullet.speed = { sinf(player.rotation * DEG2RAD) * 500, -cosf(player.rotation * DEG2RAD) * 500 };
                        bullet.active = true;
                        break;
                    }
                }
            }

            for (auto& bullet : bullets) {
                if (bullet.active) {
                    bullet.position.x += bullet.speed.x * deltaTime;
                    bullet.position.y += bullet.speed.y * deltaTime;

                    if (bullet.position.x < 0 || bullet.position.x > GetScreenWidth() ||
                        bullet.position.y < 0 || bullet.position.y > GetScreenHeight()) {
                        bullet.active = false;
                    }
                }
            }

            for (auto& meteor : meteors) {
                if (meteor.active) {
                    meteor.position.x += meteor.speed.x * deltaTime;
                    meteor.position.y += meteor.speed.y * deltaTime;

                    //ekran sarmalama
                    if (meteor.position.x < 0) meteor.position.x = GetScreenWidth();
                    if (meteor.position.x > GetScreenWidth()) meteor.position.x = 0;
                    if (meteor.position.y < 0) meteor.position.y = GetScreenHeight();
                    if (meteor.position.y > GetScreenHeight()) meteor.position.y = 0;
                }
            }

            for (auto& bullet : bullets) {
                if (bullet.active) {
                    for (auto& meteor : meteors) {
                        if (meteor.active && CheckCollisionCircles(bullet.position, 5, meteor.position, meteor.radius)) {
                            bullet.active = false;
                            meteor.active = false;
                            player.score += 5;

                            //Yeni meteor oluþturma mantýðý
                            meteor.position = { (float)(rand() % GetScreenWidth()), (float)(rand() % GetScreenHeight()) };
                            meteor.speed = { (float)((rand() % 150) - 75), (float)((rand() % 150) - 75) };
                            meteor.radius = (float)(25 + (rand() % 20));
                            meteor.active = true;
                            break;
                        }
                    }
                }
            }

            Texture2D galaxyTexture = LoadTexture("Melek Çizim.jpg"); //arka plan resmi ekleme
            BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawTexture(galaxyTexture, 0, 0, WHITE); //arka plan resmi çizdirme

            for (int i = 0; i < player.health; i++) {
                DrawCircle(30 + i * 30, 30, 10, YELLOW);
            }

            std::string scoreText = "Score: " + std::to_string(player.score);
            DrawText(scoreText.c_str(), 30, 60, 20, WHITE);

            if (!player.isInvincible || (int)(player.invincibleTimer * 10) % 2) {
                DrawCircleV(player.position, 20, WHITE);
                DrawLine(
                    player.position.x,
                    player.position.y,
                    player.position.x + sinf(player.rotation * DEG2RAD) * 30,
                    player.position.y - cosf(player.rotation * DEG2RAD) * 30,
                    BLACK
                );
            }

            for (const auto& bullet : bullets) {
                if (bullet.active) {
                    DrawCircleV(bullet.position, 5, WHITE);
                }
            }

            for (const auto& meteor : meteors) {
                if (meteor.active) {
                    DrawCircleV(meteor.position, meteor.radius, DARKGRAY);
                }
            }

            DrawText("Press ESC to exit", GetScreenWidth() - 200, GetScreenHeight() - 30, 20, GRAY);
            EndDrawing();
        } break;

        case GAME_OVER: {
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = MENU;
            }

            BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 60) / 2, GetScreenHeight() / 3, 60, RED);

            std::string finalScoreText = "Final Score: " + std::to_string(player.score);
            DrawText(finalScoreText.c_str(), GetScreenWidth() / 2 - MeasureText(finalScoreText.c_str(), 30) / 2, GetScreenHeight() / 2 - 30, 30, WHITE);

            DrawText("Press ENTER to Return to Menu", GetScreenWidth() / 2 - MeasureText("Press ENTER to Return to Menu", 30) / 2, GetScreenHeight() / 2, 30, LIGHTGRAY);
            EndDrawing();
        } break;
        }
    }

    CloseWindow();
    return 0;
}