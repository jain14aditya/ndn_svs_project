/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012-2021 University of California, Los Angeles
 *
 * This file is part of ndn-svs, synchronization library for distributed realtime
 * applications for NDN.
 *
 * ndn-svs library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, in version 2.1 of the License.
 *
 * ndn-svs library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 */

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <ndn-svs/svspubsub.hpp>
#include <stdlib.h>

#include <chrono>
#include <ctime>
#include <string>
#include <sstream>

using namespace ndn::svs;

int num_data_packets = 5;
int recv_data_packets = 0;
long long start_time;
std::string return_current_time_and_date() {
  // std::time_t result = std::time(nullptr);
  const auto p1 = std::chrono::system_clock::now();
  std::stringstream ss;
  ss << std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
  return ss.str();
}


class Options
{
public:
  Options() {}

public:
  std::string prefix;
  std::string m_id;
};

class Program
{
public:
  Program(const Options &options)
    : m_options(options)
  {
    // Use HMAC signing
    SecurityOptions securityOptions(m_keyChain);
    securityOptions.interestSigner->signingInfo.setSigningHmacKey("dGhpcyBpcyBhIHNlY3JldCBtZXNzYWdl");

    m_svspubsub = std::make_shared<SVSPubSub>(
      ndn::Name(m_options.prefix),
      ndn::Name(m_options.m_id),
      face,
      std::bind(&Program::onMissingData, this, _1),
      securityOptions);

    std::cout << "SVS client starting:" << m_options.m_id << std::endl;
    m_signingInfo.setSha256Signing();

    // m_svspubsub->subscribeToProducer(ndn::Name("/ndn"), [&] (SVSPubSub::SubscriptionData subData)
    // {
    //   const size_t data_size = subData.data.getContent().value_size();

    //   const std::string content_str((char *) subData.data.getContent().value(), data_size);

    //   std::cout << subData.producerPrefix << "[" << subData.seqNo << "] : " <<
    //                subData.data.getName() << " : " << content_str << std::endl;
    // }, true);

     m_svspubsub->subscribeToProducer(ndn::Name("/ndn"), [&] (SVSPubSub::SubscriptionData subData)
    {
      recv_data_packets++;

      const size_t data_size = subData.data.getContent().value_size();
      const std::string content_str((char *) subData.data.getContent().value(), data_size);

      if(recv_data_packets==1) {
        start_time = std::stoll(content_str.substr(0, content_str.find("=")));
      }
      // std::cout << "recv_data_packets = " << recv_data_packets << std::endl;
      // int segments = subData.data.getFinalBlock()->toNumber();
      
      if(recv_data_packets==num_data_packets) {
        std::string curr_time_str = return_current_time_and_date();
        long long curr_time_ll = std::stoll(curr_time_str);
        long long lat = curr_time_ll - start_time;
        std::cout << "latency = " << lat << "ms end_time = " << curr_time_ll << " start_time = " << start_time << " " << subData.producerPrefix << "[" << subData.seqNo << "] : "  << std::endl;
      }
       
      // std::cout << subData.producerPrefix << "[" << subData.seqNo << "] : " <<
      //              subData.data.getName() << " : " << content_str << " finalBlockId = " << segments << std::endl;
      // fetchOutStandingVoiceSegements(subData.data.getName(), segments);
    }, true);
  }
   

  void
  run()
  {
    std::thread thread_svs([this] { face.processEvents(); });
    // std::string init_msg = "User " + m_options.m_id + " has joined the groupchat";
    // publishMsg(init_msg);
    // publishMsg("hello from B");
    std::string userInput = "";
    test();
    // while (true) {
      // std::getline(std::cin, userInput);
      // publishMsg(userInput);
    // }

    thread_svs.join();
  }

protected:
  void
  onMissingData(const std::vector<MissingDataInfo>& v)
  {
  }

  void
  publishMsg(std::string msg)
  {
    // Content block
    ndn::Block block = ndn::encoding::makeBinaryBlock(
      ndn::tlv::Content, reinterpret_cast<const uint8_t*>(msg.c_str()), msg.size());

    // Data packet
    ndn::Name name(m_options.m_id);
    name.appendTimestamp();

    ndn::Data data(name);
    data.setContent(block);
    data.setFreshnessPeriod(ndn::time::milliseconds(1000));
    m_keyChain.sign(data, m_signingInfo);

    m_svspubsub->publishData(data);
  }

  void test() {
    // Abort if received one interrupt
    // if (receivedSigInt) return;
    // m_participantPrefix
    // Number of data segments

    ndn::Name name("/ndn");
    name.append(m_options.m_id);
    name.appendTimestamp();
    name.appendVersion(0);

    // Create all data segments
    for (int i = 0; i < num_data_packets; i++) {

        // Generate a block of random Data
        // std::array<__uint8_t, 1000> buf{};
        // ndn::random::generateSecureBytes(buf.data(), 512);
        // ndn::Block block = ndn::encoding::makeBinaryBlock(
        //         ndn::tlv::Content, buf.data(), 512);

        // Generate a block of random Data
        std::array<__uint8_t, 7514> buf{};
        ndn::random::generateSecureBytes(buf.data(), 7514);
        std::string s = return_current_time_and_date();
        for(int j=0; j<13; j++) {
          buf[j] = s[j];
        }
        buf[13] = '=';

        ndn::Block block = ndn::encoding::makeBinaryBlock(
                ndn::tlv::Content, buf.data(), 7514);
        // std::string msg = return_current_time_and_date() + "=hello_world";
        // ndn::Block block = ndn::encoding::makeBinaryBlock(
          // ndn::tlv::Content, reinterpret_cast<const uint8_t*>(msg.c_str()), msg.size());

        // Data packet
        ndn::Name realName(name);
        realName.appendSegment(i);

        std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(realName);
        data->setContent(block);
        data->setFreshnessPeriod(ndn::time::milliseconds(1000));
        data->setFinalBlock(ndn::name::Component::fromNumber(num_data_packets - 1));
        // *
        m_keyChain.sign(*data, m_signingInfo);

        // m_dataStore.insert(*data);

        // Publish first segment of voice data using publish channel
        // if (i == 0) {
        m_svspubsub->publishData(*data);
            // BOOST_LOG_TRIVIAL(info) << "PUBL_MSG::" << realName.toUri();
        // std::cout << "Publish voice data: " << data->getName() << " (" << msg.size() * data_size << " bytes)"
        //               << std::endl;
        // std::cout << "Publish voice data: " << data->getName() << " (" << buf.size() << " bytes)" << std::endl;
        // }
    }
  }

public:
  const Options m_options;
  ndn::Face face;
  std::shared_ptr<SVSPubSub> m_svspubsub;
  ndn::KeyChain m_keyChain;
  ndn::security::SigningInfo m_signingInfo;
};

int main(int argc, char **argv)
{
  // if (argc != 2) {
  //   std::cout << "Usage: client <prefix>" << std::endl;
  //   exit(1);
  // }

  Options opt;
  opt.prefix = "/ndn/svs";
  opt.m_id = argv[1];
  num_data_packets=atoi(argv[3]);
  // initlogger(std::string(argv[2]));
  Program program(opt);
  program.run();
  return 0;
}
