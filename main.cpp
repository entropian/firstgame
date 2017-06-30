/*
  TODO:
  solve when track and ship are initlized to be colliding  
  make the main game loop timestep based
  make ship velocity work with timestep
  add some kind of grid for track editing
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <string>
#include "mat.h"
#include "ship.h"
#include "box.h"
#include "camera.h"
#include "track.h"
#include "globalclock.h"
#include "linegrid.h"
#include "manipulator.h"
#include "selected.h"
#include "globaldata.h"
#include "input.h"
#include "editor.h"

bool EXIT = false;

GlobalData g;
Input g_input;

void calcShipAccelState(int accel_states[3], Input& input)
{    
    // 1 is accelerating towards positive
    // -1 is accelerating towards negative
    // 0 is accelerating towards 0
    if((input.w && input.s) || (!input.w && !input.s))
    {
        accel_states[2] = 0;
    }else
    {
        if(input.w)
        {
            // forwared is negative z
            accel_states[2] = -1;
        }
        if(input.s)
        {
            accel_states[2] = 1;
        }
    }

    if((input.a && input.d) || (!input.a && !input.d))
    {
        accel_states[0] = 0;
    }else
    {
        if(input.a)
        {
            accel_states[0] = -1;
        }
        if(input.d)
        {
            accel_states[0] = 1;
        }
    }

    if(input.jump_request)
    {
        accel_states[1] = 1;
        input.jump_request = false;
    }else
    {
        accel_states[1] = -1;
    }
}

void moveCamera(Camera& camera, const Input& input, const float dt)
{
    const float camera_speed = 10.0f * dt;
    // Keyboard
    if(input.w == 1)
    {
        camera.move(FORWARD, camera_speed);
    }
    if(input.s == 1)
    {
        camera.move(BACKWARD, camera_speed);
    }
    if(input.a == 1)
    {
        camera.move(LEFT, camera_speed);
    }
    if(input.d == 1)
    {
        camera.move(RIGHT, camera_speed);
    }
    if(input.r == 1)
    {
        camera.move(UP, camera_speed);
    }
    if(input.f == 1)
    {
        camera.move(DOWN, camera_speed);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
        case GLFW_KEY_W:
        {
            g_input.w = 1;
        } break;
        case GLFW_KEY_A:
        {
            g_input.a = 1;
        } break;
        case GLFW_KEY_S:
        {
            g_input.s = 1;
        } break;
        case GLFW_KEY_D:
        {
            g_input.d = 1;
        } break;
        case GLFW_KEY_R:
        {
            g_input.r = 1;
        } break;
        case GLFW_KEY_F:
        {
            g_input.f = 1;
        } break;
        case GLFW_KEY_Q:
        {
            g_input.q = 1;
        } break;
        case GLFW_KEY_N:
        {
            g_input.n = 1;
        } break;
        case GLFW_KEY_B:
        {
            g_input.b = 1;
        } break;        
        case GLFW_KEY_O:
        {
            g_input.o = 1;
        } break;
        case GLFW_KEY_C:
        {
            g_input.c = 1;
        } break;
        case GLFW_KEY_G:
        {
            g_input.g = 1;
        } break;
        case GLFW_KEY_P:
        {
            g_input.p = 1;
        } break;
        case GLFW_KEY_H:
        {
            g_input.h = 1;
        } break;                
        case GLFW_KEY_SPACE:
        {
            g_input.jump_request = true;
        } break;
        case GLFW_KEY_LEFT_CONTROL:
        {
            g_input.left_ctrl = 1;
        } break;
        }
    }else if(action == GLFW_RELEASE)
    {
        switch(key)
        {
        case GLFW_KEY_W:
        {
            g_input.w = 0;
        } break;
        case GLFW_KEY_A:
        {
            g_input.a = 0;
        } break;
        case GLFW_KEY_S:
        {
            g_input.s = 0;
        } break;
        case GLFW_KEY_D:
        {
            g_input.d = 0;
        } break;
        case GLFW_KEY_R:
        {
            g_input.r = 0;
        } break;
        case GLFW_KEY_F:
        {
            g_input.f = 0;
        } break;        
        case GLFW_KEY_M:
        {
            g.mode_change = true;
            if(g.game_mode == PLAY)
            {
                g.game_mode = EDITOR;
            }else if(g.game_mode == EDITOR)
            {
                g.game_mode = PLAY;
            }
        } break;
        case GLFW_KEY_Q:
        {
            g_input.q = 0;
            EXIT = true;
        } break;
        case GLFW_KEY_N:
        {
            g_input.n = 0;
        } break;
        case GLFW_KEY_B:
        {
            g_input.b = 0;
        } break;        
        case GLFW_KEY_O:
        {
            g_input.o = 0;
        } break;
        case GLFW_KEY_C:
        {
            g_input.c = 0;
        } break;
        case GLFW_KEY_G:
        {
            g_input.g = 0;
        } break;
        case GLFW_KEY_P:
        {
            g_input.p = 0;
        } break;
        case GLFW_KEY_H:
        {
            g_input.h = 0;
            // TODO: temp
            g.editor_multi_view = !g.editor_multi_view;
        } break;                        
        case GLFW_KEY_LEFT_CONTROL:
        {
            g_input.left_ctrl = 0;
        } break;        
        }
    }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_PRESS)
        {
            g_input.left_click = true;
            glfwGetCursorPos(window, &(g_input.left_click_x), &(g_input.left_click_y));
            int window_height, window_width;
            glfwGetWindowSize(window, &window_width, &window_height);
            g_input.left_click_y = window_height - g_input.left_click_y;
            std::cout << "xpos: " << g_input.left_click_x << " ypos: " << g_input.left_click_y << std::endl;
        }else if(action == GLFW_RELEASE)
        {
            g_input.left_click = false;
        }
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if(action == GLFW_PRESS)
        {
            g_input.right_click = true;
            glfwGetCursorPos(window, &(g_input.right_click_x), &(g_input.right_click_y));
            int window_height, window_width;
            glfwGetWindowSize(window, &window_width, &window_height);
            g_input.right_click_y = window_height - g_input.right_click_y;
            std::cout << "xpos: " << g_input.right_click_x << " ypos: " << g_input.right_click_y << std::endl;
        }else if(action == GLFW_RELEASE)
        {
            g_input.right_click = false;
        }
    }
}

void cursorPosCallback(GLFWwindow *window, double x, double y)
{
    //g_input.cursor_moved_last_frame = true;
}

void getNormalizedWindowCoord(float& x, float& y, const unsigned int x_pos, const unsigned int y_pos)
{
    x = (float)(x_pos - g.window_width/2.0f) / (g.window_width / 2.0f);
    y = (float)(y_pos - g.window_height/2.0f) / (g.window_height / 2.0f);
}

void gameModeFrame(PerspectiveCamera& camera, Ship& ship, Track& track)
{
    // Update ship position and velocity based on velocity from last frame
    // Update ship velocity based on keyboard input
    if(g.mode_change)
    {
        // Last frame was in editor mode
        // Save editor mode camera pos and orientation
        g.editor_camera_pos = camera.getPosition();
        g.editor_camera_euler_ang = camera.getEulerAng();
        camera.setPosAndOrientation(Vec3(), Vec3());
        g.mode_change = false;
    }
    if(g_input.r == 1)
    {
        ship.resetPosition();
    }
    int accel_states[3];
    calcShipAccelState(accel_states, g_input);
    ship.calcVelocity(accel_states, g.dt);
    ship.updatePosAndVelocity(g.dt, track);
    camera.setPosRelativeToShip(ship);

    Mat4 view_transform = camera.getViewTransform();
    ship.updateDynamicUniforms(view_transform);
    ship.draw();
    track.setViewTransform(view_transform);
    track.draw();
}


GLFWwindow* initWindow(unsigned int width, unsigned int height)
{
	// Init GLFW
	if (glfwInit() != GL_TRUE)
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		//return -1;
	}

	// Create a rendering window with OpenGL 3.2 context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(width, height, "firstgame", NULL, NULL);
	glfwSetWindowPos(window, 600, 100);
	glfwMakeContextCurrent(window);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return NULL;
	}
	return window;
}

void updateMouseInput(GLFWwindow* window)
{
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    double cur_cursor_x, cur_cursor_y;
    glfwGetCursorPos(window, &cur_cursor_x, &cur_cursor_y);                    
    cur_cursor_y = (double)window_height - cur_cursor_y;
    //static double last_cursor_x = cursor_x;
    //static double last_cursor_y = cursor_y;
    g.cursor_movement_x = cur_cursor_x - g.cursor_x;
    g.cursor_movement_y = cur_cursor_y - g.cursor_y;
    g.cursor_x = cur_cursor_x;
    g.cursor_y = cur_cursor_y;
}

int main()
{
    unsigned int window_width = 1600;
    unsigned int window_height = 900;
    g.window_width = window_width;
    g.window_height = window_height;
	GLFWwindow *window = initWindow(window_width, window_height);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // Setup ImGui binding
    //ImGui_ImplGlfwGL3_Init(window, true);

    GLint num_tex_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_tex_units);
    std::cout << "Number of texture units: " << num_tex_units << std::endl;
    
    // Light
    Vec3 dir_light(normalize(Vec3(0.7f, 2.0f, 1.0f)));
        
    // Cameras
    float aspect_ratio  = (float)window_width / (float)window_height;
    float fov = 90.0f;    
    PerspectiveCamera pers_camera(Vec3(0.0f, 0.0f, -1.0f),
                             Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 4.0f), fov, aspect_ratio);

    const float view_volume_width = 20.0f;
    const float view_volume_height = view_volume_width / aspect_ratio;
    OrthographicCamera ortho_camera_z(Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 10.0f),
                                      view_volume_width, view_volume_height);
    OrthographicCamera ortho_camera_x(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(-10.0f, 0.0f, 0.0f),
                              view_volume_width, view_volume_height);
    OrthographicCamera ortho_camera_y(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f),
                                      Vec3(00.0f, 10.0f, 0.0f),
                                      view_volume_width, view_volume_height);
    Mat4 proj_transform(Mat4::makePerspective(fov, aspect_ratio, 0.001f, 20.0f));
    Mat4 ortho_transform(Mat4::makeOrthographic(view_volume_width, view_volume_height, 0.001f, 200.0f));
    proj_transform.print();
    Mat4 view_transform = pers_camera.getViewTransform();

    // Ship transforms
    //Mat4 ship_normal_transform = ((view_transform * model.inverse())).transpose();
    Ship ship;
    ship.setStaticUniforms(proj_transform, dir_light);
    ship.move(Vec3(0.0f, 2.0f, 0.0f));

    // Box transforms
    Mat4 transform = view_transform;
    Mat4 normal_transform = (transform.inverse()).transpose();

    // Track stuff
    Track track;
    track.setUniforms(transform, normal_transform, proj_transform, dir_light);
    track.readFromFile("track1.txt");
    
    // Line grid
    // TODO
    //LineGrid line_grid(1.0f, 0.0f, 500, view_transform, proj_transform);
    LineGrid line_grid(1.0f, 0.0f, 500, proj_transform);

    // IMGUI stuff
    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    float lineWidth[2];
    glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidth);
    std::cout << "Max line width " << lineWidth[1] << "\n";

    GlobalClock gclock;

    Editor editor(track, ship, aspect_ratio, fov, proj_transform);

    glEnable(GL_DEPTH_TEST);
    int count = 0;
	while (!glfwWindowShouldClose(window) && !EXIT)
	{
        glfwPollEvents();
        gclock.update();
        g.dt = gclock.getDtSeconds();
        float dt = g.dt;
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /*
        count++;
        if(count == 100)
        {
            printf("dt %f\n", dt);
            count = 0;
        }
        */
        updateMouseInput(window);
        
        /*
        ImGui_ImplGlfwGL3_NewFrame();
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            if (ImGui::Button("Another Window")) show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        */        
        if(g.game_mode == EDITOR)
        {
            editor.frame();
        }else if(g.game_mode == PLAY)
        {
            gameModeFrame(pers_camera, ship, track);
        }

        //last_cursor_x = cursor_x;
        //last_cursor_y = cursor_y;        
        //ImGui::Render();
		glfwSwapBuffers(window); // Takes about 0.017 sec or 1/60 sec
	}

    //ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}

