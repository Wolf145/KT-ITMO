#ifndef UNITTESTS_TEST_HELPER_H
#define UNITTESTS_TEST_HELPER_H

#include <string>
#include <vector>

namespace test_helper {
void write_sample(const std::string &input, const std::string &expected);

void write_sample(const std::vector<std::string> &input,
                  const std::string &expected);

void write_sample(const std::string &input,
                  const std::vector<std::string> &expected);

void write_sample(const std::vector<std::string> &input,
                  const std::vector<std::string> &expected);
} // namespace test_helper

#endif /* UNITTESTS_TEST_HELPER_H */
