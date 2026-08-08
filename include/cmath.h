#ifndef CMATH_H
#define CMATH_H

typedef struct vec2f_s {
	float x, y;
} vec2f_t;

typedef struct vec3f_s {
	float x, y, z;
} vec3f_t;

typedef struct vec4f_s {
	float x, y, z, w;
} vec4f_t;

typedef struct mat4f_s {
	float m[16];
} mat4f_t;

vec2f_t vec2f(float x, float y);
vec2f_t vec2f_add(vec2f_t a, vec2f_t b);
vec2f_t vec2f_sub(vec2f_t a, vec2f_t b);
vec2f_t vec2f_mul(vec2f_t a, vec2f_t b);
vec2f_t vec2f_div(vec2f_t a, vec2f_t b);
vec2f_t vec2f_scale(vec2f_t v, float s);
float vec2f_dot(vec2f_t a, vec2f_t b);
float vec2f_len2(vec2f_t v);

vec3f_t vec3f(float x, float y, float z);
vec3f_t vec3f_add(vec3f_t a, vec3f_t b);
vec3f_t vec3f_sub(vec3f_t a, vec3f_t b);
vec3f_t vec3f_mul(vec3f_t a, vec3f_t b);
vec3f_t vec3f_div(vec3f_t a, vec3f_t b);
vec3f_t vec3f_scale(vec3f_t v, float s);
float vec3f_dot(vec3f_t a, vec3f_t b);
float vec3f_len2(vec3f_t v);
vec3f_t vec3f_cross(vec3f_t a, vec3f_t b);

vec4f_t vec4f(float x, float y, float z, float w);
vec4f_t vec4f_add(vec4f_t a, vec4f_t b);
vec4f_t vec4f_sub(vec4f_t a, vec4f_t b);
vec4f_t vec4f_mul(vec4f_t a, vec4f_t b);
vec4f_t vec4f_div(vec4f_t a, vec4f_t b);
vec4f_t vec4f_scale(vec4f_t v, float s);
float vec4f_dot(vec4f_t a, vec4f_t b);
float vec4f_len2(vec4f_t v);

mat4f_t mat4f_identity(void);
mat4f_t mat4f_transpose(mat4f_t m);
mat4f_t mat4f_mul(mat4f_t a, mat4f_t b);
vec4f_t mat4f_mul_vec4(mat4f_t m, vec4f_t v);
mat4f_t mat4f_translate(vec3f_t v);
mat4f_t mat4f_scale(vec3f_t v);
mat4f_t mat4f_rotate_x(float c, float s);
mat4f_t mat4f_rotate_y(float c, float s);
mat4f_t mat4f_rotate_z(float c, float s);
mat4f_t mat4f_ortho(float left, float right, float bottom, float top, float near, float far);
mat4f_t mat4f_frustum(float left, float right, float bottom, float top, float near, float far);

#endif
