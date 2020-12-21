// Benchmark class from AISDI classes
#include <chrono>

template <typename D = std::chrono::microseconds> class Benchmark {
public:
  Benchmark(bool printOnExit = false) : m_print(printOnExit) {
    start = std::chrono::high_resolution_clock::now();
  }
  typename D::rep elapsed() const {
    auto end = std::chrono::high_resolution_clock::now();
    auto result = std::chrono::duration_cast<D>(end - start);
    return result.count();
  }
  ~Benchmark() { auto result = elapsed(); }

private:
  std::chrono::high_resolution_clock::time_point start;
  bool m_print = true;
};