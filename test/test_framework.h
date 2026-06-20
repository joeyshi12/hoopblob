#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

// Minimal dependency-free test framework. No external libraries so the suite
// builds anywhere with just a C++ compiler.

#include <cstdio>
#include <vector>
#include <string>

struct TestCase
{
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& test_registry()
{
    static std::vector<TestCase> registry;
    return registry;
}

inline int& failure_count() { static int n = 0; return n; }
inline int& check_count()   { static int n = 0; return n; }

struct TestRegistrar
{
    TestRegistrar(const char* name, void (*fn)())
    {
        test_registry().push_back({name, fn});
    }
};

#define TEST(name)                                                       \
    static void name();                                                  \
    static TestRegistrar reg_##name(#name, name);                        \
    static void name()

#define CHECK(cond)                                                      \
    do {                                                                 \
        ++check_count();                                                 \
        if(!(cond)) {                                                    \
            ++failure_count();                                           \
            std::printf("  FAIL %s:%d: CHECK(%s)\n",                     \
                        __FILE__, __LINE__, #cond);                      \
        }                                                                \
    } while(0)

#define CHECK_NEAR(actual, expected, eps)                                \
    do {                                                                 \
        ++check_count();                                                 \
        double _a = (double)(actual);                                    \
        double _e = (double)(expected);                                  \
        double _d = _a - _e; if(_d < 0) _d = -_d;                        \
        if(_d > (eps)) {                                                 \
            ++failure_count();                                           \
            std::printf("  FAIL %s:%d: %s ~= %s (got %.5f, want %.5f)\n",\
                        __FILE__, __LINE__, #actual, #expected, _a, _e); \
        }                                                                \
    } while(0)

inline int run_all_tests()
{
    int failed_cases = 0;
    for(const TestCase& tc : test_registry())
    {
        int before = failure_count();
        std::printf("RUN  %s\n", tc.name);
        tc.fn();
        if(failure_count() == before)
        {
            std::printf("PASS %s\n", tc.name);
        }
        else
        {
            std::printf("FAIL %s\n", tc.name);
            ++failed_cases;
        }
    }

    std::printf("\n%d checks, %d failures across %zu tests\n",
                check_count(), failure_count(), test_registry().size());

    if(failed_cases == 0)
    {
        std::printf("ALL TESTS PASSED\n");
        return 0;
    }
    std::printf("%d TEST(S) FAILED\n", failed_cases);
    return 1;
}

#endif
