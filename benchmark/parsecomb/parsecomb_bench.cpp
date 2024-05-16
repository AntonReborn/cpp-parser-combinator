#include <benchmark/benchmark.h>

#include "pc/parsecomb/parsecomb.h"

static void BM_SomeFunction(benchmark::State& state) {
    for (auto _ : state) {

    }
}

BENCHMARK(BM_SomeFunction);
BENCHMARK_MAIN();