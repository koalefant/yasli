#pragma once

#ifdef FOR_EACH
# undef FOR_EACH
#endif
#define FOR_EACH(container, it) \
    for(it = (container).begin(); it != (container).end(); ++it)


#ifdef ARRAY_LEN
# undef ARRAY_LEN
#endif
#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))
