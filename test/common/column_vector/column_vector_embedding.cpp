//
// Created by JinHai on 2022/12/6.
//


#include <gtest/gtest.h>
#include "base_test.h"
#include "common/column_vector/column_vector.h"
#include "common/types/value.h"
#include "main/logger.h"
#include "main/stats/global_resource_usage.h"
#include "common/types/info/embedding_info.h"

class ColumnVectorEmbeddingTest : public BaseTest {
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

TEST_F(ColumnVectorEmbeddingTest, flat_embedding) {
    using namespace infinity;

    auto embedding_info = EmbeddingInfo::Make(EmbeddingDataType::kElemFloat, 16);
    DataType data_type(LogicalType::kEmbedding, embedding_info);
    ColumnVector column_vector(data_type);
    column_vector.Initialize();

    EXPECT_THROW(column_vector.SetDataType(DataType(LogicalType::kEmbedding)), TypeException);
    EXPECT_THROW(column_vector.SetVectorType(ColumnVectorType::kFlat), TypeException);

    EXPECT_EQ(column_vector.capacity(), DEFAULT_VECTOR_SIZE);
    EXPECT_EQ(column_vector.Size(), 0);
    EXPECT_THROW(column_vector.ToString(), TypeException);
    EXPECT_THROW(column_vector.GetValue(0), TypeException);
    EXPECT_EQ(column_vector.tail_index_, 0);
    EXPECT_EQ(column_vector.data_type_size_, 4 * 16);
    EXPECT_NE(column_vector.data_ptr_, nullptr);
    EXPECT_EQ(column_vector.vector_type(), ColumnVectorType::kFlat);
    EXPECT_EQ(column_vector.data_type(), data_type);
    EXPECT_EQ(column_vector.buffer_->buffer_type_, VectorBufferType::kStandard);

    EXPECT_NE(column_vector.buffer_, nullptr);
    EXPECT_NE(column_vector.nulls_ptr_, nullptr);
    EXPECT_TRUE(column_vector.initialized);
    column_vector.Reserve(DEFAULT_VECTOR_SIZE - 1);
    auto tmp_ptr = column_vector.data_ptr_;
    EXPECT_EQ(column_vector.capacity(), DEFAULT_VECTOR_SIZE);
    EXPECT_EQ(tmp_ptr, column_vector.data_ptr_);

    for(i64 i = 0; i < DEFAULT_VECTOR_SIZE; ++ i) {
        Value v = Value::MakeEmbedding(embedding_info->Type(), embedding_info->Dimension());
        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            ((float*)(v.value_.embedding.ptr))[j] = static_cast<float>(i) + static_cast<float>(j) + 0.5f;
        }
        column_vector.AppendValue(v);
        Value vx = column_vector.GetValue(i);
        EXPECT_EQ(vx.type().type(), LogicalType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->type(), TypeInfoType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->Size(), 64);

        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            EXPECT_FLOAT_EQ(((float*)(vx.value_.embedding.ptr))[j], ((float*)(v.value_.embedding.ptr))[j]);
        }

        v.value_.embedding.Reset();
        EXPECT_THROW(column_vector.GetValue(i + 1), TypeException);
    }

    column_vector.Reserve(DEFAULT_VECTOR_SIZE* 2);

    ColumnVector clone_column_vector(data_type);
    clone_column_vector.ShallowCopy(column_vector);
    EXPECT_EQ(column_vector.tail_index_, clone_column_vector.tail_index_);
    EXPECT_EQ(column_vector.capacity_, clone_column_vector.capacity_);
    EXPECT_EQ(column_vector.data_type_, clone_column_vector.data_type_);
    EXPECT_EQ(column_vector.data_ptr_, clone_column_vector.data_ptr_);
    EXPECT_EQ(column_vector.data_type_size_, clone_column_vector.data_type_size_);
    EXPECT_EQ(column_vector.nulls_ptr_, clone_column_vector.nulls_ptr_);
    EXPECT_EQ(column_vector.buffer_, clone_column_vector.buffer_);
    EXPECT_EQ(column_vector.initialized, clone_column_vector.initialized);
    EXPECT_EQ(column_vector.vector_type_, clone_column_vector.vector_type_);

    for(i64 i = 0; i < DEFAULT_VECTOR_SIZE; ++ i) {
        Value v = Value::MakeEmbedding(embedding_info->Type(), embedding_info->Dimension());
        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            ((float*)(v.value_.embedding.ptr))[j] = static_cast<float>(i) + static_cast<float>(j) + 0.5f;
        }

        Value vx = column_vector.GetValue(i);
        EXPECT_EQ(vx.type().type(), LogicalType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->type(), TypeInfoType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->Size(), 64);

        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            EXPECT_FLOAT_EQ(((float*)(vx.value_.embedding.ptr))[j], ((float*)(v.value_.embedding.ptr))[j]);
        }

        v.value_.embedding.Reset();
    }

    EXPECT_EQ(column_vector.tail_index_, DEFAULT_VECTOR_SIZE);
    EXPECT_EQ(column_vector.capacity(), 2* DEFAULT_VECTOR_SIZE);

    for(i64 i = DEFAULT_VECTOR_SIZE; i < 2 * DEFAULT_VECTOR_SIZE; ++ i) {
        Value v = Value::MakeEmbedding(embedding_info->Type(), embedding_info->Dimension());
        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            ((float*)(v.value_.embedding.ptr))[j] = static_cast<float>(i) + static_cast<float>(j) + 0.5f;
        }
        column_vector.AppendValue(v);
        Value vx = column_vector.GetValue(i);
        EXPECT_EQ(vx.type().type(), LogicalType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->type(), TypeInfoType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->Size(), 64);

        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            EXPECT_FLOAT_EQ(((float*)(vx.value_.embedding.ptr))[j], ((float*)(v.value_.embedding.ptr))[j]);
        }

        v.value_.embedding.Reset();
        EXPECT_THROW(column_vector.GetValue(i + 1), TypeException);
    }

    column_vector.Reset();
    EXPECT_EQ(column_vector.capacity(), 0);
    EXPECT_EQ(column_vector.tail_index_, 0);
    EXPECT_NE(column_vector.buffer_, nullptr);
    EXPECT_EQ(column_vector.buffer_->heap_mgr_, nullptr);
    EXPECT_NE(column_vector.data_ptr_, nullptr);
    EXPECT_EQ(column_vector.initialized, false);

    // ====
    column_vector.Initialize();
    EXPECT_THROW(column_vector.SetDataType(DataType(LogicalType::kDecimal128)), TypeException);
    EXPECT_THROW(column_vector.SetVectorType(ColumnVectorType::kFlat), TypeException);

    EXPECT_EQ(column_vector.capacity(), DEFAULT_VECTOR_SIZE);
    EXPECT_EQ(column_vector.Size(), 0);
    EXPECT_THROW(column_vector.ToString(), TypeException);
    EXPECT_THROW(column_vector.GetValue(0), TypeException);
    EXPECT_EQ(column_vector.tail_index_, 0);
    EXPECT_EQ(column_vector.data_type_size_, 16 * 4);
    EXPECT_NE(column_vector.data_ptr_, nullptr);
    EXPECT_EQ(column_vector.vector_type(), ColumnVectorType::kFlat);
    EXPECT_EQ(column_vector.data_type(), data_type);
    EXPECT_EQ(column_vector.buffer_->buffer_type_, VectorBufferType::kStandard);

    EXPECT_NE(column_vector.buffer_, nullptr);
    EXPECT_NE(column_vector.nulls_ptr_, nullptr);
    EXPECT_TRUE(column_vector.initialized);
    column_vector.Reserve(DEFAULT_VECTOR_SIZE - 1);
    tmp_ptr = column_vector.data_ptr_;
    EXPECT_EQ(column_vector.capacity(), DEFAULT_VECTOR_SIZE);
    EXPECT_EQ(tmp_ptr, column_vector.data_ptr_);
    for(i64 i = 0; i < DEFAULT_VECTOR_SIZE; ++ i) {
        Value v = Value::MakeEmbedding(embedding_info->Type(), embedding_info->Dimension());
        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            ((float*)(v.value_.embedding.ptr))[j] = static_cast<float>(i) + static_cast<float>(j) + 0.5f;
        }
        column_vector.AppendValue(v);
        Value vx = column_vector.GetValue(i);
        EXPECT_EQ(vx.type().type(), LogicalType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->type(), TypeInfoType::kEmbedding);
        EXPECT_EQ(vx.type().type_info()->Size(), 64);

        for(i64 j = 0; j < embedding_info->Dimension(); ++ j) {
            EXPECT_FLOAT_EQ(((float*)(vx.value_.embedding.ptr))[j], ((float*)(v.value_.embedding.ptr))[j]);
        }

        v.value_.embedding.Reset();
        EXPECT_THROW(column_vector.GetValue(i + 1), TypeException);
    }
}