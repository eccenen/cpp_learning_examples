# C++ Friend 关键字详细使用示例

本文档演示了 C++ 中 `friend` 关键字的各种使用场景和最佳实践。

## 概述

`friend` 关键字允许类或函数访问另一个类的私有和保护成员。这种机制破坏了封装性，但有时是必要的。本示例展示了友元的所有主要用法。

## 示例内容

### 1. 友元函数 (Friend Functions)

```cpp
class BankAccount {
  friend void PrintAccountDetails(const BankAccount& account);
  // ...
};
```

- 友元函数可以访问类的私有成员
- 常用于需要访问多个类私有成员的操作（如转账）

### 2. 友元类 (Friend Classes)

```cpp
class BankAccount {
  friend class AccountManager;
  // ...
};
```

- 整个类成为友元，可以访问所有私有成员
- 适用于紧密耦合的类关系

### 3. 友元成员函数 (Friend Member Functions)

```cpp
class SystemMonitor {
  friend void Logger::LogError(const std::string& message);
  // ...
};
```

- 只允许特定成员函数访问，而不是整个类
- 提供更细粒度的访问控制

### 4. 运算符重载中的友元 (Friends in Operator Overloading)

```cpp
class ComplexNumber {
  friend ComplexNumber operator+(const ComplexNumber& lhs,
                                  const ComplexNumber& rhs);
  // ...
};
```

- 运算符重载经常需要访问私有成员
- 友元允许非成员函数实现运算符

### 5. 模板类中的友元 (Friends in Templates)

```cpp
template <typename T>
class SmartPointer {
  template <typename U>
  friend void Swap(SmartPointer<U>& lhs, SmartPointer<U>& rhs);
  // ...
};
```

- 友元函数模板可以与模板类一起使用
- 允许泛型友元关系

### 6. 友元最佳实践 (Friend Best Practices)

```cpp
class DataProcessor {
  friend void ProcessDataHelper(DataProcessor& processor);
  // ...
};
```

- 只将必要的函数声明为友元
- 避免过度使用友元破坏封装

## 运行示例

```bash
# 构建项目
./compile.sh

# 运行友元示例
./build/bin/basic/basic_friend_example_demo
```

## 重要注意事项

1. **最小化使用**: 友元破坏封装，只在必要时使用
2. **单向关系**: 友元关系不是相互的
3. **不继承**: 友元关系不被继承
4. **访问控制**: 友元可以访问所有私有和保护成员

## Google C++ 规范符合性

- 使用命名空间和适当的包含保护
- 遵循命名约定（驼峰命名等）
- 适当的注释和文档
- 错误处理和资源管理

## 学习建议

1. 先理解封装的重要性
2. 学习何时以及为何需要友元
3. 练习各种友元声明语法
4. 理解友元关系的影响