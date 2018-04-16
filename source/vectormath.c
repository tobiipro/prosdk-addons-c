#include  "vectormath.h"

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

void point3_mul(TobiiResearchPoint3D* point, float factor) {
    point->x *= factor;
    point->y *= factor;
    point->z *= factor;
}

void vector3_create_from_points(TobiiResearchVector3D* vector,
    const TobiiResearchPoint3D* from, const TobiiResearchPoint3D* to) {
    // TODO: Implement
}

void vector3_set_zero(TobiiResearchVector3D* vector) {
    vector->x = 0.0f;
    vector->y = 0.0f;
    vector->z = 0.0f;
}

void vector3_normalize(TobiiResearchVector3D* vector) {
    // TODO: Implement
}

float vector3_angle(const TobiiResearchVector3D* first, const TobiiResearchVector3D* second) {
    // TODO: Implement
    return 0.0f;
}

TobiiResearchPoint3D calculate_normalized_point2_to_point3(
    const TobiiResearchDisplayArea* display_area, const TobiiResearchNormalizedPoint2D* target_point) {
    // TODO: Implement
    TobiiResearchPoint3D point;
    return point;
}
