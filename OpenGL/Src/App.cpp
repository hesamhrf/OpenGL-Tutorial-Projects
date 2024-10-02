#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <assert.h>

#define ASSERT(x) if(!(x)) assert(false)
#define GlCall(f) GlClearErrors();\
    f;\
    ASSERT(GlCheckErrors(#f,__FILE__,__LINE__))
    

struct ShaderResult {
    std::string vertexShader;
    std::string fragmentShader;
};

static void GlClearErrors() {
    while (glGetError() != GL_NO_ERROR);
}

static bool GlCheckErrors(const char* functionName,const char* fileName,int lineNumber)
{
    static const std::unordered_map<GLenum, const char*> errorDescriptions = {
        {GL_INVALID_ENUM, "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument."},
        {GL_INVALID_VALUE, "GL_INVALID_VALUE: A numeric argument is out of range."},
        {GL_INVALID_OPERATION, "GL_INVALID_OPERATION: The specified operation is not allowed in the current state."},
        {GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete."},
        {GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command."},
        {GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW: An operation would cause the stack to underflow."},
        {GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW: An operation would cause the stack to overflow."}
    };

    bool noErrors = true;

    while (GLenum error = glGetError()) {
        noErrors = false;
        std::cout << "[OpenGL Error] " << " In File : " << fileName << " Function : " << functionName << " Line : " << lineNumber << " \n";

        auto it = errorDescriptions.find(error);
        if (it != errorDescriptions.end()) {
            std::cout << it->second;
        }
        else {
            std::cout << "Unrecognized Error (Code: " << error << ")";
        }

        std::cout << std::endl;
    }

    return noErrors;
}

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

    float positions[8] = {
       -0.5f, -0.5f,
        0.5f,  0.5f,
        0.5f, -0.5f,
        -0.5f, 0.5f
    };

    unsigned int indices[6] = {
        0,3,1,
        0,1,2
    };

    unsigned int buffer;
    GlCall(glGenBuffers(1, &buffer));
    GlCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GlCall(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW));
    GlCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));
    GlCall(glEnableVertexAttribArray(0));

    unsigned int indexBuffer;
    GlCall(glGenBuffers(1, &indexBuffer));
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    GlCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    ShaderResult result = ParseShaders("Src/Shaders/test_shader.shader");
    
    unsigned int shaderProgram = CreateShader(result.vertexShader, result.fragmentShader);
    GlCall(glUseProgram(shaderProgram));
    //std::cout << result.vertexShader << result.fragmentShader << std::endl;
    GlCall(int location = glGetUniformLocation(shaderProgram, "u_Color"));
    
    float redValue = 0.5f, blueValue = 0.5f;
    float inc = 0.01f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        
        GlCall(glUniform4f(location, redValue, 0.0f, blueValue, 1.0f));
        GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (redValue >= 1.0f || blueValue >= 1.0f) {
            inc = -inc;
        }
        redValue += inc;
        blueValue -= inc;
        

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}