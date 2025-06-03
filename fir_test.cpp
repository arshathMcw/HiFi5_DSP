#include <iostream>
#include <vector>

struct FilterInput {
  const float* x;  // input signal with (N_h-1) zeros appended
  size_t inputLength;   // N_x
  const float* c;  // reversed filter coefficients
  size_t filterLength;  // N_h
  float* y;  // output (filtered) signal
  size_t outputLength; // should be N_x
};

float* applyFirFilterSingle(FilterInput& input) {
  const auto* x = input.x;
  const auto* c = input.c;
  auto* y = input.y;

  for (auto i = 0u; i < input.outputLength; ++i) {
    y[i] = 0.f;
    for (auto j = 0u; j < input.filterLength; ++j) {
      y[i] += x[i + j] * c[j];
    }
  }
  return y;
}

int main() {
  std::vector<float> inputSignal = {1, 2, 3, 4, 5};
  std::vector<float> filterCoeffs = {1.0f/3, 1.0f/3, 1.0f/3};
  size_t N_x = inputSignal.size();
  size_t N_h = filterCoeffs.size();
  std::vector<float> paddedInput(inputSignal);
  paddedInput.resize(N_x + N_h - 1, 0.0f);  
  std::vector<float> output(N_x, 0.0f);
  FilterInput input = {
    .x = paddedInput.data(),
    .inputLength = N_x,
    .c = filterCoeffs.data(),
    .filterLength = N_h,
    .y = output.data(),
    .outputLength = N_x
  };
  applyFirFilterSingle(input);
  std::cout << "Filtered output:\n";
  for (size_t i = 0; i < N_x; ++i) {
    std::cout << output[i] << " ";
  }
  std::cout << std::endl;
  return 0;
}
