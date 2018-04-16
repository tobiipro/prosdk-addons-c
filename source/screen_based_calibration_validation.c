#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tobii_research.h"
#include "tobii_research_streams.h"

#include "screen_based_calibration_validation.h"
#include "vectormath.h"
#include "stopwatch.h"

#define SAMPLE_COUNT_MIN (10)
#define SAMPLE_COUNT_DEFAULT (30)
#define SAMPLE_COUNT_MAX (3000)
#define TIMEOUT_MIN (100)
#define TIMEOUT_DEFAULT (1000)
#define TIMEOUT_MAX (3000)

typedef enum {
    CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE,
    CALIBRATION_VALIDATION_STATE_NOT_COLLECTING_DATA,
    CALIBRATION_VALIDATION_STATE_COLLECTING_DATA,
} CalibrationValidationState;

typedef struct {
    TobiiResearchNormalizedPoint2D screen_point;
    TobiiResearchGazeData** gaze_data;
    size_t gaze_data_count;
} CollectedDataPoint;

struct CalibrationValidator {
    TobiiResearchEyeTracker* eyetracker;
    CalibrationValidationState state;
    size_t sample_count;
    int timeout;

    /* Temporary data for current data collection */
    CollectedDataPoint *new_point;

    /* Stored data for successfully collected data points */
    CollectedDataPoint **collected_points;
    size_t collected_points_count;
    size_t collected_points_capacity;

    // TODO: Investigate if synchronization is needed

    Stopwatch* stopwatch;
};


static void gaze_data_callback(TobiiResearchGazeData* gaze_data, void* user_data);

static void create_temporary_data(CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point);
static void destroy_temporary_data(CalibrationValidator* validator);
static void create_collected_data(CalibrationValidator* validator);
static void extend_collected_data(CalibrationValidator* validator);
static void destroy_collected_data(CalibrationValidator* validator);

static float calculate_eye_accuracy(TobiiResearchPoint3D* gaze_origin_mean,
    TobiiResearchPoint3D* gaze_point_mean, TobiiResearchPoint3D* stimuli_point);
static float calculate_eye_precision(TobiiResearchVector3D* direction_gaze_point_all,
    TobiiResearchVector3D* direction_gaze_point_mean_all, size_t vector_count);
static float calculate_eye_precision_rms(TobiiResearchVector3D* direction_gaze_point_all, size_t vector_count);


CalibrationValidationStatus tobii_research_screen_based_calibration_validation_init(
    CalibrationValidator* validator, const char* address,
    size_t sample_count, int timeout) {

    // TODO: Check if validator already is used?

    if (!(sample_count >= SAMPLE_COUNT_MIN && sample_count <= SAMPLE_COUNT_MAX)) {
        return CALIBRATION_VALIDATION_STATUS_INVALID_SAMPLE_COUNT;
    }
    if (!(timeout >= TIMEOUT_MIN && timeout <= TIMEOUT_MAX)) {
        return CALIBRATION_VALIDATION_STATUS_INVALID_TIMEOUT;
    }

    TobiiResearchStatus status = tobii_research_get_eyetracker(address, &validator->eyetracker);
    if (status != TOBII_RESEARCH_STATUS_OK) {
        return CALIBRATION_VALIDATION_STATUS_INVALID_EYETRACKER;
    }

    validator->sample_count = sample_count;
    validator->timeout = timeout;
    validator->state = CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE;

    validator->new_point = NULL;
    validator->collected_points = NULL;
    validator->collected_points_capacity = 0;
    validator->collected_points_count = 0;

    validator->stopwatch = stopwatch_init();

    return CALIBRATION_VALIDATION_STATUS_OK;
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_init_default(
    CalibrationValidator* validator, const char* address) {
    return tobii_research_screen_based_calibration_validation_init(
        validator, address, SAMPLE_COUNT_DEFAULT, TIMEOUT_DEFAULT);
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_destroy(
    CalibrationValidator* validator) {
    if (validator->state == CALIBRATION_VALIDATION_STATE_COLLECTING_DATA) {
        return CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION;
    }

    if (validator->state == CALIBRATION_VALIDATION_STATE_NOT_COLLECTING_DATA) {
        TobiiResearchStatus status = tobii_research_unsubscribe_from_gaze_data(
            validator->eyetracker, gaze_data_callback);
        if (status != TOBII_RESEARCH_STATUS_OK)
            return CALIBRATION_VALIDATION_STATUS_INTERNAL_ERROR;
    }

    destroy_temporary_data(validator);
    destroy_collected_data(validator);
    free(validator->stopwatch);

    validator->state = CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE;

    return CALIBRATION_VALIDATION_STATUS_OK;
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_enter_validation_mode(
    CalibrationValidator* validator) {
    if (validator->state != CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE) {
        return CALIBRATION_VALIDATION_STATUS_ALREADY_IN_VALIDATION_MODE;
    }

    TobiiResearchStatus status = tobii_research_subscribe_to_gaze_data(
        validator->eyetracker, gaze_data_callback, validator);
    if (status != TOBII_RESEARCH_STATUS_OK)
        return CALIBRATION_VALIDATION_STATUS_INTERNAL_ERROR;

    destroy_collected_data(validator);
    create_collected_data(validator);

    validator->state = CALIBRATION_VALIDATION_STATE_NOT_COLLECTING_DATA;

    return CALIBRATION_VALIDATION_STATUS_OK;
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_leave_validation_mode(
    CalibrationValidator* validator) {
    if (validator->state == CALIBRATION_VALIDATION_STATE_COLLECTING_DATA) {
        return CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION;
    } else if (validator->state == CALIBRATION_VALIDATION_STATE_COLLECTING_DATA) {
        return CALIBRATION_VALIDATION_STATUS_NOT_IN_VALIDATION_MODE;
    }

    TobiiResearchStatus status = tobii_research_unsubscribe_from_gaze_data(
        validator->eyetracker, gaze_data_callback);
    if (status != TOBII_RESEARCH_STATUS_OK)
        return CALIBRATION_VALIDATION_STATUS_INTERNAL_ERROR;

    validator->state = CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE;

    destroy_temporary_data(validator);

    return CALIBRATION_VALIDATION_STATUS_OK;
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_start_collecting_data(
    CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point) {
    if (validator->state == CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE) {
        return CALIBRATION_VALIDATION_STATUS_NOT_IN_VALIDATION_MODE;
    } else if (validator->state == CALIBRATION_VALIDATION_STATE_COLLECTING_DATA) {
        return CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION;
    }

    if (!(screen_point->x >= 0.0f && screen_point->x <= 1.0f &&
          screen_point->y >= 0.0f && screen_point->y <= 1.0f)) {
        return CALIBRATION_VALIDATION_STATUS_INVALID_SCREEN_POINT;
    }

    destroy_temporary_data(validator);
    create_temporary_data(validator, screen_point);

    validator->timeout = 0;

    stopwatch_reset(validator->stopwatch);
    stopwatch_start(validator->stopwatch);

    validator->state = CALIBRATION_VALIDATION_STATE_COLLECTING_DATA;

    return CALIBRATION_VALIDATION_STATUS_OK;
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_clear_collected_data(
    CalibrationValidator* validator) {
    if (validator->state == CALIBRATION_VALIDATION_STATE_COLLECTING_DATA) {
        return CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION;
    }
    destroy_temporary_data(validator);
    destroy_collected_data(validator);
    create_collected_data(validator);

    return CALIBRATION_VALIDATION_STATUS_OK;
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_discard_collected_data(
    CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point) {
    if (validator->state == CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE) {
        return CALIBRATION_VALIDATION_STATUS_NOT_IN_VALIDATION_MODE;
    }
    if (validator->state == CALIBRATION_VALIDATION_STATE_COLLECTING_DATA) {
        return CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION;
    }

    // TODO: Implement

    return CALIBRATION_VALIDATION_STATUS_OK;
}

CalibrationValidationStatus tobii_research_screen_based_calibration_validation_compute(
    CalibrationValidator* validator, CalibrationValidationResult** result) {
    if (validator->state == CALIBRATION_VALIDATION_STATE_COLLECTING_DATA) {
        return CALIBRATION_VALIDATION_STATUS_OPERATION_NOT_ALLOWED_DURING_DATA_COLLECTION;
    }

    TobiiResearchDisplayArea display_area;
    TobiiResearchStatus status = tobii_research_get_display_area(validator->eyetracker, &display_area);
    if (status != TOBII_RESEARCH_STATUS_OK) {
        return CALIBRATION_VALIDATION_STATUS_INTERNAL_ERROR;
    }

    CalibrationValidationPoint* points = malloc(validator->collected_points_count * sizeof(*points));
    float accuracy_left_eye_average = 0.0f;
    float accuracy_right_eye_average = 0.0f;
    float precision_left_eye_average = 0.0f;
    float precision_right_eye_average = 0.0f;
    float precision_rms_left_eye_average = 0.0f;
    float precision_rms_right_eye_average = 0.0f;

    int valid_points_count = 0;

    for (size_t i = 0; i < validator->collected_points_count; ++i) {
        CollectedDataPoint* collected_data_point = validator->collected_points[i];

        if  (collected_data_point->gaze_data_count < validator->sample_count) {
            /* Timeout before collecting enough valid samples, no calculations to be done. */
            points[i].accuracy_left_eye = NAN;
            points[i].accuracy_right_eye = NAN;
            points[i].precision_left_eye = NAN;
            points[i].precision_right_eye = NAN;
            points[i].precision_rms_left_eye = NAN;
            points[i].precision_rms_right_eye = NAN;
            points[i].timed_out = 1;
            points[i].screen_point = collected_data_point->screen_point;
            points[i].gaze_data = collected_data_point->gaze_data;
            points[i].gaze_data_count = collected_data_point->gaze_data_count;
            continue;
        }

        TobiiResearchPoint3D stimuli_point = calculate_normalized_point2_to_point3(
            &display_area, &collected_data_point->screen_point);

        /* Calculate mean points */
        TobiiResearchPoint3D gaze_origin_left_mean;
        point3_set_zero(&gaze_origin_left_mean);
        TobiiResearchPoint3D gaze_origin_right_mean;
        point3_set_zero(&gaze_origin_right_mean);
        TobiiResearchPoint3D gaze_point_left_mean;
        point3_set_zero(&gaze_point_left_mean);
        TobiiResearchPoint3D gaze_point_right_mean;
        point3_set_zero(&gaze_point_right_mean);

        for(size_t j = 0; j < collected_data_point->gaze_data_count; ++j) {
            TobiiResearchGazeData* gaze_data = collected_data_point->gaze_data[j];

            point3_add(&gaze_origin_left_mean, &gaze_data->left_eye.gaze_origin.position_in_user_coordinates);
            point3_add(&gaze_origin_right_mean, &gaze_data->right_eye.gaze_origin.position_in_user_coordinates);
            point3_add(&gaze_point_left_mean, &gaze_data->left_eye.gaze_point.position_in_user_coordinates);
            point3_add(&gaze_point_right_mean, &gaze_data->right_eye.gaze_point.position_in_user_coordinates);
        }
        float denominator_factor = 1.0f / collected_data_point->gaze_data_count;
        point3_mul(&gaze_origin_left_mean, denominator_factor);
        point3_mul(&gaze_origin_right_mean, denominator_factor);
        point3_mul(&gaze_point_left_mean, denominator_factor);
        point3_mul(&gaze_point_right_mean, denominator_factor);

        /* Calculate gaze vectors needed for validation statistics */
        TobiiResearchVector3D *direction_gaze_point_left_all = malloc(
            collected_data_point->gaze_data_count * sizeof(*direction_gaze_point_left_all));
        vector3_set_zero(direction_gaze_point_left_all);
        TobiiResearchVector3D *direction_gaze_point_left_mean_all = malloc(
            collected_data_point->gaze_data_count * sizeof(*direction_gaze_point_left_mean_all));
        vector3_set_zero(direction_gaze_point_left_mean_all);
        TobiiResearchVector3D *direction_gaze_point_right_all = malloc(
            collected_data_point->gaze_data_count * sizeof(*direction_gaze_point_right_all));
        vector3_set_zero(direction_gaze_point_right_all);
        TobiiResearchVector3D *direction_gaze_point_right_mean_all = malloc(
            collected_data_point->gaze_data_count * sizeof(*direction_gaze_point_right_mean_all));
        vector3_set_zero(direction_gaze_point_right_mean_all);

        for(size_t j = 0; j < collected_data_point->gaze_data_count; ++j) {
            TobiiResearchGazeData* gaze_data = collected_data_point->gaze_data[j];

            vector3_create_from_points(&direction_gaze_point_left_all[j],
                &gaze_data->left_eye.gaze_origin.position_in_user_coordinates,
                &gaze_data->left_eye.gaze_point.position_in_user_coordinates);
            vector3_normalize(&direction_gaze_point_left_all[j]);

            vector3_create_from_points(&direction_gaze_point_left_mean_all[j],
                &gaze_data->left_eye.gaze_origin.position_in_user_coordinates,
                &gaze_point_left_mean);
            vector3_normalize(&direction_gaze_point_left_mean_all[j]);

            vector3_create_from_points(&direction_gaze_point_right_all[j],
                &gaze_data->right_eye.gaze_origin.position_in_user_coordinates,
                &gaze_data->right_eye.gaze_point.position_in_user_coordinates);
            vector3_normalize(&direction_gaze_point_right_all[j]);

            vector3_create_from_points(&direction_gaze_point_right_mean_all[j],
                &gaze_data->right_eye.gaze_origin.position_in_user_coordinates,
                &gaze_point_right_mean);
            vector3_normalize(&direction_gaze_point_right_mean_all[j]);
        }

        /* Accuracy calculations */
        float accuracy_left_eye = calculate_eye_accuracy(
            &gaze_origin_left_mean, &gaze_point_left_mean, &stimuli_point);
        float accuracy_right_eye = calculate_eye_accuracy(
            &gaze_origin_right_mean, &gaze_point_right_mean, &stimuli_point);

        /* Precision calculations */
        float precision_left_eye = calculate_eye_precision(
            direction_gaze_point_left_all, direction_gaze_point_left_mean_all,
            collected_data_point->gaze_data_count);
        float precision_right_eye = calculate_eye_precision(
            direction_gaze_point_right_all, direction_gaze_point_right_mean_all,
            collected_data_point->gaze_data_count);

        /* RMS precision calculations */
        float precision_rms_left_eye = calculate_eye_precision_rms(
            direction_gaze_point_left_all, collected_data_point->gaze_data_count);
        float precision_rms_right_eye = calculate_eye_precision_rms(
            direction_gaze_point_right_all, collected_data_point->gaze_data_count);

        free(direction_gaze_point_left_all);
        free(direction_gaze_point_left_mean_all);
        free(direction_gaze_point_right_all);
        free(direction_gaze_point_right_mean_all);

        /* Prepare calibration validation point */
        points[i].accuracy_left_eye = accuracy_left_eye;
        points[i].accuracy_right_eye = accuracy_right_eye;
        points[i].precision_left_eye = precision_left_eye;
        points[i].precision_right_eye = precision_right_eye;
        points[i].precision_rms_left_eye = precision_rms_left_eye;
        points[i].precision_rms_right_eye = precision_rms_right_eye;
        points[i].timed_out = 0;
        points[i].screen_point = collected_data_point->screen_point;
        points[i].gaze_data = collected_data_point->gaze_data;
        collected_data_point->gaze_data = NULL;
        points[i].gaze_data_count = collected_data_point->gaze_data_count;

        /* Ackumulate values for average calculation */
        accuracy_left_eye_average += accuracy_left_eye;
        accuracy_right_eye_average += accuracy_right_eye;
        precision_left_eye_average += precision_left_eye;
        precision_right_eye_average = precision_right_eye;
        precision_rms_left_eye_average = precision_rms_left_eye;
        precision_rms_right_eye_average = precision_rms_right_eye;

        valid_points_count++;
    }

    if (valid_points_count > 0) {
        accuracy_left_eye_average /= valid_points_count;
        accuracy_right_eye_average /= valid_points_count;
        precision_left_eye_average /= valid_points_count;
        precision_right_eye_average /= valid_points_count;
        precision_rms_left_eye_average /= valid_points_count;
        precision_rms_right_eye_average /= valid_points_count;
    } else {
        accuracy_left_eye_average = NAN;
        accuracy_right_eye_average = NAN;
        precision_left_eye_average = NAN;
        precision_right_eye_average = NAN;
        precision_rms_left_eye_average = NAN;
        precision_rms_right_eye_average = NAN;
    }

    CalibrationValidationResult* result_tmp = malloc(sizeof(*result));
    result_tmp->average_accuracy_left = accuracy_left_eye_average;
    result_tmp->average_accuracy_right = accuracy_right_eye_average;
    result_tmp->average_precision_left = precision_left_eye_average;
    result_tmp->average_precision_right = precision_right_eye_average;
    result_tmp->average_precision_rms_left = precision_rms_left_eye_average;
    result_tmp->average_precision_rms_right = precision_rms_right_eye_average;
    result_tmp->points = points;
    result_tmp->points_count = validator->collected_points_count;
    *result = result_tmp;

    return CALIBRATION_VALIDATION_STATUS_OK;
}

static void gaze_data_callback(TobiiResearchGazeData* gaze_data, void* user_data) {
    CalibrationValidator* validator = (CalibrationValidator*)user_data;

    switch(validator->state) {
        case CALIBRATION_VALIDATION_STATE_NOT_IN_VALIDATION_MODE:
            /* Do nothing */
            break;

        case CALIBRATION_VALIDATION_STATE_NOT_COLLECTING_DATA:
            /* Do nothing */
            break;

        case CALIBRATION_VALIDATION_STATE_COLLECTING_DATA:
            if (stopwatch_elapsed(validator->stopwatch) > validator->timeout) {
                validator->timeout = 1;
                validator->state = CALIBRATION_VALIDATION_STATE_NOT_COLLECTING_DATA;
            } else if (validator->new_point->gaze_data_count < validator->sample_count) {
                if (gaze_data->left_eye.gaze_point.validity == TOBII_RESEARCH_VALIDITY_VALID &&
                    gaze_data->right_eye.gaze_point.validity == TOBII_RESEARCH_VALIDITY_VALID) {
                    /* Store gaze data sample. */
                    validator->new_point->gaze_data[validator->new_point->gaze_data_count++] = gaze_data;
                }
            } else {
                /* Data collecting stopped on sample count condition. */

                /* Store collected data for this point. */
                if (validator->collected_points_count >= validator->collected_points_capacity) {
                    extend_collected_data(validator);
                }
                validator->collected_points[validator->collected_points_count++] = validator->new_point;
                validator->new_point = NULL;

                validator->state = CALIBRATION_VALIDATION_STATE_NOT_COLLECTING_DATA;
            }
            break;

        default:
            /* Should not happen */
            break;
    }
}

static void create_temporary_data(CalibrationValidator* validator, const TobiiResearchNormalizedPoint2D* screen_point) {
    CollectedDataPoint *data_point = malloc(sizeof(*data_point));
    data_point->screen_point = *screen_point;
    data_point->gaze_data = malloc(validator->sample_count * sizeof(*data_point->gaze_data));
    data_point->gaze_data_count = 0;
    validator->new_point = data_point;
}

static void destroy_temporary_data(CalibrationValidator* validator) {
    if (validator->new_point) {
        for (size_t i = 0; i < validator->new_point->gaze_data_count; ++i) {
            free(validator->new_point->gaze_data[i]);
        }
        free(validator->new_point->gaze_data);
        free(validator->new_point);
        validator->new_point = NULL;
    }
}

static void create_collected_data(CalibrationValidator* validator) {
    validator->collected_points_capacity = 5;
    validator->collected_points_count = 0;
    validator->collected_points = calloc(validator->collected_points_capacity, sizeof(*validator->collected_points));
}

static void extend_collected_data(CalibrationValidator* validator) {
    validator->collected_points_capacity *= 2;
    validator->collected_points = realloc(validator->collected_points,
        validator->collected_points_capacity * sizeof(CollectedDataPoint));
    for (size_t i = validator->collected_points_count; i < validator->collected_points_capacity; ++i) {
        validator->collected_points[i] = NULL;
    }
}

static void destroy_collected_data(CalibrationValidator* validator) {
    if (validator->collected_points_capacity > 0) {
        for (size_t i = 0; i < validator->collected_points_count; ++i) {
            for (size_t j = 0; j < validator->collected_points[i]->gaze_data_count; ++j) {
                if (validator->collected_points[i]->gaze_data[j] !=  NULL) {
                    free(validator->collected_points[i]->gaze_data[j]);
                }
            }
            free(validator->collected_points[i]->gaze_data);
            free(validator->collected_points[i]);
        }
        free(validator->collected_points);
        validator->collected_points = NULL;
        validator->collected_points_capacity = 0;
        validator->collected_points_count = 0;
    }
}

static float calculate_eye_accuracy(TobiiResearchPoint3D* gaze_origin_mean,
    TobiiResearchPoint3D* gaze_point_mean, TobiiResearchPoint3D* stimuli_point) {
    TobiiResearchVector3D direction_gaze_point;
    TobiiResearchVector3D direction_target;
    vector3_create_from_points(&direction_gaze_point, gaze_origin_mean, gaze_point_mean);
    vector3_normalize(&direction_gaze_point);
    vector3_create_from_points(&direction_target, gaze_origin_mean, stimuli_point);
    vector3_normalize(&direction_target);
    return vector3_angle(&direction_gaze_point, &direction_target);
}

static float calculate_eye_precision(TobiiResearchVector3D* direction_gaze_point_all,
    TobiiResearchVector3D* direction_gaze_point_mean_all, size_t vector_count) {
    float variance = 0.0f;
    for (size_t i = 0; i < vector_count; ++i) {
        float angle = vector3_angle(&direction_gaze_point_all[i], &direction_gaze_point_mean_all[i]);
        variance += angle*angle;
    }
    variance /= vector_count;
    float standard_deviation = (float)sqrt(variance);
    return standard_deviation;
}

static float calculate_eye_precision_rms(TobiiResearchVector3D* direction_gaze_point_all, size_t vector_count) {
    float variance = 0.0f;
    for (size_t i = 1; i < vector_count; ++i) {
        float angle = vector3_angle(&direction_gaze_point_all[i-1], &direction_gaze_point_all[i]);
        variance += angle*angle;
    }
    variance /= vector_count - 1;
    float rms = (float)sqrt(variance);
    return rms;
}
