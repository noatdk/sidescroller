#include <raylib.h>
#include <raymath.h>

#define SCREEN_WIDTH 1280.0
#define SCREEN_HEIGHT 720.0
#define PLAYER_HEIGHT 64.0
#define PLAYER_SPEED 400.0
#define GRAVITY 2000.0
#define JUMP_SPEED -600.0
int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "sidescroller");
    SetTargetFPS(60);
    SetWindowFocused();
    Vector2 sizes = {PLAYER_HEIGHT, PLAYER_HEIGHT};
    Vector2 pos = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    Vector2 velo = {};
    Vector2 dir = {};
    bool is_grounded = false;
    Texture2D player_run_text = LoadTexture("assets/cat_run.png");
    int player_run_frames = 4;
    float player_run_width = player_run_text.width;
    float player_run_height = player_run_text.height;
    float player_run_frame_width = player_run_width / player_run_frames;
    float player_run_frame_timer = 0.0f;
    float player_run_frame_len = 0.1f;
    int player_run_current_frame = 0;
    Rectangle draw_player_source = {};
    int last_dir_x = 1;

    while (!WindowShouldClose()) {
        // player_run_frame_timer += GetFrameTime();
        float dt = GetFrameTime();
        float pos_offset = GetScreenHeight() - PLAYER_HEIGHT;
        player_run_current_frame = (int)(GetTime() / player_run_frame_len) % player_run_frames;
        // if (player_run_frame_timer > player_run_frame_len) {
        //     player_run_frame_timer = 0;
        //     player_run_current_frame += 1;
        //     if (player_run_current_frame >= player_run_frames) {
        //         player_run_current_frame = 0;
        //     }
        // }
        dir.x = 0;
        if (IsKeyDown(KEY_A)) {
            dir.x = -PLAYER_SPEED;
            last_dir_x = -1;
        }
        if (IsKeyDown(KEY_D)) {
            dir.x = PLAYER_SPEED;
            last_dir_x = 1;
        }
        dir.y += GRAVITY * dt;

        if (is_grounded && IsKeyPressed(KEY_SPACE)) {
            dir.y = JUMP_SPEED;
            is_grounded = false;
        }
        pos = Vector2Add(pos, Vector2Scale(dir, dt));
        if (pos.y >= pos_offset) {
            pos.y = pos_offset;
            is_grounded = true;
        }

        draw_player_source = (Rectangle){
            3 * player_run_frame_width,
            0,
            player_run_frame_width * last_dir_x,
            player_run_height};

        if (dir.x != 0) {
            draw_player_source.x =
                player_run_current_frame * player_run_frame_width;
        }

        Rectangle draw_player_dest = {
            pos.x,
            pos.y,
            player_run_frame_width * 4,
            player_run_height * 4};

        BeginDrawing();
        {
            ClearBackground((Color){110, 184, 168, 255});
            DrawTexturePro(player_run_text, draw_player_source, draw_player_dest, (Vector2){0, 0}, 0, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
