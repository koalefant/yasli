#pragma once

#include <assert.h>
#include <string>

namespace ErrorHelpers {
    template<class ExceptionType>
    ExceptionType& throwExceptionFrom(const ExceptionType& _exception, const char* fileName, const char* functionName, int line){
        static ExceptionType exception;
        exception = _exception;
        exception.fileName_ = fileName;
        exception.functionName_ = functionName;
        exception.line_ = line;
        return exception;
    }
	//void trace(const char* text);
}

#ifdef RAISE
#warning RAISE macro already defined!
#undef RAISE
#else
# define RAISE(exception) throw ErrorHelpers::throwExceptionFrom(exception, __FILE__, __FUNCTION__, __LINE__)
#endif


#ifndef NDEBUG
# define ASSERT(x) assert((x))
# define VERIFY(x) assert(x);
# define TRACE(x) ErrorHelpers::trace((x))
#else
# define ASSERT(x)
# define VERIFY(x) (x)
# define TRACE(x)
#endif

struct ErrorGeneric{
    ErrorGeneric()
    : fileName_("unitialized exception!")
    , functionName_("")
    , line_(0)
    {
    }
    virtual ~ErrorGeneric() {}
    virtual const char* what() const = 0;
    const char* fileName() const{ return fileName_; }
    const char* functionName() const{ return functionName_; };
    int line() const{ return line_; };

    const char* fileName_;
    const char* functionName_;
    int line_;
};

struct ErrorLogic : ErrorGeneric{
    ErrorLogic() {}
    ErrorLogic(const char* message)
    : message_(message)
    {}
    const char* what() const{ return message_.c_str(); }
private:
    std::string message_;
};

struct ErrorRuntime : ErrorGeneric{
    ErrorRuntime() {}
    ErrorRuntime(const char* message)
    : message_(message)
    {}
    const char* what() const{ return message_.c_str(); }
protected:
    void setMessage(const char* message) { message_ = message; }
private:
    std::string message_;
};

struct ErrorContent : ErrorRuntime{
    ErrorContent()
    {}
    ErrorContent(const char* message)
    : ErrorRuntime(message)
    {}
};
