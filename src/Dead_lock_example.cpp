#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;
using namespace std;
const int heavy = 100000;

int main() {
  const int N  = 3;
  using Vector = float[N];

  Vector a = {1, 2, 3};
  Vector b = {5, 6, 8};

  float c[N];

  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue(host_selector{});

    // Create buffers from a & b vectors with 2 different syntax
    buffer<float> A(a, range<1>(N));
    buffer<float> B(b, range<1>(N));

    // A buffer of N float using the storage of c
    buffer<float> C(c, N);

    /* The command group describing all operations needed for the kernel
       execution */
    myQueue.submit([&](handler& cgh) {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::mode::read_write>(cgh);

      cgh.single_task<class Large_wait>([=]() {
        // std::cout << "start_heavy_task" << std::endl;
        for (int i = 0; i < heavy; i++) {
          ka[0] = ka[0];
        }
        // std::cout << "end_heavy_task" << std::endl;
      });
    }); // End of our commands for this queue

    myQueue.submit([&](handler& cgh) {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::mode::write>(cgh);
      auto kb = B.get_access<access::mode::write>(cgh);
      // Enqueue a single, simple task
      cgh.single_task<class A_B>([=]() {
        // std::cout << "A_B task" << std::endl;
      });
    }); // End of our commands for this queue
    myQueue.submit([&](handler& cgh) {
      // In the kernel A and B are read, but C is written
      auto kb = B.get_access<access::mode::write>(cgh);
      auto ka = A.get_access<access::mode::write>(cgh);
      // Enqueue a single, simple task
      cgh.single_task<class B_A>([=]() {
        // std::cout << "B_A task" << std::endl;
      });
    }); // End of our commands for this queue

    myQueue.wait();
  } // End scope, so we wait for the queue to complete

  std::cout << "Result:" << std::endl;
  for (size_t i = 0; i != N; i++)
    std::cout << c[i] << " ";
  std::cout << std::endl;
}
