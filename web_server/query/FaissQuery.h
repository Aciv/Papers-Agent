#pragma once

#include <faiss/Index.h>
#include <faiss/index_io.h>
#include <faiss/IndexIVF.h>

#include <vector>
#include <string>
#include <memory>
#include <mutex>

namespace VectorQuery {

// 查询结果结构体
struct QueryResult {
    std::vector<faiss::idx_t> ids;      // 最近邻向量的ID
    std::vector<float> distances;        // 对应的距离
    int dimension;                        // 向量维度
    int k;                               // 返回的结果数量
    bool success;                        // 查询是否成功
    std::string errorMsg;                // 错误信息
};

// Faiss向量查询类
class FaissQuery {
public:
    // 获取单例实例
    static FaissQuery& getInstance();

    // 禁用拷贝和赋值
    FaissQuery(const FaissQuery&) = delete;
    FaissQuery& operator=(const FaissQuery&) = delete;

    // 初始化：加载索引文件
    bool initialize(const std::string& indexPath);

    // 检查是否已初始化
    bool isInitialized() const;

    // 获取向量维度
    int getDimension() const;

    // 获取索引中的向量数量
    size_t getIndexSize() const;

    // 执行向量查询
    // @param queryVector: 查询向量
    // @param vector_ids: 指定要查询的向量ID列表
    // @param k: 返回的最近邻数量
    // @param nprobe: IVF索引的探测数（可选，仅对IVF索引有效）
    QueryResult search(const std::vector<float>& queryVector, const std::vector<int64_t>& vector_ids, int k, int nprobe = 10);

    // 批量查询
    // @param queryVectors: 多个查询向量（展平的一维数组）
    // @param vector_ids: 指定要查询的向量ID列表
    // @param numQueries: 查询向量的数量
    // @param k: 每个查询返回的最近邻数量
    // @param nprobe: IVF索引的探测数
    QueryResult batchSearch(const std::vector<float>& queryVectors, const std::vector<int64_t>& vector_ids, int numQueries, int k, int nprobe = 10);

    // 释放索引资源
    void release();

private:
    FaissQuery() = default;
    ~FaissQuery();

    std::unique_ptr<faiss::Index> index_;
    mutable std::mutex mutex_;
    bool initialized_ = false;
    std::string indexPath_;
};

} // namespace VectorQuery
