/*
Copyright 2018 Tobii AB

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <math.h>

#include  "vectormath.h"

#define CLAMP(x, low, high)  (fmax(low, fmin(high, x)))

#define REL_TOL (1e-9f)
#define ABS_TOL (0.0f)
#define IS_CLOSE(a, b)  (fabsf(a - b) <= fmaxf(REL_TOL * fmaxf(fabsf(a), fabsf(b)), ABS_TOL))

int point2_equal(const TobiiResearchNormalizedPoint2D* first, const TobiiResearchNormalizedPoint2D* second) {
    return IS_CLOSE(first->x, second->x) && IS_CLOSE(second->y, second->y);
}

void point3_set_zero(TobiiResearchPoint3D* point) {
    point->x = 0.0f;
    point->y = 0.0f;
    point->z = 0.0f;
}

void point3_add(TobiiResearchPoint3D* to, const TobiiResearchPoint3D* from) {
    to->x += from->x;
    to->y += from->y;
    to->z += from->z;
}

void point3_sub(TobiiResearchPoint3D* lhs, const TobiiResearchPoint3D* rhs) {
    lhs->x -= rhs->x;
    lhs->y -= rhs->y;
    lhs->z -= rhs->z;
}

void point3_mul(TobiiResearchPoint3D* point, float factor) {
    point->x *= factor;
    point->y *= factor;
    point->z *= factor;
}

void vector3_create_from_points(TobiiResearchVector3D* vector,
    const TobiiResearchPoint3D* from, const TobiiResearchPoint3D* to) {
    vector->x = to->x - from->x;
    vector->y = to->y - from->y;
    vector->z = to->z - from->z;
}

void vector3_set_zero(TobiiResearchVector3D* vector) {
    vector->x = 0.0f;
    vector->y = 0.0f;
    vector->z = 0.0f;
}

void vector3_add(TobiiResearchVector3D* to, const TobiiResearchVector3D* from) {
    point3_add((TobiiResearchPoint3D*)to, (const TobiiResearchPoint3D*)from);
}

void vector3_mul(TobiiResearchVector3D* vector, float factor) {
    point3_mul((TobiiResearchPoint3D*)vector, factor);
}

double vector3_dot_product(const TobiiResearchVector3D* first, const TobiiResearchVector3D* second) {
    return (double)first->x * (double)second->x +
           (double)first->y * (double)second->y +
           (double)first->z * (double)second->z;
}

double vector3_magnitude(const TobiiResearchVector3D* vector) {
    return sqrt((double)vector->x * (double)vector->x +
                (double)vector->y * (double)vector->y +
                (double)vector->z * (double)vector->z);
}

void vector3_normalize(TobiiResearchVector3D* vector) {
    float factor = 1.0f / (float)vector3_magnitude(vector);
    vector3_mul(vector, factor);
}

float vector3_angle(const TobiiResearchVector3D* first, const TobiiResearchVector3D* second) {
    double x = vector3_dot_product(first, second) / (vector3_magnitude(first) * vector3_magnitude(second));
    double angle = acos(CLAMP(x, -1.0, 1.0));
    return (float)(angle * 180 / M_PI);
}

void calculate_normalized_point2_to_point3(TobiiResearchPoint3D* result,
    const TobiiResearchDisplayArea* display_area, const TobiiResearchNormalizedPoint2D* target_point) {
    TobiiResearchPoint3D dx, dy;

    dx = display_area->top_right;
    point3_sub(&dx, &display_area->top_left);
    point3_mul(&dx, target_point->x);
    dy = display_area->bottom_left;
    point3_sub(&dy, &display_area->top_left);
    point3_mul(&dy, target_point->y);

    *result = display_area->top_left;
    point3_add(result, &dx);
    point3_add(result, &dy);
}
