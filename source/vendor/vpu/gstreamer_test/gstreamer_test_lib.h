#pragma once

#include <gtest/gtest.h>

// This class represents the MediaPlayer testing framework and provides
// helpers and callbacks for GUnit to use for testing.
class MediaTest : public testing::Test {
 protected:
  // SetUp initializes the MediaPlayer
  // void SetUp();

  // TearDown cleans up
  // void TearDown();
};

class GstPlayerTest : public MediaTest,
                      public testing::WithParamInterface<int> {};
