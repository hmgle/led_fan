#include <SDL.h>

struct point_s {
	double x;
	double y;
};

struct led_s {
	struct point_s center;
	double r;
	double start_angle;	/* 起始角度 */
	double period;		/* 旋转周期 */
	uint32_t color;		/* 发光颜色 */
};

int main(int argc, char **argv)
{
	return 0;
}
