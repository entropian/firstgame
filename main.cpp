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

bool EXIT = false;

// Game mode
enum GameMode
{
    EDITOR,
    PLAY
};
GameMode g_game_mode = PLAY;
bool g_mode_change = false;

enum EditorAction
{
    NONE,
    WRITE_TO_FILE,
    REMOVE_SELECTED_BOX,
    COPY_SELECTED_BOX,
    ADD_NEW_BOX,
    SELECT_BOX,
    DESELECT,
    CHANGE_BOX_LENGTH,
    MOVE_SELECTED_BOX
};

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

    bool jump_request;

    // Mouse
    double click_x;
    double click_y;
    bool cursor_moved_last_frame;

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
        jump_request = false;
        click_x = 0;
        click_y = 0;
        cursor_moved_last_frame = false;
        left_click = false;
        right_click = false;
    }
};
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
        case GLFW_KEY_SPACE:
        {
            g_input.jump_request = true;
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
            g_mode_change = true;
            if(g_game_mode == PLAY)
            {
                g_game_mode = EDITOR;
            }else if(g_game_mode == EDITOR)
            {
                g_game_mode = PLAY;
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
            glfwGetCursorPos(window, &g_input.click_x, &g_input.click_y);
            int window_height, window_width;
            glfwGetWindowSize(window, &window_width, &window_height);
            g_input.click_y = window_height - g_input.click_y;
            std::cout << "xpos: " << g_input.click_x << " ypos: " << g_input.click_y << std::endl;
        }else if(action == GLFW_RELEASE)
        {
            g_input.left_click = false;
        }
    }else if(button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if(action == GLFW_PRESS)
        {
            g_input.right_click = true;
        }else if(action == GLFW_RELEASE)
        {
            g_input.right_click = false;
        }
    }
}

void cursorPosCallback(GLFWwindow *window, double x, double y)
{
    g_input.cursor_moved_last_frame = true;
}

void getNormalizedWindowCoord(float& x, float& y, const unsigned int x_pos, const unsigned int y_pos,
                              GLFWwindow *window)
{
    int window_width = 0, window_height = 0;
    glfwGetWindowSize(window, &window_width, &window_height);
    x = (float)(x_pos - window_width/2.0f) / (window_width / 2.0f);
    y = (float)(y_pos - window_height/2.0f) / (window_height / 2.0f);
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

int main()
{
    unsigned int window_width = 1280;
    unsigned int window_height = 720;
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
    Vec3 dir_light_1(normalize(Vec3(-1.0f, 1.0f, 1.0f)));
    Vec3 dir_light_2(normalize(Vec3(1.0f, 1.0f, 1.0f)));
        
    // Transforms
    float aspect_ratio  = (float)window_width / (float)window_height;
    float fov = 90.0f;
    // NOTE: where to put proj_transform?
    Mat4 proj_transform(Mat4::makePerspective(fov, aspect_ratio, 0.001f, 20.0f));
    Camera camera(Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 4.0f), fov, aspect_ratio);
    Vec3 editor_camera_pos, editor_camera_euler_ang;
    Mat4 view_transform = camera.getViewTransform();

    // Ship transforms
    //Mat4 ship_normal_transform = ((view_transform * model.inverse())).transpose();
    Ship ship;
    ship.setStaticUniforms(proj_transform, dir_light_1, dir_light_2);
    ship.move(Vec3(0.0f, 2.0f, 0.0f));

    // Box transforms
    Mat4 transform = view_transform;
    Mat4 normal_transform = (transform.inverse()).transpose();

    // Track stuff
    Track track;
    track.setUniforms(transform, normal_transform, proj_transform, dir_light_1, dir_light_2);
    track.readFromFile("tmp_track.txt");
    
    // Line grid
    LineGrid line_grid(1.0f, 0.0f, 500, view_transform, proj_transform);

    // IMGUI stuff
    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    float lineWidth[2];
    glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidth);
    std::cout << "Max line width " << lineWidth[1] << "\n";

    GlobalClock gclock;
    BoxWireframeDrawer bwfd(proj_transform);

    Manipulator manip(proj_transform);
    Vec3 original_box_color;
    
    Box *selected_box_ptr = NULL;
    
    glEnable(GL_DEPTH_TEST);
    int count = 0;
	while (!glfwWindowShouldClose(window) && !EXIT)
	{
        glfwPollEvents();
        gclock.update();
        float dt = gclock.getDtSeconds();
        /*
        count++;
        if(count == 100)
        {
            printf("dt %f\n", dt);
            count = 0;
        }
        */
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        

        // cursor position and movement
        double cursor_x, cursor_y;
        glfwGetCursorPos(window, &cursor_x, &cursor_y);                    
        cursor_y = (double)window_height - cursor_y;
        static double last_cursor_x = cursor_x;
        static double last_cursor_y = cursor_y;
        float cursor_movement_x = cursor_x - last_cursor_x;
        float cursor_movement_y = cursor_y - last_cursor_y;        
        
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
        if(g_game_mode == EDITOR)
        {
            if(g_mode_change)
            {
                // Changed mode from play to editor
                camera.setPosAndOrientation(editor_camera_pos, editor_camera_euler_ang);
                g_mode_change = false;
            }
            // Camera movement
            if(g_input.right_click && g_input.cursor_moved_last_frame)
            {
                camera.turn(cursor_movement_x, cursor_movement_y);
                g_input.cursor_moved_last_frame = false;
            }
            moveCamera(camera, g_input, dt);

            static int box_hit_side = -1;
            static Vec3 raycast_hit_point;            
            Box* hit_box_ptr = nullptr;
            EditorAction editor_action = NONE;
            {
                static bool left_clicking = false;
                static bool clicking_on_selected_box = false;
                static bool clicking_on_manipulator = false;
                static int* last_active_key = nullptr;

                if(last_active_key && !*(last_active_key))
                {
                    last_active_key = nullptr;
                }
                if(last_active_key && *(last_active_key)) 
                {
                    editor_action = NONE;
                }else if(g_input.left_click)
                {
                    if(!left_clicking)
                    {
                        float click_x, click_y;
                        getNormalizedWindowCoord(click_x, click_y, g_input.click_x, g_input.click_y, window);
                        Ray ray = camera.calcRayFromScreenCoord(click_x, click_y);
                        float t;
                        if(selected_box_ptr)
                        {
                            if(manip.rayIntersect(t, ray))
                            {
                                clicking_on_manipulator = true;
                                editor_action = NONE;
                            }else
                            {
                                hit_box_ptr = track.rayIntersectTrack(box_hit_side, t, ray);
                                if(!hit_box_ptr)
                                {
                                    editor_action = DESELECT;
                                }else if(hit_box_ptr == selected_box_ptr)
                                {
                                    raycast_hit_point = ray.calcPoint(t);
                                    clicking_on_selected_box = true;
                                    editor_action = NONE;
                                }else
                                {
                                    raycast_hit_point = ray.calcPoint(t);
                                    editor_action = SELECT_BOX;
                                }
                            }
                        }else
                        {
                            hit_box_ptr = track.rayIntersectTrack(box_hit_side, t, ray);
                            if(!hit_box_ptr)
                            {
                                editor_action = NONE;
                            }else
                            {
                                raycast_hit_point = ray.calcPoint(t);
                                editor_action = SELECT_BOX;
                            }               
                        }
                        left_clicking = true;
                    }else
                    {
                        if(clicking_on_manipulator)
                        {
                            editor_action = MOVE_SELECTED_BOX;
                        }else if(clicking_on_selected_box)
                        {
                            editor_action = CHANGE_BOX_LENGTH;
                        }else
                        {
                            editor_action = NONE;
                        }
                    }
                }else if(left_clicking && !g_input.left_click)
                {
                    clicking_on_manipulator = false;
                    clicking_on_selected_box = false;
                    left_clicking = false;
                    editor_action = NONE;
                }else
                {
                    if(g_input.o)
                    {
                        editor_action = WRITE_TO_FILE;
                        last_active_key = &(g_input.o);
                    }else if(g_input.b && selected_box_ptr)
                    {
                        editor_action = REMOVE_SELECTED_BOX;
                        last_active_key = &(g_input.b);
                    }else if(g_input.c && selected_box_ptr)
                    {
                        editor_action = COPY_SELECTED_BOX;
                        last_active_key = &(g_input.c);
                    }else if(g_input.n)
                    {
                        editor_action = ADD_NEW_BOX;
                        last_active_key = &(g_input.n);
                    }
                }
            }

            switch(editor_action)
            {
            case NONE:
                break;
            case WRITE_TO_FILE:
            {
                std::string output_file_name;
                std::cout << "Output file name: ";
                std::cin >> output_file_name;
                track.writeToFile(output_file_name.c_str());
            } break;
            case REMOVE_SELECTED_BOX:
            {
                track.removeBox(selected_box_ptr);
                selected_box_ptr = nullptr;
            } break;
            case COPY_SELECTED_BOX:
            {
                selected_box_ptr->setColor(original_box_color);
                Box new_box = selected_box_ptr->makeCopy();
                original_box_color = new_box.getColor();
                new_box.setColor(Vec3(1.0f, 105.0f / 255.0f, 180.0f / 255.0f));
                selected_box_ptr = track.addBox(new_box);                
            } break;
            case ADD_NEW_BOX:
            {
                // Place new box in track
                // take camera z axis, move some distance forward, then project it on to the xz plane at y = 0
                // make a box there
                Vec3 camera_z_axis = -camera.getZAxis();
                Vec3 camera_pos = camera.getPosition();
                const float dist = 5.0f;
                Vec3 box_center = camera_z_axis * dist + camera_pos;
                box_center[1] = 0.0f;
                Box new_box(box_center, 1.0f, 1.0f, 1.0f);
                if(selected_box_ptr)
                {
                    selected_box_ptr->setColor(original_box_color);
                }
                selected_box_ptr = track.addBox(new_box);
                original_box_color = selected_box_ptr->getColor();
                selected_box_ptr->setColor(Vec3(0.5f, 0.5f, 0.5f));
            } break;
            case SELECT_BOX:
            {
                if(selected_box_ptr)
                {
                    selected_box_ptr->setColor(original_box_color);
                }
                selected_box_ptr = hit_box_ptr;
                original_box_color = selected_box_ptr->getColor();
                selected_box_ptr->setColor(Vec3(0.5f, 0.5f, 0.5f));
            } break;
            case DESELECT:
            {
                selected_box_ptr->setColor(original_box_color);
                selected_box_ptr = nullptr;
            } break;
            case CHANGE_BOX_LENGTH:
            {
                float x_norm = cursor_movement_x / window_width * aspect_ratio;
                float y_norm = cursor_movement_y / window_height;
                Vec3 cursor_vec = Vec3(camera.getCameraTransform() * Vec4(x_norm, y_norm, 0.0f, 0.0f));
                Vec3 cam_pos = camera.getPosition();                
                Vec3 box_normal = selected_box_ptr->getSideNormal(box_hit_side);
                float dist_cam_to_hitpoint = fabs((cam_pos - raycast_hit_point).length());
                float amount = dot(cursor_vec, box_normal) * dist_cam_to_hitpoint;
                selected_box_ptr->changeLength(box_hit_side, amount);
            } break;
            case MOVE_SELECTED_BOX:
            {
                float x_norm = cursor_movement_x / window_width * aspect_ratio;
                float y_norm = cursor_movement_y / window_height;
                Vec3 cursor_vec = Vec3(camera.getCameraTransform() * Vec4(x_norm, y_norm, 0.0f, 0.0f));
                Vec3 cam_pos = camera.getPosition();
                manip.moveBox(*selected_box_ptr, cursor_vec);
            } break;
            default:
                break;
            }
        }else if(g_game_mode == PLAY)
        {
            // Update ship position and velocity based on velocity from last frame
            // Update ship velocity based on keyboard input
            if(g_mode_change)
            {
                // Last frame was in editor mode
                // Save editor mode camera pos and orientation
                editor_camera_pos = camera.getPosition();
                editor_camera_euler_ang = camera.getEulerAng();
                camera.setPosAndOrientation(Vec3(), Vec3());
                g_mode_change = false;
            }
            if(g_input.r == 1)
            {
                ship.resetPosition();
            }
            int accel_states[3];
            calcShipAccelState(accel_states, g_input);
            ship.calcVelocity(accel_states);
            ship.updatePosAndVelocity(dt, track);
            camera.setPosRelativeToShip(ship);
        }
        // NOTE: separate rendering code for different motes
        view_transform = camera.getViewTransform();
        ship.updateDynamicUniforms(view_transform);
        ship.draw();
        track.setViewTransform(view_transform);
        track.draw();
        if(g_game_mode == EDITOR)
        {
            line_grid.setViewTransform(view_transform);
            glEnable(GL_BLEND);    
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            line_grid.draw();
            glDisable(GL_BLEND);
            if(selected_box_ptr)
            {
                bwfd.drawWireframeOnBox(*selected_box_ptr, view_transform);
                glDisable(GL_DEPTH_TEST);
                manip.attachToBox(*selected_box_ptr);                                
                manip.draw(view_transform);
                glEnable(GL_DEPTH_TEST);
            }
        }

        last_cursor_x = cursor_x;
        last_cursor_y = cursor_y;        
        //ImGui::Render();
		glfwSwapBuffers(window); // Takes about 0.017 sec or 1/60 sec
	}

    //ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}
