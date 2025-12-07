/**
 ******************************************************************************
 * @file    test_logger.c
 * @brief   Unit tests for logger.c
 ******************************************************************************
 */

#include "unity.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>

/* Test setup and teardown */
void setUp(void) {
    /* Called before each test */
}

void tearDown(void) {
    /* Called after each test */
}

/* Test cases */
void test_logger_with_error_level(void) {
    /* Test that logger can be called with ERROR level without crashing */
    logger(LOG_LEVEL_ERROR, __FILE__, __LINE__, "Test error message");
    TEST_PASS();
}

void test_logger_with_info_level(void) {
    /* Test that logger can be called with INFO level without crashing */
    logger(LOG_LEVEL_INFO, __FILE__, __LINE__, "Test info message");
    TEST_PASS();
}

void test_logger_with_warn_level(void) {
    /* Test that logger can be called with WARN level without crashing */
    logger(LOG_LEVEL_WARN, __FILE__, __LINE__, "Test warning message");
    TEST_PASS();
}

void test_logger_with_debug_level(void) {
    /* Test that logger can be called with DEBUG level without crashing */
    logger(LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Test debug message");
    TEST_PASS();
}

void test_logger_with_formatted_string(void) {
    /* Test that logger can handle formatted strings */
    logger(LOG_LEVEL_INFO, __FILE__, __LINE__, "Test with number: %d", 42);
    TEST_PASS();
}

void test_logger_with_invalid_level_below_range(void) {
    /* Test that logger ignores NONE level */
    logger(LOG_LEVEL_NONE, __FILE__, __LINE__, "This should be ignored");
    TEST_PASS();
}

void test_logger_with_invalid_level_above_range(void) {
    /* Test that logger handles out-of-range levels gracefully */
    logger((logLevel_t)99, __FILE__, __LINE__, "This should be ignored");
    TEST_PASS();
}

void test_logger_macros(void) {
    /* Test that logger macros work correctly */
    LOG_ERROR("Error macro test");
    LOG_INFO("Info macro test");
    LOG_WARN("Warn macro test");
    LOG_DEBUG("Debug macro test");
    TEST_PASS();
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_logger_with_error_level);
    RUN_TEST(test_logger_with_info_level);
    RUN_TEST(test_logger_with_warn_level);
    RUN_TEST(test_logger_with_debug_level);
    RUN_TEST(test_logger_with_formatted_string);
    RUN_TEST(test_logger_with_invalid_level_below_range);
    RUN_TEST(test_logger_with_invalid_level_above_range);
    RUN_TEST(test_logger_macros);
    
    return UNITY_END();
}
