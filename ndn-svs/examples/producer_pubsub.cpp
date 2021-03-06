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
// #include "date.h"
#include <ctime>
#include <string>
#include <sstream>

std::string return_current_time_and_date() {
  // std::time_t result = std::time(nullptr);
  const auto p1 = std::chrono::system_clock::now();
  std::stringstream ss;
  ss << std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
  // ss << std::asctime(std::localtime(&result)) << result;
  // std::cout << "testing = " << ss.str() << std::endl;
  return ss.str();
  // auto now = std::chrono::system_clock::now();

  
  // ss << 
  // ss << today << ' ' << date::make_time(now - today) << " UTC";
  // return ss.str();
}

using namespace ndn::svs;

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

    m_svspubsub->subscribeToProducer(ndn::Name("/ndn"), [&] (SVSPubSub::SubscriptionData subData)
    {
      const size_t data_size = subData.data.getContent().value_size();
      const std::string content_str((char *) subData.data.getContent().value(), data_size);
      std::string s1 = return_current_time_and_date();
      long long t1 = std::stoll(s1);
      long long t = std::stoll(content_str.substr(0, content_str.find("=")));
      long long lat = t1 - t;
      std::cout << "latency =  " << lat << "ms timestamp = " << s1 << " " << subData.producerPrefix << "[" << subData.seqNo << "] : " <<
                   subData.data.getName() << " : " << content_str << std::endl;
    }, true);
  }

  void
  run()
  {
    std::thread thread_svs([this] { face.processEvents(); });

    // std::string init_msg = "User " + m_options.m_id + " has joined the groupchat";
    // publishMsg(init_msg);

    std::string userInput = m_options.m_id;
  

    // std::string s = date::format("%F %T", std::chrono::system_clock::now());
    std::string s1 = return_current_time_and_date() + "=hello world";
    publishMsg(s1);
    // publishMsg(return_current_time_and_date());
    // publishMsg("hello world");

    std::string s2 = return_current_time_and_date() + "=testing second msg";
    publishMsg(s2);

    // publishMsg(return_current_time_and_date());
    // publishMsg("testing second msg");
    // int cnt = 0;
    while (true) {
      // std::string s2 = return_current_time_and_date() + "=testing second msg";
      // publishMsg(s2);
      // publishMsg(return_current_time_and_date());
      // std::getline(std::cin, userInput);
      // userInput = userInput + std::to_string(cnt);
      // publishMsg(userInput);
    }
    // publishMsg("qwerty hi1");
    // publishMsg("qwerty hi2");
    // publishMsg("qwerty hi3");
    // publishMsg("qwerty hi4");
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
    // data.setContent(block);
    data.setFreshnessPeriod(ndn::time::milliseconds(1000));
    m_keyChain.sign(data, m_signingInfo);

    m_svspubsub->publishData(data);

    // m_svspubsub->publishData(reinterpret_cast<const uint8_t*>(msg.c_str()),  
    //                    msg.size(),ndn::time::milliseconds(1000));
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
  // initlogger(std::string(argv[2]));
  Program program(opt);
  program.run();
  return 0;
}
