#pragma once

// Game mode
enum GameMode
{
    EDITOR,
    PLAY
};

struct GlobalData
{
    Vec3 editor_camera_pos;
    Vec3 editor_camera_euler_ang;
    GameMode game_mode;
    bool mode_change;
    bool editor_multi_view;
    double cursor_x;
    double cursor_y;
    float cursor_movement_x;
    float cursor_movement_y;
    float dt;
    int window_width;
    int window_height;
    

    void initGlobalData()
    {
        game_mode = PLAY;
        mode_change = false;
        editor_multi_view = false;
        cursor_x = 0.0;
        cursor_y = 0.0;
        cursor_movement_x = 0.0f;
        cursor_movement_y = 0.0f;
        dt = 0.0f;
    }       
};
