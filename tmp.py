# -*- Mode:python; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
#
# Copyright (C) 2015-2020, The University of Memphis,
#                          Arizona Board of Regents,
#                          Regents of the University of California.
#
# This file is part of Mini-NDN.
# See AUTHORS.md for a complete list of Mini-NDN authors and contributors.
#
# Mini-NDN is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Mini-NDN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Mini-NDN, e.g., in COPYING.md file.
# If not, see <http://www.gnu.org/licenses/>.

import random
import time
import configparser
import psutil
import os
from collections import defaultdict

from mininet.log import setLogLevel, info

from minindn.apps.application import Application

from minindn.helpers.nfdc import Nfdc
from minindn.minindn import Minindn
from minindn.util import MiniNDNCLI
from minindn.apps.app_manager import AppManager
from minindn.apps.nfd import Nfd
from minindn.helpers.ndn_routing_helper import NdnRoutingHelper
from mininet.node import OVSController

from tqdm import tqdm

# ======================= CONFIGURATION ============================
OVERALL_RUN = 2
DEBUG_GDB = False
NUM_NODES = 2
# PUB_TIMING_VALS = [1000, 5000, 10000, 15000]

PUB_TIMING_VALS = []
i=5
while i<=100:
    PUB_TIMING_VALS.append(i)
    i+=5
# num_data_packets = 5
# RUN_NUMBER_VALS = list(range(1, 4))
RUN_NUMBER_VALS = list(range(1, 2))

cwd=os.getcwd()+"/"

LOG_PREFIX = "default_topology_2_n"
TOPO_FILE = cwd+"topologies/default_topology_2_n.conf"


# LOG_PREFIX = "default_topology_4_n"
# TOPO_FILE = cwd+"topologies/default_topology_4_n.conf"

# LOG_PREFIX = "default_topology_3_n"
# TOPO_FILE = cwd+"topologies/default_topology_3_n.conf"

# LOG_PREFIX = "default_topology_4_n"
# TOPO_FILE = cwd+"topologies/default_topology_4s_n.conf"

# LOG_PREFIX = "geant"
# TOPO_FILE = cwd+"topologies/geant.conf"

SYNC_EXEC_VALS = [
    cwd+"ndn-svs/build/examples/eval",          # SVS
    # cwd+"ndn-svs/build/examples/chat",          # SVS
    #"/home/vagrant/mini-ndn/work/ChronoSync/build/examples/eval",       # Chronosync
    # "/home/vagrant/mini-ndn/work/PSync/build/examples/psync-eval",      # PSync
    #"/home/vagrant/mini-ndn/work/syncps/eval",                          # syncps
]

LOG_MAIN_PATH = "/home/vagrant/mini-ndn/work/log/{}/".format(OVERALL_RUN)
LOG_MAIN_DIRECTORY_VALS = [
    LOG_MAIN_PATH + "svs/",                                       # SVS
    #LOG_MAIN_PATH + "chronosync/",                                # ChronoSync
    # LOG_MAIN_PATH + "psync/",                                     # PSync
    #LOG_MAIN_PATH + "syncps/",                                    # syncps
]
# ==================================================================

RUN_NUMBER = 0
PUB_TIMING = 0
SYNC_EXEC = None
LOG_MAIN_DIRECTORY = None

def getLogPath():
    LOG_NAME = "{}-{}-{}".format(LOG_PREFIX, PUB_TIMING, RUN_NUMBER)
    logpath = LOG_MAIN_DIRECTORY + LOG_NAME

    if not os.path.exists(logpath):
        os.makedirs(logpath)
        os.chown(logpath, 1000, 1000)

        os.makedirs(logpath + '/stdout')
        os.chown(logpath + '/stdout', 1000, 1000)
        os.makedirs(logpath + '/stderr')
        os.chown(logpath + '/stdout', 1000, 1000)

    return logpath

class SvsChatApplication(Application):
    """
    Wrapper class to run the chat application from each node
    """
    def get_svs_identity(self):
        return "/ndn/{0}-site/{0}/svs_chat/{0}".format(self.node.name)

    def start(self): 
        # exe = SYNC_EXEC
        # exe =  cwd+"ndn-svs/build/examples/producer_pubsub"
        exe =  cwd+"ndn-svs/build/examples/consumer_pubsub"
        print(self.node.name)
        # if self.node.name == "a": 
        #     exe =  cwd+"ndn-svs/build/examples/producer_chat" 
        # if self.node.name == "a": 
        #     exe =  cwd+"ndn-svs/build/examples/producer_pubsub"  
        # if self.node.name == "b": 
        #     exe =  cwd+"ndn-svs/build/examples/consumer_pubsub"  
        identity = self.get_svs_identity()

        if DEBUG_GDB:
            run_cmd = "gdb -batch -ex run -ex=\"set confirm off\" -ex \"bt full\" -ex quit --args {0} {1} {2}/{3}.log {4} >{2}/stdout/{3}.log 2>{2}/stderr/{3}.log &".format(
                exe, identity, getLogPath(), self.node.name, PUB_TIMING)
        else:
            run_cmd = "{0} {1} {2}/{3}.log {4} >{2}/stdout/{3}.log 2>{2}/stderr/{3}.log &".format(
                exe, identity, getLogPath(), self.node.name, PUB_TIMING)

        ret = self.node.cmd(run_cmd)
        info("[{}] running {} == {}\n".format(self.node.name, run_cmd, ret))


class ProducerChatApplication(Application):
    """
    Wrapper class to run the chat application from each node
    """
    def get_svs_identity(self):
        return "/ndn/{0}-site/{0}/svs_chat/{0}".format(self.node.name)

    def start(self):
        exe =  cwd+"ndn-svs/build/examples/producer_chat"
        identity = self.get_svs_identity()

        if DEBUG_GDB:
            run_cmd = "gdb -batch -ex run -ex=\"set confirm off\" -ex \"bt full\" -ex quit --args {0} {1} {2}/{3}.log {4} >{2}/stdout/{3}.log 2>{2}/stderr/{3}.log &".format(
                exe, identity, getLogPath(), self.node.name, PUB_TIMING)
        else:
            run_cmd = "{0} {1} {2}/{3}.log {4} >{2}/stdout/{3}.log 2>{2}/stderr/{3}.log &".format(
                exe, identity, getLogPath(), self.node.name, PUB_TIMING)

        ret = self.node.cmd(run_cmd)
        info("[{}] running {} == {}\n".format(self.node.name, run_cmd, ret))


def count_running(pids):
    return sum(psutil.pid_exists(pid) for pid in pids)

def get_pids():
    pids = []
    for proc in psutil.process_iter():
        try:
            pinfo = proc.as_dict(attrs=['pid', 'name', 'create_time'])
            # Check if process name contains the given name string.
            # if ("gdb" if DEBUG_GDB else "producer_pubsub") in pinfo['name'].lower():
            #     pids.append(pinfo['pid'])
            if ("gdb" if DEBUG_GDB else "consumer_pubsub") in pinfo['name'].lower():
                pids.append(pinfo['pid'])
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
    return pids

if __name__ == '__main__':
    print(LOG_PREFIX, TOPO_FILE, SYNC_EXEC_VALS)

    setLogLevel('info')

    Minindn.cleanUp()
    Minindn.verifyDependencies()

    ndn = Minindn(topoFile=TOPO_FILE, controller = OVSController)

    ndn.start()

    info('Starting NFD on nodes\n')
    nfds = AppManager(ndn, ndn.net.hosts, Nfd)
    info('Sleeping 10 seconds\n')
    time.sleep(3 if DEBUG_GDB else 10)

    info('Setting NFD strategy to multicast on all nodes with prefix')
    for node in tqdm(ndn.net.hosts):
        Nfdc.setStrategy(node, "/ndn/svs", Nfdc.STRATEGY_MULTICAST)

    info('Adding static routes to NFD\n')
    start = int(time.time() * 1000)

    grh = NdnRoutingHelper(ndn.net, 'udp', 'link-state')
    for host in ndn.net.hosts:
        grh.addOrigin([ndn.net[host.name]], ["/ndn/svs/"])

    grh.calculateNPossibleRoutes()

    end = int(time.time() * 1000)
    info('Added static routes to NFD in {} ms\n'.format(end - start))
    info('Sleeping 10 seconds\n')
    time.sleep(3 if DEBUG_GDB else 1)

    for exec_i, sync_exec in enumerate(SYNC_EXEC_VALS):
        for pub_timing in PUB_TIMING_VALS:
            for run_number in RUN_NUMBER_VALS:
                # Set globals
                RUN_NUMBER = run_number
                PUB_TIMING = pub_timing
                SYNC_EXEC = sync_exec
                LOG_MAIN_DIRECTORY = LOG_MAIN_DIRECTORY_VALS[exec_i]

                # Clear content store
                for node in ndn.net.hosts:
                    cmd = 'nfdc cs erase /'
                    node.cmd(cmd)

                    with open("{}/report-start-{}.status".format(getLogPath(), node.name), "w") as f:
                        f.write(node.cmd('nfdc status report'))

                time.sleep(1)

                # random.seed(RUN_NUMBER)
                allowed_hosts = [x for x in ndn.net.hosts if len(x.intfList()) < 8]
                print(allowed_hosts)
                # pub_hosts = random.sample(allowed_hosts, NUM_NODES)

                # ================= SVS BEGIN ====================================

                # identity_app = AppManager(ndn, pub_hosts, IdentityApplication)
                # svs_chat_app = AppManager(ndn, pub_hosts, SvsChatApplication
                svs_chat_app = AppManager(ndn, allowed_hosts, SvsChatApplication)
                # producer_chat_app = AppManager(ndn, allowed_hosts, ProducerChatApplication)

                # =================== SVS END ====================================

                pids = get_pids()
                info("pids: {}\n".format(pids))
                count = count_running(pids)
                wait_itr=0
                while count > 0:
                    wait_itr+=1
                    if wait_itr==3:
                        LOG_NAME = "{}-{}-{}".format(LOG_PREFIX, PUB_TIMING, RUN_NUMBER)
                        dir_path=LOG_MAIN_DIRECTORY+LOG_NAME+"/stdout"
                        files_generated=os.listdir(dir_path)
                        for file in files_generated:
                            with open(dir_path+"/"+file, "r") as f:
                                with open("/home/vagrant/mini-ndn/work/results/"+str(PUB_TIMING)+"-"+file, "w") as output:
                                    for line in f:
                                        output.write(line)
                        for pid in pids:
                            p=psutil.Process(pid)
                            p.terminate()
                        break
                    info("{} nodes are runnning\n".format(count))
                    time.sleep(5)
                    count = count_running(pids)

                for node in ndn.net.hosts:
                    with open("{}/report-end-{}.status".format(getLogPath(), node.name), "w") as f:
                        f.write(node.cmd('nfdc status report'))

    ndn.stop()

    print(LOG_PREFIX, TOPO_FILE, SYNC_EXEC_VALS)
