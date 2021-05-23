#ifndef DQNET_MEMORY_MANAGER_H_
#define DQNET_MEMORY_MANAGER_H_

class MemoryManager {
public:
  MemoryManager() {};
  ~MemoryManager() {};

  template <typename T, int X>
  void memcpy(const T *x, T (&y)[X]) {
#pragma HLS INLINE
    Memcpy:
    for (int i = 0; i < X; ++i) {
#pragma HLS PIPELINE II=1
      y[i] = x[i];
    }
  }

  template <typename T, int X, int Y>
  void memcpy(const T *x, T (&y)[X][Y]) {
#pragma HLS INLINE
    Memcpy:
    for (int i = 0; i < X; ++i) {
      for (int j = 0; j < Y; ++j) {
#pragma HLS LOOP_FLATTEN
#pragma HLS PIPELINE II=1
        y[i][j] = x[i * Y + j];
      }
    }
  }

  template <typename T, int X, int Y, int Z>
  void memcpy(const T *x, T (&y)[X][Y][Z]) {
#pragma HLS INLINE
    Memcpy:
    for (int i = 0; i < X; ++i) {
      for (int j = 0; j < Y; ++j) {
        for (int k = 0; k < Z; ++k) {
#pragma HLS LOOP_FLATTEN
#pragma HLS PIPELINE II=1
          y[i][j][k] = x[i * Y * Z + j * Z + k];
        }
      }
    }
  }

  template <typename T, int X, int Y, int Z, int K>
  void memcpy(const T *x, T (&y)[X][Y][Z][K]) {
#pragma HLS INLINE
    Memcpy:
    for (int i = 0; i < X; ++i) {
      for (int j = 0; j < Y; ++j) {
        for (int k = 0; k < Z; ++k) {
          for (int ii = 0; ii < K; ++ii) {
#pragma HLS LOOP_FLATTEN
#pragma HLS PIPELINE II=1
            y[i][j][k][ii] = x[i * Y * Z * K + j * Z * K + k * K + ii];
          }
        }
      }
    }
  }
};

#endif // end #define DQNET_MEMORY_MANAGER_H_