#pragma once

class Refrigerator{};

template<class RefrigeratorType = Refrigerator>
class FrozenSignal : public sigslot::signal1<RefrigeratorType*>{
public:
    using sigslot::signal1<RefrigeratorType*>::emit;
    FrozenSignal()
    : counter_(0)
    {}
    void freeze(RefrigeratorType* refrigerator){
        if(counter_ == 0)
            refrigerator_ = refrigerator;
        ++counter_;
    }
    RefrigeratorType* unfreeze(){
        ASSERT(counter_ > 0);
        --counter_;
        if(counter_ == 0){
            emit(refrigerator_);
            return refrigerator_;
        }
        return 0;
    }
protected:
    int counter_;
    RefrigeratorType* refrigerator_;
};
