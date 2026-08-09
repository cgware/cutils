#include "cmath.h"

float float_clamp(float value, float min, float max)
{
	if (value < min) {
		return min;
	}
	if (value > max) {
		return max;
	}
	return value;
}

float float_wrap_angle(float angle)
{
	const float pi	   = 3.14159265358979323846f;
	const float two_pi = 6.28318530717958647692f;
	while (angle > pi) {
		angle -= two_pi;
	}
	while (angle < -pi) {
		angle += two_pi;
	}
	return angle;
}

static float float_sin_poly(float x)
{
	float x2 = x * x;
	return x * (1.0f + x2 * (-0.1666666667f + x2 * (0.0083333333f + x2 * -0.0001984127f)));
}

static float float_cos_poly(float x)
{
	float x2 = x * x;
	return 1.0f + x2 * (-0.5f + x2 * (0.0416666667f + x2 * -0.0013888889f));
}

float float_sin(float angle)
{
	const float pi	    = 3.14159265358979323846f;
	const float half_pi = 1.57079632679489661923f;
	float x		    = float_wrap_angle(angle);
	if (x > half_pi) {
		x = pi - x;
	} else if (x < -half_pi) {
		x = -pi - x;
	}
	return float_sin_poly(x);
}

float float_cos(float angle)
{
	const float pi	    = 3.14159265358979323846f;
	const float half_pi = 1.57079632679489661923f;
	float x		    = float_wrap_angle(angle);
	float sign	    = 1.0f;
	if (x > half_pi) {
		x    = pi - x;
		sign = -1.0f;
	} else if (x < -half_pi) {
		x    = -pi - x;
		sign = -1.0f;
	}
	return sign * float_cos_poly(x);
}

float float_sqrt(float value)
{
	if (value <= 0.0f) {
		return 0.0f;
	}
	float x = value > 1.0f ? value : 1.0f;
	for (int i = 0; i < 8; i++) {
		x = 0.5f * (x + value / x);
	}
	return x;
}

vec2f_t vec2f(float x, float y)
{
	return (vec2f_t){x, y};
}

vec2f_t vec2f_add(vec2f_t a, vec2f_t b)
{
	return vec2f(a.x + b.x, a.y + b.y);
}

vec2f_t vec2f_sub(vec2f_t a, vec2f_t b)
{
	return vec2f(a.x - b.x, a.y - b.y);
}

vec2f_t vec2f_mul(vec2f_t a, vec2f_t b)
{
	return vec2f(a.x * b.x, a.y * b.y);
}

vec2f_t vec2f_div(vec2f_t a, vec2f_t b)
{
	return vec2f(a.x / b.x, a.y / b.y);
}

vec2f_t vec2f_scale(vec2f_t v, float s)
{
	return vec2f(v.x * s, v.y * s);
}

float vec2f_dot(vec2f_t a, vec2f_t b)
{
	return a.x * b.x + a.y * b.y;
}

float vec2f_len2(vec2f_t v)
{
	return vec2f_dot(v, v);
}

vec3f_t vec3f(float x, float y, float z)
{
	return (vec3f_t){x, y, z};
}

vec3f_t vec3f_add(vec3f_t a, vec3f_t b)
{
	return vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3f_t vec3f_sub(vec3f_t a, vec3f_t b)
{
	return vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3f_t vec3f_mul(vec3f_t a, vec3f_t b)
{
	return vec3f(a.x * b.x, a.y * b.y, a.z * b.z);
}

vec3f_t vec3f_div(vec3f_t a, vec3f_t b)
{
	return vec3f(a.x / b.x, a.y / b.y, a.z / b.z);
}

vec3f_t vec3f_scale(vec3f_t v, float s)
{
	return vec3f(v.x * s, v.y * s, v.z * s);
}

float vec3f_dot(vec3f_t a, vec3f_t b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3f_len2(vec3f_t v)
{
	return vec3f_dot(v, v);
}

float vec3f_len(vec3f_t v)
{
	return float_sqrt(vec3f_len2(v));
}

vec3f_t vec3f_normalize(vec3f_t v)
{
	float len = vec3f_len(v);
	if (len <= 0.0f) {
		return vec3f(0.0f, 0.0f, 0.0f);
	}
	return vec3f_scale(v, 1.0f / len);
}

vec3f_t vec3f_cross(vec3f_t a, vec3f_t b)
{
	return vec3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

vec4f_t vec4f(float x, float y, float z, float w)
{
	return (vec4f_t){x, y, z, w};
}

vec4f_t vec4f_add(vec4f_t a, vec4f_t b)
{
	return vec4f(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

vec4f_t vec4f_sub(vec4f_t a, vec4f_t b)
{
	return vec4f(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

vec4f_t vec4f_mul(vec4f_t a, vec4f_t b)
{
	return vec4f(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

vec4f_t vec4f_div(vec4f_t a, vec4f_t b)
{
	return vec4f(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

vec4f_t vec4f_scale(vec4f_t v, float s)
{
	return vec4f(v.x * s, v.y * s, v.z * s, v.w * s);
}

float vec4f_dot(vec4f_t a, vec4f_t b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float vec4f_len2(vec4f_t v)
{
	return vec4f_dot(v, v);
}

mat4f_t mat4f_identity(void)
{
	// clang-format off
	return (mat4f_t){ .m = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	}};
	// clang-format on
}

mat4f_t mat4f_transpose(mat4f_t m)
{
	mat4f_t r = {0};
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			r.m[col * 4 + row] = m.m[row * 4 + col];
		}
	}
	return r;
}

mat4f_t mat4f_mul(mat4f_t a, mat4f_t b)
{
	mat4f_t r = {0};
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			for (int i = 0; i < 4; i++) {
				r.m[col * 4 + row] += a.m[i * 4 + row] * b.m[col * 4 + i];
			}
		}
	}
	return r;
}

vec4f_t mat4f_mul_vec4(mat4f_t m, vec4f_t v)
{
	return vec4f(m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w,
		     m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w,
		     m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w,
		     m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w);
}

mat4f_t mat4f_translate(vec3f_t v)
{
	mat4f_t m = mat4f_identity();
	m.m[12]	  = v.x;
	m.m[13]	  = v.y;
	m.m[14]	  = v.z;
	return m;
}

mat4f_t mat4f_scale(vec3f_t v)
{
	mat4f_t m = mat4f_identity();
	m.m[0]	  = v.x;
	m.m[5]	  = v.y;
	m.m[10]	  = v.z;
	return m;
}

mat4f_t mat4f_rotate_x(float c, float s)
{
	mat4f_t m = mat4f_identity();
	m.m[5]	  = c;
	m.m[6]	  = s;
	m.m[9]	  = -s;
	m.m[10]	  = c;
	return m;
}

mat4f_t mat4f_rotate_y(float c, float s)
{
	mat4f_t m = mat4f_identity();
	m.m[0]	  = c;
	m.m[2]	  = -s;
	m.m[8]	  = s;
	m.m[10]	  = c;
	return m;
}

mat4f_t mat4f_rotate_z(float c, float s)
{
	mat4f_t m = mat4f_identity();
	m.m[0]	  = c;
	m.m[1]	  = s;
	m.m[4]	  = -s;
	m.m[5]	  = c;
	return m;
}

mat4f_t mat4f_ortho(float left, float right, float bottom, float top, float near, float far)
{
	mat4f_t m = {0};
	m.m[0]	  = 2.0f / (right - left);
	m.m[5]	  = 2.0f / (top - bottom);
	m.m[10]	  = -2.0f / (far - near);
	m.m[12]	  = -(right + left) / (right - left);
	m.m[13]	  = -(top + bottom) / (top - bottom);
	m.m[14]	  = -(far + near) / (far - near);
	m.m[15]	  = 1.0f;
	return m;
}

mat4f_t mat4f_frustum(float left, float right, float bottom, float top, float near, float far)
{
	mat4f_t m = {0};
	m.m[0]	  = (2.0f * near) / (right - left);
	m.m[5]	  = (2.0f * near) / (top - bottom);
	m.m[8]	  = (right + left) / (right - left);
	m.m[9]	  = (top + bottom) / (top - bottom);
	m.m[10]	  = -(far + near) / (far - near);
	m.m[11]	  = -1.0f;
	m.m[14]	  = -(2.0f * far * near) / (far - near);
	return m;
}

mat4f_t mat4f_look_to(vec3f_t eye, vec3f_t forward, vec3f_t up)
{
	vec3f_t f    = vec3f_normalize(forward);
	vec3f_t r    = vec3f_normalize(vec3f_cross(f, up));
	vec3f_t u    = vec3f_cross(r, f);
	vec3f_t back = vec3f_scale(f, -1.0f);

	mat4f_t view = mat4f_identity();
	view.m[0]    = r.x;
	view.m[4]    = r.y;
	view.m[8]    = r.z;
	view.m[12]   = -vec3f_dot(r, eye);
	view.m[1]    = u.x;
	view.m[5]    = u.y;
	view.m[9]    = u.z;
	view.m[13]   = -vec3f_dot(u, eye);
	view.m[2]    = back.x;
	view.m[6]    = back.y;
	view.m[10]   = back.z;
	view.m[14]   = -vec3f_dot(back, eye);
	return view;
}
