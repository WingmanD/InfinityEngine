﻿#pragma once

#include "UtilMacros.h"

#define REFLECTED(...)
#define PROPERTY(...)
#define METHOD(...)

#define GENERATED() CONCAT(CONCAT(CONCAT(GENERATED_, FILENAME), _), CONCAT(__LINE__, ()))
