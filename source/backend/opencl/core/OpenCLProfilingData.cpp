
#include <algorithm>
#include <iomanip>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "backend/opencl/core/OpenCLRunningUtils.hpp"
#include "backend/opencl/core/OpenCLProfilingData.hpp"

namespace MNN {
namespace OpenCL {

std::string DoubleToString(double val) {
  std::stringstream stream;
  stream << std::setprecision(3) << std::setiosflags(std::ios::fixed) << val;
  return stream.str();
}

std::string DoubleToStringFilter(double val) {
  if (0 == val) {
    return "";
  } else {
    return DoubleToString(val);
  }
}

std::string DimsToString(std::vector<int> dims) {
  std::stringstream stream;
  stream << "[";
  for (int i = 0; i < dims.size() - 1; ++i) {
    stream << dims[i] << ", ";
  }
  stream << dims[dims.size() - 1] << "]";

  return stream.str();
}

struct CmpByValue {
  bool operator()(const std::pair<std::string, std::vector<float>> &lhs,
                  const std::pair<std::string, std::vector<float>> &rhs) {
    return lhs.second[0] > rhs.second[0];
  }
};

std::vector<std::pair<std::string, std::vector<float>>> 
SortMapByValue(std::map<std::string, std::vector<float>> &map) {
  std::vector<std::pair<std::string, std::vector<float>>> pair_vec(map.begin(),
                                                                   map.end());
  std::sort(pair_vec.begin(), pair_vec.end(), CmpByValue());
  return pair_vec;
}

std::ostream &FormatRow(std::ostream &stream, int width) {
  stream << std::right << std::setw(width);
  return stream;
}

std::string Table(const std::string &title,
                  const std::vector<std::string> &header,
                  const std::vector<std::vector<std::string>> &data) {
  if (header.empty())
    return "";
  const size_t column_size = header.size();
  const size_t data_size = data.size();
  std::vector<int> max_column_len(header.size(), 0);
  for (size_t col_idx = 0; col_idx < column_size; ++col_idx) {
    max_column_len[col_idx] = std::max<int>(
        max_column_len[col_idx], static_cast<int>(header[col_idx].size()));
    for (size_t data_idx = 0; data_idx < data_size; ++data_idx) {
      if (col_idx < data[data_idx].size()) {
        max_column_len[col_idx] =
            std::max<int>(max_column_len[col_idx],
                          static_cast<int>(data[data_idx][col_idx].size()));
      }
    }
  }
  const size_t row_length =
      std::accumulate(max_column_len.begin(), max_column_len.end(), 0,
                      std::plus<size_t>()) +
      2 * column_size + column_size + 1;
  const std::string dash_line(row_length, '-');
  std::stringstream stream;
  stream << dash_line << std::endl;
  FormatRow(stream, static_cast<int>(row_length / 2 + title.size() / 2))
      << title << std::endl;
  stream << dash_line << std::endl;
  // format header
  stream << "|";
  for (size_t h_idx = 0; h_idx < column_size; ++h_idx) {
    stream << " ";
    FormatRow(stream, max_column_len[h_idx]) << header[h_idx];
    stream << " |";
  }
  stream << std::endl << dash_line << std::endl;
  // format data
  for (size_t data_idx = 0; data_idx < data_size; ++data_idx) {
    stream << "|";
    for (size_t h_idx = 0; h_idx < column_size; ++h_idx) {
      stream << " ";
      FormatRow(stream, max_column_len[h_idx]) << data[data_idx][h_idx];
      stream << " |";
    }
    stream << std::endl;
  }
  stream << dash_line << std::endl;
  return stream.str();
}

void GetProfilingTime(ProfilingData *p) {
  cl_int error = CL_SUCCESS;
  error = p->event.wait();
  MNN_CHECK_CL_SUCCESS(error, "clWaitForEvents failed");
  p->event_queued =
      (double)p->event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(&error);
  MNN_CHECK_CL_SUCCESS(error, "clWaitForEvents failed");
  p->event_submit =
      (double)p->event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>(&error);
  MNN_CHECK_CL_SUCCESS(error, "clWaitForEvents failed");
  p->event_start =
      (double)p->event.getProfilingInfo<CL_PROFILING_COMMAND_START>(&error);
  MNN_CHECK_CL_SUCCESS(error, "clWaitForEvents failed");
  p->event_end =
      (double)p->event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&error);
  MNN_CHECK_CL_SUCCESS(error, "clWaitForEvents failed");
  p->enqueue_time = (p->event_submit - p->event_queued) / 1000000.0;
  p->submit_time = (p->event_start - p->event_submit) / 1000000.0;
  p->kernel_time = (p->event_end - p->event_start) / 1000000.0;
}


} // namespace OpenCL
} // namespace MNN
