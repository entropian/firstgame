/*
  TODO:
  make the main game loop timestep based
  make ship velocity work with timestep
  move ship scale into class itself
  figure out what sides the bbox collided on
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
#include "clock.h"

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

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
        case GLFW_KEY_W:
        {
            g_camera_ptr->move(4, 0.1f); 
        } break;
        case GLFW_KEY_A:
        {
            g_camera_ptr->move(1, 0.1f);
        } break;
        case GLFW_KEY_S:
        {
            g_camera_ptr->move(5, 0.1f); 
        } break;
        case GLFW_KEY_D:
        {
            g_camera_ptr->move(0, 0.1f);
        } break;
        case GLFW_KEY_T:
        {
            g_camera_ptr->turnSideways(5.0f);
        } break;
        case GLFW_KEY_R:
        {
            g_camera_ptr->turnSideways(-5.0f);
        } break;
        case GLFW_KEY_G:
        {
            if(g_box_ptr && g_box_side != -1)
            {
                g_box_ptr->changeLength(g_box_side, 0.1f);
            }
        } break;
        case GLFW_KEY_H:
        {
            if(g_box_ptr && g_box_side != -1)
            {
                g_box_ptr->changeLength(g_box_side, -0.1f);
            }
        } break;        
        case GLFW_KEY_Q:
        {
            EXIT = true;
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
    Camera camera(MINUS_Z, UP, ORIGIN, fov, aspect_ratio);
    g_camera_ptr = &camera;
    Mat4 view_transform = camera.getViewTransform();


    // Ship transforms
    Mat4 rotation = Mat4::makeYRotation(180.0f) * Mat4::makeXRotation(90.0f);
    Mat4 translation = Mat4::makeTranslation(Vec3(0.0f, 0.0f, -2.0f));
    Mat4 model = translation * rotation;
    Mat4 ship_normal_transform = ((view_transform * model.inverse())).transpose();

    Ship ship;
    ship.setUniforms(model, view_transform, ship_normal_transform, proj_transform, dir_light_1, dir_light_2);
    BBox ship_bbox = ship.getBBox();
    ship_length = fabs(ship_bbox.min[2] - ship_bbox.max[2]);


    // Box transforms
    Mat4 transform = view_transform;
    Mat4 normal_transform = (transform.inverse()).transpose();
    
    Vec3 min(-0.5f, -0.5f, -4.0f);
    Vec3 max(0.5f, 0.5f, -2.0f);
    Vec3 center = (max - min) * 0.5 + min;
    Box box(min, max);    
    box.setUniforms(transform, normal_transform, proj_transform, dir_light_1, dir_light_2);

    std::vector<Box> boxes;
    boxes.push_back(box);

    // IMGUI stuff
    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    bool left_clicking = false;
    glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window) && !EXIT)
	{
        glfwPollEvents();
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
            // To get a vector in world space
            // get window coords
            // get difference
            // divide by window physical dimension
            // multiply by camera x and y basis
            // dot the vector with the box's vector
            // multiply the dot product with a certain constant
            // change the box's length by that amount
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
        
        // Update view transform in shaders        
        view_transform = camera.getViewTransform();
        //ship.setViewTransform(view_transform);
        //ship.draw();
        box.setViewTransform(view_transform);
        box.draw();        

        //ImGui::Render();
		glfwSwapBuffers(window);
	}

    //ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}
