#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <sstream>

struct ShaderResult {
    std::string vertexShader;
    std::string fragmentShader;
};



static ShaderResult ParseShaders(const std::string& path) {
    enum class ShaderType : char {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };
    ShaderType type = ShaderType::NONE;
    std::fstream shaderFile(path);
    if (!shaderFile.is_open()) {
        throw std::runtime_error("Error: shader file can't open");
    }
    std::stringstream ss[2];
    std::string line;
    while (getline(shaderFile, line)) {
        if (line.find("$shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            if (type != ShaderType::NONE) {
                ss[(int)type] << line << "\n";
            }
        }

    }
    return { ss[(int)ShaderType::VERTEX].str(), ss[(int)ShaderType::FRAGMENT].str() };
}

static int CompileShader(const std::string &shader,unsigned int type) {

    unsigned int id = glCreateShader(type);
    const char* src = shader.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result != GL_TRUE) {
        int len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char* message = (char*)alloca(sizeof(char) * len);
        glGetShaderInfoLog(id, len, &len, message);
        std::cout << "Shader Error : \n" << message << std::endl;

    }
    return id;

}

static int CreateShader(const std::string &vertexShader,const std::string &fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "Error!" << std::endl;
    }

    float positions[6] = {
       -0.5f, -0.5f,
        0.0f,  0.5f,
        0.5f, -0.5f
        
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    ShaderResult result = ParseShaders("Src/Shaders/test_shader.shader");
    
    unsigned int shaderProgram = CreateShader(result.vertexShader, result.fragmentShader);
    glUseProgram(shaderProgram);
    std::cout << result.vertexShader << result.fragmentShader << std::endl;


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

       

        glDrawArrays(GL_TRIANGLES, 0, 3);


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}