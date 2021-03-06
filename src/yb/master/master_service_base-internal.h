// Copyright (c) YugaByte, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.
//

#ifndef YB_MASTER_MASTER_SERVICE_BASE_INTERNAL_H
#define YB_MASTER_MASTER_SERVICE_BASE_INTERNAL_H

#include "yb/master/master_service_base.h"
#include "yb/master/catalog_manager.h"
#include "yb/rpc/rpc_context.h"

namespace yb {
namespace master {

// Template member function definitions must go into a header file.

template<class RespClass>
void MasterServiceBase::CheckRespErrorOrSetUnknown(const Status& s, RespClass* resp) {
  if (PREDICT_FALSE(!s.ok() && !resp->has_error())) {
    StatusToPB(s, resp->mutable_error()->mutable_status());
    resp->mutable_error()->set_code(MasterErrorPB::UNKNOWN_ERROR);
  }
}

template <class ReqType, class RespType, class FnType>
void MasterServiceBase::HandleOnLeader(const ReqType* req,
                                       RespType* resp,
                                       rpc::RpcContext* rpc,
                                       FnType f) {
  CatalogManager::ScopedLeaderSharedLock l(server_->catalog_manager());
  if (!l.CheckIsInitializedAndIsLeaderOrRespond(resp, rpc)) {
    return;
  }

  const Status s = f();
  CheckRespErrorOrSetUnknown(s, resp);
  rpc->RespondSuccess();
}

template <class HandlerType, class ReqType, class RespType>
void MasterServiceBase::HandleIn(const ReqType* req,
                                 RespType* resp,
                                 rpc::RpcContext* rpc,
                                 Status (HandlerType::*f)(RespType*)) {
  HandleOnLeader(req, resp, rpc, [=]() -> Status {
      return (handler(static_cast<HandlerType*>(nullptr))->*f)(resp); });
}

template <class HandlerType, class ReqType, class RespType>
void MasterServiceBase::HandleIn(const ReqType* req,
                                 RespType* resp,
                                 rpc::RpcContext* rpc,
                                 Status (HandlerType::*f)(const ReqType*, RespType*)) {
  HandleOnLeader(req, resp, rpc, [=]() -> Status {
      return (handler(static_cast<HandlerType*>(nullptr))->*f)(req, resp); });
}

template <class HandlerType, class ReqType, class RespType>
void MasterServiceBase::HandleIn(const ReqType* req,
                                 RespType* resp,
                                 rpc::RpcContext* rpc,
                                 Status (HandlerType::*f)(
                                     const ReqType*, RespType*, rpc::RpcContext*)) {
  HandleOnLeader(req, resp, rpc, [=]() -> Status {
      return (handler(static_cast<HandlerType*>(nullptr))->*f)(req, resp, rpc); });
}

} // namespace master
} // namespace yb

#endif // YB_MASTER_MASTER_SERVICE_BASE_INTERNAL_H
