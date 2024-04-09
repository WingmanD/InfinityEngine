#pragma once

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define PADDING() \
private: \
float CONCAT(_padding, __LINE__) = 0.0f; \
public:
