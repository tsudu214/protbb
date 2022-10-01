/*********************************************************
**********************************************************
*Presentation of flow_graph_opencl_node and the Heterogeneous capabilities in FlowGraph

-Build instructions:

Linux:   g++ Hello_OCLNode.cpp -o  -ltbb -std=c++11 -lOpenCL -O3
MacOS:   g++-6 Hello_OCLNode.cpp -o example -ltbb -std=c++11 -framework OpenCL -w -O3

-Execute instructions:
./example
***************************************************************/
#define TBB_PREVIEW_FLOW_GRAPH_NODES 1
#define TBB_PREVIEW_FLOW_GRAPH_FEATURES 1

#include <cstdio>
#include "tbb/flow_graph.h"
#include "tbb/flow_graph_opencl_node.h"
#include "tbb/tick_count.h"

using namespace tbb;
using namespace tbb::flow;
using namespace std;

const int NUM_RAND = 256;
int RandomNumber () { return (std::rand()%NUM_RAND); }
tbb::tick_count t0_p;

class gpu_device_selector1 {
public:
  template<typename DeviceFilter> opencl_device operator()(opencl_factory<DeviceFilter> &f) {
    return *(++f.devices().cbegin());
  }
};

class gpu_device_selector2 {
public:
  template<typename DeviceFilter> opencl_device operator()(opencl_factory<DeviceFilter> &f) {
    //tbb::flow::opencl_device_list::const_iterator it = std::find_if(
    auto it = std::find_if(
      f.devices().cbegin(), f.devices().cend(),
      [](const tbb::flow::opencl_device &d) {
        if(  d.type() == CL_DEVICE_TYPE_GPU) {
          std::cout << "Found GPU!" << std::endl;
          return true;
        }
        return false;
      });

      if (it == f.devices().cend()) {
        std::cout << "Info: could not find any GPU devices. Choosing the first available device (default behaviour)." << std::endl;
        return {*f.devices().cbegin()};
      }
//      it++;
      std::cout << "Running OpenCL code on "<< (*it).name() << std::endl;
      return {*it};
    }
  };

int main(int argc, const char* argv[]) {

  int rows = 4;
  int cols = 4;
  int vsize = rows * cols;

  tbb::flow::graph g;

  using buffer_f = tbb::flow::opencl_buffer<cl_float>;
  buffer_f Adevice(vsize);
  buffer_f Bdevice(vsize);
  buffer_f Cdevice(vsize);
  float* Ahost=Adevice.data();
  std::generate(Ahost, Ahost+vsize, RandomNumber);
  std::generate(Bdevice.begin(), Bdevice.end(), RandomNumber);

  //GPU node 1:
  tbb::flow::opencl_program<> program(std::string("twogpus.cl"));
  using tuple_gpu = std::tuple<buffer_f>;

  opencl_node<tuple_gpu> gpu_node1(g, program.get_kernel("cl_add"), [](auto &f){
    auto d=*(f.devices().begin() + 1);
    std::cout << "Running gpu_node1 on " << d.name()<< '\n';
    return d;
  });
  gpu_node1.set_range({{rows,cols}});
  gpu_node1.set_args(Adevice,Bdevice,port_ref<0>);

  //GPU node 2:
  opencl_node<tuple_gpu> gpu_node2(g, program.get_kernel("cl_sub"), [](auto &f){
    auto d=*(f.devices().begin() + 2);
    std::cout << "Running gpu_node2 on " << d.name()<< '\n';
    return d;
  });
  gpu_node2.set_range({{rows,cols}});
  gpu_node2.set_args(Adevice,Bdevice,port_ref<0>);

  make_edge(output_port<0>(gpu_node1),input_port<0>(gpu_node2));
  tbb::flow::input_port<0>(gpu_node1).try_put(Cdevice);

  g.wait_for_all();
  float* Chost=Cdevice.data();
  if (! std::equal (Chost,Chost+vsize,Ahost))
    std::cout << "Errors in the heterogeneous computation.\n";
}
