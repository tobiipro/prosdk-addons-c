#ifndef SCREEN_BASED_CALIBRATION_VALIDATION_H_
#define SCREEN_BASED_CALIBRATION_VALIDATION_H_

#include "tobii_research.h"
#include "tobii_research_streams.h"

#ifdef __cplusplus
extern "C" {
#endif

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

extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_init(
    CalibrationValidator* validator, const char* address, size_t sample_count, int timeout);
extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_init_default(
    CalibrationValidator* validator, const char* address);
extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_destroy(
    CalibrationValidator* validator);
// TODO: Implement status to string function

extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_enter_validation_mode(
    CalibrationValidator* validator);
extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_leave_validation_mode(
    CalibrationValidator* validator);
extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_start_collecting_data(
    CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point);
extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_clear_collected_data(
    CalibrationValidator* validator);
extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_discard_collected_data(
    CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point);
extern CalibrationValidationStatus tobii_research_screen_based_calibration_validation_compute(
    CalibrationValidator* validator, CalibrationValidationResult** result);

#ifdef __cplusplus
}
#endif

#endif  /* SCREEN_BASED_CALIBRATION_VALIDATION_H_ */
