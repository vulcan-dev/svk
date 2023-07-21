#ifndef PRIMITIVES_H
#define PRIMITIVES_H

static svkVertex triangleVertices[3] =
{
    {{  0.0f,  -0.15f }, { 0.99f, 0.02f, 0.99f }},
    {{  0.15f,  0.15f }, { 0.99f, 0.99f, 0.02f }},
    {{ -0.15f,  0.15f }, { 0.02f, 0.99f, 0.99f }}
};

static svkVertex triangleVertices2[3] =
{
    {{  0.7f, -0.7f }, { 0.02f, 0.99f, 0.99f }},
    {{  0.7f,  0.7f }, { 0.99f, 0.99f, 0.02f }},
    {{ -0.7f,  0.7f }, { 0.99f, 0.02f, 0.99f }}
};

static svkVertex cubeVertices[8] = {
    // Vertex positions          // Vertex colors
    {{ -0.5f, -0.5f, -0.5f }, { 0.02f, 0.99f, 0.99f }},
    {{  0.5f, -0.5f, -0.5f }, { 0.99f, 0.99f, 0.02f }},
    {{  0.5f,  0.5f, -0.5f }, { 0.99f, 0.02f, 0.99f }},
    {{ -0.5f,  0.5f, -0.5f }, { 0.02f, 0.56f, 0.99f }},
    {{ -0.5f, -0.5f,  0.5f }, { 0.02f, 0.99f, 0.99f }},
    {{  0.5f, -0.5f,  0.5f }, { 0.99f, 0.99f, 0.02f }},
    {{  0.5f,  0.5f,  0.5f }, { 0.99f, 0.02f, 0.99f }},
    {{ -0.5f,  0.5f,  0.5f }, { 0.02f, 0.56f, 0.99f }}
};

static uint16_t cubeIndices[36] = {
    // Front face
    0, 1, 2,
    2, 3, 0,

    // Back face
    4, 5, 6,
    6, 7, 4,

    // Left face
    7, 3, 0,
    0, 4, 7,

    // Right face
    1, 5, 6,
    6, 2, 1,

    // Top face
    3, 2, 6,
    6, 7, 3,

    // Bottom face
    0, 1, 5,
    5, 4, 0
};

static uint16_t rectIndices[6] = { 0, 1, 2, 2, 3, 0 };

#endif