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

  class TestMonitor {
  public:
    std::string getTestName();
    void report();

    static void addMonitor(const std::string& testName);
    static bool anyTestFailed();
    static void removeMonitor();
    static void addCase(bool passed);
    static TestMonitor& getMostRecent();

  private:
    static int overallyFailed;
    static std::list<TestMonitor> testMonitors;

    TestMonitor(const std::string& testName);

    int passed, failed;
    std::string testName;
  };

  // }}}
  // ----------------------------------------
  // CaseMonitor class
  // ----------------------------------------
  // {{{

  class CaseMonitor {
  public:
    CaseMonitor(): failed(0) { }

    void init(std::string caseName);
    std::string getCaseName();
    void report();
    void addCheck(bool passed);

  private:
    std::string caseName;
    int failed;
  };

  CaseMonitor case_monitor;

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
    case_monitor.addCheck(passed);
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
    std::cout << prefix << std::string(4, ' ') << std::boolalpha << "    " << val << std::endl;
  }

  // }}}
  // ----------------------------------------
  // TestMonitor definitions
  // ----------------------------------------
  // {{{

  int TestMonitor::overallyFailed = 0;
  std::list<TestMonitor> TestMonitor::testMonitors;

  void TestMonitor::addMonitor(const std::string& testName)
  {
    testMonitors.push_back(TestMonitor(testName));
    
    prefix += "    ";
  }

  void TestMonitor::removeMonitor()
  {
    testMonitors.pop_back();

    prefix = prefix.substr(0, prefix.length() - 4);
  }

  TestMonitor& TestMonitor::getMostRecent()
  {
    return testMonitors.back();
  }

  TestMonitor::TestMonitor(const std::string& testName): passed(0), failed(0), testName(testName)
  {
  }

  std::string TestMonitor::getTestName()
  {
    return testName;
  }

  void TestMonitor::report()
  {
    int tests = passed + failed;
    std::string prefix = tester::prefix.substr(0, tester::prefix.length() - 4);

    std::cerr << prefix << "Test group \"" << getTestName() << "\" ended" << std::endl;

    std::cerr << prefix << "Executed ";
    if (tests > 0)
      std::cerr << tests;
    else
      std::cerr << "no";
    std::cerr << " case" << (tests == 1 ? "":"s") << " in group.";
    if (tests > 0)
    {
      std:: cerr << "Passed: " <<
        std::fixed << std::setprecision(2) <<
        static_cast<double>(100*passed)/tests << "% ( " << passed << " / " <<
        tests << " )";
    }
    std::cerr << std::endl;

  }

  void TestMonitor::addCase(bool passed)
  {
    for (std::list<TestMonitor>::iterator it = testMonitors.begin(); it != testMonitors.end(); ++it)
    {
      it->passed += passed;
      it->failed += !passed;
    }
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

  void CaseMonitor::init(std::string caseName)
  {
    this->caseName = caseName;
    failed = 0;

    prefix += "    ";
  }

  std::string CaseMonitor::getCaseName()
  {
    return caseName;
  }

  void CaseMonitor::report()
  {
    prefix = prefix.substr(0, prefix.length() - 4);

    if (failed == 0)
      std::cerr << prefix << "Case \"" << getCaseName() << "\" passed" << std::endl;
    else
      std::cerr << prefix << "Case \"" << getCaseName() << "\" FAILED" << std::endl;

    TestMonitor::getMostRecent().addCase(failed == 0);
  }

  void CaseMonitor::addCheck(bool passed)
  {
    this->failed += !passed;
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

}

  // }}}
  // ----------------------------------------
  // Macros
  // ----------------------------------------
  // {{{

#define CHECK(expr) tester::Evaluer(#expr, __FILE__, __LINE__) << expr;

#define TEST_GROUP_BEGIN(name) std::cerr << tester::prefix << "Starting test group \"" << name << "\" ("  << __FILE__ << ":" << __LINE__ << ")" << std::endl;\
  tester::TestMonitor::addMonitor(name);

#define TEST_GROUP_END() tester::TestMonitor::getMostRecent().report();\
  tester::TestMonitor::removeMonitor();

#define TEST_CASE_BEGIN(name) std::cout << tester::prefix << "Test case \"" << name << "\" ("  << __FILE__ << ":" << __LINE__ << ")" << std::endl;\
  tester::case_monitor.init(name);

#define TEST_CASE_END() tester::case_monitor.report();

#define TEST_GROUP(fun) std::cout << tester::prefix << "Running test group \"" << #fun << "\" ("  << __FILE__ << ":" << __LINE__ << ")" << std::endl;\
  tester::TestMonitor::addMonitor(#fun);\
  fun();\
  tester::TestMonitor::getMostRecent().report();\
  tester::TestMonitor::removeMonitor();

#define TEST_CASE(fun) std::cout << tester::prefix << "Running test case \"" << #fun << "\" ("  << __FILE__ << ":" << __LINE__ << ")" << std::endl;\
  tester::case_monitor.init(#fun);\
  fun();\
  tester::case_monitor.report();

#define TEST_RESULT tester::TestMonitor::anyTestFailed();

#define START_TIMER(name) std::cout << "Starting timer \"" << name << "\" (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;\
  tester::timeTesters[name] = tester::TimeTester(name);\
  tester::timeTesters[name].start();

#define STOP_TIMER(name) tester::timeTesters[name].stop();\
  std::cout << "Stopping timer \"" << name << "\"" << std::endl;\

#define REPORT_TIMER(name) tester::timeTesters[name].report();

  // }}}

#endif

