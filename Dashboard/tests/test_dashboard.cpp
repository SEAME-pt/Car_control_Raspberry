#include "../incs/Dashboard.hpp"
#include "../incs/DashboardMacros.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(DashboardTest, DefaultDimensions) {
	Dashboard d;
	EXPECT_EQ(d.width(), WIDTH);
	EXPECT_EQ(d.height(), HEIGHT);
	EXPECT_EQ(d.margin(), MARGIN);
	EXPECT_EQ(d.padding_x(), PADDING_X);
	EXPECT_EQ(d.padding_y(), PADDING_Y);
	EXPECT_EQ(d.topHeight(), TOP_HEIGHT);
	EXPECT_EQ(d.centerHeight(), 1.0 - TOP_HEIGHT - BOTTOM_HEIGHT - (2 * PADDING_Y));
	EXPECT_EQ(d.bottomHeight(), BOTTOM_HEIGHT);
	EXPECT_EQ(d.leftWidth(), LEFT_WIDTH);
	EXPECT_EQ(d.centerWidth(), 1.0 - LEFT_WIDTH - RIGHT_WIDTH - (2 * PADDING_X));
	EXPECT_EQ(d.rightWidth(), RIGHT_WIDTH);
}