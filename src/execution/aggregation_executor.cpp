//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.cpp
//
// Identification: src/execution/aggregation_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>
#include <vector>

#include "execution/executors/aggregation_executor.h"

namespace bustub {

AggregationExecutor::AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                                         std::unique_ptr<AbstractExecutor> &&child)
    : AbstractExecutor(exec_ctx), aht_{plan->GetAggregates(), plan->GetAggregateTypes()}, aht_iterator_(aht_.Begin()) {
    plan_ = plan;
    child_ = std::move(child);
}

void AggregationExecutor::Init() {
    child_->Init();
    Tuple tup;
    RID rid;
    while (child_->Next(&tup, &rid)) {
        auto key = MakeAggregateKey(&tup);
        auto value = MakeAggregateValue(&tup);
        aht_.InsertCombine(key, value);
    }
    aht_iterator_ = aht_.Begin();
}

auto AggregationExecutor::Next(Tuple *tuple, RID *rid) -> bool {
    if (aht_iterator_ == aht_.End()) { return false; }

    while (aht_iterator_ != aht_.End()) {
        if (plan_->GetHaving() == nullptr ||
        plan_->GetHaving()->EvaluateAggregate(aht_iterator_.Key().group_bys_, aht_iterator_.Val().aggregates_).GetAs<bool>()) {
            std::vector<Value> values;
            for (auto col : GetOutputSchema()->GetColumns()) {
                values.emplace_back(col.GetExpr()->EvaluateAggregate(aht_iterator_.Key().group_bys_, aht_iterator_.Val().aggregates_));
            }
            *tuple = Tuple(values, GetOutputSchema());
            ++aht_iterator_;
            return true;
        }
        ++aht_iterator_;
    }
    return false;
}

auto AggregationExecutor::GetChildExecutor() const -> const AbstractExecutor * { return child_.get(); }

}  // namespace bustub
