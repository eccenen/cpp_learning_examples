已更改 CMake 输出目录

说明

- 目标：将各示例（尤其是 `design_patterns` 下的示例）生成的可执行文件统一放在构建目录下的 `build/bin/<subdir>/` 中，便于查找和运行。

已做修改

- 修改文件：
  - `csrc/design_patterns/CMakeLists.txt`
    - 在目录级别设置了 `RUNTIME_OUTPUT_DIRECTORY`（含各配置），默认指向 `${CMAKE_BINARY_DIR}/bin/design_patterns`，使该目录及其子目录的目标默认输出到 `build/bin/design_patterns`。
  - `csrc/design_patterns/template_method/CMakeLists.txt`
    - 在为每个 `add_executable` 的 target 设置了 `RUNTIME_OUTPUT_DIRECTORY` 属性，确保可执行文件被放置在 `build/bin/design_patterns`。

如何构建

- 使用现有脚本（项目根目录）：

```bash
./compile.sh
```

- 或者手动：

```bash
cmake -S . -B build -DEXPORT_COMPILE_COMMANDS=ON
cmake --build build --config Release
```

可执行文件位置

- 现在 `design_patterns` 下的可执行文件将位于：
  - `build/bin/design_patterns/`
 - 现在 `basic` 下的可执行文件将位于：
  - `build/bin/basic/`
 - 现在 `techniques` 下的可执行文件将位于：
  - `build/bin/techniques/`

示例：

```bash
ls -la build/bin/design_patterns
./build/bin/design_patterns/design_patterns_template_method
```

注意

- 对于其他子目录（如 `basic`、`techniques`）当前未强制更改输出目录；如果希望统一所有子目录，可采用类似方式在相应 CMakeLists 中设置 `RUNTIME_OUTPUT_DIRECTORY` 或在更高层级统一设置 `CMAKE_RUNTIME_OUTPUT_DIRECTORY`。

新增示例

- `csrc/basic/any_example.cpp`: 一个演示 `std::any` 用法的示例，包含：
  - 存储不同类型
  - `any_cast<T>` 抛出 `std::bad_any_cast` 的场景
  - `any_cast<T*>` 返回 nullptr 的情况
  - 移动语义示例
  - 存放并访问自定义类型（Point）

使用：

```bash
./compile.sh
./build/bin/basic/basic_any_example
```

注：`any_example.cpp` 现在使用 vcpkg 提供的 `fmt` 库进行格式化输出，已在 `csrc/basic/CMakeLists.txt` 中为 basic 目标链接 `fmt::fmt`。
