#pragma once

#include "gstreamer_middleware_api.h"

#define DECODE_TEST_FILE_NAME "/data/test.mp4"

struct TestSourceBuffer : gstreamer_middleware_output_buffer_t {
    hw_buffer_t *bo;
};

class TestSource
{
public:
    virtual ~TestSource(){};
    virtual int get_test_encode_buffer(TestSourceBuffer *buffer) = 0;
    virtual int release_test_encode_buffer(TestSourceBuffer *buffer) = 0;
};