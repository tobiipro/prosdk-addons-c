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

#ifndef SCREEN_BASED_CALIBRATION_VALIDATION_H_
#define SCREEN_BASED_CALIBRATION_VALIDATION_H_

#include "tobii_research.h"
#include "tobii_research_streams.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifdef TOBII_STATIC_LIB
#define TOBII_RESEARCH_CALL
#define TOBII_RESEARCH_API
#else
#define TOBII_RESEARCH_CALL __cdecl
#ifdef TOBII_EXPORTING
#define TOBII_RESEARCH_API __declspec(dllexport)
#else
#define TOBII_RESEARCH_API __declspec(dllimport)
#endif /* TOBII_EXPORTING */
#endif /* TOBII_STATIC_LIB */
#else
#define TOBII_RESEARCH_API
#define TOBII_RESEARCH_CALL
#endif /* _WIN32 */

typedef enum {
    CALIBRATION_VALIDATION_STATUS_OK,
    CALIBRATION_VALIDATION_STATUS_INVALID_EYETRACKER,
    CALIBRATION_VALIDATION_STATUS_INVALID_SAMPLE_COUNT,
    CALIBRATION_VALIDATION_STATUS_INVALID_TIMEOUT,
    CALIBRATION_VALIDATION_STATUS_NOT_IN_VALIDATION_MODE,
    CALIBRATION_VALIDATION_STATUS_ALREADY_IN_VALIDATION_MODE,
    CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION,
    CALIBRATION_VALIDATION_STATUS_INVALID_SCREEN_POINT,
    CALIBRATION_VALIDATION_STATUS_INTERNAL_ERROR,
} CalibrationValidationStatus;

typedef struct {
    float accuracy_left_eye;
    float accuracy_right_eye;
    float precision_left_eye;
    float precision_right_eye;
    float precision_rms_left_eye;
    float precision_rms_right_eye;
    int timed_out;
    TobiiResearchNormalizedPoint2D screen_point;
    TobiiResearchGazeData** gaze_data;
    size_t gaze_data_count;
} CalibrationValidationPoint;

typedef struct {
    float average_accuracy_left;
    float average_accuracy_right;
    float average_precision_left;
    float average_precision_right;
    float average_precision_rms_left;
    float average_precision_rms_right;
    CalibrationValidationPoint* points;
    size_t points_count;
} CalibrationValidationResult;

typedef struct CalibrationValidator CalibrationValidator;

TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_init(
        const char* address, size_t sample_count, int timeout, CalibrationValidator** validator);
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_init_default(
        const char* address, CalibrationValidator** validator);
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_destroy(
        CalibrationValidator* validator);

TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_enter_validation_mode(
        CalibrationValidator* validator);
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_leave_validation_mode(
        CalibrationValidator* validator);
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_start_collecting_data(
        CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point);
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_clear_collected_data(
        CalibrationValidator* validator);
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_discard_collected_data(
        CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point);
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_compute(
        CalibrationValidator* validator, CalibrationValidationResult** result);

// TODO: Help function: convert status to string.
// TODO: Help function: Free/destroy CalibrationValidationResult structure.

#ifdef __cplusplus
}
#endif

#endif  /* SCREEN_BASED_CALIBRATION_VALIDATION_H_ */
