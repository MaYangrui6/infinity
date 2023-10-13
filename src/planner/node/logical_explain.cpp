//
// Created by jinhai on 23-3-13.
//

#include "logical_explain.h"
#include "planner/column_binding.h"
#include "common/types/data_type.h"

namespace infinity {

Vector<ColumnBinding> LogicalExplain::GetColumnBindings() const { return {}; }

SharedPtr<Vector<String>> LogicalExplain::GetOutputNames() const { return MakeShared<Vector<String>>(); }

SharedPtr<Vector<SharedPtr<DataType>>> LogicalExplain::GetOutputTypes() const { return MakeShared<Vector<SharedPtr<DataType>>>(); }

String LogicalExplain::ToString(i64 &space) const { return "LogicalExplain"; }

} // namespace infinity