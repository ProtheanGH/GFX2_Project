#pragma once

#include "Object.h"

bool SortByDistance(Object* _a, Object* _b)
{
	return _a->DistanceFromCamera > _b->DistanceFromCamera;
}