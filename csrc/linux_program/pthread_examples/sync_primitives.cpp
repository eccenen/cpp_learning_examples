#include <stdexcept>
#include <system_error>

#include "sync_primitives.h"

// MutexWrapper implementation
void SyncPrimitives::MutexWrapper::lock() {
    mutex_.lock();
}

void SyncPrimitives::MutexWrapper::unlock() {
    mutex_.unlock();
}

bool SyncPrimitives::MutexWrapper::tryLock() {
    return mutex_.try_lock();
}

// RWLockWrapper implementation
void SyncPrimitives::RWLockWrapper::readLock() {
    rw_lock_.lock_shared();
}

void SyncPrimitives::RWLockWrapper::writeLock() {
    rw_lock_.lock();
}

void SyncPrimitives::RWLockWrapper::unlock() {
    rw_lock_.unlock();
}

// CondVarExample implementation
void SyncPrimitives::CondVarExample::produce(int item) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
    queue_.push(item);
    not_empty_.notify_one();
}

int SyncPrimitives::CondVarExample::consume() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [this] { return !queue_.empty(); });
    int item = queue_.front();
    queue_.pop();
    not_full_.notify_one();
    return item;
}

// SemaphoreWrapper implementation
SyncPrimitives::SemaphoreWrapper::SemaphoreWrapper(unsigned int value) {
    if (sem_init(&sem_, 0, value) != 0) {
        throw std::system_error(errno, std::system_category(), "sem_init failed");
    }
}

SyncPrimitives::SemaphoreWrapper::~SemaphoreWrapper() {
    sem_destroy(&sem_);
}

void SyncPrimitives::SemaphoreWrapper::wait() {
    if (sem_wait(&sem_) != 0) {
        throw std::system_error(errno, std::system_category(), "sem_wait failed");
    }
}

void SyncPrimitives::SemaphoreWrapper::post() {
    if (sem_post(&sem_) != 0) {
        throw std::system_error(errno, std::system_category(), "sem_post failed");
    }
}

// BarrierWrapper implementation
SyncPrimitives::BarrierWrapper::BarrierWrapper(size_t count) : threshold_(count), count_(count) {}

void SyncPrimitives::BarrierWrapper::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (--count_ == 0) {
        count_ = threshold_;
        cv_.notify_all();
    } else {
        cv_.wait(lock, [this] { return count_ == threshold_; });
    }
}
