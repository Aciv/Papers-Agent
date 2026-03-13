#include "FaissQuery.h"
#include "../log_wrapper.h"

#include <stdexcept>
#include <fstream>

namespace VectorQuery {

FaissQuery& FaissQuery::getInstance() {
    static FaissQuery instance;
    return instance;
}

FaissQuery::~FaissQuery() {
    release();
}

bool FaissQuery::initialize(const std::string& indexPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        APP_LOG_WARN("FaissQuery already initialized, re-initializing...");
        index_.reset();
    }

    try {
        // 检查文件是否存在
        std::ifstream file(indexPath);
        if (!file.good()) {
            APP_LOG_ERROR_FMT("Index file not found: %s", indexPath.c_str());
            return false;
        }
        file.close();

        // 加载faiss索引
        faiss::Index* rawIndex = faiss::read_index(indexPath.c_str());
        if (!rawIndex) {
            APP_LOG_ERROR("Failed to read faiss index");
            return false;
        }

        index_.reset(rawIndex);
        indexPath_ = indexPath;
        initialized_ = true;

        APP_LOG_INFO_FMT("FaissQuery initialized successfully. Dimension: %d, Index size: %zu", 
                         index_->d, index_->ntotal);

        return true;

    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("Failed to initialize FaissQuery: %s", e.what());
        return false;
    }
}

bool FaissQuery::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

int FaissQuery::getDimension() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !index_) {
        return 0;
    }
    return index_->d;
}

size_t FaissQuery::getIndexSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !index_) {
        return 0;
    }
    return index_->ntotal;
}

QueryResult FaissQuery::search(const std::vector<float>& queryVector, const std::vector<int64_t>& vector_ids, int k, int nprobe) {
    QueryResult result;
    result.success = false;

    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_ || !index_) {
        result.errorMsg = "FaissQuery not initialized";
        APP_LOG_ERROR("FaissQuery not initialized");
        return result;
    }

    // 验证查询向量维度
    if (static_cast<int>(queryVector.size()) != index_->d) {
        result.errorMsg = "Query vector dimension mismatch. Expected: " + 
                          std::to_string(index_->d) + ", Got: " + 
                          std::to_string(queryVector.size());
        APP_LOG_ERROR_FMT("Dimension mismatch: expected %d, got %zu", 
                          index_->d, queryVector.size());
        return result;
    }

    // 验证k值
    if (k <= 0) {
        result.errorMsg = "k must be positive";
        return result;
    }

    // 调整k不超过索引大小
    k = std::min(k, static_cast<int>(index_->ntotal));

    try {

        // 分配结果空间
        result.ids.resize(k);
        result.distances.resize(k);
        result.dimension = index_->d;
        result.k = k;

        // 执行搜索
        if(vector_ids.size() > 0) {
            faiss::IDSelectorBatch sel(vector_ids.size(), vector_ids.data());
            faiss::SearchParametersIVF search_params;
            search_params.sel = &sel;          // 关联我们创建的 ID 选择器
            search_params.nprobe = nprobe;    // 设置 nprobe 参数

            index_->search(1, queryVector.data(), k, result.distances.data(), result.ids.data(), &search_params);

        }
        else{
            faiss::IndexIVF* ivfIndex = dynamic_cast<faiss::IndexIVF*>(index_.get());
            if (ivfIndex) {
                ivfIndex->nprobe = nprobe;
            }

            index_->search(1, queryVector.data(), k, result.distances.data(), result.ids.data());
        }


        result.success = true;
        APP_LOG_DEBUG_FMT("Search completed. Found %d neighbors", k);

    } catch (const std::exception& e) {
        result.errorMsg = std::string("Search failed: ") + e.what();
        APP_LOG_ERROR_FMT("Search failed: %s", e.what());
    }

    return result;
}

QueryResult FaissQuery::batchSearch(const std::vector<float>& queryVectors, const std::vector<int64_t>& vector_ids, int numQueries, int k, int nprobe) {
    QueryResult result;
    result.success = false;

    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_ || !index_) {
        result.errorMsg = "FaissQuery not initialized";
        APP_LOG_ERROR("FaissQuery not initialized");
        return result;
    }

    // 验证输入
    size_t expectedSize = static_cast<size_t>(numQueries) * index_->d;
    if (queryVectors.size() != expectedSize) {
        result.errorMsg = "Query vectors size mismatch. Expected: " + 
                          std::to_string(expectedSize) + ", Got: " + 
                          std::to_string(queryVectors.size());
        return result;
    }

    if (k <= 0 || numQueries <= 0) {
        result.errorMsg = "k and numQueries must be positive";
        return result;
    }

    // 调整k
    k = std::min(k, static_cast<int>(index_->ntotal));

    try {

        // 分配结果空间
        size_t totalResults = static_cast<size_t>(numQueries) * k;
        result.ids.resize(totalResults);
        result.distances.resize(totalResults);
        result.dimension = index_->d;
        result.k = k;

        // 执行批量搜索
        if(vector_ids.size() > 0) {
            faiss::IDSelectorBatch sel(vector_ids.size(), vector_ids.data());
            faiss::SearchParametersIVF search_params;
            search_params.sel = &sel;          // 关联我们创建的 ID 选择器
            search_params.nprobe = nprobe;    // 设置 nprobe 参数

            index_->search(numQueries, queryVectors.data(), k, result.distances.data(), result.ids.data(), &search_params);

        }
        else{
            faiss::IndexIVF* ivfIndex = dynamic_cast<faiss::IndexIVF*>(index_.get());
            if (ivfIndex) {
                ivfIndex->nprobe = nprobe;
            }

            index_->search(numQueries, queryVectors.data(), k, result.distances.data(), result.ids.data());
        }

        result.success = true;
        APP_LOG_DEBUG_FMT("Batch search completed. Queries: %d, k: %d", numQueries, k);

    } catch (const std::exception& e) {
        result.errorMsg = std::string("Batch search failed: ") + e.what();
        APP_LOG_ERROR_FMT("Batch search failed: %s", e.what());
    }

    return result;
}

void FaissQuery::release() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (index_) {
        index_.reset();
        APP_LOG_INFO("FaissQuery index released");
    }
    initialized_ = false;
    indexPath_.clear();
}

} // namespace VectorQuery
