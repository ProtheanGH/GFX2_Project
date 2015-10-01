#include "Color.h"

float* Color::Red()
{
	float red[] = { 1, 0, 0, 1 };
	return red;
}

float* Color::Green()
{
	float green[] = { 0, 1, 0, 1 };
	return green;
}

float* Color::Blue()
{
	float blue[] = { 0, 0, 1, 1 };
	return blue;
}

float* Color::White()
{
	float white[] = { 1, 1, 1, 1 };
	return white;
}

float* Color::Black()
{
	float black[] = { 0, 0, 0, 1 };
	return black;
}
