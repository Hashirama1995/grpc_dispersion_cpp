#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <ctime>
#include <math.h>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
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

static int COUNT = 5;
static int STATUS = 1;


class DispGreeterClient {
public:
    DispGreeterClient(std::shared_ptr<Channel> channel): stub_(DGreeter::NewStub(channel)) 
    {
        std::cout << "Data in client:  ";
        s = new float[COUNT];
        srand(time(NULL));
        for (int i = 0; i < COUNT; i++)
        {
            s[i] = 1 + rand() % (10 - 1 + 1);
            std::cout << s[i] << " ";
        }
        std::cout << std::endl << std::endl << std::endl;
    }

    ~DispGreeterClient()
    {
        delete[] s;
    }

    void calculateTotal()
    {
        localSUM = 0;
        for (int i = 0; i < COUNT; i++)
        {
            localSUM += s[i];
        }
    }

    void calculateLocalDisp()
    {
        localDisp = 0;
        for (int i = 0; i < COUNT; i++)
        {
            localDisp += pow(average_x - s[i], 2);
        }
        localDisp /= n;
    }

    int checkID(int id)
    {
        CheckIDRequest request;
        request.set_id(id);
        CheckIDReply reply;
        ClientContext context;
        Status status = stub_->CheckID(&context, request, &reply);
        if (status.ok()) {
            return reply.status();
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return 0;
        }
    }

    int getStatus(int id)
    {
        ServerStatus request;
        request.set_id(id);
        ServerStatusReply reply;
        ClientContext context;
        Status status = stub_->GetStatus(&context, request, &reply);
        if (status.ok()) {
            return reply.status();
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return 0;
        }
    }

    int sendTotal(int id)
    {
        calculateTotal();
        Total request;
        request.set_id(id);
        request.set_total(localSUM);
        TotalCallbackServer reply;
        ClientContext context;
        Status status = stub_->sendTotalSum(&context, request, &reply);
        if (status.ok()) {
            return reply.status();
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return 0;
        }
    }

    int getCountAndAverage(int id)
    {
        getAverage request;
        request.set_id(id);
        AverageCallbackServer reply;
        ClientContext context;
        Status status = stub_->getCountAndAverage(&context, request, &reply);
        if (status.ok()) {
            average_x= reply.average();
            n = reply.n();

            return reply.status();
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return 0;
        }
    }

    int sendLocalDisp(int id)
    {
        calculateLocalDisp();
        SendDispersion request;
        request.set_id(id);
        request.set_currentdisp(localDisp);
        DispersionCallback reply;
        ClientContext context;
        Status status = stub_->sendLocalDisp(&context, request, &reply);
        if (status.ok()) {
            
            return reply.status();
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return 0;
        }
    }

    float getDispersion(int id)
    {
        getTotalDispersion request;
        request.set_id(id);
        CallbackTotalDisp reply;
        ClientContext context;
        Status status = stub_->getTotalDisp(&context, request, &reply);
        if (status.ok()) {

            return reply.dispersion();  // TOTAL RESULT DISPERSION
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return 0;
        }
    }

 private:
    std::unique_ptr<DGreeter::Stub> stub_;
    float* s;
    float localSUM = 0.0f;
    float average_x = 0.0f;
    int n = 0;
    float localDisp = 0.0f;
};


int main(int argc, char** argv) {
    
  std::cout << "Hello World!" << std::endl;

  std::string target_str;
  target_str = "localhost:50051";

  int ID = 999;
  DispGreeterClient DispGreeter(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));   //checkID
  {
      bool goodID = false;
      while (!goodID)
      {
          std::cout << "Please enter you ID" << std::endl;
          std::cin >> ID;

          if (DispGreeter.checkID(ID))
          {
              std::cout << "You ID: " << ID << std::endl << std::endl;
              goodID = true;
          }
          else
          {
              std::cout << "That ID: " << ID << " is already in use" << std::endl;
          }
      }
  }
  

  while (STATUS)
  {
      switch (STATUS)
      {
          case 1: {
              STATUS = DispGreeter.sendTotal(ID /*, DispGreeter.get_localSUM() */ );
              while (STATUS == 1) {
                  STATUS = DispGreeter.getStatus(ID);
              }
              break;
          }
          case 2: {
              STATUS = DispGreeter.getCountAndAverage(ID);
              while (STATUS == 2) {
                  STATUS = DispGreeter.getStatus(ID);
              }
              break;
          }
          case 3: {
              STATUS = DispGreeter.sendLocalDisp(ID);
              while (STATUS == 3) {
                  STATUS = DispGreeter.getStatus(ID);
              }
              break;
          }
          case 4: {
              std::cout << "SERVER HAVE CURRENT DISPERSION DATA !" << STATUS << std::endl;
              float dispTest = DispGreeter.getDispersion(ID);
              std::cout << "DISPERSION = " << dispTest << std::endl << std::endl;
              while (STATUS == 4) {
                  STATUS = DispGreeter.getStatus(ID);
              }
              break;
          }
          default: {
              std::cout << "ERROR STATUS" << std::endl;
              while (1);
              break; }
      }    
  }
  std::cout << "end programm" << STATUS <<std::endl;
  return 0;
}