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

#ifndef VECTORMATH_H_
#define VECTORMATH_H_

#include "tobii_research.h"
#include "tobii_research_eyetracker.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int point2_equal(const TobiiResearchNormalizedPoint2D* first, const TobiiResearchNormalizedPoint2D* second);

extern void point3_set_zero(TobiiResearchPoint3D* point);
extern void point3_add(TobiiResearchPoint3D* to, const TobiiResearchPoint3D* from);
extern void point3_sub(TobiiResearchPoint3D* lhs, const TobiiResearchPoint3D* rhs);
extern void point3_mul(TobiiResearchPoint3D* point, float factor);

typedef TobiiResearchPoint3D TobiiResearchVector3D;

extern void vector3_create_from_points(TobiiResearchVector3D* vector,
    const TobiiResearchPoint3D* from, const TobiiResearchPoint3D* to);
extern void vector3_set_zero(TobiiResearchVector3D* vector);
extern void vector3_add(TobiiResearchVector3D* to, const TobiiResearchVector3D* from);
extern void vector3_mul(TobiiResearchVector3D* point, float factor);
extern double vector3_dot_product(const TobiiResearchVector3D* first, const TobiiResearchVector3D* second);
extern double vector3_magnitude(const TobiiResearchVector3D* vector);
extern void vector3_normalize(TobiiResearchVector3D* vector);
extern float vector3_angle(const TobiiResearchVector3D* first, const TobiiResearchVector3D* second);

extern void calculate_normalized_point2_to_point3(TobiiResearchPoint3D* result,
    const TobiiResearchDisplayArea* display_area, const TobiiResearchNormalizedPoint2D* target_point);

#ifdef __cplusplus
}
#endif

#endif  /* VECTORMATH_H_ */
