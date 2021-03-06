//--------------------------------------------------------------------------------------------------
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
//--------------------------------------------------------------------------------------------------

#ifndef YB_YQL_PGGATE_PG_DML_WRITE_H_
#define YB_YQL_PGGATE_PG_DML_WRITE_H_

#include "yb/yql/pggate/pg_dml.h"

namespace yb {
namespace pggate {

//--------------------------------------------------------------------------------------------------
// DML WRITE - Insert, Update, Delete.
//--------------------------------------------------------------------------------------------------

class PgDmlWrite : public PgDml {
 public:
  // Abstract class without constructors.
  virtual ~PgDmlWrite();

  // Prepare write operations.
  virtual CHECKED_STATUS Prepare();

  // Setup internal structures for binding values during prepare.
  void PrepareColumns();

  // Execute.
  CHECKED_STATUS Exec();

 protected:
  // Constructor.
  PgDmlWrite(PgSession::ScopedRefPtr pg_session, const PgObjectId& table_id, StmtOp stmt_op);

  // Allocate write request.
  virtual void AllocWriteRequest() = 0;

  // Allocate column expression.
  PgsqlExpressionPB *AllocColumnBindPB(PgColumn *col) override;

  // Allocate target for selected or returned expressions.
  PgsqlExpressionPB *AllocTargetPB() override;

  // Allocate column expression.
  PgsqlExpressionPB *AllocColumnAssignPB(PgColumn *col) override;

  // Delete allocated target for columns that have no bind-values.
  CHECKED_STATUS DeleteEmptyPrimaryBinds();

  // Protobuf code.
  PgsqlWriteRequestPB *write_req_ = nullptr;
};

}  // namespace pggate
}  // namespace yb

#endif // YB_YQL_PGGATE_PG_DML_WRITE_H_
