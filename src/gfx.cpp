#include "gfx.h"
Vec3 TriNormal(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3)
{
	Vec3 A = v2 - v1;
	Vec3 B = v3 - v1;
	float Nx = A.y * B.z - A.z * B.y;
	float Ny = A.z * B.x - A.x * B.z;
	float Nz = A.x * B.y - A.y * B.x;
	return {Nx, Ny, Nz};
}

Rect bounding_box2d(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2)
{
	float minx = min(min(v0.x, v1.x), v2.x);
	float miny = min(min(v0.y, v1.y), v2.y);

	float maxx = max(max(v0.x, v1.x), v2.x);
	float maxy = max(max(v0.y, v1.y), v2.y);
	return {
		.min = {minx, miny},
		.max = {maxx, maxy}};
}


Vec3 get_tex(float u, float v, int w, int h, const uint32_t *tex)
{
	if (u >= 1 || v >= 1 || u < 0 || v < 0)
	{
		// printf("over\n");
	}

	u = my_clamp(u, 0, .9999999); 
	v = my_clamp(1.f - v, 0, .9999999);

	// return Vec3{u * .75f, v * .75f, 0.f};

	int ui = (int)(u * (float)w);
	int vi = (int)(v * (float)h);

	uint32_t c = tex[vi * w + ui];

	int r = (c >> 16) & 0xFF;
	int g = (c >> 8) & 0xFF;
	int b = (c)&0xFF;

	return Vec3{(float)(r) / 256.f, (float)(g) / 256.f, (float)(b) / 256.f};
}


// float edgeFunction(const Vec3 &a, const Vec3 &b, const Vec3 &c)
// {
//     return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
// }

// tri_info insideTri(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &pixel_NDC)
// {
//     float area = edgeFunction(v1, v2, v3);
//     float w1 = edgeFunction(v2, v3, pixel_NDC) / area;
//     float w2 = edgeFunction(v3, v1, pixel_NDC) / area;
//     float w3 = edgeFunction(v1, v2, pixel_NDC) / area;
//     bool inside = (w1 >= 0) && (w2 >= 0) && (w3 >= 0);
//     return {inside, area, w1, w2, w3};
// }
