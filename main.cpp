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

double g_click_xpos = 0.0;
double g_click_ypos = 0.0;
double g_cursor_xpos = 0.0;
double g_cursor_ypos = 0.0;
bool g_left_clicking = false;
Camera *g_camera_ptr = NULL;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
        case GLFW_KEY_W:
        {
            Vec3 camera_movement(0.0f, 0.0f, 0.1f);
            g_camera_ptr->move(camera_movement); 
        } break;
        case GLFW_KEY_A:
        {
            Vec3 camera_movement(-0.1f, 0.0f, 0.0f);
            g_camera_ptr->move(camera_movement);
        } break;
        case GLFW_KEY_S:
        {
            Vec3 camera_movement(0.0f, 0.0f, -0.1f);
            g_camera_ptr->move(camera_movement); 
        } break;
        case GLFW_KEY_D:
        {
            Vec3 camera_movement(0.1f, 0.0f, 0.0f);
            g_camera_ptr->move(camera_movement);
        } break;            
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
    Camera camera(MINUS_Z, UP, ORIGIN);
    g_camera_ptr = &camera;
    Mat4 camera_transform = camera.camera_transform;
    Mat4 view_transform = camera.view_transform;
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
    
    Vec3 min(-0.5f, -0.5f, -3.0f);
    Vec3 max(0.5f, 0.5f, -1.0f);
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

        camera_transform = camera.camera_transform;
        view_transform = camera.view_transform;

        if(!left_clicking && g_left_clicking)
        {
            Vec3 ray_dir, ray_origin;
            calcRay(ray_origin, ray_dir, g_click_xpos, g_click_ypos, window_width, window_height, fov, aspect_ratio,
                    camera_transform);

            // Intersection
            float t = box.rayIntersect(ray_origin, ray_dir);
            std::cout << "t " << t << std::endl;
            Vec3 hit_point = ray_origin + ray_dir * t;
            printf("hit_point: %f, %f, %f\n", hit_point[0], hit_point[1], hit_point[2]); 
            std::cout << std::endl;
            left_clicking = true;
        }else if(left_clicking && !g_left_clicking)
        {
            left_clicking = false;
        }

        //ship.draw();
        box.setTransform(view_transform);
        box.draw();        

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
