#include <GL/glew.h>

static GLuint loadShader(const char *file_path, const GLenum shader_type)
{
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &(file_path), NULL);
	glCompileShader(shader);

    GLint status;
    GLchar info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        if(shader_type == GL_VERTEX_SHADER)
        {
            fprintf(stderr, "Vertex shader compiled incorrectly.\n");
        }else if(shader_type == GL_FRAGMENT_SHADER)
        {
            fprintf(stderr, "Fragment shader compiled incorrectly.\n");
        }
        fprintf(stderr, "%s\n", info_log);
    }
    return shader;
}
