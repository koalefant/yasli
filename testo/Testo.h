#ifndef __TESTO_TESTO_H_INCLUDED__
#define __TESTO_TESTO_H_INCLUDED__

#include <vector>
#include <string>

#ifdef _MSC_VER
# pragma warning(disable: 4251 4275)
#endif

#include "utils/SharedLibApi.h"
#ifdef TESTO_EXPORTED
# define TESTO_API EXPORT_SYMBOL
#else
# define TESTO_API IMPORT_SYMBOL
#endif

namespace Testo{

    class TESTO_API Message{
    public:
        Message(const char* text, const char* filename, int line)
        : text_(text)
        , filename_(filename)
        , line_(line)
        {}
        const char* filename() const{ return filename_; }
        const char* text() const{ return text_.c_str(); }
        int line() const{ return line_; }
    protected:
        std::string text_;
        const char* filename_;
        int line_;
    };
    typedef std::vector<Message> Messages;

    class TESTO_API Log{
    public:
        Log(int testsCount);
        void begin(const char* groupName, const char* name);
        void end(bool success);
        void operator()(const Message& message);
        void summary();

        int failed() const{ return failed_; }
        int succeed() const{ return succeed_; }
    protected:
        int failed_;
        int succeed_;
        int testsCount_;
        int currentTest_;
    };


    class TESTO_API TesterBase{
    public:
        TesterBase(const char* groupName, const char* name);
        virtual ~TesterBase() {}

        bool test(Log& log);
        virtual void invoke() = 0;

        const char* name() const{ return name_; }
        const char* groupName() const{ return groupName_; }

        void report(const Message& message);
        static TesterBase*& current();

        void ensure(bool condition, const char* name, const char* filename, int line);
        void error(const char* description, const char* filename, int line);
    protected:

        Messages messages_;
        const char* name_;
        const char* groupName_;
    };




    /*
    class TestSummary{
    public:
        TestSummary();
        float time() const{ return time_; }
        int failed() const{ return failed_; }
        Messages& messages()
        typedef std::vector<std::string> Messages;
    protected:
        Messages messages_;
        float time_;
        int failed_;
    };

    */

    template<class TestType>
    class Tester : public TesterBase{
    public:
        Tester(const char* groupName, const char* name)
        : TesterBase(groupName, name)
        {
        }
        void invoke(){
            TestType testedOne;
            testedOne.invoke();
        }
    };

    class TESTO_API Manager{
    public:
        void add(TesterBase* tester);
        int invokeAll();

        static Manager& the();
    protected:
        typedef std::vector<TesterBase*> Testers;
        Testers testers_;
    };
}

using Testo::Tester;

#define TESTO_BEGIN() namespace Test{
#define TESTO_END() }

#define TESTO_ADD_TEST(groupName, TestClass) \
    Tester<TestClass> TestClass##Tester(groupName, #TestClass); \
    const char* TestClass##name = TestClass##Tester.name();

#define TESTO_ENSURE(expression) \
    Testo::TesterBase::current()->ensure((expression), #expression, __FILE__, __LINE__)

#endif
