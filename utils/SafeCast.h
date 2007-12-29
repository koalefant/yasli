#pragma once

#include "utils/Errors.h"

template<class _Dest, class _Source>
_Dest safe_cast(_Source source){
#ifdef _DEBUG
    _Dest result = dynamic_cast<_Dest>(source);
    ASSERT(!source || result && "safe_cast failed!");
    return result;
#else
    return static_cast<_Dest>(source);
#endif
}
