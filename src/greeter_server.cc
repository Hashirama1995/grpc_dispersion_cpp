#include <iostream>
#include <memory>
#include <string>
#include <map>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif
#include "helloworld.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using helloworld::DGreeter;

using helloworld::CheckIDRequest;
using helloworld::CheckIDReply;
using helloworld::ServerStatus;
using helloworld::ServerStatusReply;
using helloworld::Total;
using helloworld::TotalCallbackServer;
using helloworld::getAverage;
using helloworld::AverageCallbackServer;
using helloworld::SendDispersion;
using helloworld::DispersionCallback;
using helloworld::getTotalDispersion;
using helloworld::CallbackTotalDisp;

struct ClientInfo {
    ClientInfo() {}
    ClientInfo(int _status, float _localSum):status(_status),localSum(_localSum)
    {    }
    int status = 0;
    float localSum = 0;
    float localDisp = 0;
    
};


static int TOTALDISPERSION = 0;
static int COUNT = 5;
static int SERVERSTATUS = 1;
static std::map<int, ClientInfo> sumFromClient;


class DispGreeterServiceImpl final : public DGreeter::Service {///CheckID 

    Status CheckID(ServerContext* context, const CheckIDRequest* request,
        CheckIDReply* reply) override {
        auto element = sumFromClient.find(request->id());
        if (element == sumFromClient.end()) {
            reply->set_status(1);
        }
        else {
            reply->set_status(0);
        }
            
        //std::cout << "Status SERVER " << SERVERSTATUS << std::endl;
        return Status::OK;
    }

    Status GetStatus(ServerContext* context, const ServerStatus* request,
        ServerStatusReply* reply) override {
        reply->set_status(SERVERSTATUS);
        //std::cout << "Status SERVER " << SERVERSTATUS << std::endl;
        return Status::OK;
    }

    Status sendTotalSum(ServerContext* context, const Total* request,
        TotalCallbackServer* reply) override {

        auto element = sumFromClient.find(request->id());
        if (element == sumFromClient.end())
        {
            std::cout << "create client #" + std::to_string(request->id()) << std::endl;
            sumFromClient.emplace(request->id(), ClientInfo(1, request->total()));
        }
        else
        {
            std::cout << "update client #"+ std::to_string(request->id()) << std::endl;
            element->second.localSum = request->total();
            element->second.status = 1;
        }
        
        reply->set_callback("Client" + std::to_string(request->id()) + " sum added");

        for (auto& iter: sumFromClient) {
            if (iter.second.status != 1)
            {
                reply->set_status(1);
                SERVERSTATUS = 1;
                return Status::OK;
            }
        }
        for (auto& iter : sumFromClient) {
            iter.second.status = 0;
        }
        SERVERSTATUS = 2;
        reply->set_status(2);
        return Status::OK;
    }

    Status getCountAndAverage(ServerContext* context, const getAverage* request,
        AverageCallbackServer* reply) override {
        int n = COUNT * sumFromClient.size();
        reply->set_n(n);

        float average_x = 0;

        for (auto& iter : sumFromClient) {
            average_x += iter.second.localSum;
        }

        average_x /= n;
        reply->set_average(average_x);
        reply->set_status(3);
        SERVERSTATUS = 3;
        return Status::OK;
    }

    Status sendLocalDisp(ServerContext* context, const SendDispersion* request,
        DispersionCallback* reply) override {

        auto element = sumFromClient.find(request->id());
        if (element != sumFromClient.end())
        {
            std::cout << "update client dispersion #" + std::to_string(request->id()) << std::endl;
            element->second.localDisp = request->currentdisp();
        }
        SERVERSTATUS = 4;
        reply->set_status(4);
        return Status::OK;
    }

    Status getTotalDisp(ServerContext* context, const getTotalDispersion* request,
        CallbackTotalDisp* reply) override {

        float resultDisp = 0.0f;
        for (auto& iter : sumFromClient) {
            resultDisp+= iter.second.localDisp;
        }
        reply->set_dispersion(resultDisp);
        std::cout << "CURRENT DISPERSION = " + std::to_string(resultDisp) << std::endl;
        return Status::OK;
    }
};






void RunServer() {
  std::string server_address("0.0.0.0:50051");

  DispGreeterServiceImpl serviceDisp;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&serviceDisp);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  SERVERSTATUS = 1;

  server->Wait();
}

int main(int argc, char** argv)
{
   RunServer();
   return 0;
}
