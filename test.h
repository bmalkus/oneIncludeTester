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
#include <vector>

#define WIDTH TERM
#define DEFAULT_EPS 1e-8
#define DOUBLE_PRINT_PREC 9

#ifdef __clang__
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"
#endif

namespace tester
{
  template <typename T>
  class LeftValue;

  namespace checker
  {
    template <typename T>
      class CheckIf;

    template <typename T, bool streamable = CheckIf<T>::streamable, bool castable = CheckIf<T>::castable>
      struct Dummy;
  };

  std::string prefix;

#if WIDTH == TERM

#include <sys/ioctl.h>
#include <stdio.h>

  int _width_setter()
  {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return w.ws_col;
  }
  const int _WIDTH = _width_setter();

#else

  const int _WIDTH = WIDTH;

#endif

  const int _DOUBLE_PRECISION = DOUBLE_PRINT_PREC + 1;

  // ----------------------------------------
  // Evaluer class
  // ----------------------------------------
  // {{{

  struct AlmostEqualType {
    bool res;
    long double d1, d2, eps;
  };

  class Evaluer
  {
  public:
    Evaluer(std::string expr, std::string filename, int line_no):
      expr(expr), filename(filename), line_no(line_no) { }

    template <typename T>
      LeftValue<T> operator<< (T left_val);

    std::string get_expr() { return expr; }
    std::string get_fname() { return filename; }
    int get_line_no() { return line_no; }

  private:
    std::string expr, filename;
    int line_no;

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
      LeftValue(U& left_value, Evaluer& evaluer): left_value(left_value), evaluer(evaluer) { }

      template <typename V>
        void operator== (V right_value);

      template <typename V>
        void operator!= (V right_value);

      template <typename V>
        void operator<= (V right_value);

      template <typename V>
        void operator>= (V right_value);

      template <typename V>
        void operator< (V right_value);

      template <typename V>
        void operator> (V right_value);

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator&& (V right_value);

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator|| (V right_value);

      template <typename V>
        void assert(bool val, std::string op, V right_value);

    private:
      U left_value;
      Evaluer& evaluer;
    };

  template<>
    class LeftValue<bool>
    {
    public:
      LeftValue(bool left_value, Evaluer& evaluer_): evaluer(evaluer_) { assert(left_value); }

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator&& (V right_value);

      template <typename V>
        PUT_COMPLEX_LOGICAL_EXPRESSIONS_IN_PARENTHESIS operator|| (V right_value);

      void assert(bool val);

    private:
      Evaluer& evaluer;
    };

  template<>
    class LeftValue<AlmostEqualType>
    {
    public:
      LeftValue(AlmostEqualType left_value, Evaluer& evaluer_): evaluer(evaluer_) { assert(left_value); }

      void assert(AlmostEqualType val);

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
    static void register_test_case(TestCase *ptc);

    static bool any_test_failed();
    static void test_case_result(bool passed);
    static void run_rests();

  private:
    static int overally_failed;
    static int overally_run;
    static std::vector<TestCase*> test_cases;
  };

  // }}}
  // ----------------------------------------
  // TestCase class
  // ----------------------------------------
  // {{{

  class TestCase
  {
  public:
    static TestCase *get_current() { return current; }

    bool run();
    void add_check(bool passed) { this->failed += !passed; this->passed += passed; }
    bool add_subcase();

  protected:
    TestCase(std::string name, std::string file, int line);

  private:
    static TestCase *current;

    std::string name, file;
    int line;
    int failed, passed;
    int subcases;
    int subcases_done;
    int rerun;

    virtual void _run() = 0;
  };

  TestCase* TestCase::current = nullptr;

  bool TestCase::run()
  {
    std::cout << prefix << name << " - case starting";
    std::cout << std::endl;
    prefix += "    ";

    current = this;

    rerun = true;
    while (rerun)
    {
      rerun = false;
      subcases = 0;
      this->_run();
      subcases_done += 1;
    }

    prefix = prefix.substr(0, prefix.length() - 4);
    std::ostringstream pref, suff;
    pref << prefix << name << "  ";

    int tests = passed + failed;

    if (failed == 0)
      suff << "  " << int(double(100*passed)/tests) << "% ( " << passed << " / "
        << tests << " ) - passed";
    else
    {
      suff << "  " << int(double(100*passed)/tests) << "% ( " << passed << " / "
        << tests << " ) - FAILED";
    }
    std::cerr << pref.str() << std::string(std::max(_WIDTH - signed(pref.str().length()) - signed(suff.str().length()), 3), '.') << suff.str() << std::endl;

    current = nullptr;

    return failed == 0;
  }

  bool TestCase::add_subcase()
  {
    bool ret = false;
    if (subcases_done == subcases)
      ret = true;
    else if (subcases_done < subcases)
      rerun = true;
    subcases += 1;
    return ret;
  }

  TestCase::TestCase(std::string name, std::string file, int line):
    name(name), file(file), line(line), failed(0), passed(0), subcases(0), subcases_done(0)
  {
    // empty
  }

  // }}}
  // ----------------------------------------
  // TestSubcase class
  // ----------------------------------------
  // {{{

  class TestSubcase
  {
  public:
    static TestSubcase *get_current() { return current; }

    bool run();
    operator bool() { return should_run; }
    void add_check(bool passed) { this->failed += !passed; }

    TestSubcase(std::string name, std::string file, int line);
    ~TestSubcase();

  private:
    static TestSubcase *current;

    std::string name, file;
    int line;
    int failed;
    bool should_run;
  };

  TestSubcase* TestSubcase::current = nullptr;

  TestSubcase::TestSubcase(std::string name, std::string file, int line):
    name(name), file(file), line(line), failed(0)
  {
    should_run = TestCase::get_current()->add_subcase();
    current = this;
    if (should_run && name.length() > 0)
    {
      std::cout << prefix << name << " - subcase";
      std::cout << std::endl;
      prefix += "    ";
    }
  }

  TestSubcase::~TestSubcase()
  {
    if (should_run && name.length() > 0)
    {
      prefix = prefix.substr(0, prefix.length() - 4);
      std::ostringstream pref, suff;
      pref << prefix << name << "  ";

      if (failed == 0)
        suff << "  subcase passed";
      else
      {
        suff << "  subcase FAILED";
      }
      std::cerr << pref.str() << std::string(std::max(_WIDTH - signed(pref.str().length()) - signed(suff.str().length()), 3), '.') << suff.str() << std::endl;
    }
    current = nullptr;
  }

  // }}}
  // ----------------------------------------
  // Evaluer definitions
  // ----------------------------------------
  // {{{

  template <typename T>
    LeftValue<T> Evaluer::operator<< (T left_value)
    {
      return LeftValue<T>(left_value, *this);
    }

  template <>
    LeftValue<AlmostEqualType> Evaluer::operator<< (AlmostEqualType result)
    {
      return LeftValue<AlmostEqualType>(result, *this);
    }


  // }}}
  // ----------------------------------------
  // LeftValue definitions
  // ----------------------------------------
  // {{{

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator> (V right_value)
    {
      assert(left_value > right_value, ">", right_value);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator< (V right_value)
    {
      assert(left_value < right_value, "<", right_value);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator>= (V right_value)
    {
      assert(left_value >= right_value, ">=", right_value);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator<= (V right_value)
    {
      assert(left_value <= right_value, "<=", right_value);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator!= (V right_value)
    {
      assert(left_value != right_value, "!=", right_value);
    }

  template <typename U>
  template <typename V>
    void LeftValue<U>::operator== (V right_value)
    {
      assert(left_value == right_value, "==", right_value);
    }

  void assert_common_part(std::ostream &out, bool passed, Evaluer &evaluer)
  {
    TestCase::get_current()->add_check(passed);
    if (auto *subcase = TestSubcase::get_current())
      subcase->add_check(passed);

    std::ostringstream pref, suff;
    if (!passed)
      out << prefix << "at " << evaluer.get_fname() << ":" << evaluer.get_line_no() << ":" << std::endl;
    pref << prefix << "CHECK(" << evaluer.get_expr()  << ")  ";
    suff << (passed ? "  passed" : "  FAILED");
    int fillLen = std::max(_WIDTH - signed(pref.str().length()) - signed(suff.str().length()), 3);
    out << pref.str() << std::string(std::max(fillLen, 0), '.') << suff.str() << std::endl;
    out << prefix << "    / ";
  }

  template <typename U>
  template <typename V>
    void LeftValue<U>::assert (bool val, std::string op, V right_value)
    {
      int prec = std::cout.precision();
      std::cout.precision(_DOUBLE_PRECISION);

      std::ostream& out = val ? std::cout : std::cerr;
      assert_common_part(out, val, evaluer);

      std::string left_repr = checker::Dummy<U>::repr(left_value);
      std::string right_repr = checker::Dummy<V>::repr(right_value);

      size_t posl = left_repr.find("\n");
      size_t posr = right_repr.find("\n");
      if (posl != std::string::npos || posr != std::string::npos)
      {
        std::string repl = "\n" + prefix + "    / ";

        if (posl != std::string::npos)
          do
            left_repr.replace(posl, 1, repl);
          while ((posl = left_repr.find("\n", posl + 1)) != std::string::npos && posl != left_repr.length() - 1);

        if (posr != std::string::npos)
          do
            right_repr.replace(posr, 1, repl);
          while ((posr = left_repr.find("\n", posr + 1)) != std::string::npos && posr != right_repr.length() - 1);

        out << left_repr;
        if (posl == std::string::npos)
          out << "\n";
        out << prefix << "       " << op << repl;
        out << right_repr;
        if (posr == std::string::npos)
          out << "\n";
      }
      else
      {
        out << left_repr << " " << op << " " << right_repr << " /" << std::endl;
      }
      std::cout.precision(prec);
    }

  void LeftValue<bool>::assert (bool val)
  {
    int prec = std::cout.precision();
    std::cout.precision(_DOUBLE_PRECISION);

    std::ostream& out = val ? std::cout : std::cerr;
    assert_common_part(out, val, evaluer);
    out << std::boolalpha << val << " /" << std::endl;

    std::cout.precision(prec);
  }

  void LeftValue<AlmostEqualType>::assert (AlmostEqualType res)
  {
    int prec = std::cout.precision();
    std::cout.precision(_DOUBLE_PRECISION);

    bool val = res.res;
    std::ostream& out = val ? std::cout : std::cerr;
    assert_common_part(out, val, evaluer);
    out << res.d1 << " ~= " << res.d2 << " / ( +/- " << DEFAULT_EPS << " ) -> " << std::boolalpha << val << std::endl;

    std::cout.precision(prec);
  }

  // }}}
  // ----------------------------------------
  // TestMonitor definitions
  // ----------------------------------------
  // {{{

  int TestMonitor::overally_failed = 0;
  int TestMonitor::overally_run = 0;
  std::vector<TestCase*> TestMonitor::test_cases;

  void TestMonitor::register_test_case(TestCase *ptc)
  {
    test_cases.push_back(ptc);
  }

  void TestMonitor::run_rests()
  {
    if (test_cases.empty())
    {
      std::cerr << "No cases to run" << std::endl;
      return;
    }
    for (auto tcIt = test_cases.begin(); tcIt != test_cases.end(); ++tcIt)
    {
      bool passed = (*tcIt)->run();
      TestMonitor::test_case_result(passed);
    }

    std::cerr << std::string(std::max(_WIDTH, 0), '_') << std::endl;
    std::ostringstream suff;
    suff << "passed: "
      << int(double(100*(overally_run - overally_failed))/overally_run)
      << "% ( " << (overally_run - overally_failed) << " / " << overally_run << " )";
    std::cerr << std::string(std::max(_WIDTH - signed(suff.str().length()), 0), ' ') << suff.str() << std::endl;
  }

  void TestMonitor::test_case_result(bool passed)
  {
    overally_failed += !passed;
    ++overally_run;
  }

  bool TestMonitor::any_test_failed()
  {
    return overally_failed;
  }

  // }}}
  // ----------------------------------------
  // Stream and cast operator existence checker
  // ----------------------------------------
  // {{{

  namespace checker
  {
    struct DummyType {};

    template <typename T>
      DummyType operator<< (std::ostream& out, const T& c);

    template <typename T>
      class CheckIf
      {
        typedef char one;
        typedef long two;

        static std::ostream &out;
        static T &t;

        template <typename C>
          static one check_streamable(std::ostream&);
        template <typename C>
          static two check_streamable(DummyType);

        template <typename A, std::string (A::*)()>
          struct check_type;
        template <typename C>
          static one check_castable(check_type<C, &C::operator std::string>*);
        template <typename C>
          static two check_castable(...);

      public:
        static const bool streamable = sizeof(check_streamable<T>(out << t)) == sizeof(one);
        static const bool castable = sizeof(check_castable<T>(nullptr)) == sizeof(one);
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
          ss << t;
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
    void pretty_report();
    void simple_report();
    static void simple_report_all_timers();

  private:
    timespec start_time, stop_time, diff;
    std::string name;

  };

  std::map<std::string, TimeTester> time_resters;

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

  void TimeTester::pretty_report()
  {
    std::ostringstream pref, suff;
    pref << prefix << "Timer ";
    if (name != "")
    {
      pref << "\"" << name << "\"";
    }
    pref << " result" << "  ";

    suff << "  " << diff.tv_sec << "s ";
    long res = diff.tv_nsec;
    int nsec = res % 1000;
    res /= 1000;
    int usec = res % 1000;
    res /= 1000;
    int msec = res % 1000;

    suff << msec << "ms ";
    suff << usec << "us ";
    suff << nsec << "ns";
    suff << " / ";
    suff << diff.tv_sec << "."
      << std::setw(3) << std::setfill('0') << msec
      << std::setw(3) << std::setfill('0') << usec
      << std::setw(3) << std::setfill('0') << nsec;
    suff << "s";
    suff << " /";
    std::cout.fill(' ');

    int fillLen = std::max(_WIDTH - signed(pref.str().length()) - signed(suff.str().length()), 3);
    std::cerr << pref.str() << std::string(std::max(fillLen, 0), '.') << suff.str() << std::endl;
  }

  void TimeTester::simple_report()
  {
    std::cerr << name << "    ";
    long res = diff.tv_nsec;
    int nsec = res % 1000;
    res /= 1000;
    int usec = res % 1000;
    res /= 1000;
    int msec = res % 1000;
    std::cerr << diff.tv_sec << "."
      << std::setw(3) << std::setfill('0') << msec
      << std::setw(3) << std::setfill('0') << usec
      << std::setw(3) << std::setfill('0') << nsec;
    std::cerr << std::endl;
  }

  void TimeTester::simple_report_all_timers()
  {
    for (auto tester : time_resters)
    {
      tester.second.simple_report();
    }
  }

  // }}}
  // ----------------------------------------
  // Utils
  // ----------------------------------------
  // {{{

  AlmostEqualType almost_equal(long double d1, long double d2, long double eps=DEFAULT_EPS)
  {
    AlmostEqualType res;
    res.d1 = d1;
    res.d2 = d2;
    res.eps = eps;
    res.res = std::abs(d1 - d2) < eps;
    return res;
  }

}

  // }}}
  // ----------------------------------------
  // Macros
  // ----------------------------------------
  // {{{

#define CHECK(expr) tester::Evaluer(#expr, __FILE__, __LINE__) << expr;

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define TEST_SUBCASE(name) \
  if(tester::TestSubcase CONCAT(__test_group_, __LINE__) = tester::TestSubcase(name"", __FILE__, __LINE__))

#define TEST_CASE(name) class CONCAT(__test_case_, __LINE__) : tester::TestCase \
    { \
    public: \
      CONCAT(__test_case_, __LINE__)(std::string tc_name, std::string file, int line): TestCase(tc_name, file, line) { tester::TestMonitor::register_test_case(this); } \
    private: \
      void _run(); \
    }; \
 \
CONCAT(__test_case_, __LINE__) CONCAT(_tc_, __LINE__)(name, __FILE__, __LINE__); \
 \
void CONCAT(__test_case_, __LINE__)::_run()

#define PRINT(str) std::cout << tester::prefix << "#### " << str << " ####" << std::endl;

#define TEST_RESULT tester::TestMonitor::any_test_failed();

#define START_TIMER(name) std::cout << tester::prefix << "Starting timer \"" << name << "\" (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;\
  tester::time_resters[name] = tester::TimeTester(name);\
  tester::time_resters[name].start();

#define STOP_TIMER(name) tester::time_resters[name].stop();\
  std::cout << tester::prefix << "Stopping timer \"" << name << "\"" << std::endl;\

#define PRETTY_REPORT_TIMER(name) tester::time_resters[name].pretty_report();

#define SIMPLE_REPORT_TIMER(name) tester::time_resters[name].simple_report();

#define SIMPLE_REPORT_ALL_TIMERS() tester::TimeTester::simple_report_all_timers();

#define MAIN_RUN_ALL_TESTS() int main() \
{ \
  tester::TestMonitor::run_rests(); \
  return TEST_RESULT; \
}

#define DBG(str) std::cout << "#DBG /" << __FILE__ << ":" << __LINE__ << "/ " << #str << " = " << str << std::endl;

#define DBG_ALL(coll) std::cout << "#DBG /" << __FILE__ << ":" << __LINE__ << "/ " << #coll << " = {" << std::endl; \
std::cout << "    "; \
for (auto elem : coll) \
{ \
  std::cout << elem << "   ";\
} \
std::cout << std::endl << "}" << std::endl;

#define DBG_ALL_PTR(coll) std::cout << "#DBG /" << __FILE__ << ":" << __LINE__ << "/ " << #coll << " = {" << std::endl; \
std::cout << "    "; \
for (auto elem : coll) \
{ \
  std::cout << *elem << "   ";\
} \
std::cout << std::endl << "}" << std::endl;

#define SILENT(macro) { \
  std::cout.setstate(std::ios_base::failbit); \
  std::cerr.setstate(std::ios_base::failbit); \
  macro; \
  std::cout.clear(); \
  std::cerr.clear(); \
}

#ifdef AFFINITY_INCLUDE

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>

#define SET_AFFINITY(CPU) cpu_set_t set; \
  CPU_ZERO(&set); \
  CPU_SET(CPU, &set); \
  sched_setaffinity(0, sizeof(set), &set);

#endif

#undef WIDTH
#undef DEFAULT_EPS
#undef DOUBLE_PRINT_PREC

  // }}}

#endif

