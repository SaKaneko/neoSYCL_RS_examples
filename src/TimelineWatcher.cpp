#include <CL/sycl.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <unistd.h>
using namespace cl::sycl;
const int weight = 100000000;

int main() {
  const int N  = 3;
  using Vector = float[N];

  Vector a;
  Vector b;

  float c[N];
  { // By sticking all the SYCL work in a {} block, we ensure
    auto start = std::chrono::system_clock::now();
    queue myQueue(host_selector{}), myQueue2(host_selector{});
    buffer<float> A(a, range<1>(N));
    buffer<float> B(b, range<1>(N));
    buffer<float> C(c, N);
    myQueue.submit([&](handler& cgh) {
      auto ka = A.get_access<access::mode::read>(cgh);
      auto kb = B.get_access<access::mode::write>(cgh);

      cgh.single_task<class Task_ab>([=]() {
        auto tstart = std::chrono::system_clock::now();
        for (size_t i = 0; i != weight; i++) {
          for (size_t j = 0; j != N; j++)
            kb[j] = ka[j];
        }
        // std::this_thread::sleep_for(std::chrono::seconds(3));
        auto tend = std::chrono::system_clock::now();
        kb[0] = std::chrono::duration_cast<std::chrono::milliseconds>(tstart -
                                                                      start)
                    .count();
        kb[1] =
            std::chrono::duration_cast<std::chrono::milliseconds>(tend - start)
                .count();
      });
    }); // End of our commands for this queue
    auto blank = std::chrono::system_clock::now();
    myQueue2.submit([&](handler& cgh) {
      auto ka = A.get_access<access::mode::read>(cgh);
      auto kc = C.get_access<access::mode::write>(cgh);

      cgh.single_task<class Task_bc>([=]() {
        auto tstart = std::chrono::system_clock::now();
        for (size_t j = 0; j != weight; j++) {
          for (size_t i = 0; i != N; i++) {
            kc[i] = 0; // ka[i];
          }
        }
        auto tend = std::chrono::system_clock::now();
        kc[0] = std::chrono::duration_cast<std::chrono::milliseconds>(tstart -
                                                                      start)
                    .count();
        kc[1] =
            std::chrono::duration_cast<std::chrono::milliseconds>(tend - start)
                .count();
      });
    }); // End of our commands for this queue
    myQueue.wait();
    myQueue2.wait();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(blank -
                                                                       start)
                     .count()
              << std::endl;
  } // End scope, so we wait for the queue to complete

  std::cout << "Result:" << std::endl;
  std::cout << "First" << std::endl;
  std::cout << b[0] << "-" << b[1] << std::endl;
  std::cout << "Second" << std::endl;
  std::cout << c[0] << "-" << c[1] << std::endl;
}
