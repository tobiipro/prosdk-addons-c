#ifndef VECTORMATH_H_
#define VECTORMATH_H_

#include "tobii_research.h"
#include "tobii_research_eyetracker.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void point3_set_zero(TobiiResearchPoint3D* point);
extern void point3_add(TobiiResearchPoint3D* to, const TobiiResearchPoint3D* from);
extern void point3_mul(TobiiResearchPoint3D* point, float factor);

typedef TobiiResearchPoint3D TobiiResearchVector3D;

extern void vector3_create_from_points(TobiiResearchVector3D* vector,
    const TobiiResearchPoint3D* from, const TobiiResearchPoint3D* to);
extern void vector3_set_zero(TobiiResearchVector3D* vector);
extern void vector3_normalize(TobiiResearchVector3D* vector);
extern float vector3_angle(const TobiiResearchVector3D* first, const TobiiResearchVector3D* second);

extern TobiiResearchPoint3D calculate_normalized_point2_to_point3(
    const TobiiResearchDisplayArea* display_area, const TobiiResearchNormalizedPoint2D* target_point);

#ifdef __cplusplus
}
#endif

#endif  /* VECTORMATH_H_ */
