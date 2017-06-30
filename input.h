#pragma once

struct Input
{
    // Keyboard
    int w;
    int a;
    int s;
    int d;
    int r;
    int f;
    int q;
    int n;
    int o;
    int b;
    int c;
    int g;
    int p;
    int h;

    int left_ctrl;

    bool jump_request;

    // Mouse
    double left_click_x;
    double left_click_y;
    double right_click_x;
    double right_click_y;

    bool left_click;
    bool right_click;
    
    Input()
    {
        w = 0;
        a = 0;
        s = 0;
        d = 0;
        r = 0;
        f = 0;
        q = 0;
        n = 0;
        o = 0;
        c = 0;
        g = 0;
        p = 0;
        h = 0;
        left_ctrl = 0;
        jump_request = false;
        left_click_x = 0;
        left_click_y = 0;
        right_click_x = 0;
        right_click_y = 0;
        left_click = false;
        right_click = false;
    }
};
