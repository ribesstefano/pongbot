#include "axis_lib.h"
#include <vector>
#include <cstdlib>

void hls_fifo(const int size, ap_ufifo<32> &x, ap_ufifo<32> &y) {
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE s_axilite port=size bundle=ctrl
#pragma HLS INTERFACE axis port=x
#pragma HLS INTERFACE axis port=y
  AxiStreamInterface<32> x_axi(x);
  AxiStreamInterface<32> y_axi(y);

  x_axi.PopToStream(size, y);
}

void hls_adder(const int size, ap_ufifo<32> &x, ap_ufifo<32> &y) {
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE s_axilite port=size bundle=ctrl
#pragma HLS INTERFACE axis port=x
#pragma HLS INTERFACE axis port=y
  AxiStreamInterface<32> x_axi(x);
  AxiStreamInterface<32> y_axi(y);

  for (int i = 0; i < size; ++i) {
#pragma HLS PIPELINE II=1
    float x_val = x_axi.Pop<float>();
    x_val += 1;
    if (i == size - 1) {
      y_axi.PushLast(x_val);
    } else {
      y_axi.Push(x_val);
    }
  }
}

int test_fifo() {
  const int size = 128;
  std::vector<ap_uint<32> > x_gold;
  ap_ufifo<32> x, y;

  for (int i = 0; i < size; ++i) {
    axiu_packet<32> packet;
    auto x_val = ap_uint<32>(rand());
    x_gold.push_back(x_val);
    packet.data = x_val;
    if (i == size - 1) {
      packet.last = 1;
    }
    x.write(packet);
  }

  hls_fifo(size, x, y);

  int num_errors = 0;
  for (int i = 0; i < size; ++i) {
    axiu_packet<32> packet = y.read();
    // std::cout << "read/gold: " << packet.data << "/" << x_gold[i] << "\n";
    if (packet.data != x_gold[i]) {
      num_errors++;
    }
  }
  std::cout << "There were " << num_errors << " errors over " << size << " elements.\n";
  return num_errors;
}

int test_adder() {
  const int size = 128;
  std::vector<float> x_gold;
  ap_ufifo<32> x, y;

  for (int i = 0; i < size; ++i) {
    axiu_packet<32> packet;
    auto x_val = ap_uint<32>(rand());
    packet.data = x_val;
    if (i == size - 1) {
      packet.last = 1;
    }
    x.write(packet);
    // Determine gold result
    float x_float = *((float*)&x_val);
    x_gold.push_back(x_float + 1);
  }

  hls_adder(size, x, y);

  int num_errors = 0;
  for (int i = 0; i < size; ++i) {
    axiu_packet<32> packet = y.read();
    auto x_bit = packet.data;
    float x_read = *((float*)&x_bit);
    if (x_read != x_gold[i]) {
      std::cout << "read/gold: " << x_read << "/" << x_gold[i] << "\n";
      num_errors++;
    }
  }
  std::cout << "[INFO] There were " << num_errors << " errors over " << size << " elements.\n";
  return num_errors;
}

// int main(int argc, char const *argv[]) {

//   // int num_errors = test_fifo();
//   int num_errors = test_adder();
//   return num_errors;
// }