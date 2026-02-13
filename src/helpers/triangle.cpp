#include "triangle.h"
#include <vector>
#include <glad/glad.h>

Triangle::Triangle() {
    std::vector<float> positions = {
        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,

         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
    };

    std::vector<float> colors = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    vertex_count = 6;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    VBOs.resize(2);
    glGenBuffers(2, VBOs.data());

    //positions
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, 
        positions.size() * sizeof(float), 
        positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    
    //colors
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, 
        colors.size() * sizeof(float), 
        colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Triangle::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glBindVertexArray(0);
}

Triangle::~Triangle() {
    glDeleteBuffers(VBOs.size(), VBOs.data());
    glDeleteVertexArrays(1, &VAO);
}