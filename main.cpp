#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <string>
#include "mat.h"
#include "texture.h"

#define OBJ_LOADER_IMPLEMENTATION
#include "objloader/objloader.h"



bool EXIT = false;

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

	GLFWwindow *window = glfwCreateWindow(width, height, "CRaytracer", NULL, NULL);
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

void OBJToTriangles(std::vector<float>& triangles, const OBJShape& obj_shape)
{
	for (int i = 0; i < obj_shape.num_indices; ++i)
	{
		int index = obj_shape.indices[i] * 3;
		triangles.push_back(obj_shape.positions[index]);
		triangles.push_back(obj_shape.positions[index + 1]);
		triangles.push_back(obj_shape.positions[index + 2]);
	}
}

int main()
{
	GLFWwindow *window = initWindow(720, 480);
    glfwSetKeyCallback(window, keyCallback);
	/*
		TODO:
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
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
    
    // Loading scene data
    std::string model_base_path("models/");    
    std::string model_file_path(model_base_path + "Ship2.obj");
	OBJShape *obj_shapes;
    OBJMaterial *obj_materials;
    int num_shapes, num_mat;
    loadOBJ(&obj_shapes, &obj_materials, &num_shapes, &num_mat, model_file_path.c_str());

    // Geometry
	std::vector<float> triangles;
	OBJToTriangles(triangles, obj_shapes[0]);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * triangles.size(),
                 &(triangles[0]), GL_STATIC_DRAW);

    // Texture
    /* Loading texture files into memory  */
    Texture ship_diffuse_tex("models/Ship2_diffuse.png");
    Texture ship_normal_tex("models/Ship2_Normal.png");

    /* Load textures into texture objects */
    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint ship_diffuse_handle, ship_normal_handle;
    glGenTextures(1, &ship_diffuse_handle);
    glGenTextures(1, &ship_normal_handle);

    glBindTexture(GL_TEXTURE_2D, ship_diffuse_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ship_diffuse_tex.width, ship_diffuse_tex.height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, ship_diffuse_tex.data);

    glBindTexture(GL_TEXTURE_2D, ship_normal_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ship_normal_tex.width, ship_normal_tex.height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, ship_normal_tex.data);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Vertex shader
    std::ifstream vert_fstream("shaders/first.vs");
    std::stringstream buffer;
    buffer << vert_fstream.rdbuf();
    vert_fstream.close();
    std::string vert_src = buffer.str();
    const char* vert_src_cstr = vert_src.c_str();

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &(vert_src_cstr), NULL);
	glCompileShader(vertShader);

    // Fragment shader
    std::ifstream frag_fstream("shaders/first.fs");
    buffer.str("");
    buffer << frag_fstream.rdbuf();
    std::string frag_src = buffer.str();
    const char* frag_src_cstr = frag_src.c_str();
    
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &(frag_src_cstr), NULL);
	glCompileShader(fragShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

    // Transforms
    Vec3 camera_pos(0.0f, 0.0f, 0.0f);
    Mat4 view_transform = Mat4::makeTranslation(-camera_pos);

    Mat4 proj_transform(Mat4::makePerspective(60.0f, 16.0f/9.0f, 0.001f, 50.0f));
    
    Mat4 scale = Mat4::makeScale(Vec3(0.1f, 0.1f, 0.1f));
    Mat4 rotation = Mat4::makeXRotation(90.0f);
    Mat4 translation = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 0.0f));
    Mat4 transform = view_transform * translation * rotation * scale;

    // Setting uniforms
    GLint model_view_handle = glGetUniformLocation(shaderProgram, "model_view");
    GLint proj_handle = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(model_view_handle, 1, GL_TRUE, &(transform.data[0][0]));
    glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));

    // Setting attributes
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	while (!glfwWindowShouldClose(window) && !EXIT)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, triangles.size());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragShader);
	glDeleteShader(vertShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	for (int i = 0; i < num_shapes; ++i)
	{
		OBJShape_destroy(&(obj_shapes[i]));
	}
	glfwTerminate();
	return 0;
}
