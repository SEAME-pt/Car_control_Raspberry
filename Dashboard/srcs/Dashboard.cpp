#include "../incs/Dashboard.hpp"

int Dashboard::width() const {
    return WIDTH;
}

int Dashboard::height() const {
    return HEIGHT;
}

int Dashboard::margin() const {
    return MARGIN;
}

double Dashboard::padding_x() const {
	return PADDING_X;
}

double Dashboard::padding_y() const {
	return PADDING_Y;
}

double Dashboard::topHeight() const {
    return TOP_HEIGHT;
}

double Dashboard::centerHeight() const {
    return 1.0 - TOP_HEIGHT - BOTTOM_HEIGHT - (2 * PADDING_Y);
}

double Dashboard::bottomHeight() const {
    return BOTTOM_HEIGHT;
}

double Dashboard::rightWidth() const {
    return RIGHT_WIDTH;
}

double Dashboard::centerWidth() const {
    return 1.0 - LEFT_WIDTH - RIGHT_WIDTH - (2 * PADDING_X);
}

double Dashboard::leftWidth() const {
    return LEFT_WIDTH;
}