#pragma once

#define FOR_EACH(container, it) \
    for(it = (container).begin(); it != (container).end(); ++it)

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))
