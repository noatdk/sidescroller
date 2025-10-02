#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#define SCREEN_WIDTH 1280.0
#define SCREEN_HEIGHT 720.0
#define PIXEL_WINDOW_HEIGHT 180.0
#define PLAYER_HEIGHT 64.0
#define CROUCHING_SPEED 40.0;
#define WALKING_SPEED 110.0
#define RUNNING_SPEED 400.0
#define GRAVITY 2000.0
#define JUMP_SPEED -600.0
#define LANDING_DUR 0.3

#define max(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })

#define min(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })

char *debug = "";
typedef enum {
    IDLE = 0,
    WALK = 1,
    RUN = 2,
    JUMP = 3,
    CROUCHING = 4,
    ANIM_COUNT = 4,
} Animation_Type;

typedef enum {
    LEFT = -1,
    RIGHT = 1,
    NONE = 0,
} Direction;

typedef struct Animation {
    Texture2D texture;
    int frames;
    float frame_width;
    float frame_timer;
    float frame_len;
    int current_frame;
    Animation_Type type;
} Animation;

Animation CreateAnimation(char *location, int frames, float frame_len, Animation_Type type)
{
    Texture2D texture = LoadTexture(location);
    return (Animation){
        texture,
        frames,
        texture.width / (float)frames,
        0.0f,
        frame_len,
        0,
        type};
}

void update_animation(Animation *anim, float dt, Vector2 dir, Animation_Type *pre_anim, float prev_landing_time)
{
    // reset anim upon change since current anim is pointer
    if (*pre_anim != anim->type) {
        *pre_anim = anim->type;
        anim->current_frame = 0;
        anim->frame_timer = 0.0f;
    }

    anim->frame_timer += dt;
    switch (anim->type) {
    case JUMP:
        if (dir.y < 0) {
            if (anim->frame_len < anim->frame_timer) {
                anim->current_frame++;
                // repeats the first two frames (rising)
                if (anim->current_frame > 1) {
                    anim->current_frame = 0;
                }
                anim->frame_timer = 0.0f;
            }
        } else if (dir.y > 0) {
            if (anim->frame_len < anim->frame_timer) {
                // repeats the 3rd and 4th frames (falling)
                if (anim->current_frame > 3) {
                    anim->current_frame = 2;
                }
                anim->frame_timer = 0.0f;
            }
        } else {
            // landing animation (last frame)
            anim->current_frame = anim->frames - 1;
            anim->frame_timer = 0.0f;
        }
        break;
    case CROUCHING:
        if (anim->frame_len < anim->frame_timer) {
            anim->current_frame++;
            anim->current_frame = min(anim->current_frame, anim->frames - 1);
            anim->frame_timer = 0.0f;
        }
        break;
    default:
        // not really correct but a bit faster
        anim->current_frame = (int)(GetTime() / anim->frame_len) % anim->frames;
        // if (anim->frame_len < anim->frame_timer) {
        //     anim->current_frame++;
        //     if (anim->current_frame > anim->frames) {
        //         anim->current_frame = 0;
        //     }
        //     anim->frame_timer = 0.0f;
        // }
        break;
    }
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "sidescroller");
    SetTargetFPS(60);
    SetWindowFocused();
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Animation player_idle = CreateAnimation("assets/naruto_idle.png", 6, 0.2f, IDLE);
    Animation player_walk = CreateAnimation("assets/naruto_walk.png", 6, 0.15f, WALK);
    Animation player_run = CreateAnimation("assets/naruto_run.png", 6, 0.07f, RUN);
    Animation player_jump = CreateAnimation("assets/naruto_jump.png", 5, 0.09f, JUMP);
    Animation player_crouch = CreateAnimation("assets/naruto_crouch.png", 3, 0.1f, CROUCHING);

    Vector2 pos = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    Vector2 velo = {};
    Vector2 dir = {};
    bool is_grounded = false;
    bool is_crouching = false;
    bool is_running = false;
    float prev_landing_time = GetTime();
    Animation_Type prev_anim = IDLE;
    int last_dir_x = LEFT;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // player origin starts at the head, we needs offset that my height for the feet
        float pos_offset = GetScreenHeight() - PLAYER_HEIGHT;
        Animation *current_anim = &player_idle;
        dir.x = NONE;

        // disallows changing of speed midair
        if (is_grounded) {
            is_running = IsKeyDown(KEY_LEFT_SHIFT);
            is_crouching = IsKeyDown(KEY_S) && !is_running;
        }
        if (IsKeyDown(KEY_A)) {
            dir.x = last_dir_x = LEFT;
        }
        if (IsKeyDown(KEY_D)) {
            dir.x = last_dir_x = RIGHT;
        }
        if (dir.x != NONE) {
            if (is_running) {
                dir.x *= RUNNING_SPEED;
                current_anim = &player_run;
            } else if (is_crouching) {
                // TODO
                dir.x *= CROUCHING_SPEED;
                current_anim = &player_crouch;
            } else {
                dir.x *= WALKING_SPEED;
                current_anim = &player_walk;
            }
        } else if (is_crouching) {
            current_anim = &player_crouch;
        }

        // dir.y is acceleration (rate of change of velosity) so *dt twice before added to pos
        dir.y += GRAVITY * dt;

        if (is_grounded && IsKeyPressed(KEY_SPACE)) {
            dir.y = JUMP_SPEED;
            is_grounded = false;
            is_crouching = false;
        }
        pos = Vector2Add(pos, Vector2Scale(dir, dt));
        if (pos.y >= pos_offset) {
            if (!is_grounded) {
                prev_landing_time = GetTime();
            }
            pos.y = pos_offset;
            is_grounded = true;
        }

        if (!is_grounded) {
            current_anim = &player_jump;
        }
        // cancel the landing animation early if there's horizontal movement;
        if (GetTime() - prev_landing_time <= LANDING_DUR && dir.x == 0) {
            current_anim = &player_jump;
            if (is_crouching) {
                // skip the first crouching frame for smooth transition between landing animation and crouching
                current_anim = &player_crouch;
                current_anim->current_frame = max(1, current_anim->current_frame);
            }
            dir.y = 0;
        }

        Camera2D cam = {
            (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f},
            pos,
            0,
            GetScreenWidth() / PIXEL_WINDOW_HEIGHT,
        };

        update_animation(current_anim, dt, dir, &prev_anim, prev_landing_time);

        Rectangle draw_player_source = {
            current_anim->current_frame * current_anim->frame_width,
            0,
            current_anim->frame_width * last_dir_x,
            current_anim->texture.height};

        Rectangle draw_player_dest = {
            pos.x,
            pos.y,
            current_anim->frame_width,
            current_anim->texture.height};

        BeginDrawing();
        {
            // BeginMode2D(cam);
            ClearBackground((Color){110, 184, 168, 255});
            DrawTexturePro(current_anim->texture, draw_player_source, draw_player_dest, (Vector2){draw_player_dest.width / 2, draw_player_dest.height / 2}, 0, WHITE);
            DrawText(TextFormat("%s", debug), 59, 50, 15, RED);
            EndMode2D();
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
