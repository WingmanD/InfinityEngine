#pragma once

#define PADDING() \
private: \
float CONCAT(_padding, __LINE__) = 0.0f; \
public:
