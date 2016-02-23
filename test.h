#ifndef __TEST_H__
#define __TEST_H__

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <map>
#include <list>
#include <time.h>
#include <algorithm>
#include <cmath>

#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"

#define WIDTH 100

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
    static void filterTests(std::vector<std::string> groupsToRun);
    static void runTests();
    static TestMonitor& getMostRecent();

  private:
    static int overallyFailed;
    static std::vector<TestMonitor*> currentMonitors;
    static std::vector<std::pair<TestCase*, std::vector<TestMonitor*>>> casesWithMonitors;

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
    std::string file;
    int line;
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
    std::ostringstream pref, suff;
    if (!passed)
      out << prefix << "at " << evaluer.getFilename() << ":" << evaluer.getLineNo() << ":" << std::endl;
    pref << prefix << "CHECK(" << evaluer.getExpr()  << ")";
    suff << (passed ? "  passed" : "  FAILED");
    int fillLen = WIDTH - pref.str().length() - suff.str().length();
    out << pref.str() << std::string(std::max(fillLen, 0), '.') << suff.str() << std::endl;
    out << prefix << "     / ";
  }

  template <typename U>
  template <typename V>
    void LeftValue<U>::assert (bool val, std::string op, V rightValue)
    {
      std::ostream& out = val ? std::cout : std::cerr;
      assertCommonPart(out, val, evaluer);
      out <<  Checker::Dummy<U>::repr(leftValue) << " " << op << " " << Checker::Dummy<U>::repr(rightValue) << " /" << std::endl;
    }

  void LeftValue<bool>::assert (bool val)
  {
    std::ostream& out = val ? std::cout : std::cerr;
    assertCommonPart(out, val, evaluer);
    out << std::boolalpha << val << " /" << std::endl;
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
  std::vector<TestMonitor*> TestMonitor::currentMonitors;
  std::vector<std::pair<TestCase*, std::vector<TestMonitor*>>> TestMonitor::casesWithMonitors;

  void TestMonitor::pushMonitor(const std::string& testName, const std::string& file, int line)
  {
    currentMonitors.push_back(new TestMonitor(testName, file, line));
  }

  void TestMonitor::registerSATestCase(TestCase *ptc)
  {
    casesWithMonitors.push_back(decltype(casesWithMonitors)::value_type(ptc, currentMonitors));
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

  void TestMonitor::filterTests(std::vector<std::string> groupsToRun)
  {
    decltype(casesWithMonitors) filtered;
    auto monitorInGroupsToRun = [&groupsToRun] (TestMonitor *monitor)
    {
      return std::find(groupsToRun.begin(), groupsToRun.end(), monitor->testName) != groupsToRun.end();
    };
    std::copy_if(
        casesWithMonitors.begin(),
        casesWithMonitors.end(),
        std::back_inserter(filtered),
        [&groupsToRun, &monitorInGroupsToRun] (decltype(casesWithMonitors)::value_type &elem)
        {
          auto &monitors = elem.second;
          return std::any_of(monitors.begin(), monitors.end(), monitorInGroupsToRun);
        });

    // with below 'for' only listed groups and their children are output
    // when it's commented, parent groups will be included in output too
    for (auto &CMPair : filtered)
    {
      auto monitorsList = decltype(CMPair.second)(CMPair.second);
      auto firstToCopy = std::find_if(
          monitorsList.begin(),
          monitorsList.end(),
          monitorInGroupsToRun
          );
      CMPair.second.clear();
      CMPair.second.assign(firstToCopy, monitorsList.end());
    }

    casesWithMonitors.clear();
    casesWithMonitors.assign(filtered.begin(), filtered.end());
  }

  void TestMonitor::runTests()
  {
    if (casesWithMonitors.empty())
    {
      std::cerr << "No cases to run" << std::endl;
      return;
    }
    auto &monitorsToReport = casesWithMonitors.begin()->second;
    for (TestMonitor *monitor : monitorsToReport)
    {
      monitor->reportBegin();
    }
    for (auto stCMPairIt = casesWithMonitors.begin(); stCMPairIt != casesWithMonitors.end(); ++stCMPairIt)
    {
      bool passed = stCMPairIt->first->run();
      TestMonitor::SATestCaseResult(passed);
      for (auto monitor : stCMPairIt->second)
      {
        monitor->passed += passed;
        monitor->failed += !passed;
      }
      auto ndCMPairIt = std::next(stCMPairIt, 1);
      if (ndCMPairIt == casesWithMonitors.end())
        break;

      auto stMonitorsIt = stCMPairIt->second.begin();
      auto ndMonitorsIt = ndCMPairIt->second.begin();
      while (stMonitorsIt != stCMPairIt->second.end() && ndMonitorsIt != ndCMPairIt->second.end() && *stMonitorsIt == *ndMonitorsIt)
      {
        ++stMonitorsIt;
        ++ndMonitorsIt;
      }
      for(auto back_it1 = stCMPairIt->second.end(); back_it1-- != stMonitorsIt; )
      {
        (*back_it1)->reportEnd();
        delete *back_it1;
      }
      for( ; ndMonitorsIt != ndCMPairIt->second.end(); ++ndMonitorsIt)
        (*ndMonitorsIt)->reportBegin();
    }
    auto last = std::prev(casesWithMonitors.end());
    for(auto monitorsIt = last->second.end(); monitorsIt-- != last->second.begin(); )
    {
      (*monitorsIt)->reportEnd();
      delete *monitorsIt;
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
    std::cerr << tester::prefix << "\"" << testName << "\" - group starting";
    std::cerr << std::endl;
    prefix += "    ";
  }

  void TestMonitor::reportEnd()
  {
    int tests = passed + failed;
    prefix = prefix.substr(0, prefix.length() - 4);
    std::ostringstream pref, suff;

    pref << prefix << "\"" << getTestName() << "\"  ";

    if (tests == 0)
      suff << "  no cases";
    else
    {
      suff << "  passed: "
        << static_cast<double>(100*passed)/tests << "% ( " << passed << " / "
        << tests << " )";
    }
    std::cerr << pref.str() << std::string(WIDTH - pref.str().length() - suff.str().length(), '.') << suff.str() << std::endl;
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
    this->file = file;
    this->line = line;
    failed = 0;

    std::cout << prefix << "\"" << caseName << "\" - case starting";
    std::cout << std::endl;
    prefix += "    ";
  }

  void CaseMonitor::report()
  {
    prefix = prefix.substr(0, prefix.length() - 4);
    std::ostringstream pref, suff;
    pref << prefix << "\"" << caseName << "\"  ";

    if (failed == 0)
      suff << "  case passed";
    else
      suff << "  case FAILED";
    if (failed > 0)
      std::cerr << prefix << "at " << file << ":" << line << ":" << std::endl;
    std::cerr << pref.str() << std::string(WIDTH - pref.str().length() - suff.str().length(), '.') << suff.str() << std::endl;
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

#define MAIN_RUN_ALL_TESTS() int main(int argc, char **argv) \
{ \
  if (argc > 1) \
  { \
    std::vector<std::string> groups(argc - 1); \
    groups.assign(argv + 1, argv + argc); \
    tester::TestMonitor::filterTests(groups); \
  } \
  tester::TestMonitor::runTests(); \
  return TEST_RESULT; \
}
  // }}}

#endif

