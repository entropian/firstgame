/*
  TODO:
  add translation to ship
  make the main game loop timestep based
  make ship velocity work with timestep
  move ship scale into class itself
  add some kind of grid for track editing
  lookat for camera
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

bool EXIT = false;

bool g_left_clicking = false;
double g_click_xpos = 0.0;
double g_click_ypos = 0.0;
double g_cursor_xpos = 0.0;
double g_cursor_ypos = 0.0;
float g_aspect_ratio = 0.0f;
unsigned int g_window_width = 0;
unsigned int g_window_height = 0;

Camera *g_camera_ptr = NULL;
Box *g_box_ptr = NULL;
Box g_box_unmodded;
int g_box_side = -1;

float ship_length = 0.0f;

// Game mode
// 0 = play mode
// 1 = edit mode
unsigned int g_game_mode = 0;

struct Input
{    
    int w;
    int a;
    int s;
    int d;
    int q;

    unsigned int left_click;
    unsigned int right_click;
    Input()
    {
        w = 0;
        a = 0;
        s = 0;
        d = 0;
        q = 0;
    }
};
void initInput(Input& input)
{
    input.w = 0;
    input.a = 0;
    input.s = 0;
    input.d = 0;
    input.q = 0;
}
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
    // TODO: y
    accel_states[1] = 0;
}

const float MAX_Z_VELOCITY = 20.0f;
const float MAX_X_VELOCITY = 5.0f;
void calcShipVelocity(Ship& ship, int accel_states[3])
{
    // TODO: velocity oscillates around 0    
    // Z velocity
    if(ship.velocity[2] < 0.0f)
    {
        if(accel_states[2] == -1)
        {
            printf("not 0 positive\n");
            // Accelerating in the same direction as ship motion
            ship.velocity[2] = -MAX_Z_VELOCITY;
        }else if(accel_states[2] == 1)
        {
            // Acclerating in the opposite direction as ship motion
            ship.velocity[2] += 5.0f;
        }else
        {
            // Slowing down to 0
            ship.velocity[2] += 3.0f;
        }
    }else if(ship.velocity[2] > 0.0f)
    {
        if(accel_states[2] == -1)
        {
            // Accelerating in the opposite direction as ship motion
            ship.velocity[2] -= 5.0;
        }else if(accel_states[2] == 1)
        {
            // Acclerating in the same direction as ship motion
            ship.velocity[2] = MAX_Z_VELOCITY;
        }else
        {
            // Slowing down to 0
            ship.velocity[2] -= 3.0f;
        }        
    }else
    {
        if(accel_states[2] == -1)
        {
            // Forward
            printf("0 positive\n");
            ship.velocity[2] = -MAX_Z_VELOCITY;
        }else if(accel_states[2] == 1)
        {
            // Backward
            ship.velocity[2] = MAX_Z_VELOCITY;
        }
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
        case GLFW_KEY_Q:
        {
            g_input.q = 1;
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
        case GLFW_KEY_Q:
        {
            g_input.q = 0;
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
            g_left_clicking = true;
            glfwGetCursorPos(window, &g_click_xpos, &g_click_ypos);
            std::cout << "xpos: " << g_click_xpos << " ypos: " << g_click_ypos << std::endl;
        }else if(action == GLFW_RELEASE)
        {
            g_left_clicking = false;
        }
    }
}

void cursorPosCallback(GLFWwindow *window, double x, double y)
{
    g_cursor_xpos = x;
    g_cursor_ypos = y;
}

void getNormalizedWindowCoord(float& x, float& y, const unsigned int x_pos, const unsigned int y_pos,
                              GLFWwindow *window)
{
    int window_width = 0, window_height = 0;
    glfwGetWindowSize(window, &window_width, &window_height);
    x = (float)(x_pos - window_width/2.0f) / (window_width / 2.0f);
    y = (float)((window_height - y_pos) - window_height/2.0f) / (window_height / 2.0f);
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
    unsigned int window_width = 720;
    unsigned int window_height = 720;
    g_window_width = window_width;
    g_window_height = window_height;
	GLFWwindow *window = initWindow(window_width, window_height);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // Setup ImGui binding
    //ImGui_ImplGlfwGL3_Init(window, true);

	/*
		Input data:
			mesh
               ship
            level geometry
			texture
               ship texture
               cube map
			shaders            
	*/

    GLint num_tex_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_tex_units);
    std::cout << "Number of texture units: " << num_tex_units << std::endl;
    
    // Light
    Vec3 dir_light_1(normalize(Vec3(-1.0f, 1.0f, 1.0f)));
    Vec3 dir_light_2(normalize(Vec3(1.0f, 1.0f, 1.0f)));
        
    // Transforms
    float aspect_ratio  = (float)window_width / (float)window_height;
    g_aspect_ratio = aspect_ratio;
    float fov = 90.0f;
    // NOTE: where to put proj_transform?
    Mat4 proj_transform(Mat4::makePerspective(fov, aspect_ratio, 0.001f, 20.0f));
    Camera camera(MINUS_Z, UP, Vec3(0.0f, 1.0f, 4.0f), fov, aspect_ratio);
    g_camera_ptr = &camera;
    Mat4 view_transform = camera.getViewTransform();


    // Ship transforms
    //Mat4 ship_normal_transform = ((view_transform * model.inverse())).transpose();
    Ship ship;
    ship.setStaticUniforms(proj_transform, dir_light_1, dir_light_2);


    // Box transforms
    Mat4 transform = view_transform;
    Mat4 normal_transform = (transform.inverse()).transpose();

    // Track stuff

    Track track;
    //track.addBox(Box(Vec3(-10.0f, -0.5f, -100.0f), Vec3(10.0f, 0.0f, 10.0f)));
    track.setUniforms(transform, normal_transform, proj_transform, dir_light_1, dir_light_2);


    // Box
    Vec3 min(-0.5f, -0.5f, -4.0f);
    Vec3 max(0.5f, 0.5f, -2.0f);
    Vec3 center = (max - min) * 0.5 + min;
    Box box(min, max);    
    //track.addBox(box);

    //std::vector<Box> boxes;
    //boxes.push_back(box);



    // IMGUI stuff
    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    GlobalClock gclock;
    
    bool left_clicking = false;
    glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window) && !EXIT)
	{        
        glfwPollEvents();
        gclock.update();
        float dt = gclock.getDtSeconds();
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
        
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(g_game_mode == 1)
        {
            if(!left_clicking && g_left_clicking)
            {
                float click_x, click_y;
                getNormalizedWindowCoord(click_x, click_y, g_click_xpos, g_click_ypos, window);
                Ray ray = camera.calcRayFromScreenCoord(click_x, click_y);

                // Intersection
                // TODO: multiple boxes
                float t = box.rayIntersect(g_box_side, ray);
                if(t < TMAX)
                {
                    g_box_ptr = &box;
                    g_box_unmodded = box;
                }else
                {
                    g_box_ptr = nullptr;
                    g_box_unmodded = Box();
                }
                std::cout << "t " << t << std::endl;
                Vec3 hit_point = ray.origin + ray.dir * t;
                printf("hit_point: %f, %f, %f\n", hit_point[0], hit_point[1], hit_point[2]); 
                std::cout << std::endl;
                left_clicking = true;
            }else if(left_clicking && g_left_clicking && g_box_ptr)
            {
                double x_diff = g_cursor_xpos - g_click_xpos;
                double y_diff = -(g_cursor_ypos - g_click_ypos);
                float x_norm = x_diff / g_window_width * g_aspect_ratio;
                float y_norm = y_diff / g_window_height;
                Vec3 cursor_vec = Vec3(g_camera_ptr->getOrientation() * Vec4(x_norm, y_norm, 0.0f, 0.0f));
                Vec3 box_normal = g_box_ptr->getSideNormal(g_box_side);
                // TODO: change 0.1f to something dependent on box distance from camera
                float amount = dot(cursor_vec, box_normal) * 1.0f;
                *g_box_ptr = g_box_unmodded;
                g_box_ptr->changeLength(g_box_side, amount);
            }else if(left_clicking && !g_left_clicking)
            {
                g_box_ptr = nullptr;
                g_box_unmodded = Box();
                left_clicking = false;
            }
        }else if(g_game_mode == 0)
        {
            // Process input
            // Update ship position and velocity based on velocity from last frame
            //ship.updatePosAndVelocity(dt, track);
            // Update ship velocity based on keyboard input
            int accel_states[3];
            calcShipAccelState(accel_states, g_input);
            calcShipVelocity(ship, accel_states);
            printf("accel_states: %d, %d, %d\n", accel_states[0], accel_states[1], accel_states[2]);
            printf("velocity: %f, %f, %f\n", ship.velocity[0], ship.velocity[1], ship.velocity[2]);
            ship.updatePosAndVelocity(dt, track);
            //ship.updateDynamicUniforms(view_transform);
        }

        // Update view transform in shaders        
        view_transform = camera.getViewTransform();
        //ship.setViewTransform(view_transform);
        ship.updateDynamicUniforms(view_transform);
        ship.draw();
        //track.setViewTransform(view_transform);
        //track.draw();

        //ImGui::Render();
		glfwSwapBuffers(window); // Takes about 0.017 sec or 1/60 sec
	}

    //ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}
