// Copyright 2017 Wu Tao
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <map>
#include <memory>

#include "consensus/base/simple_channel.h"
#include "consensus/base/slice.h"
#include "consensus/base/status.h"
#include "consensus/base/task_queue.h"
#include "consensus/raft_timer.h"
#include "consensus/ready_flusher.h"
#include "consensus/wal/wal.h"

#include <silly/disallow_copying.h>
#include <yaraft/raft_info.h>

namespace consensus {

struct ReplicatedLogOptions {
  // id -> IP
  std::map<uint64_t, std::string> initial_cluster;

  uint64_t id;

  // time (in milliseconds) of a heartbeat interval.
  uint32_t heartbeat_interval;

  // time (in milliseconds) for an election to timeout.
  uint32_t election_timeout;

  // dedicated worker of the raft node.
  // there may have multiple instances sharing the same queue.
  TaskQueue* taskQueue;

  // the global timer
  RaftTimer* timer;

  // the global ready flusher
  ReadyFlusher* flusher;

  wal::WriteAheadLog* wal;
  yaraft::MemoryStorage* memstore;

  ReplicatedLogOptions();

  Status Validate() const;
};

// A ReplicatedLog is a distributed log storage with strong consistency. Every single
// write will be replicated to a majority of nodes if successes.
// If the current node is not leader, the write fails immediately.
//
// Not Thread Safe
class ReplicatedLog {
  __DISALLOW_COPYING__(ReplicatedLog);

 public:
  static StatusWith<ReplicatedLog*> New(const ReplicatedLogOptions&);

  // Write a slice of log in synchronous way.
  // Returns error `WalWriteToNonLeader` if the current node is not leader.
  Status Write(const Slice& log);

  // Asynchronously write a slice of log into storage, the call will returns immediately
  // with a SimpleChannel that's used to wait for the commit of this write.
  SimpleChannel<Status> AsyncWrite(const Slice& log);

  RaftTaskExecutor* RaftTaskExecutorInstance() const;

  yaraft::RaftInfo GetInfo();

  ~ReplicatedLog();

 private:
  ReplicatedLog() = default;

 private:
  friend class ReplicatedLogTest;

  friend class ReplicatedLogImpl;
  std::unique_ptr<ReplicatedLogImpl> impl_;
};

}  // namespace consensus
