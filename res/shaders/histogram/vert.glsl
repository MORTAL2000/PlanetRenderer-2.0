#version 330 core

//#define MULTISAMPLE

in vec2 vs_vertexPosition;

#ifdef MULTISAMPLE
uniform sampler2DMS textureSampler;
#else
uniform sampler2D textureSampler;
#endif

uniform vec2 textureSize;
uniform int histogramBinCount;
uniform int histogramBrightnessRange;
uniform int channel;

out vec4 fs_colour;

void main(void) {

#ifdef MULTISAMPLE
    vec3 colour = min(texelFetch(textureSampler, ivec2(vs_vertexPosition * textureSize), 0).rgb / histogramBrightnessRange, 1.0);
#else
    vec3 colour = min(texture(textureSampler, vs_vertexPosition).rgb / histogramBrightnessRange, 1.0);
#endif

    float bin = -10.0;
    fs_colour = vec4(-10.0);
    
    if (channel == 0) {
        fs_colour = vec4(1.0, 0.0, 0.0, 0.0);
        bin = colour.r;
    } else if (channel == 1) {
        fs_colour = vec4(0.0, 1.0, 0.0, 0.0);
        bin = colour.g;
    } else if (channel == 2) {
        fs_colour = vec4(0.0, 0.0, 1.0, 0.0);
        bin = colour.b;
    } else if (channel == 3) {
        fs_colour = vec4(0.0, 0.0, 0.0, 1.0);
        bin = dot(colour, vec3(0.299, 0.587, 0.114));
    }

    gl_PointSize = 1.0;
    gl_Position = vec4((bin * 2.0 - 1.0) * ((histogramBinCount - 1.0) / histogramBinCount), 0.0, 0.0, 1.0);
}