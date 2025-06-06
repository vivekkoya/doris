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
// This file is copied from
// https://github.com/ClickHouse/ClickHouse/blob/master/src/DataTypes/DataTypeNumber.h
// and modified by Doris

#pragma once

#include "vec/columns/column_string.h"
#include "vec/data_types/data_type_number_base.h"

namespace doris::vectorized {

template <typename T>
class DataTypeNumber final : public DataTypeNumberBase<T> {
public:
    using ColumnType = ColumnVector<T>;
    using FieldType = T;
    bool equals(const IDataType& rhs) const override { return typeid(rhs) == typeid(*this); }

    void to_string_batch(const IColumn& column, ColumnString& column_to) const final {
        DataTypeNumberBase<T>::template to_string_batch_impl<DataTypeNumber<T>>(column, column_to);
    }

    size_t number_length() const;
    void push_number(ColumnString::Chars& chars, const T& num) const;
};

using DataTypeUInt8 = DataTypeNumber<UInt8>;
using DataTypeUInt16 = DataTypeNumber<UInt16>;
using DataTypeUInt32 = DataTypeNumber<UInt32>;
using DataTypeUInt64 = DataTypeNumber<UInt64>;
using DataTypeUInt128 = DataTypeNumber<UInt128>;
using DataTypeInt8 = DataTypeNumber<Int8>;
using DataTypeInt16 = DataTypeNumber<Int16>;
using DataTypeInt32 = DataTypeNumber<Int32>;
using DataTypeInt64 = DataTypeNumber<Int64>;
using DataTypeInt128 = DataTypeNumber<Int128>;
using DataTypeFloat32 = DataTypeNumber<Float32>;
using DataTypeFloat64 = DataTypeNumber<Float64>;
using DataTypeBool = DataTypeUInt8;

template <typename DataType>
constexpr bool IsDataTypeNumber = false;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<UInt8>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<UInt16>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<UInt32>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<UInt64>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<UInt128>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<Int8>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<Int16>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<Int32>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<Int64>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<Int128>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<Float32>> = true;
template <>
inline constexpr bool IsDataTypeNumber<DataTypeNumber<Float64>> = true;

} // namespace doris::vectorized
