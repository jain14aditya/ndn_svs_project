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

#include <ndn-svs/svsync-base.hpp>

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
    std::cout << "SVS client starting:" << m_options.m_id << std::endl;
    m_signingInfo.setSha256Signing();
  }

  void
  run()
  {
    face.processEvents(); 
    test();
    // std::thread thread_svs([this] { face.processEvents(); });

    // std::string init_msg = "User " + m_options.m_id + " has joined the groupchat";
    // publishMsg(init_msg);

    // std::string userInput = "";

    // while (true) {
    //   std::getline(std::cin, userInput);
    //   publishMsg(userInput);
    // }

    // thread_svs.join();
  }

protected:
  void
  onMissingData(const std::vector<ndn::svs::MissingDataInfo>& v)
  {
    for (size_t i = 0; i < v.size(); i++)
    {
      for (ndn::svs::SeqNo s = v[i].low; s <= v[i].high; ++s)
      {
        ndn::svs::NodeID nid = v[i].nodeId;
        m_svs->fetchData(nid, s, [nid] (const ndn::Data& data)
          {
            const size_t data_size = data.getContent().value_size();
            const std::string content_str((char *)data.getContent().value(), data_size);
            std::cout << data.getName() << " : " << content_str << std::endl;
          });
      }
    }
  }

  void
  publishMsg(std::string msg)
  {
    // Content block
    ndn::Block block = ndn::encoding::makeBinaryBlock(
      ndn::tlv::Content,
      reinterpret_cast<const uint8_t*>(msg.c_str()), msg.size());
    m_svs->publishData(block, ndn::time::milliseconds(1000));
  }

  void test(){
    // Abort if received one interrupt
    if (receivedSigInt) return;

    // Number of data segments
    // int voiceSize = m_voiceDataSizeDist(m_rng);
    int voiceSize = 100;

    ndn::Name name("/ndn/file");
    name.append(m_options.m_id);
    name.appendTimestamp();
    name.appendVersion(0);

    // Create all data segments
    for (int i = 0; i < voiceSize; i++) {

        // Generate a block of random Data
        std::array<__uint8_t, 1000> buf{};
        ndn::random::generateSecureBytes(buf.data(), 512);
        ndn::Block block = ndn::encoding::makeBinaryBlock(
                ndn::tlv::Content, buf.data(), 512);

        // Data packet
        ndn::Name realName(name);
        realName.appendSegment(i);

        std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(realName);
        data->setContent(block);
        data->setFreshnessPeriod(ndn::time::milliseconds(1000));
        data->setFinalBlock(ndn::name::Component::fromNumber(voiceSize - 1));
        m_keyChain.sign(*data, m_signingInfo);

        m_dataStore.insert(*data);

        // Publish first segment of voice data using publish channel
        if (i == 0) {
            publishData(*data);
            BOOST_LOG_TRIVIAL(info) << "PUBL_MSG::" << realName.toUri();
            std::cout << "Publish voice data: " << data->getName() << " (" << buf.size() * voiceSize << " bytes)"
                      << std::endl;
        }

        // Todo: Log published Data
    }
  }

public:
  const Options m_options;
  ndn::Face face;
  std::shared_ptr<ndn::svs::SVSyncBase> m_svs;
  ndn::KeyChain m_keyChain;
  ndn::security::SigningInfo m_signingInfo;
};

template <typename T>
int
callMain(int argc, char **argv) {
 // if (argc != 2) {
 //   std::cout << "Usage: client <prefix>" << std::endl;
  //  exit(1);
  //}

  Options opt;
  opt.prefix = "/ndn/svs";
  opt.m_id = argv[1];
  //opt.m_id = "a";
  initlogger(std::string(argv[2]));
  T program(opt);
  program.run();
  return 0;
}
