//
// Created by jinhai on 22-12-24.
//

#include <gtest/gtest.h>
#include "base_test.h"
#include "common/column_vector/column_vector.h"
#include "common/types/value.h"
#include "main/logger.h"
#include "main/stats/global_resource_usage.h"
#include "function/cast/geography_cast.h"
#include "common/types/info/varchar_info.h"

class PolygonCastTest : public BaseTest {
    void
    SetUp() override {
        infinity::Logger::Initialize();
        infinity::GlobalResourceUsage::Init();
    }

    void
    TearDown() override {
        infinity::Logger::Shutdown();
        EXPECT_EQ(infinity::GlobalResourceUsage::GetObjectCount(), 0);
        EXPECT_EQ(infinity::GlobalResourceUsage::GetRawMemoryCount(), 0);
        infinity::GlobalResourceUsage::UnInit();
    }
};

TEST_F(PolygonCastTest, polygon_cast0) {
    using namespace infinity;

    // Try to cast circle type to wrong type.
    {
        PointT p1(static_cast<f64>(1) + 0.1f, static_cast<f64>(1) - 0.3f);
        PointT p2(static_cast<f64>(1) + 0.5f, static_cast<f64>(1) - 0.7f);
        PointT p3(static_cast<f64>(1) + 0.2f, static_cast<f64>(1) - 0.4f);
        PointT p4(static_cast<f64>(1) + 0.6f, static_cast<f64>(1) - 0.8f);
        PolygonT source;
        source.Initialize(4);
        source.SetPoint(0, p1);
        source.SetPoint(1, p2);
        source.SetPoint(2, p3);
        source.SetPoint(3, p4);
        TinyIntT target;
        EXPECT_THROW(GeographyTryCastToVarlen::Run(source, target, nullptr), FunctionException);
    }
    {
        PointT p1(static_cast<f64>(1) + 0.1f, static_cast<f64>(1) - 0.3f);
        PointT p2(static_cast<f64>(1) + 0.5f, static_cast<f64>(1) - 0.7f);
        PointT p3(static_cast<f64>(1) + 0.2f, static_cast<f64>(1) - 0.4f);
        PointT p4(static_cast<f64>(1) + 0.6f, static_cast<f64>(1) - 0.8f);
        PolygonT source;
        source.Initialize(4);
        source.SetPoint(0, p1);
        source.SetPoint(1, p2);
        source.SetPoint(2, p3);
        source.SetPoint(3, p4);

        VarcharT target;

        auto varchar_info = VarcharInfo::Make(65);
        DataType data_type(LogicalType::kVarchar, varchar_info);
        ColumnVector col_varchar(data_type);
        col_varchar.Initialize();

        EXPECT_THROW(GeographyTryCastToVarlen::Run(source, target, &col_varchar), NotImplementException);
    }
}

TEST_F(PolygonCastTest, polygon_cast1) {
    using namespace infinity;

    // Call BindGeographyCast with wrong type of parameters
    {
        DataType source_type(LogicalType::kPolygon);
        DataType target_type(LogicalType::kDecimal16);
        EXPECT_THROW(BindGeographyCast<PolygonT>(source_type, target_type), TypeException);
    }

    DataType source_type(LogicalType::kPolygon);
    ColumnVector col_source(source_type);
    col_source.Initialize();
    for (i64 i = 0; i < DEFAULT_VECTOR_SIZE; ++ i) {
        PointT p1(static_cast<f64>(i) + 0.1f, static_cast<f64>(i) - 0.3f);
        PointT p2(static_cast<f64>(i) + 0.5f, static_cast<f64>(i) - 0.7f);
        PointT p3(static_cast<f64>(i) + 0.2f, static_cast<f64>(i) - 0.4f);
        PointT p4(static_cast<f64>(i) + 0.6f, static_cast<f64>(i) - 0.8f);
        PolygonT polygon;
        polygon.Initialize(4);
        polygon.SetPoint(0, p1);
        polygon.SetPoint(1, p2);
        polygon.SetPoint(2, p3);
        polygon.SetPoint(3, p4);
        Value v = Value::MakePolygon(polygon);
        col_source.AppendValue(v);
        Value vx = col_source.GetValue(i);
    }
    for (i64 i = 0; i < DEFAULT_VECTOR_SIZE; ++ i) {
        PointT p1(static_cast<f64>(i) + 0.1f, static_cast<f64>(i) - 0.3f);
        PointT p2(static_cast<f64>(i) + 0.5f, static_cast<f64>(i) - 0.7f);
        PointT p3(static_cast<f64>(i) + 0.2f, static_cast<f64>(i) - 0.4f);
        PointT p4(static_cast<f64>(i) + 0.6f, static_cast<f64>(i) - 0.8f);
        BoxT bounding_box(PointT(static_cast<f64>(i) + 0.1f, static_cast<f64>(i) - 0.3f),
                          PointT(static_cast<f64>(i) + 0.6f, static_cast<f64>(i) - 0.8f));

        Value vx = col_source.GetValue(i);
        EXPECT_EQ(vx.type().type(), LogicalType::kPolygon);
        EXPECT_EQ(vx.value_.polygon.point_count, 4);
        EXPECT_EQ(*((PointT*)(vx.value_.polygon.ptr)), p1);
        EXPECT_EQ(*((PointT*)(vx.value_.polygon.ptr) + 1), p2);
        EXPECT_EQ(*((PointT*)(vx.value_.polygon.ptr) + 2), p3);
        EXPECT_EQ(*((PointT*)(vx.value_.polygon.ptr) + 3), p4);
        EXPECT_DOUBLE_EQ(vx.value_.polygon.bounding_box.upper_left.x, bounding_box.upper_left.x);
        EXPECT_DOUBLE_EQ(vx.value_.polygon.bounding_box.upper_left.y, bounding_box.upper_left.y);
        EXPECT_DOUBLE_EQ(vx.value_.polygon.bounding_box.lower_right.x, bounding_box.lower_right.x);
        EXPECT_DOUBLE_EQ(vx.value_.polygon.bounding_box.lower_right.y, bounding_box.lower_right.y);
    }
    // cast circle column vector to varchar column vector
    {
        DataType target_type(LogicalType::kVarchar);
        auto source2target_ptr = BindGeographyCast<PolygonT>(source_type, target_type);
        EXPECT_NE(source2target_ptr.function, nullptr);

        ColumnVector col_target(target_type);
        col_target.Initialize();

        CastParameters cast_parameters;
        EXPECT_THROW(source2target_ptr.function(col_source, col_target, DEFAULT_VECTOR_SIZE, cast_parameters), NotImplementException);
    }
}
