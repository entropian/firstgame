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

bool EXIT = false;

double g_click_xpos = 0.0;
double g_click_ypos = 0.0;
double g_cursor_xpos = 0.0;
double g_cursor_ypos = 0.0;
bool g_left_clicking = false;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
        case GLFW_KEY_Q:
            EXIT = true;
            break;
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
        }else
        {
            g_left_clicking = false;
        }
    }
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
    int window_width = 720;
    int window_height = 720;
	GLFWwindow *window = initWindow(window_width, window_height);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
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
    Vec3 camera_pos(0.0f, 0.0f, 0.0f);
    //Vec3 camera_dir = MINUS_Z;
    Vec3 camera_dir = PLUS_Z;
    Mat4 camera_transform = Mat4::makeTranslation(camera_pos) * Mat4::makeYRotation(180.0f);
    // NOTE: it looks like that image plane of the camera is at the origin instead of at z = -1
    //       To compensate, I subtract the unit camera vector from the camera position to move it back
    //       by 1
    Mat4 view_transform = Mat4::makeTranslation(camera_pos) *
        Mat4::makeYRotation(180.0f) * Mat4::makeTranslation(-camera_dir);
    float aspect_ratio  = (float)window_width / (float)window_height;
    float fov = 90.0f;
    Mat4 proj_transform(Mat4::makePerspective(fov, aspect_ratio, -0.001f, -20.0f));

    /*
    // Ship transforms
    Mat4 scale = Mat4::makeScale(Vec3(0.2f, 0.2f, 0.2f));
    Mat4 rotation = Mat4::makeYRotation(180.0f) * Mat4::makeXRotation(90.0f);
    Mat4 translation = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 0.0f));
    Mat4 model = translation * rotation * scale;
    Mat4 transform = view_transform * model;
    Mat4 normal_transform = (model.inverse()).transpose();

    Ship ship;
    ship.setUniforms(transform, normal_transform, proj_transform, dir_light_1, dir_light_2);
    */

    // Box transforms
    Mat4 model;
    Mat4 transform = view_transform * model;
    Mat4 normal_transform = (model.inverse()).transpose();
    
    Vec3 min(-0.5f, -0.5f, 2.0f);
    Vec3 max(0.5f, 0.5f, 3.0f);
    Vec3 center = (max - min) * 0.5 + min;
    Box box(min, max);    
    box.setUniforms(transform, normal_transform, proj_transform, dir_light_1, dir_light_2);

    Vec3 top_left_back(min[0], max[1], max[2]);
    printf("top_left_back: %f, %f, %f\n", top_left_back[0], top_left_back[1], top_left_back[2]);

    Vec3 ray_origin(0.0f, 0.0f, 0.0f);
    Vec3 ray_dir = normalize(top_left_back - ray_origin);
    printf("ray_dir: %f, %f, %f\n", ray_dir[0], ray_dir[1], ray_dir[2]);

    std::vector<Box> boxes;
    boxes.push_back(box);

    // TODO: test in positive z
    
    bool left_clicking = false;
    glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window) && !EXIT)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(!left_clicking && g_left_clicking)
        {
            // TODO: calc ray
            // Assume focal length = 1
            std::cout << std::endl;
            //float half_frame_width = sinf((fov * 0.5f) / 180 * PI);
            //float half_frame_height = half_frame_width / aspect_ratio;
            // NOTE: HALF FRAME WIDTH ISN"T 1?!?!?!?!?!?!
            float boost_y = 1.0f / cos(fov*0.5f*(float)PI/180.0f);
            float half_frame_height = sin(fov*0.5f*(float)PI/180.0f) * boost_y;
            float half_frame_width = half_frame_height * aspect_ratio;
            std::cout << "half_frame_width " << half_frame_width << std::endl;
            std::cout << "half_frame_height " << half_frame_height << std::endl;

            float click_x = (float)(g_click_xpos - window_width/2.0f) / (window_width / 2.0f);
            float click_y = (float)((window_height - g_click_ypos) - window_height/2.0f) / (window_height / 2.0f);

            Vec3 ray_dir;
            ray_dir[0] = click_x * half_frame_width;
            ray_dir[1] = click_y * half_frame_height;
            ray_dir[2] = -1.0f;
            printf("ray_dir unnormalized %f, %f, %f\n", ray_dir[0], ray_dir[1], ray_dir[2]);
            ray_dir = ray_dir.normalize();
            Vec3 ray_origin;

            // Transform ray
            ray_dir = camera_transform * Vec4(ray_dir, 0.0f);
            ray_origin = camera_transform * Vec4(ray_origin, 1.0f);

            printf("origin: %f, %f, %f\n", ray_origin[0], ray_origin[1], ray_origin[2]);
            printf("dir: %f, %f, %f\n", ray_dir[0], ray_dir[1], ray_dir[2]);
            

            float t = box.rayIntersect(ray_origin, ray_dir);
            std::cout << "t " << t << std::endl;
            Vec3 hit_point = ray_origin + ray_dir * t;
            printf("hit_point: %f, %f, %f\n", hit_point[0], hit_point[1], hit_point[2]); 

            std::cout << "click_x " << click_x << " click_y " << click_y << std::endl;
            std::cout << std::endl;
            left_clicking = true;
        }else if(left_clicking && !g_left_clicking)
        {
            left_clicking = false;
        }

        //ship.draw();
        box.draw();        

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
