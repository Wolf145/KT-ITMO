#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "test_helper.h"

#ifndef TESTS_TESTDATA_ROOT
#define TESTS_TESTDATA_ROOT ""
#endif

class SampleWriter {
public:
  SampleWriter();

  void write(const std::string &input, const std::string &expected);

private:
  std::filesystem::path m_root;
  int m_counter;
};

SampleWriter g_sample_writer{};

SampleWriter::SampleWriter() : m_root(TESTS_TESTDATA_ROOT), m_counter(0) {
  m_root /= "testdata";
  std::filesystem::create_directory(m_root);
}

void SampleWriter::write(const std::string &input,
                         const std::string &expected) {
  m_counter++;

  const std::filesystem::path input_path =
      m_root / std::format("input-{}.txt", m_counter);
  const std::filesystem::path expected_path =
      m_root / std::format("expected-{}.txt", m_counter);

  std::ofstream input_stream{input_path};
  std::ofstream expected_stream{expected_path};

  input_stream << input << '\n';
  expected_stream << expected << '\n';
}

static std::string list_to_string(const std::vector<std::string> &lines) {
  std::string result = lines[0];

  for (std::size_t i = 1; i < lines.size(); i++) {
    result.push_back('\n');
    result.append(lines[i]);
  }

  return result;
}

void test_helper::write_sample(const std::string &input,
                               const std::string &expected) {
  g_sample_writer.write(input, expected);
}

void test_helper::write_sample(const std::vector<std::string> &lines,
                               const std::string &expected) {
  const std::string input = list_to_string(lines);
  g_sample_writer.write(input, expected);
}

void test_helper::write_sample(const std::string &input,
                               const std::vector<std::string> &lines) {
  const std::string expected = list_to_string(lines);
  g_sample_writer.write(input, expected);
}

void test_helper::write_sample(const std::vector<std::string> &lines_input,
                               const std::vector<std::string> &lines_expected) {
  const std::string input = list_to_string(lines_input);
  const std::string expected = list_to_string(lines_expected);
  g_sample_writer.write(input, expected);
}
