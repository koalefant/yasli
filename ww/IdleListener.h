#pragma once

namespace ww{

class IdleListenerImpl;
class IdleListener{
public:
	IdleListener();
	virtual void onIdle() = 0;
protected:
	IdleListenerImpl* impl_;
};

}

