#include "StdAfx.h"
#include "testo/Testo.h"
#include "utils/Errors.h"
#include <iostream>
#include <algorithm>

namespace Testo{

    Log::Log(int testsCount)
    : failed_(0)
    , succeed_(0)
    , testsCount_(testsCount)
    , currentTest_(0)
    {
    }
    void Log::begin(const char* groupName, const char* name)
    {
        char buf[4];
        sprintf(buf, "%02i", std::min(99, int(float(currentTest_ * 100.0f)/float(testsCount_))));
        std::cout << "[" << buf << "%] Testing " << groupName << "::" << name << "... " << std::flush;
    }
    void Log::operator()(const Message& message)
    {
#ifdef _MSC_VER
        std::cout << "\n" << message.filename() << "(" << message.line() << ") : test error: " << message.text();
#else
        std::cout << "\n" << message.filename() << ":" << message.line() << ": " << message.text();
#endif
    }
    void Log::end(bool success)
    {
        if(success){
            std::cout << "ok\n";
            ++succeed_;
        }
        else{
            std::cout << "\nFAILED!\n";
            ++failed_;
        }
        ++currentTest_;
    }

    void Log::summary()
    {
        std::cout << " * Testing done! Failed: " << failed_  << ", Succeed: " << succeed_ << std::endl;
    }

    // ---------------------------------------------------------------------------

    Manager& Manager::the()
    {
        static Manager manager;
        return manager;
    }

    void Manager::add(TesterBase* tester)
    {
        testers_.push_back(tester);
    }

    int Manager::invokeAll()
    {
        Log log(testers_.size());

        Testers::iterator it;
        for(it = testers_.begin(); it != testers_.end(); ++it){
            TesterBase* tester = *it;
            ASSERT(tester);
            tester->test(log);
        }

        log.summary();
        return log.failed();
    }

    // ---------------------------------------------------------------------------
    TesterBase::TesterBase(const char* groupName, const char* name)
    : name_(name)
    , groupName_(groupName)
    {
        Manager::the().add(this);
    }

    TesterBase*& TesterBase::current()
    {
        static TesterBase* currentOne;
        return currentOne;
    }

    bool TesterBase::test(Log& log)
    {
        ASSERT(!current());
        current() = this;
        log.begin(groupName(), name());
        try{
            invoke();
        }
        catch(ErrorGeneric& err){
            error(err.what(), err.fileName(), err.line());
        }
        bool result = messages_.empty();
        Messages::iterator it;

        for(it = messages_.begin(); it != messages_.end(); ++it)
            log(*it);
        log.end(messages_.empty());
        current() = 0;
        return result;
    }

    void TesterBase::report(const Message& message)
    {
        ASSERT(this);
        messages_.push_back(message);
    }

    void TesterBase::ensure(bool condition, const char* name, const char* filename, int line)
    {
        ASSERT(this);
        if(!condition){
            report(Message(name, filename, line));
        }
    }

    void TesterBase::error(const char* description, const char* filename, int line)
    {
        ASSERT(this);
        report(Message(description, filename, line));
    }

}
