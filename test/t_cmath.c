#include "cmath.h"

#include "test.h"

#define EXPECT_VEC2F(_v, _x, _y)                                                                                                           \
	do {                                                                                                                               \
		vec2f_t _t_v = (_v);                                                                                                       \
		EXPECT_EQ(_t_v.x, (_x));                                                                                                   \
		EXPECT_EQ(_t_v.y, (_y));                                                                                                   \
	} while (0)

#define EXPECT_VEC3F(_v, _x, _y, _z)                                                                                                       \
	do {                                                                                                                               \
		vec3f_t _t_v = (_v);                                                                                                       \
		EXPECT_EQ(_t_v.x, (_x));                                                                                                   \
		EXPECT_EQ(_t_v.y, (_y));                                                                                                   \
		EXPECT_EQ(_t_v.z, (_z));                                                                                                   \
	} while (0)

#define EXPECT_VEC4F(_v, _x, _y, _z, _w)                                                                                                   \
	do {                                                                                                                               \
		vec4f_t _t_v = (_v);                                                                                                       \
		EXPECT_EQ(_t_v.x, (_x));                                                                                                   \
		EXPECT_EQ(_t_v.y, (_y));                                                                                                   \
		EXPECT_EQ(_t_v.z, (_z));                                                                                                   \
		EXPECT_EQ(_t_v.w, (_w));                                                                                                   \
	} while (0)

#define EXPECT_MAT4F(_m, _v)                                                                                                               \
	do {                                                                                                                               \
		mat4f_t _t_m = (_m);                                                                                                       \
		for (int _t_i = 0; _t_i < 16; _t_i++) {                                                                                    \
			EXPECT_EQ(_t_m.m[_t_i], (_v)[_t_i]);                                                                               \
		}                                                                                                                          \
	} while (0)

static int float_near(float value, float expected, float tolerance)
{
	float difference = value - expected;
	return difference >= -tolerance && difference <= tolerance;
}

TEST(cmath_float)
{
	START;

	EXPECT_EQ(float_clamp(2.0f, 0.0f, 1.0f), 1.0f);
	EXPECT_EQ(float_clamp(-1.0f, 0.0f, 1.0f), 0.0f);
	EXPECT_EQ(float_clamp(0.5f, 0.0f, 1.0f), 0.5f);
	EXPECT_EQ(float_mod(7.0f, 3.0f), 1.0f);
	EXPECT_EQ(float_mod(-7.0f, 3.0f), -1.0f);
	EXPECT_EQ(float_deg_to_rad(0.0f), 0.0f);
	EXPECT_EQ(float_deg_to_rad(180.0f), CMATH_PI);
	EXPECT_EQ(float_wrap_angle(7.0f), 0.7168145f);
	EXPECT_EQ(float_wrap_angle(-7.0f), -0.7168145f);
	EXPECT_EQ(float_sin(0.0f), 0.0f);
	EXPECT_EQ(float_sin(3.1415927f), 0.0f);
	EXPECT_EQ(float_sin(-3.1415927f), 0.0f);
	EXPECT_EQ(float_cos(0.0f), 1.0f);
	EXPECT_EQ(float_cos(3.1415927f), -1.0f);
	EXPECT_EQ(float_cos(-3.1415927f), -1.0f);
	EXPECT_EQ(float_tan(0.0f), 0.0f);
	EXPECT_EQ(float_near(float_tan(CMATH_PI / 4.0f), 1.0f, 0.01f), 1);
	EXPECT_EQ(float_sqrt(0.0f), 0.0f);
	EXPECT_EQ(float_sqrt(4.0f), 2.0f);

	END;
}

TEST(cmath_vec2f)
{
	START;

	vec2f_t a = vec2f(2.0f, 8.0f);
	vec2f_t b = vec2f(4.0f, 2.0f);

	EXPECT_VEC2F(vec2f_add(a, b), 6.0f, 10.0f);
	EXPECT_VEC2F(vec2f_sub(a, b), -2.0f, 6.0f);
	EXPECT_VEC2F(vec2f_mul(a, b), 8.0f, 16.0f);
	EXPECT_VEC2F(vec2f_div(a, b), 0.5f, 4.0f);
	EXPECT_VEC2F(vec2f_scale(a, 3.0f), 6.0f, 24.0f);
	EXPECT_EQ(vec2f_dot(a, b), 24.0f);
	EXPECT_EQ(vec2f_len2(a), 68.0f);

	END;
}

TEST(cmath_vec3f)
{
	START;

	vec3f_t a = vec3f(2.0f, 8.0f, 4.0f);
	vec3f_t b = vec3f(4.0f, 2.0f, 1.0f);

	EXPECT_VEC3F(vec3f_add(a, b), 6.0f, 10.0f, 5.0f);
	EXPECT_VEC3F(vec3f_sub(a, b), -2.0f, 6.0f, 3.0f);
	EXPECT_VEC3F(vec3f_mul(a, b), 8.0f, 16.0f, 4.0f);
	EXPECT_VEC3F(vec3f_div(a, b), 0.5f, 4.0f, 4.0f);
	EXPECT_VEC3F(vec3f_scale(a, 3.0f), 6.0f, 24.0f, 12.0f);
	EXPECT_EQ(vec3f_dot(a, b), 28.0f);
	EXPECT_EQ(vec3f_len2(a), 84.0f);
	EXPECT_EQ(vec3f_len(vec3f(0.0f, 3.0f, 4.0f)), 5.0f);
	EXPECT_VEC3F(vec3f_normalize(vec3f(0.0f, 0.0f, 0.0f)), 0.0f, 0.0f, 0.0f);
	EXPECT_VEC3F(vec3f_normalize(vec3f(0.0f, 0.0f, 2.0f)), 0.0f, 0.0f, 1.0f);
	EXPECT_VEC3F(vec3f_cross(vec3f(1.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f)), 0.0f, 0.0f, 1.0f);

	END;
}

TEST(cmath_vec4f)
{
	START;

	vec4f_t a = vec4f(2.0f, 8.0f, 4.0f, 6.0f);
	vec4f_t b = vec4f(4.0f, 2.0f, 1.0f, 3.0f);

	EXPECT_VEC4F(vec4f_add(a, b), 6.0f, 10.0f, 5.0f, 9.0f);
	EXPECT_VEC4F(vec4f_sub(a, b), -2.0f, 6.0f, 3.0f, 3.0f);
	EXPECT_VEC4F(vec4f_mul(a, b), 8.0f, 16.0f, 4.0f, 18.0f);
	EXPECT_VEC4F(vec4f_div(a, b), 0.5f, 4.0f, 4.0f, 2.0f);
	EXPECT_VEC4F(vec4f_scale(a, 3.0f), 6.0f, 24.0f, 12.0f, 18.0f);
	EXPECT_EQ(vec4f_dot(a, b), 46.0f);
	EXPECT_EQ(vec4f_len2(a), 120.0f);

	END;
}

TEST(cmath_mat4f_identity_transpose)
{
	START;

	// clang-format off
	const float identity[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	mat4f_t m = {.m = {
		1.0f, 2.0f, 3.0f, 4.0f,
		5.0f, 6.0f, 7.0f, 8.0f,
		9.0f, 10.0f, 11.0f, 12.0f,
		13.0f, 14.0f, 15.0f, 16.0f,
	}};
	const float transposed[] = {
		1.0f, 5.0f, 9.0f, 13.0f,
		2.0f, 6.0f, 10.0f, 14.0f,
		3.0f, 7.0f, 11.0f, 15.0f,
		4.0f, 8.0f, 12.0f, 16.0f,
	};
	// clang-format on

	EXPECT_MAT4F(mat4f_identity(), identity);
	EXPECT_MAT4F(mat4f_transpose(m), transposed);

	END;
}

TEST(cmath_mat4f_mul_transform)
{
	START;

	mat4f_t t = mat4f_translate(vec3f(2.0f, 3.0f, 4.0f));
	mat4f_t s = mat4f_scale(vec3f(5.0f, 6.0f, 7.0f));
	mat4f_t m = mat4f_mul(t, s);

	// clang-format off
	const float transform[] = {
		5.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 6.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 7.0f, 0.0f,
		2.0f, 3.0f, 4.0f, 1.0f,
	};
	// clang-format on
	EXPECT_MAT4F(m, transform);
	EXPECT_VEC4F(mat4f_mul_vec4(m, vec4f(1.0f, 2.0f, 3.0f, 1.0f)), 7.0f, 15.0f, 25.0f, 1.0f);

	END;
}

TEST(cmath_mat4f_rotate)
{
	START;

	EXPECT_VEC4F(mat4f_mul_vec4(mat4f_rotate_x(0.0f, 1.0f), vec4f(0.0f, 1.0f, 0.0f, 1.0f)), 0.0f, 0.0f, 1.0f, 1.0f);
	EXPECT_VEC4F(mat4f_mul_vec4(mat4f_rotate_y(0.0f, 1.0f), vec4f(0.0f, 0.0f, 1.0f, 1.0f)), 1.0f, 0.0f, 0.0f, 1.0f);
	EXPECT_VEC4F(mat4f_mul_vec4(mat4f_rotate_z(0.0f, 1.0f), vec4f(1.0f, 0.0f, 0.0f, 1.0f)), 0.0f, 1.0f, 0.0f, 1.0f);

	END;
}

TEST(cmath_mat4f_projection)
{
	START;

	// clang-format off
	const float ortho[] = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.25f, 0.0f, 0.0f,
		0.0f, 0.0f, -0.125f, 0.0f,
		0.0f, 0.0f, -1.25f, 1.0f,
	};
	const float frustum[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -3.0f, -1.0f,
		0.0f, 0.0f, -4.0f, 0.0f,
	};
	// clang-format on

	EXPECT_MAT4F(mat4f_ortho(-2.0f, 2.0f, -4.0f, 4.0f, 2.0f, 18.0f), ortho);

	EXPECT_MAT4F(mat4f_frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 2.0f), frustum);

	END;
}

TEST(cmath_mat4f_look_to)
{
	START;

	// clang-format off
	const float view[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -3.0f, -4.0f, 1.0f,
	};
	// clang-format on

	EXPECT_MAT4F(mat4f_look_to(vec3f(2.0f, 3.0f, 4.0f), vec3f(0.0f, 0.0f, -1.0f), vec3f(0.0f, 1.0f, 0.0f)), view);

	END;
}

TEST(cmath_mat4f_transform_normal)
{
	START;

	// clang-format off
	const float transform[] = {
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 4.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 8.0f, 0.0f,
		3.0f, 5.0f, 7.0f, 1.0f,
	};
	const float normal[] = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.25f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.125f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	// clang-format on

	vec3f_t translation    = vec3f(3.0f, 5.0f, 7.0f);
	vec3f_t rotation       = vec3f(0.0f, 0.0f, 0.0f);
	vec3f_t scale	       = vec3f(2.0f, 4.0f, 8.0f);
	mat4f_t rotation_basis = mat4f_rotation(rotation);

	EXPECT_MAT4F(mat4f_transform(translation, rotation_basis, scale), transform);
	EXPECT_MAT4F(mat4f_normal(rotation_basis, scale), normal);

	END;
}

TEST(cmath_mat4f_perspective)
{
	START;

	mat4f_t perspective = mat4f_perspective(CMATH_PI / 2.0f, 2.0f, 1.0f, 3.0f);

	EXPECT_EQ(float_near(perspective.m[0], 0.5f, 0.01f), 1);
	EXPECT_EQ(float_near(perspective.m[5], 1.0f, 0.01f), 1);
	EXPECT_EQ(perspective.m[10], 1.5f);
	EXPECT_EQ(perspective.m[11], 1.0f);
	EXPECT_EQ(perspective.m[14], -1.5f);
	EXPECT_EQ(perspective.m[15], 0.0f);

	END;
}

TEST(cmath_mat4f_view_rotation)
{
	START;

	vec3f_t position       = vec3f(2.0f, 3.0f, 4.0f);
	vec3f_t rotation       = vec3f(0.0f, 0.0f, 0.0f);
	mat4f_t rotation_basis = mat4f_rotation(rotation);
	mat4f_t view	       = mat4f_view(position, rotation_basis);
	mat4f_t inverse	       = mat4f_inverse_view(position, rotation_basis);

	EXPECT_VEC4F(mat4f_mul_vec4(view, vec4f(2.0f, 3.0f, 4.0f, 1.0f)), 0.0f, 0.0f, 0.0f, 1.0f);
	EXPECT_VEC4F(mat4f_mul_vec4(inverse, vec4f(0.0f, 0.0f, 0.0f, 1.0f)), 2.0f, 3.0f, 4.0f, 1.0f);

	rotation.z     = CMATH_PI / 2.0f;
	rotation_basis = mat4f_rotation(rotation);
	view	       = mat4f_view(position, rotation_basis);
	inverse	       = mat4f_inverse_view(position, rotation_basis);

	EXPECT_EQ(float_near(inverse.m[1], 1.0f, 0.01f), 1);
	EXPECT_EQ(float_near(inverse.m[4], -1.0f, 0.01f), 1);
	EXPECT_EQ(view.m[1], inverse.m[4]);
	EXPECT_EQ(view.m[4], inverse.m[1]);
	EXPECT_EQ(view.m[12], -(inverse.m[0] * position.x + inverse.m[1] * position.y + inverse.m[2] * position.z));

	END;
}

STEST(cmath)
{
	SSTART;

	RUN(cmath_float);
	RUN(cmath_vec2f);
	RUN(cmath_vec3f);
	RUN(cmath_vec4f);
	RUN(cmath_mat4f_identity_transpose);
	RUN(cmath_mat4f_mul_transform);
	RUN(cmath_mat4f_rotate);
	RUN(cmath_mat4f_projection);
	RUN(cmath_mat4f_look_to);
	RUN(cmath_mat4f_transform_normal);
	RUN(cmath_mat4f_perspective);
	RUN(cmath_mat4f_view_rotation);

	SEND;
}
