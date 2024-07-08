#include <dpu>
#include <iostream>

using namespace dpu;

int main(void) {

  try {
    auto dpu = DpuSet::allocate(1);
    dpu.load("helloworld_bin");
    dpu.exec();
    dpu.log(std::cout);
  }
  catch (const DpuError & e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}