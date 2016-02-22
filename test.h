#ifndef __TEST_H__
#define __TEST_H__

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <list>
#include <time.h>
#include <algorithm>
#include <cmath>

#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"

namespace tester
{
  template <typename T>
  class LeftValue;

  namespace Checker
  {
    template <typename T>
      class CheckIf;

    template <typename T, bool streamable = CheckIf<T>::streamable, bool castable = CheckIf<T>::castable>
      struct Dummy;
  };

  std::string prefix;

  // ----------------------------------------
  // Evaluer class
  // ----------------------------------------
  // {{{

  class Evaluer
  {
  public:
    Evaluer(std::string expr_, std::string filename_, int lineNo_):
      expr(expr_), filename(filename_), lineNo(lineNo_)  { }

    template <typename T>
      LeftValue<T> operator<< (T leftVal);

    std::string getExpr() { return expr; }
    std::string getFilename() { return filename; }
    int getLineNo() { return lineNo; }

  private:
    std::string expr, filename;
    int lineNo;

  };

  // }}}
  // ----------------------------------------
  // LeftValue class
  // ----------------------------------------
  // {{{

  struct PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS;

  template <typename U>
    class LeftValue
    {
    public:
      LeftValue(U& leftValue_, Evaluer& evaluer_): leftValue(leftValue_), evaluer(evaluer_) { }

      template <typename V>
        void operator== (V rightValue);

      template <typename V>
        void operator!= (V rightValue);

      template <typename V>
        void operator<= (V rightValue);

      template <typename V>
        void operator>= (V rightValue);

      template <typename V>
        void operator< (V rightValue);

      template <typename V>
        void operator> (V rightValue);

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator&& (V rightValue);

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator|| (V rightValue);

      template <typename V>
        void assert(bool val, std::string op, V rightValue);

    private:
      U leftValue;
      Evaluer& evaluer;
    };

  template<>
    class LeftValue<bool>
    {
    public:
      LeftValue(bool leftValue, Evaluer& evaluer_): evaluer(evaluer_) { assert(leftValue); }

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator&& (V rightValue);

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator|| (V rightValue);

      void assert(bool val);

    private:
      Evaluer& evaluer;
    };

  // }}}
  // ----------------------------------------
  // TestMonitor class
  // ----------------------------------------
  // {{{

  class TestCase;

  class TestMonitor {
  public:
    std::string getTestName();
    void reportBegin();
    void reportEnd();

    static void pushMonitor(const std::string& testName, const std::string& file, int line);
    static void popSAMonitor();
    static void popAndDeleteMonitor();

    static void registerSATestCase(TestCase *ptc);

    static bool anyTestFailed();
    static void testCaseResult(bool passed);
    static void SATestCaseResult(bool passed);
    static void runAllTests();
    static TestMonitor& getMostRecent();

  private:
    static int overallyFailed;
    static std::list<TestMonitor*> currentMonitors;
    static std::list<std::pair<TestCase*, std::list<TestMonitor*>>> monitorsForCase;

    TestMonitor(const std::string& testName, const std::string& file, int line);

    int passed, failed;
    std::string testName, file;
    int line;
  };

  // }}}
  // ----------------------------------------
  // CaseMonitor class
  // ----------------------------------------
  // {{{

  class CaseMonitor {
  public:
    void init(std::string caseName, std::string file, int line);
    void report();
    void addCheck(bool passed);
    bool passed();

    static CaseMonitor& onlyInstance();

    CaseMonitor(CaseMonitor& nd) = delete;
    void operator=(CaseMonitor& nd) = delete;

  private:
    std::string caseName;
    int failed;

    CaseMonitor(): failed(0) { }

  };

  // }}}
  // ----------------------------------------
  // Evaluer definitions
  // ----------------------------------------
  // {{{

  template <typename T>
    LeftValue<T> Evaluer::operator<< (T leftValue)
    {
      return LeftValue<T>(leftValue, *this);
    }

  // }}}
  // ----------------------------------------
  // LeftValue definitions
  // ----------------------------------------
  // {{{

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator> (V rightValue)
    {
      assert(leftValue > rightValue, ">", rightValue);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator< (V rightValue)
    {
      assert(leftValue < rightValue, "<", rightValue);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator>= (V rightValue)
    {
      assert(leftValue >= rightValue, ">=", rightValue);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator<= (V rightValue)
    {
      assert(leftValue <= rightValue, "<=", rightValue);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator!= (V rightValue)
    {
      assert(leftValue != rightValue, "!=", rightValue);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator== (V rightValue)
    {
      assert(leftValue == rightValue, "==", rightValue);
    }

  void assertCommonPart(std::ostream &out, bool passed, Evaluer &evaluer)
  {
    CaseMonitor::onlyInstance().addCheck(passed);
    std::string note = passed ? "Passed check" : "FAILED check";
    out << prefix << note << " (" <<  evaluer.getFilename() << ":" << evaluer.getLineNo() << ") "<< std::endl;
    std::cout << prefix << "    " << evaluer.getExpr()  << std::endl;
    out << prefix << "    Evaluated:" << std::endl;
  }

  template <typename U>
  template <typename V>
    void LeftValue<U>::assert (bool val, std::string op, V rightValue)
    {
      std::ostream& out = val ? std::cout : std::cerr;
      assertCommonPart(out, val, evaluer);
      out << prefix << std::string(8, ' ') <<  Checker::Dummy<U>::repr(leftValue) << " " << op << " " << Checker::Dummy<U>::repr(rightValue) << std::endl;
    }

  void LeftValue<bool>::assert (bool val)
  {
    std::ostream& out = val ? std::cout : std::cerr;
    assertCommonPart(out, val, evaluer);
    out << prefix << std::string(4, ' ') << std::boolalpha << "    " << val << std::endl;
  }

  // }}}
  // ----------------------------------------
  // TestMonitor definitions
  // ----------------------------------------
  // {{{

  class TestCase
  {
  public:
    bool run();
  protected:
    TestCase(std::string name, std::string file, int line): name(name), file(file), line(line) { }
  private:
    std::string name, file;
    int line;
    virtual void _run() = 0;
  };

  int TestMonitor::overallyFailed = 0;
  std::list<TestMonitor*> TestMonitor::currentMonitors;
  std::list<std::pair<TestCase*, std::list<TestMonitor*>>> TestMonitor::monitorsForCase;

  void TestMonitor::pushMonitor(const std::string& testName, const std::string& file, int line)
  {
    currentMonitors.push_back(new TestMonitor(testName, file, line));
  }

  void TestMonitor::registerSATestCase(TestCase *ptc)
  {
    monitorsForCase.push_back(std::pair<TestCase*, std::list<TestMonitor*>>(ptc, currentMonitors));
  }

  void TestMonitor::popSAMonitor()
  {
    currentMonitors.pop_back();
  }

  void TestMonitor::popAndDeleteMonitor()
  {
    delete currentMonitors.back();
    currentMonitors.pop_back();
  }

  TestMonitor& TestMonitor::getMostRecent()
  {
    return *currentMonitors.back();
  }

  void TestMonitor::runAllTests()
  {
    if (monitorsForCase.empty())
    {
      std::cerr << "No cases to run" << std::endl;
      return;
    }
    auto monitorsToReport = monitorsForCase.begin()->second;
    for (auto it = monitorsToReport.begin(); it != monitorsToReport.end(); ++it)
    {
      (*it)->reportBegin();
    }
    for (auto st = monitorsForCase.begin(); st != monitorsForCase.end(); ++st)
    {
      bool passed = st->first->run();
      TestMonitor::SATestCaseResult(passed);
      for (auto it = st->second.begin(); it != st->second.end(); ++it)
      {
        (*it)->passed += passed;
        (*it)->failed += !passed;
      }
      auto nd = std::next(st, 1);
      if (nd == monitorsForCase.end())
        break;

      auto it1 = st->second.begin();
      auto it2 = nd->second.begin();
      while (it1 != st->second.end() && it2 != nd->second.end() && *it1 == *it2)
      {
        ++it1;
        ++it2;
      }
      for(auto back_it1 = st->second.end(); back_it1-- != it1; )
      {
        (*back_it1)->reportEnd();
        delete *back_it1;
      }
      for( ; it2 != nd->second.end(); ++it2)
        (*it2)->reportBegin();
    }
    auto last = std::prev(monitorsForCase.end());
    for(auto it = last->second.end(); it-- != last->second.begin(); )
    {
      (*it)->reportEnd();
      delete *it;
    }
  }

  TestMonitor::TestMonitor(const std::string& testName, const std::string& file, int line): passed(0), failed(0), testName(testName), file(file), line(line)
  {
  }

  std::string TestMonitor::getTestName()
  {
    return testName;
  }

  void TestMonitor::reportBegin()
  {
    std::cout << tester::prefix << "\"" << testName << "\" - group starting ("  << file << ":" << line << ")" << std::endl;
    prefix += "    ";
  }

  void TestMonitor::reportEnd()
  {
    int tests = passed + failed;
    prefix = prefix.substr(0, prefix.length() - 4);

    std::cerr << prefix << "\"" << getTestName() << "\"";

    if (tests == 0)
      std::cerr << " - no cases";
    else
    {
      std:: cerr << " - passed: "
        << static_cast<double>(100*passed)/tests << "% ( " << passed << " / "
        << tests << " case" << (tests == 1 ? " )":"s )");
    }
    std::cerr << std::endl;
  }

  void TestMonitor::testCaseResult(bool passed)
  {
    for (auto it = currentMonitors.begin(); it != currentMonitors.end(); ++it)
    {
      TestMonitor *ptr = *it;
      ptr->passed += passed;
      ptr->failed += !passed;
    }
    overallyFailed += !passed;
  }

  void TestMonitor::SATestCaseResult(bool passed)
  {
    overallyFailed += !passed;
  }

  bool TestMonitor::anyTestFailed()
  {
    return overallyFailed;
  }

  // }}}
  // ----------------------------------------
  // CaseMonitor definitions
  // ----------------------------------------
  // {{{

  void CaseMonitor::init(std::string caseName, std::string file, int line)
  {
    this->caseName = caseName;
    failed = 0;

    std::cout << prefix << "\"" << caseName << "\" - case starting ("  << file << ":" << line << ")" << std::endl;
    prefix += "    ";
  }

  void CaseMonitor::report()
  {
    prefix = prefix.substr(0, prefix.length() - 4);

    if (failed == 0)
      std::cerr << prefix << "\"" << caseName << "\" - passed" << std::endl;
    else
      std::cerr << prefix << "\"" << caseName << "\" - FAILED" << std::endl;
  }

  void CaseMonitor::addCheck(bool passed)
  {
    this->failed += !passed;
  }

  bool CaseMonitor::passed()
  {
    return failed == 0;
  }

  CaseMonitor& CaseMonitor::onlyInstance()
  {
    static CaseMonitor onlyInstance;
    return onlyInstance;
  }

  // }}}
  // ----------------------------------------
  // Stream and cast operator existence checker
  // ----------------------------------------
  // {{{

  namespace Checker
  {
    struct DummyType {};

    template <typename T>
      DummyType operator<< (std::ostream& out, T& c);

    template <typename T>
      class CheckIf
      {
        typedef char one;
        typedef long two;

        static std::ostream &out;
        static T &t;

        template <typename C>
          static one checkStreamable(std::ostream&);
        template <typename C>
          static two checkStreamable(DummyType);

        template <typename A, std::string (A::*)()>
          struct check_type;
        template <typename C>
          static one checkCastable(check_type<C, &C::operator std::string>*);
        template <typename C>
          static two checkCastable(...);

      public:
        static const bool streamable = sizeof(checkStreamable<T>(out << t)) == sizeof(char);
        static const bool castable = sizeof(checkCastable<T>(0)) == sizeof(char);
      };

    template <typename T, bool streamable, bool castable>
      struct Dummy
      {
        static std::string repr(T t);
      };

    template <typename T, bool castable>
      struct Dummy<T, true, castable>
      {
        static std::string repr(T t)
        {
          std::ostringstream ss;
          ss  << t;
          return ss.str();
        }
      };

    template <typename T, bool streamable>
      struct Dummy<T, streamable, true>
      {
        static std::string repr(T t)
        {
          return (std::string) t;
        }
      };

    template <typename T>
      struct Dummy<T, false, false>
      {
        static std::string repr(T t)
        {
          return "(?)";
        }
      };
  }
  
  // }}}
  // ----------------------------------------
  // TimeTester class with definitions
  // ----------------------------------------
  // {{{
  class TimeTester
  {
  public:
    TimeTester() {}
    TimeTester(std::string name);

    void start();
    void stop();
    timespec get_diff();
    void report();

  private:
    timespec start_time, stop_time, diff;
    std::string name;

  };

  std::map<std::string, TimeTester> timeTesters;

  TimeTester::TimeTester(std::string name): name(name) { }

  void TimeTester::start()
  {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
  }

  void TimeTester::stop()
  {
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop_time);
    if (stop_time.tv_nsec - start_time.tv_nsec < 0)
    {
      diff.tv_sec = stop_time.tv_sec - start_time.tv_sec - 1;
      diff.tv_nsec = 1000000000 + stop_time.tv_nsec - start_time.tv_nsec;
    }
    else
    {
      diff.tv_sec = stop_time.tv_sec - start_time.tv_sec;
      diff.tv_nsec = stop_time.tv_nsec - start_time.tv_nsec;
    }
  }

  timespec TimeTester::get_diff()
  {
    return diff;
  }

  void TimeTester::report()
  {
    std::cout << "Timer ";
    if (name != "")
    {
      std::cerr << "\"" << name << "\"";
    }
    std::cout << " result";
    std::cerr << ": ";
    std::cout << diff.tv_sec << "s ";
    long res = diff.tv_nsec;
    int nsec = res % 1000;
    res /= 1000;
    int usec = res % 1000;
    res /= 1000;
    int msec = res % 1000;

    std::cout << msec << "ms ";
    std::cout << usec << "us ";
    std::cout << nsec << "ns";
    std::cout << " ( ";
    std::cerr << diff.tv_sec << "."
      << std::setw(3) << std::setfill('0') << msec
      << std::setw(3) << std::setfill('0') << usec
      << std::setw(3) << std::setfill('0') << nsec;
    std::cout << "s )";
    std::cerr << std::endl;
    std::cout.fill(' ');
  }

  // }}}
  // ----------------------------------------
  // Utils
  // ----------------------------------------
  // {{{

  bool almostEqual(double d1, double d2, double eps=1e-8)
  {
    return fabs(d1 - d2) < eps;
  }

  bool TestCase::run()
  {
    CaseMonitor::onlyInstance().init(name, file, line);
    _run();
    CaseMonitor::onlyInstance().report();
    return CaseMonitor::onlyInstance().passed();
  }

}

  // }}}
  // ----------------------------------------
  // Macros
  // ----------------------------------------
  // {{{

#define CHECK(expr) tester::Evaluer(#expr, __FILE__, __LINE__) << expr;

#define TEST_GROUP_BEGIN(name) tester::TestMonitor::pushMonitor(name, __FILE__, __LINE__); \
  tester::TestMonitor::getMostRecent().reportBegin();

#define TEST_GROUP_END() tester::TestMonitor::getMostRecent().reportEnd();\
  tester::TestMonitor::popAndDeleteMonitor();

#define TEST_CASE_BEGIN(name) tester::CaseMonitor::onlyInstance().init(name, __FILE__, __LINE__);

#define TEST_CASE_END() tester::TestMonitor::testCaseResult(tester::CaseMonitor::onlyInstance().passed()); \
  tester::CaseMonitor::onlyInstance().report();

#define TEST_GROUP(fun) tester::TestMonitor::pushMonitor(#fun, __FILE__, __LINE__); \
  tester::TestMonitor::getMostRecent().reportBegin(); \
  fun(); \
  tester::TestMonitor::getMostRecent().reportEnd(); \
  tester::TestMonitor::popAndDeleteMonitor();

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define SA_TEST_GROUP_BEGIN(name) struct CONCAT(__test_group_, __LINE__) \
{ \
  CONCAT(__test_group_, __LINE__)() { tester::TestMonitor::pushMonitor(name, __FILE__, __LINE__); } \
}; \
CONCAT(__test_group_, __LINE__) CONCAT(_tg_, __LINE__);


#define SA_TEST_GROUP_END() struct CONCAT(__test_group_, __LINE__) \
{ \
  CONCAT(__test_group_, __LINE__)() { tester::TestMonitor::popSAMonitor(); } \
}; \
CONCAT(__test_group_, __LINE__) CONCAT(_tg_, __LINE__);


#define SA_TEST_CASE(name) class CONCAT(__test_case_, __LINE__) : tester::TestCase \
    { \
    public: \
      CONCAT(__test_case_, __LINE__)(std::string tc_name, std::string file, int line): TestCase(tc_name, file, line) { tester::TestMonitor::registerSATestCase(this); } \
    private: \
      void _run(); \
    }; \
 \
CONCAT(__test_case_, __LINE__) CONCAT(_tc_, __LINE__)(name, __FILE__, __LINE__); \
 \
void CONCAT(__test_case_, __LINE__)::_run()

#define PRINT(str) std::cout << tester::prefix << "--- " << str << " ---" << std::endl;

#define TEST_CASE(fun) tester::CaseMonitor::onlyInstance().init(#fun, __FILE__, __LINE__);\
  fun();\
  tester::TestMonitor::testCaseResult(tester::CaseMonitor::onlyInstance().passed()); \
  tester::CaseMonitor::onlyInstance().report();

#define TEST_RESULT tester::TestMonitor::anyTestFailed();

#define START_TIMER(name) std::cout << "Starting timer \"" << name << "\" (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;\
  tester::timeTesters[name] = tester::TimeTester(name);\
  tester::timeTesters[name].start();

#define STOP_TIMER(name) tester::timeTesters[name].stop();\
  std::cout << "Stopping timer \"" << name << "\"" << std::endl;\

#define REPORT_TIMER(name) tester::timeTesters[name].report();

#define MAIN_RUN_ALL_TESTS() int main() \
{ \
  tester::TestMonitor::runAllTests(); \
  return TEST_RESULT; \
}

  // }}}

#endif

