# cpp-qa-lab
记录使用 C++ 过程中产生的问题及解决方法，和有助于学会 C++ 的代码示例。

本项目包含一个演示如何强制类“只能在堆上创建”的示例（`csrc/heap_only.*`），并包含基于 doctest 的单元测试。

构建与测试
-------------

有三种常见方式来构建并运行测试：

1) 推荐（使用 vcpkg toolchain）

	如果你使用 vcpkg 管理依赖（推荐），在调用 cmake 时传入 toolchain 文件：

	```bash
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux
	cmake --build build -j
	ctest --test-dir build
	```

2) 通过环境变量指定 vcpkg 根路径（不推荐提交硬编码路径）

	可以把 vcpkg 的根路径放到环境变量 `VCPKG_PATH`，脚本会把它传入 CMake：

	```bash
	export VCPKG_PATH=/home/you/software/vcpkg
	./build.sh
	```

	或者直接用 cmake 参数：

	```bash
	cmake -S . -B build -DVCPKG_PATH=/home/you/software/vcpkg -DVCPKG_TARGET_TRIPLET=x64-linux
	cmake --build build -j
	```

3) 无 vcpkg 时的回退（FetchContent）

	如果既不使用 toolchain，也没有设置 `VCPKG_PATH`，构建系统会自动通过 CMake 的 FetchContent 下载 doctest（需要联网），并继续构建测试。你也可以直接运行项目自带的便利脚本：

	```bash
	./build.sh
	```

使用 `build.sh`
----------------

脚本 `build.sh` 在项目根，简化了 configure/build/test 流程：

- 默认读取环境变量 `VCPKG_PATH`（若已设置会传递给 CMake）；
- 支持参数 `--toolchain /path/to/vcpkg/scripts/buildsystems/vcpkg.cmake` 来直接使用 vcpkg toolchain；
- 自动构建并在成功后运行测试可执行文件（`test_heap_only`）。

示例：

```bash
# 使用环境变量
VCPKG_PATH=/home/you/software/vcpkg ./build.sh

# 或者直接使用 toolchain
./build.sh --toolchain /home/you/software/vcpkg/scripts/buildsystems/vcpkg.cmake
```

注意事项
---------

- 为了不泄露个人路径，仓库中不应该包含像 `/home/yourname/...` 的硬编码路径；请改用上面任一方法在你本地或 CI 中注入 vcpkg 的位置。
- 如果你已经不小心把个人路径提交到 git 历史中，请谨慎使用工具（如 BFG 或 git filter-branch）清理历史，或在 README 中说明如何本地覆盖。需要我帮你审查提交历史并给出清理建议吗？

如需我把 README 中的某段再调整或扩展到 CONTRIBUTING 指南、CI 配置示例（GitHub Actions/CircleCI）里，告诉我你想要的 CI 平台，我可以接着添加。 
