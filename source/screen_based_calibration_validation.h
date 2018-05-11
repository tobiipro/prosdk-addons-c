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

/**
 * @file screen_based_calibration_validation.h
 * @brief <b>Collect data and calculate statistics and metrics for a calibration.</b>
 *
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

/**
Status codes returned by the addon.
 */
typedef enum {
    /**
    No error.
    */
    CALIBRATION_VALIDATION_STATUS_OK,

    /**
    Invalid eyetracker, i.e. could not find an eyetracker with that address.
    */
    CALIBRATION_VALIDATION_STATUS_INVALID_EYETRACKER,

    /**
    Invalid sample count argument.
    */
    CALIBRATION_VALIDATION_STATUS_INVALID_SAMPLE_COUNT,

    /**
    Invalid timeout argument.
    */
    CALIBRATION_VALIDATION_STATUS_INVALID_TIMEOUT,

    /**
    Operation not allowed when not in validation mode.
    */
    CALIBRATION_VALIDATION_STATUS_NOT_IN_VALIDATION_MODE,

    /**
    Already in validation mode.
    */
    CALIBRATION_VALIDATION_STATUS_ALREADY_IN_VALIDATION_MODE,

    /**
    Operation not allowed during data collection.
    */
    CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION,

    /**
    Invalid screen point argument.
    */
    CALIBRATION_VALIDATION_STATUS_INVALID_SCREEN_POINT,

    /**
    No data collected to process.
    */
    CALIBRATION_VALIDATION_STATUS_NO_DATA_COLLECTED,

    /**
    Internal error.
    */
    CALIBRATION_VALIDATION_STATUS_INTERNAL_ERROR,
} CalibrationValidationStatus;

/**
Represents a collected point that goes into the calibration validation. It contains calculated values
for accuracy and precision as well as the original gaze samples collected for the point.
*/
typedef struct {
    /**
    The accuracy in degrees for the left eye.
    */
    float accuracy_left_eye;
    /**
    The accuracy in degrees for the right eye.
    */
    float accuracy_right_eye;
    /**
    The precision (standard deviation) in degrees for the left eye.
    */
    float precision_left_eye;
    /**
    The precision (standard deviation) in degrees for the right eye.
    */
    float precision_right_eye;
    /**
    The precision (root mean square of sample-to-sample error) in degrees for the left eye.
    */
    float precision_rms_left_eye;
    /**
    The precision (root mean square of sample-to-sample error) in degrees for the right eye.
    */
    float precision_rms_right_eye;
    /**
    A boolean indicating if there was a timeout while collecting data for this point.
    */
    int timed_out;
    /**
    The 2D coordinates of this point (in Active Display Coordinate System).
    */
    TobiiResearchNormalizedPoint2D screen_point;
    /**
    The gaze data samples collected for this point. These samples are the base for the
    calculated accuracy and precision.
    */
    TobiiResearchGazeData* gaze_data;
    /**
    Number of gaze data samples collected for this point.
    */
    size_t gaze_data_count;
} CalibrationValidationPoint;

/**
Contains the result of the calibration validation.
*/
typedef struct {
    /**
    The accuracy in degrees averaged over all collected points for the left eye.
    */
    float average_accuracy_left;
    /**
    The accuracy in degrees averaged over all collected points for the right eye.
    */
    float average_accuracy_right;
    /**
    The precision (standard deviation) in degrees averaged over all collected points for the left eye.
    */
    float average_precision_left;
    /**
    The precision (standard deviation) in degrees averaged over all collected points for the right eye.
    */
    float average_precision_right;
    /**
    The precision (root mean square of sample-to-sample error) in degrees averaged over all collected points
    for the left eye.
    */
    float average_precision_rms_left;
    /**
    The precision (root mean square of sample-to-sample error) in degrees averaged over all collected points
    for the right eye.
    */
    float average_precision_rms_right;
    /**
    The results of the calibration validation per point (same points as were collected).
    */
    CalibrationValidationPoint* points;
    /**
    Number of points collected in result.
    */
    size_t points_count;
} CalibrationValidationResult;

/**
Opaque representation of a calibration validator struct.
*/
typedef struct CalibrationValidator CalibrationValidator;

/**
@brief Initialize a calibration validator struct.

@param address: Address of eye tracker to get data for.
@param sample_count: The number of samples to collect. Default 30, minimum 10, maximum 3000.
@param timeout: Timeout in milliseconds. Default 1000, minimum 100, maximum 3000.
@param validator: Calibration validator struct returned. Should be destroyed by user using
@ref tobii_research_screen_based_calibration_validation_destroy when done.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_init(
        const char* address, size_t sample_count, int timeout, CalibrationValidator** validator);

/**
@brief Initialize a calibration validator struct with default arguments.

@param address: Address of eye tracker to get data for.
@param validator: Calibration validator struct returned. Should be destroyed by user using
@ref tobii_research_screen_based_calibration_validation_destroy when done.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_init_default(
        const char* address, CalibrationValidator** validator);

/**
@brief Destroy a calibration validator struct (i.e. free used memory).
After this operation the validator struct cannot be used.

@param validator: Calibration validator struct pointer returned during initialization.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_destroy(
        CalibrationValidator* validator);

/**
@brief Enter the calibration validation mode and starts subscribing to gaze data from the eye tracker.

@param validator: Calibration validator struct pointer returned during initialization.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_enter_validation_mode(
        CalibrationValidator* validator);

/**
@brief Leaves the calibration validation mode, clears all collected data, and unsubscribes from the eye tracker.

@param validator: Calibration validator struct pointer returned during initialization.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_leave_validation_mode(
        CalibrationValidator* validator);

/**
@brief Starts collecting data for a calibration validation point. The argument used is the point the
user is assumed to be looking at and is given in the active display area coordinate system. Please check
@ref tobii_research_screen_based_calibration_validation_is_collecting_data to know when data collection
is completed (or timed out).

@param validator: Calibration validator struct pointer returned during initialization.
@param screen_point: The normalized 2D point on the display area.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_start_collecting_data(
        CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point);

/**
@brief Clear all collected data.

@param validator: Calibration validator struct pointer returned during initialization.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_clear_collected_data(
        CalibrationValidator* validator);

/**
@brief Removes the collected data for a specific calibration validation point.

@param validator: Calibration validator struct pointer returned during initialization.
@param screen_point: The calibration point to remove.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_discard_collected_data(
        CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point);

/**
@brief Uses the collected data and tries to compute accuracy and precision values for all points.
If there are insufficient data to compute the results for a certain point that @ref CalibrationValidationPoint
will contain invalid data (NaN) for the results. Gaze data will still be untouched. If there are no valid data
for any point, the average results of @ref CalibrationValidationResult will be invalid (NaN) as well.

@param validator: Calibration validator struct pointer returned during initialization.
@param result: Calibration validation result struct returned. Should be destroyed by user using
@ref tobii_research_screen_based_calibration_validation_destroy_result when done.
@returns A @ref CalibrationValidationStatus code.
*/
TOBII_RESEARCH_API CalibrationValidationStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_compute(
        CalibrationValidator* validator, CalibrationValidationResult** result);

/**
@brief Destroy a calibration validation result struct (i.e. free used memory).

@param result: Calibration validation result struct pointer returned during
@ref tobii_research_screen_based_calibration_validation_compute.
*/
TOBII_RESEARCH_API void TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_destroy_result(
        CalibrationValidationResult* result);

/**
@brief Check if calibration validator is in validation mode.

@param validator: Calibration validator struct pointer returned during initialization.
@returns A boolean indicating if in validation mode.
*/
TOBII_RESEARCH_API int TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_is_validation_mode(
        CalibrationValidator* validator);

/**
@brief Check if calibration validator is collecting data.

@param validator: Calibration validator struct pointer returned during initialization.
@returns A boolean indicating if data is collected.
*/
TOBII_RESEARCH_API int TOBII_RESEARCH_CALL
    tobii_research_screen_based_calibration_validation_is_collecting_data(
        CalibrationValidator* validator);

#ifdef __cplusplus
}
#endif

#endif  /* SCREEN_BASED_CALIBRATION_VALIDATION_H_ */
