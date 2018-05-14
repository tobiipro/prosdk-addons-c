#include <stdio.h>
#include <stdlib.h>

#include "screen_based_calibration_validation.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static void sleep_ms(int time) {
    Sleep(time);
}
#else
#include <unistd.h>
static void sleep_ms(int time) {
    usleep(time * 1000);
}
#endif

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: sample <eyetracker address>\n");
    }

    char* address = argv[1];
    CalibrationValidator* validator = NULL;
    CalibrationValidationStatus status;

    /* Initialize calibration validation API. */
    status = tobii_research_screen_based_calibration_validation_init_default(address, &validator);
    if (status == CALIBRATION_VALIDATION_STATUS_INVALID_EYETRACKER) {
        printf("Couldn't find eyetracker with address %s!\n", address);
        exit(1);
    } else if (status != CALIBRATION_VALIDATION_STATUS_OK) {
        printf("Unknown error!\n");
        exit(1);
    }

    /*  Enter the calibration validation mode. This will start subscribing to gaze data from the eye tracker. */
    status = tobii_research_screen_based_calibration_validation_enter_validation_mode(validator);
    if (status != CALIBRATION_VALIDATION_STATUS_OK) {
        printf("Unknown error!\n");
        exit(1);
    }

    /* Stimuli points or screen points to use for validation. */
    TobiiResearchNormalizedPoint2D stimuli_points[5];
    stimuli_points[0].x = 0.3f;
    stimuli_points[0].y = 0.3f;

    stimuli_points[1].x = 0.3f;
    stimuli_points[1].y = 0.7f;

    stimuli_points[2].x = 0.5f;
    stimuli_points[2].y = 0.5f;

    stimuli_points[3].x = 0.7f;
    stimuli_points[3].y = 0.3f;

    stimuli_points[4].x = 0.7f;
    stimuli_points[4].y = 0.7f;

    for (size_t i = 0; i < 5; ++i) {
        /* In a proper implementation of a calibration validation, each stimuli point
         * should be visualized on the scrren using some graphics library or framework.
         */
        printf("Collecting data for stimuli point (%f, %f)...\n", stimuli_points[i].x, stimuli_points[i].y);

        /* Collect data for this stimuli point. */
        status = tobii_research_screen_based_calibration_validation_start_collecting_data(
            validator, &stimuli_points[i]);
        if (status != CALIBRATION_VALIDATION_STATUS_OK) {
            printf("Unknown error!\n");
            exit(1);
        }

        /* Wait until enough gaze data is collected. */
        while (tobii_research_screen_based_calibration_validation_is_collecting_data(validator)) {
            sleep_ms(100);
        }
    }

    /* Compute a validation result for all stimuli points and current calibration. */
    CalibrationValidationResult* result = NULL;
    status = tobii_research_screen_based_calibration_validation_compute(validator, &result);
    if (status != CALIBRATION_VALIDATION_STATUS_OK) {
        printf("Unknown error!\n");
        exit(1);
    }

    printf("Calibration validation result (average over %zd collected points):\n", result->points_count);
    printf("  Left eye:\n");
    printf("    Accuracy: %f\n", result->average_accuracy_left);
    printf("    Precision: %f\n", result->average_precision_left);
    printf("    Precision (RMS): %f\n", result->average_precision_rms_left);
    printf("  Right eye:\n");
    printf("    Accuracy: %f\n", result->average_accuracy_right);
    printf("    Precision: %f\n", result->average_precision_right);
    printf("    Precision (RMS): %f\n", result->average_precision_rms_right);

    /* Destroy or free memory for current calibration validation context. */
    tobii_research_screen_based_calibration_validation_destroy_result(result);
    status = tobii_research_screen_based_calibration_validation_destroy(validator);
    if (status != CALIBRATION_VALIDATION_STATUS_OK) {
        printf("Unknown error!\n");
        exit(1);
    }

    return 0;
}
