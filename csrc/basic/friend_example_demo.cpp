// Copyright 2025 The cpp-qa-lab Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "common.h"

namespace cpp_qa_lab {
namespace basic {

// ============================================================================
// 1. 友元函数示例 - 访问私有成员
// ============================================================================

class BankAccount {
  public:
    explicit BankAccount(std::string account_id, double balance = 0.0) :
        account_id_(std::move(account_id)),
        balance_(balance) {}

    // 友元函数声明 - 可以访问私有成员
    friend void   PrintAccountDetails(const BankAccount & account);
    friend void   TransferMoney(BankAccount & from, BankAccount & to, double amount);
    friend double GetBalance(const BankAccount & account);

    // 友元类声明
    friend class AccountManager;

    // 公共接口
    const std::string & GetAccountId() const { return account_id_; }

    void Deposit(double amount);
    bool Withdraw(double amount);

  private:
    std::string account_id_;
    double      balance_;
};

// 友元函数 - 打印账户详情
void PrintAccountDetails(const BankAccount & account);

// 友元函数 - 转账功能
void TransferMoney(BankAccount & from, BankAccount & to, double amount);

// 友元函数 - 获取余额
double GetBalance(const BankAccount & account);

// 实现：BankAccount 与相关友元函数（紧跟声明）
void BankAccount::Deposit(double amount) {
    if (amount > 0) {
        balance_ += amount;
        spdlog::info("Deposited ${} to account {}. New balance: ${}", amount, account_id_,
                     balance_);
    }
}

bool BankAccount::Withdraw(double amount) {
    if (amount > 0 && balance_ >= amount) {
        balance_ -= amount;
        spdlog::info("Withdrew ${} from account {}. New balance: ${}", amount, account_id_,
                     balance_);
        return true;
    }
    spdlog::warn("Insufficient funds in account {}", account_id_);
    return false;
}

void PrintAccountDetails(const BankAccount & account) {
    // 友元函数可以访问私有成员
    spdlog::info("=== Account Details ===");
    spdlog::info("Account ID: {}", account.account_id_);
    spdlog::info("Balance: ${}", account.balance_);
    spdlog::info("======================");
}

void TransferMoney(BankAccount & from, BankAccount & to, double amount) {
    if (amount <= 0) {
        spdlog::warn("Invalid transfer amount");
        return;
    }

    if (from.balance_ < amount) {
        spdlog::warn("Insufficient funds for transfer");
        return;
    }

    // 直接访问私有成员进行转账
    from.balance_ -= amount;
    to.balance_ += amount;

    spdlog::info("Transferred ${} from {} to {}", amount, from.account_id_, to.account_id_);
}

double GetBalance(const BankAccount & account) {
    // 友元函数可以访问私有成员
    return account.balance_;
}

// ============================================================================
// 2. 友元类示例 - 账户管理器
// ============================================================================

class AccountManager {
  public:
    explicit AccountManager(std::string manager_id) : manager_id_(std::move(manager_id)) {}

    // 作为 BankAccount 的友元类，可以访问其私有成员
    void AuditAccount(const BankAccount & account) const;
    void FreezeAccount(BankAccount & account);
    void UnfreezeAccount(BankAccount & account);
    void SetBalance(BankAccount & account, double new_balance);

    // 管理多个账户
    void AddAccount(BankAccount * account);
    void RemoveAccount(const std::string & account_id);
    void PrintAllAccounts() const;

  private:
    std::string                manager_id_;
    std::vector<BankAccount *> managed_accounts_;
    bool                       is_frozen_ = false;
};

// 实现：AccountManager 方法（紧跟声明）
void AccountManager::AuditAccount(const BankAccount & account) const {
    // 作为友元类，可以访问 BankAccount 的私有成员
    spdlog::info("=== Account Audit ===");
    spdlog::info("Manager: {}", manager_id_);
    spdlog::info("Account ID: {}", account.account_id_);
    spdlog::info("Current Balance: ${}", account.balance_);
    spdlog::info("Account Status: {}", (is_frozen_ ? "Frozen" : "Active"));
    spdlog::info("====================");
}

void AccountManager::FreezeAccount(BankAccount & account) {
    // 模拟冻结账户 - 在实际应用中可能设置状态标志
    spdlog::info("Account {} has been frozen by manager {}", account.account_id_, manager_id_);
    is_frozen_ = true;
}

void AccountManager::UnfreezeAccount(BankAccount & account) {
    spdlog::info("Account {} has been unfrozen by manager {}", account.account_id_, manager_id_);
    is_frozen_ = false;
}

void AccountManager::SetBalance(BankAccount & account, double new_balance) {
    // 作为友元类，可以直接修改私有成员
    spdlog::info("Manager {} changed balance of account {} from ${} to {}", manager_id_,
                 account.account_id_, account.balance_, new_balance);
    account.balance_ = new_balance;
}

void AccountManager::AddAccount(BankAccount * account) {
    if (account != nullptr) {
        managed_accounts_.push_back(account);
        spdlog::info("Account {} added to manager {}", account->GetAccountId(), manager_id_);
    }
}

void AccountManager::RemoveAccount(const std::string & account_id) {
    auto it = std::find_if(
        managed_accounts_.begin(), managed_accounts_.end(),
        [&account_id](const BankAccount * acc) { return acc->GetAccountId() == account_id; });

    if (it != managed_accounts_.end()) {
        spdlog::info("Account {} removed from manager {}", account_id, manager_id_);
        managed_accounts_.erase(it);
    }
}

void AccountManager::PrintAllAccounts() const {
    spdlog::info("=== Accounts managed by {} ===", manager_id_);
    for (const auto * account : managed_accounts_) {
        spdlog::info("Account: {}, Balance: ${}", account->GetAccountId(), GetBalance(*account));
    }
    spdlog::info("==============================================");
}

// ============================================================================
// 3. 友元成员函数示例 - 部分访问控制
// ============================================================================

class Logger {
  public:
    static void LogInfo(const std::string & message);
    static void LogError(const std::string & message);

  private:
    static std::string GetTimestamp();
    static std::string FormatMessage(const std::string & level, const std::string & message);
};

class SystemMonitor {
  public:
    SystemMonitor() = default;

    // 只有 Logger 的特定成员函数是友元
    friend void Logger::LogError(const std::string & message);

    void ReportCriticalError(const std::string & error);

  private:
    int                      critical_error_count_ = 0;
    std::vector<std::string> error_history_;
};

// 实现：Logger / SystemMonitor（紧跟声明）
void Logger::LogInfo(const std::string & message) {
    spdlog::info("{} [INFO] {}", GetTimestamp(), message);
}

void Logger::LogError(const std::string & message) {
    spdlog::warn("{} [ERROR] {}", GetTimestamp(), message);
}

std::string Logger::GetTimestamp() {
    // 简化的时间戳实现
    return "[2025-10-24 12:00:00]";
}

std::string Logger::FormatMessage(const std::string & level, const std::string & message) {
    return GetTimestamp() + " [" + level + "] " + message;
}

void SystemMonitor::ReportCriticalError(const std::string & error) {
    critical_error_count_++;
    error_history_.push_back(error);

    // 只有 Logger::LogError 是友元函数，所以可以调用它
    Logger::LogError("Critical system error #" + std::to_string(critical_error_count_) + ": " +
                     error);

    // 不能调用 Logger::LogInfo，因为它不是友元
    // Logger::LogInfo("This would cause compilation error");  // 错误！
}

// ============================================================================
// 4. 运算符重载中的友元函数示例
// ============================================================================

class ComplexNumber {
  public:
    ComplexNumber(double real = 0.0, double imag = 0.0) : real_(real), imag_(imag) {}

    // 友元运算符重载 - 需要访问私有成员
    friend ComplexNumber operator+(const ComplexNumber & lhs, const ComplexNumber & rhs);
    friend ComplexNumber operator-(const ComplexNumber & lhs, const ComplexNumber & rhs);
    friend ComplexNumber operator*(const ComplexNumber & lhs, const ComplexNumber & rhs);
    friend bool          operator==(const ComplexNumber & lhs, const ComplexNumber & rhs);

    // 友元输出运算符
    friend std::ostream & operator<<(std::ostream & os, const ComplexNumber & c);

    double GetReal() const { return real_; }

    double GetImag() const { return imag_; }

    double Magnitude() const;

  private:
    double real_;
    double imag_;
};

// 实现：ComplexNumber 方法与运算符（紧跟声明）
double ComplexNumber::Magnitude() const {
    return std::sqrt(real_ * real_ + imag_ * imag_);
}

// 友元运算符重载实现
ComplexNumber operator+(const ComplexNumber & lhs, const ComplexNumber & rhs) {
    return ComplexNumber(lhs.real_ + rhs.real_, lhs.imag_ + rhs.imag_);
}

ComplexNumber operator-(const ComplexNumber & lhs, const ComplexNumber & rhs) {
    return ComplexNumber(lhs.real_ - rhs.real_, lhs.imag_ - rhs.imag_);
}

ComplexNumber operator*(const ComplexNumber & lhs, const ComplexNumber & rhs) {
    return ComplexNumber(lhs.real_ * rhs.real_ - lhs.imag_ * rhs.imag_,
                         lhs.real_ * rhs.imag_ + lhs.imag_ * rhs.real_);
}

bool operator==(const ComplexNumber & lhs, const ComplexNumber & rhs) {
    return lhs.real_ == rhs.real_ && lhs.imag_ == rhs.imag_;
}

std::ostream & operator<<(std::ostream & os, const ComplexNumber & c) {
    os << c.real_;
    if (c.imag_ >= 0) {
        os << "+" << c.imag_ << "i";
    } else {
        os << c.imag_ << "i";
    }
    return os;
}

// ============================================================================
// 5. 模板类中的友元示例
// ============================================================================

template <typename T> class SmartPointer {
  public:
    explicit SmartPointer(T * ptr = nullptr) : ptr_(ptr) {}

    ~SmartPointer() { delete ptr_; }

    // 友元函数模板 - 可以在模板类中声明
    template <typename U> friend void Swap(SmartPointer<U> & lhs, SmartPointer<U> & rhs);

    // 访问器
    T * Get() const { return ptr_; }

    T & operator*() const { return *ptr_; }

    T * operator->() const { return ptr_; }

  private:
    T * ptr_;
};

// 友元函数模板的定义
template <typename T> void Swap(SmartPointer<T> & lhs, SmartPointer<T> & rhs) {
    T * temp = lhs.ptr_;
    lhs.ptr_ = rhs.ptr_;
    rhs.ptr_ = temp;
}

// ============================================================================
// 6. 友元的最佳实践示例 - 最小化访问
// ============================================================================

class DataProcessor {
  public:
    DataProcessor() = default;

    // 只将必要的函数声明为友元，而不是整个类
    friend void ProcessDataHelper(DataProcessor & processor);

    void ProcessData(const std::vector<int> & data);

    const std::vector<int> & GetResults() const { return results_; }

  private:
    std::vector<int> raw_data_;
    std::vector<int> results_;
    int              processing_state_ = 0;
};

// 友元辅助函数 - 只访问必要的私有成员
void ProcessDataHelper(DataProcessor & processor);

// 实现：DataProcessor（紧跟声明）
void DataProcessor::ProcessData(const std::vector<int> & data) {
    raw_data_ = data;
    results_.clear();
    processing_state_ = 1;

    // 调用友元辅助函数来完成实际处理
    ProcessDataHelper(*this);

    processing_state_ = 2;
    spdlog::info("Data processing completed. Results size: {}", results_.size());
}

// 友元辅助函数实现
void ProcessDataHelper(DataProcessor & processor) {
    // 友元函数可以访问私有成员
    if (processor.raw_data_.empty()) {
        return;
    }

    // 简单的处理：计算每个元素的平方
    for (int value : processor.raw_data_) {
        processor.results_.push_back(value * value);
    }

    // 更新处理状态
    processor.processing_state_ = 2;
}

// ============================================================================
// 演示函数
// ============================================================================

void DemonstrateFriendFunctions() {
    spdlog::info("\n=== 友元函数示例 ===");

    BankAccount account1("ACC001", 1000.0);
    BankAccount account2("ACC002", 500.0);

    spdlog::info("初始状态:");
    PrintAccountDetails(account1);
    PrintAccountDetails(account2);

    // 使用友元函数进行转账
    TransferMoney(account1, account2, 200.0);

    spdlog::info("转账后:");
    PrintAccountDetails(account1);
    PrintAccountDetails(account2);
}

void DemonstrateFriendClasses() {
    spdlog::info("\n=== 友元类示例 ===");

    BankAccount    account("ACC003", 1500.0);
    AccountManager manager("MGR001");

    // 账户管理器作为友元类可以审计账户
    manager.AuditAccount(account);

    // 管理器可以直接修改余额
    manager.SetBalance(account, 2000.0);

    // 添加账户到管理器
    manager.AddAccount(&account);
    manager.PrintAllAccounts();

    // 冻结账户
    manager.FreezeAccount(account);
}

void DemonstrateFriendOperators() {
    spdlog::info("\n=== 运算符重载中的友元函数示例 ===");

    ComplexNumber c1(3.0, 4.0);
    ComplexNumber c2(1.0, 2.0);

    spdlog::info("c1 = {}", c1);
    spdlog::info("c2 = {}", c2);
    spdlog::info("c1 + c2 = {}", c1 + c2);
    spdlog::info("c1 * c2 = {}", c1 * c2);
    spdlog::info("c1 == c2: {}", (c1 == c2 ? "true" : "false"));
    spdlog::info("|c1| = {}", c1.Magnitude());
}

void DemonstrateFriendTemplates() {
    spdlog::info("\n=== 模板类中的友元示例 ===");

    SmartPointer<int> ptr1(new int(42));
    SmartPointer<int> ptr2(new int(24));

    spdlog::info("Before swap:");
    spdlog::info("*ptr1 = {}", *ptr1);
    spdlog::info("*ptr2 = {}", *ptr2);

    // 使用友元函数模板进行交换
    Swap(ptr1, ptr2);

    spdlog::info("After swap:");
    spdlog::info("*ptr1 = {}", *ptr1);
    spdlog::info("*ptr2 = {}", *ptr2);
}

void DemonstrateFriendBestPractices() {
    spdlog::info("\n=== 友元最佳实践示例 ===");

    DataProcessor    processor;
    std::vector<int> data = { 1, 2, 3, 4, 5 };

    spdlog::info("Processing data: {}", fmt::join(data, " "));

    processor.ProcessData(data);

    const auto & results = processor.GetResults();
    spdlog::info("Results: {}", fmt::join(results, " "));
}

} // namespace basic
} // namespace cpp_qa_lab

// 提供 fmt 格式化器特化，使 ComplexNumber 可直接用于 fmt::print / spdlog
namespace fmt {
template <> struct formatter<cpp_qa_lab::basic::ComplexNumber> {
    // 使用默认解析
    constexpr auto parse(format_parse_context & ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const cpp_qa_lab::basic::ComplexNumber & c,
                FormatContext &                          ctx) const -> decltype(ctx.out()) {
        if (c.GetImag() >= 0) {
            return fmt::format_to(ctx.out(), "{}+{}i", c.GetReal(), c.GetImag());
        }
        return fmt::format_to(ctx.out(), "{}{}i", c.GetReal(), c.GetImag());
    }
};
} // namespace fmt

int main() {
    spdlog::info("C++ Friend 关键字详细使用示例");
    spdlog::info("=====================================");

    // 演示友元函数
    cpp_qa_lab::basic::DemonstrateFriendFunctions();

    // 演示友元类
    cpp_qa_lab::basic::DemonstrateFriendClasses();

    // 演示运算符重载中的友元函数
    cpp_qa_lab::basic::DemonstrateFriendOperators();

    // 演示模板中的友元
    cpp_qa_lab::basic::DemonstrateFriendTemplates();

    // 演示友元最佳实践
    cpp_qa_lab::basic::DemonstrateFriendBestPractices();

    spdlog::info("\n=====================================");
    spdlog::info("所有友元示例演示完成！");

    return 0;
}
