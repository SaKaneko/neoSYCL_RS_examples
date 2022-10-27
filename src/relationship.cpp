#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;
const int itr = 5;

int main() {
  const int N  = 3;
  using Vector = float[N];

  Vector a   = {2, 2, 2};
  Vector b   = {1, 2, 3};
  float c[N] = {0, 0, 0};
  // c = ((a*b + a)*b + a)*b...
  Vector ans = {0, 0, 0};
  for (int i = 0; i < itr; i++) {
    for (int j = 0; j < N; j++) {
      ans[j] += a[j];
    }
    for (int j = 0; j < N; j++) {
      ans[j] *= b[j];
    }
  }

  std::cout << "answer:" << std::endl
            << ans[0] << " " << ans[1] << " " << ans[2] << std::endl;

  {
    // Create a queue to work on
    queue myQueue(host_selector{});

    buffer<float> A(a, range<1>(N));
    buffer<float> B(b, range<1>(N));

    buffer<float> C(c, N);

    for (int i = 0; i < itr; i++) {
      myQueue.submit([&](handler& cgh) {
        auto ka = A.get_access<access::mode::read>(cgh);
        auto kc = C.get_access<access::mode::read_write>(cgh);

        cgh.single_task<class sequential_vector_add>([=]() {
          for (size_t i = 0; i != N; i++) {
            kc[i] = kc[i] + ka[i];
          }
        });
      });

      myQueue.submit([&](handler& cgh) {
        auto kb = B.get_access<access::mode::read>(cgh);
        auto kc = C.get_access<access::mode::read_write>(cgh);

        cgh.single_task<class sequential_vector_multi>([=]() {
          for (size_t i = 0; i != N; i++) {
            kc[i] = kc[i] * kb[i];
          }
        });
      });
    }
    myQueue.wait();
  }

  std::cout << "Result:" << std::endl;
  for (size_t i = 0; i != N; i++)
    std::cout << c[i] << " ";
  std::cout << std::endl;
  bool flag = true;

  // Validation
  for (size_t i = 0; i < N; i++) {
    if (c[i] != ans[i])
      flag = false;
  }
  if (flag) {
    std::cout << "\033[33mPASSED! \033[m" << std::endl;
  }
  else {
    std::cout << "\033[31mFAILED! \033[m" << std::endl;
  }
}
