#pragma once

#include "yasli/Assert.h"

namespace yasli{

#pragma warning (push)
#pragma warning (disable:4127)

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

#pragma warning (pop)


}