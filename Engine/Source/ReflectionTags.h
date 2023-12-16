#pragma once

#define REFLECTED(...)
#define PROPERTY(...)
#define METHOD(...)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define GENERATED() CONCAT(CONCAT(CONCAT(GENERATED_, FILENAME), _), CONCAT(__LINE__, ()))
