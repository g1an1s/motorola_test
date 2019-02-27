#include <algorithm>
#include <tuple>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "helper.h"
#include "test.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
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


class MobilityImpl final : public ngcp::test::MobilityService::Service
{
public:
  explicit MobilityImpl(const std::string &db)
  {
    profileList_.clear();
  }

  // ------------------------------------------------------CREATE
  Status CreateRadioProfile(ServerContext *context, const CreateRadioProfileRequest *req,
                    CreateRadioProfileResponse *res) override
  {     
    for (std::tuple<CreateRadioProfileRequest, std::string> tupProfile : profileList_)
    {
      auto profile = std::get<0>(tupProfile);

      if ( (profile.id() == req->id()) || (profile.alias() == req->alias()) )
      {
        //std::cout << "Profile already exists." << std::endl;
        return Status::CANCELLED;
      }
    }
    auto element = std::make_tuple(*req, "undefined");
    profileList_.push_back(element);
    return Status::OK;
  }

  // ------------------------------------------------------SET
  Status SetRadioLocation(ServerContext *context, const SetRadioLocationRequest *req,
    SetRadioLocationResponse *res) override
  {
    
      for (size_t p=0; p < profileList_.size(); ++p)
      {
        ngcp::test::CreateRadioProfileRequest profile = std::get<0>(profileList_[p]);

        if (profile.id() == req->radio_id())
        {
            for (size_t i=0; i < profile.allowed_locations().size(); ++i)
            {
                if ( (profile.allowed_locations()[i] == req->location()) &&
                     (profile.allowed_locations()[i] != "" ) )
                {
                    std::get<1>(profileList_[p]) = req->location();
                    res->set_success(true);
                    //std::cout<<"Location updated to " << std::get<1>(profileList_[p]) << std::endl;
                    return Status::OK;
                }
            }
        }

      }
    res->set_success(false);
    //std::cout<<"Update not allowed" << std::endl;
    return Status::CANCELLED;
  }

  // ------------------------------------------------------GET
  Status GetRadioLocation(ServerContext *context, const GetRadioLocationRequest *req,
                    GetRadioLocationResponse *res) override
  {
    for (std::tuple<CreateRadioProfileRequest, std::string> tupProfile : profileList_)
    {
      ngcp::test::CreateRadioProfileRequest profile = std::get<0>(tupProfile);
      
      if ( profile.id() == req->radio_id() )
      {
        //std::cout << "id exists in DB " << std::get<1>(tupProfile)  << std::endl;
        res->set_location( std::get<1>(tupProfile) );
        return Status::OK;
      }
    }

    //std::cout << "id does not exist in DB." << std::endl;
    res->set_location( "undefined" );
    return Status::CANCELLED;
  }

  // ------------------------------------------------------DELETE
  Status DeleteRadioProfile(ServerContext *context, const DeleteRadioProfileRequest *req,
                    DeleteRadioProfileResponse *res) override
  {
    for (size_t i=0; i < profileList_.size(); ++i)
    {
      ngcp::test::CreateRadioProfileRequest profile = std::get<0>(profileList_[i]);

      if (profile.id() == req->id())
      {
        profileList_.erase(profileList_.begin()+i);
        //std::cout << "Profile deleted." << std::endl;
        return Status::OK;
      }
    }
    return Status::CANCELLED;
  }

private:
  
  std::vector< std::tuple<ngcp::test::CreateRadioProfileRequest, std::string> > profileList_;
};

void RunServer(const std::string &db_path) {
  std::string server_address("0.0.0.0:50051");
  MobilityImpl service(db_path);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char **argv) {
  std::string db = ngcp::test::GetDbFileContent(argc, argv);
  RunServer(db);

  return 0;
}

