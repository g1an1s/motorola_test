
#ifndef GRPC_COMMON_CPP_TEST_HELPER_H_
#define GRPC_COMMON_CPP_TEST_HELPER_H_

#include <string>
#include <vector>


namespace ngcp {
namespace test {
class CreateRadioProfileRequest;

std::string GetDbFileContent(int argc, char** argv);

//void ParseDb(const std::string& db, std::vector<ngcp::test::CreateRadioProfileRequest>* profile_list);

}  // namespace test
}  // namespace ngcp

#endif  // GRPC_COMMON_CPP_TEST_HELPER_H_
