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

#include "consensus/base/status.h"

#include <yaraft/memory_storage.h>
#include <yaraft/pb/raftpb.pb.h>

namespace consensus {
namespace wal {

using PBEntryVec = std::vector<yaraft::pb::Entry>;
using PBEntriesIterator = PBEntryVec::iterator;
using ConstPBEntriesIterator = PBEntryVec::const_iterator;

class WriteAheadLogOptions {
 public:
  // Approximate size of wal packed per segment.
  // Default: 64MB
  size_t log_segment_size;

  // Whether to verify the checksum of each entry during recovery.
  // Default: true
  bool verify_checksum;

  std::string log_dir;

  WriteAheadLogOptions();
};

class WriteAheadLog;
using WriteAheadLogUPtr = std::unique_ptr<WriteAheadLog>;

// WriteAheadLog provides an abstraction for writing log entries and raft state
// into the underlying storage.
class WriteAheadLog {
 public:
  virtual ~WriteAheadLog() = default;

  // Save log entries and raft state into underlying storage.
  virtual Status Write(const PBEntryVec& vec, const yaraft::pb::HardState* hs) = 0;

  // write log entries only
  Status Write(const PBEntryVec& vec) {
    return Write(vec, nullptr);
  }

  // write hard state only
  Status Write(const yaraft::pb::HardState* hs) {
    return Write(PBEntryVec(), hs);
  }

  virtual Status Sync() = 0;

  virtual Status Close() = 0;

  struct CompactionHint {};

  // Abandon the unused logs.
  virtual Status GC(CompactionHint* hint) = 0;

  // Default implementation of WAL.
  static Status Default(const WriteAheadLogOptions& options, WriteAheadLogUPtr* wal,
                        yaraft::MemStoreUptr* memstore);
};

extern WriteAheadLogUPtr TEST_CreateWalStore(const std::string& testDir, yaraft::MemStoreUptr* pMemstore);

inline WriteAheadLogUPtr TEST_CreateWalStore(const std::string& testDir) {
  yaraft::MemStoreUptr memstore;
  return TEST_CreateWalStore(testDir, &memstore);
}

}  // namespace wal
}  // namespace consensus