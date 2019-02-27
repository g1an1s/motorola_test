#include <chrono>
#include <tuple>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "helper.h"
#include "test.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using ngcp::test::CreateRadioProfileRequest;
using ngcp::test::CreateRadioProfileResponse;
using ngcp::test::DeleteRadioProfileRequest;
using ngcp::test::DeleteRadioProfileResponse;
using ngcp::test::SetRadioLocationRequest;
using ngcp::test::SetRadioLocationResponse;
using ngcp::test::GetRadioLocationRequest;
using ngcp::test::GetRadioLocationResponse;
using ngcp::test::MobilityService;

#define MAX_LOC_ALLOWED 3


class MobilityImplClient {
 public:
  MobilityImplClient(std::shared_ptr<Channel> channel, const std::string& db)
      : stub_(MobilityService::NewStub(channel)) {
  }

  // ------------------------------------------------------CREATE
  void CreateRadioProfile(size_t id, std::string alias,
    std::vector<std::string> allowed_locations)
  {
    CreateRadioProfileRequest req;
    CreateRadioProfileResponse res;
    req.set_id(id);
    req.set_alias(alias);
    req.add_allowed_locations(allowed_locations.at(0));
    req.add_allowed_locations(allowed_locations.at(1));
    req.add_allowed_locations(allowed_locations.at(2));

    CreateOneRadioProfile(req, &res);
  }

  // ------------------------------------------------------SET
  void SetRadioLocation(size_t id, std::string location)
  {
    SetRadioLocationRequest req;
    SetRadioLocationResponse res;
    req.set_radio_id(id);
    req.set_location(location);

    SetOneRadioLocation(req, &res);
  }

  // ------------------------------------------------------GET
  void GetRadioLocation(size_t id)
  {
    GetRadioLocationRequest req;
    GetRadioLocationResponse res;
    req.set_radio_id(id);

    GetOneRadioLocation(req, &res);
  }

  // ------------------------------------------------------DELETE
  void DeleteRadioProfile(size_t id)
  {
    DeleteRadioProfileRequest req;
    DeleteRadioProfileResponse res;
    req.set_id(id);

    DeleteOneRadioProfile(req, &res);
  }


 private:

  // ------------------------------------------------------CREATEONE
  bool CreateOneRadioProfile(const CreateRadioProfileRequest &req, CreateRadioProfileResponse *res)
  {
    ClientContext context;

    Status status = stub_->CreateRadioProfile(&context, req, res);
    if (!status.ok()) {
      std::cout << "CREATE RPC failed." << std::endl;
      return false;
    }
    return true;
  }

  // ------------------------------------------------------SETONE
  bool SetOneRadioLocation(const SetRadioLocationRequest &req, SetRadioLocationResponse *res)
  {
      ClientContext context;
      Status status = stub_->SetRadioLocation(&context, req, res);
      if (!status.ok())
      {
        std::cout << "SET RPC failed." << std::endl;
        return false;
      }
      std::cout << res->success() << std::endl;
      return true;
  }

  // ------------------------------------------------------GETONE
  bool GetOneRadioLocation(const GetRadioLocationRequest &req, GetRadioLocationResponse *res) {
    ClientContext context;

    Status status = stub_->GetRadioLocation(&context, req, res);
    if (!status.ok()) {
      std::cout << "GET RPC failed." << std::endl;
      return false;
    }
    else
    {
      std::cout << res->location() << std::endl;
      return true;
    }
  }

  // ------------------------------------------------------DELETEONE
  bool DeleteOneRadioProfile(const DeleteRadioProfileRequest &req, DeleteRadioProfileResponse *res) {
    ClientContext context;

    Status status = stub_->DeleteRadioProfile(&context, req, res);
    if (!status.ok()) {
      std::cout << "DELETE RPC failed." << std::endl;
      return false;
    }
    else
    {
      return true;
    }
  }

  std::unique_ptr<MobilityService::Stub> stub_;
};

int main(int argc, char **argv) {
  std::string db = ngcp::test::GetDbFileContent(argc, argv);
  MobilityImplClient mob_client(
      grpc::CreateChannel("localhost:50051",
                          grpc::InsecureChannelCredentials()), db);



  // --------------------- TEST --------------------------------------------
  // -----------------------------------------------------------------------

  std::cout << "Type one of: 'create', 'get', 'set', 'delete', 'quit'" << std::endl;

  std::string choice;
  while( getline(std::cin, choice) )
  {
    if (choice == "quit")
    {
      break;
    }

    if (choice == "create") // CREATE
    {
      std::string id = "";
      std::string alias = "";
      std::string loc = "";
      std::vector<std::string> allowed_locations(MAX_LOC_ALLOWED);

      std::cout << "Enter id:" << std::endl;
      getline(std::cin, id);
      std::cout << "Enter alias:" << std::endl;
      getline(std::cin, alias);

      for (size_t i = 0; i < MAX_LOC_ALLOWED; ++i)
      {
        std::cout << "Enter allowed location " << i+1 << ":" << std::endl;
        getline(std::cin, loc);
        allowed_locations[i] = loc;
      }
      mob_client.CreateRadioProfile(std::stoi( id ), alias, allowed_locations);
      
      std::cout << "Thank you.\n" << std::endl;
    }

    if (choice == "set") // SET
    {
        std::string id = "";
        std::string loc = "";
        std::cout << "Enter id:" << std::endl;
        getline(std::cin, id);
        std::cout << "Enter location:" << std::endl;
        getline(std::cin, loc);

        mob_client.SetRadioLocation(std::stoi( id ), loc);

        std::cout << "Thank you.\n" << std::endl;
    }

    if (choice == "get") // GET
    {
      std::string id = "";
      std::cout << "Enter id:" << std::endl;
      getline(std::cin, id);

      mob_client.GetRadioLocation(std::stoi( id ));
      
      std::cout << "Thank you.\n" << std::endl;
    }

    if (choice == "delete") // DELETE
    {
      std::string id = "";
      std::cout << "Enter id:" << std::endl;
      getline(std::cin, id);

      mob_client.DeleteRadioProfile(std::stoi( id ));
      
      std::cout << "Thank you.\n" << std::endl;
    }


    std::cin.clear();
  }
  return 0;
}
