// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "vec/aggregate_functions/aggregate_function_topn.h"

#include <fmt/format.h>
#include <glog/logging.h>

#include "vec/aggregate_functions/helpers.h"
#include "vec/core/types.h"
#include "vec/data_types/data_type.h"

namespace doris::vectorized {
#include "common/compile_check_begin.h"

AggregateFunctionPtr create_aggregate_function_topn(const std::string& name,
                                                    const DataTypes& argument_types,
                                                    const bool result_is_nullable,
                                                    const AggregateFunctionAttr& attr) {
    if (argument_types.size() == 2) {
        return creator_without_type::create<AggregateFunctionTopN<AggregateFunctionTopNImplInt>>(
                argument_types, result_is_nullable);
    } else if (argument_types.size() == 3) {
        return creator_without_type::create<AggregateFunctionTopN<AggregateFunctionTopNImplIntInt>>(
                argument_types, result_is_nullable);
    }
    return nullptr;
}

template <template <typename, bool> class AggregateFunctionTemplate, bool has_default_param,
          bool is_weighted>
AggregateFunctionPtr create_topn_array(const DataTypes& argument_types,
                                       const bool result_is_nullable) {
    switch (argument_types[0]->get_primitive_type()) {
    case PrimitiveType::TYPE_BOOLEAN:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<UInt8, has_default_param>, UInt8, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_TINYINT:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Int8, has_default_param>, Int8, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_SMALLINT:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Int16, has_default_param>, Int16, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_INT:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Int32, has_default_param>, Int32, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_BIGINT:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Int64, has_default_param>, Int64, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_LARGEINT:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Int128, has_default_param>, Int128, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_FLOAT:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Float32, has_default_param>, Float32, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DOUBLE:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Float64, has_default_param>, Float64, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DECIMAL32:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Decimal32, has_default_param>, Decimal32, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DECIMAL64:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Decimal64, has_default_param>, Decimal64, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DECIMAL128I:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Decimal128V3, has_default_param>, Decimal128V3,
                is_weighted>>(argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DECIMALV2:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Decimal128V2, has_default_param>, Decimal128V2,
                is_weighted>>(argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DECIMAL256:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Decimal256, has_default_param>, Decimal256, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_STRING:
    case PrimitiveType::TYPE_CHAR:
    case PrimitiveType::TYPE_VARCHAR:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<std::string, has_default_param>, std::string,
                is_weighted>>(argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DATE:
    case PrimitiveType::TYPE_DATETIME:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<Int64, has_default_param>, Int64, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DATEV2:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<UInt32, has_default_param>, UInt32, is_weighted>>(
                argument_types, result_is_nullable);
    case PrimitiveType::TYPE_DATETIMEV2:
        return creator_without_type::create<AggregateFunctionTopNArray<
                AggregateFunctionTemplate<UInt64, has_default_param>, UInt64, is_weighted>>(
                argument_types, result_is_nullable);
    default:
        LOG(WARNING) << fmt::format(
                "Illegal argument  type for aggregate function topn_array is: {}",
                remove_nullable(argument_types[0])->get_name());
        return nullptr;
    }
}

AggregateFunctionPtr create_aggregate_function_topn_array(const std::string& name,
                                                          const DataTypes& argument_types,
                                                          const bool result_is_nullable,
                                                          const AggregateFunctionAttr& attr) {
    bool has_default_param = (argument_types.size() == 3);
    if (has_default_param) {
        return create_topn_array<AggregateFunctionTopNImplArray, true, false>(argument_types,
                                                                              result_is_nullable);
    } else {
        return create_topn_array<AggregateFunctionTopNImplArray, false, false>(argument_types,
                                                                               result_is_nullable);
    }
}

AggregateFunctionPtr create_aggregate_function_topn_weighted(const std::string& name,
                                                             const DataTypes& argument_types,
                                                             const bool result_is_nullable,
                                                             const AggregateFunctionAttr& attr) {
    bool has_default_param = (argument_types.size() == 4);
    if (has_default_param) {
        return create_topn_array<AggregateFunctionTopNImplWeight, true, true>(argument_types,
                                                                              result_is_nullable);
    } else {
        return create_topn_array<AggregateFunctionTopNImplWeight, false, true>(argument_types,
                                                                               result_is_nullable);
    }
}

void register_aggregate_function_topn(AggregateFunctionSimpleFactory& factory) {
    factory.register_function_both("topn", create_aggregate_function_topn);
    factory.register_function_both("topn_array", create_aggregate_function_topn_array);
    factory.register_function_both("topn_weighted", create_aggregate_function_topn_weighted);
}

} // namespace doris::vectorized