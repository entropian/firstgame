#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

void calcRay(Vec3& ray_origin, Vec3& ray_dir, const unsigned int click_xpos,
             const unsigned int click_ypos, const unsigned int window_width, const unsigned int window_height,
             const float fov, const float aspect_ratio, const Mat4& camera_transform)
{
    // Assume focal length = 1
    std::cout << std::endl;
    float boost_y = 1.0f / cos(fov*0.5f*(float)PI/180.0f);
    float half_frame_height = sin(fov*0.5f*(float)PI/180.0f) * boost_y;
    float half_frame_width = half_frame_height * aspect_ratio;
    std::cout << "half_frame_width " << half_frame_width << std::endl;
    std::cout << "half_frame_height " << half_frame_height << std::endl;

    float click_x = (float)(click_xpos - window_width/2.0f) / (window_width / 2.0f);
    float click_y = (float)((window_height - click_ypos) - window_height/2.0f) / (window_height / 2.0f);

    Vec3 dir;
    dir[0] = click_x * half_frame_width;
    dir[1] = click_y * half_frame_height;
    dir[2] = -1.0f;
    printf("dir unnormalized %f, %f, %f\n", dir[0], dir[1], dir[2]);
    dir = dir.normalize();
    Vec3 origin;

    // Transform ray
    dir = camera_transform * Vec4(dir, 0.0f);
    origin = camera_transform * Vec4(origin, 1.0f);

    printf("origin: %f, %f, %f\n", origin[0], origin[1], origin[2]);
    printf("dir: %f, %f, %f\n", dir[0], dir[1], dir[2]);
    std::cout << "click_x " << click_x << " click_y " << click_y << std::endl;
    ray_origin = origin;
    ray_dir = dir;
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
	/*
		TODO:
		glfwSetCursorPosCallback(window, cursorPosCallback);
	*/

	/*
		Input data:
			mesh
               ship
            level geometry
			texture
               ship texture
               cube map
			shaders            

		Questions:
			Does uploading mesh to vbo means the mesh now also lives on the GPU?
	*/

    GLint num_tex_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_tex_units);
    std::cout << "Number of texture units: " << num_tex_units << std::endl;
    
    // Light
    Vec3 dir_light_1(normalize(Vec3(-1.0f, 1.0f, 1.0f)));
    Vec3 dir_light_2(normalize(Vec3(1.0f, 1.0f, 1.0f)));
        
    // Transforms
    Camera camera(MINUS_Z, UP, ORIGIN);
    g_camera_ptr = &camera;
    Mat4 camera_transform = camera.camera_transform;
    Mat4 view_transform = camera.view_transform;
    //Mat4 view_transform;
    float aspect_ratio  = (float)window_width / (float)window_height;
    g_aspect_ratio = aspect_ratio;
    float fov = 90.0f;
    Mat4 proj_transform(Mat4::makePerspective(fov, aspect_ratio, 0.001f, 20.0f));

    // Ship transforms
    /*
    Mat4 scale = Mat4::makeScale(Vec3(0.2f, 0.2f, 0.2f));
    Mat4 rotation = Mat4::makeYRotation(180.0f) * Mat4::makeXRotation(90.0f);
    Mat4 translation = Mat4::makeTranslation(Vec3(0.0f, 0.0f, -2.0f));
    Mat4 model = translation * rotation * scale;
    Mat4 ship_normal_transform = ((view_transform * model.inverse())).transpose();

    Ship ship;
    ship.setUniforms(model, view_transform, ship_normal_transform, proj_transform, dir_light_1, dir_light_2);
    */

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

    bool left_clicking = false;
    glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window) && !EXIT)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera_transform = camera.camera_transform;
        view_transform = camera.view_transform;

        if(!left_clicking && g_left_clicking)
        {
            Vec3 ray_dir, ray_origin;
            calcRay(ray_origin, ray_dir, g_click_xpos, g_click_ypos, window_width, window_height, fov, aspect_ratio,
                    camera_transform);

            // Intersection
            // TODO: multiple boxes
            float t = box.rayIntersect(g_box_side, ray_origin, ray_dir);
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
            Vec3 hit_point = ray_origin + ray_dir * t;
            printf("hit_point: %f, %f, %f\n", hit_point[0], hit_point[1], hit_point[2]); 
            std::cout << std::endl;
            left_clicking = true;
        }else if(left_clicking && g_left_clicking)
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
            Vec3 cursor_vec = Vec3(g_camera_ptr->orientation * Vec4(x_norm, y_norm, 0.0f, 0.0f));
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

        //ship.setViewTransform(view_transform);
        //ship.draw();
        box.setViewTransform(view_transform);
        box.draw();        

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
