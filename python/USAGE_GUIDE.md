# 模型测试脚本使用指南

## 开发板信息
- **IP地址**: 192.168.23.62
- **用户名**: admin
- **密码**: admin

## 快速开始

### 第一步：在ARM开发板上部署服务器

1. **上传服务器脚本到开发板**
```bash
# 在x86端（WSL2）执行
scp python/arm_server.py admin@192.168.23.62:/home/admin/
```

2. **SSH登录到开发板**
```bash
ssh admin@192.168.23.62
# 输入密码: admin
```

3. **在开发板上准备环境**
```bash
# 创建必要的目录
mkdir -p /home/admin/test/model
mkdir -p /home/admin/test/bin

# 给脚本添加执行权限
chmod +x /home/admin/arm_server.py

# 确认npu_runner存在（根据实际情况调整路径）
ls -la /home/admin/test/bin/npu_runner
```

4. **启动服务器**
```bash
# 方式1: 前台运行（推荐用于测试）
python3 /home/admin/arm_server.py

# 方式2: 后台运行
nohup python3 /home/admin/arm_server.py > /tmp/arm_server.log 2>&1 &

# 查看后台日志
tail -f /tmp/arm_server.log

# 停止后台服务
pkill -f arm_server.py
```

### 第二步：在x86端运行客户端

1. **确认模型目录存在**
```bash
# 检查模型目录
ls -la /mnt/f/测试模型v2.9.3/v293_model_collect/

# 查看某个模型的结构
ls -la /mnt/f/测试模型v2.9.3/v293_model_collect/alexnet_imagenet_224x224x3/
```

2. **运行客户端脚本**
```bash
cd /home/eccenen/project/my_project/cpp_learning_examples/python

# 运行测试
python3 x86_client.py \
    --host 192.168.23.62 \
    --models-dir /mnt/f/测试模型v2.9.3/v293_model_collect
```

3. **查看结果**
```bash
# 测试结果会保存在当前目录的 test_results 文件夹
ls -la test_results/

# 查看汇总报告
cat test_results/summary_*.txt

# 查看某个模型的详细结果
cat test_results/alexnet_imagenet_224x224x3_*.txt
```

## 常用命令

### ARM开发板端

```bash
# 查看服务器是否在运行
ps aux | grep arm_server

# 查看端口占用
netstat -tuln | grep 9999

# 查看模型目录
ls -la /home/admin/test/model/

# 手动清理模型文件（如果需要）
rm -rf /home/admin/test/model/*

# 测试npu_runner是否可用
/home/admin/test/bin/npu_runner --help
```

### x86端

```bash
# 测试与开发板的连接
ping 192.168.23.62

# 测试端口是否开放
nc -zv 192.168.23.62 9999

# 只测试单个模型（需要修改脚本或手动指定）
# 建议先测试一个小模型确认流程正确

# 查看结果文件
ls -lht test_results/ | head

# 统计成功和失败的模型
grep -c "成功" test_results/summary_*.txt
grep -c "失败" test_results/summary_*.txt
```

## 脚本参数说明

### arm_server.py (ARM端)

```bash
python3 arm_server.py [选项]

选项:
  --port PORT           监听端口 (默认: 9999)
  --model-dir PATH      模型存放目录 (默认: /home/admin/test/model)
  --runner PATH         npu_runner路径 (默认: /home/admin/test/bin/npu_runner)

示例:
  python3 arm_server.py --port 8888 --model-dir /tmp/models
```

### x86_client.py (x86端)

```bash
python3 x86_client.py [选项]

选项:
  --host HOST           ARM开发板IP地址 (必需)
  --port PORT           ARM开发板端口 (默认: 9999)
  --models-dir PATH     模型根目录 (必需)

示例:
  python3 x86_client.py --host 192.168.23.62 --models-dir /mnt/f/测试模型v2.9.3/v293_model_collect
```

## 故障排查

### 1. 连接失败

**问题**: `[Errno 111] Connection refused`

**解决方案**:
```bash
# 1. 确认ARM服务器已启动
ssh admin@192.168.23.62
ps aux | grep arm_server

# 2. 检查端口是否监听
netstat -tuln | grep 9999

# 3. 检查防火墙（如果有）
sudo ufw status
sudo ufw allow 9999/tcp

# 4. 重启服务器
pkill -f arm_server.py
python3 /home/admin/arm_server.py &
```

### 2. 文件传输失败

**问题**: `错误: 接收文件失败`

**解决方案**:
```bash
# 检查磁盘空间
df -h /home/admin/test/model

# 检查目录权限
ls -ld /home/admin/test/model
chmod 755 /home/admin/test/model

# 检查网络连接
ping 192.168.23.62
```

### 3. 模型运行失败

**问题**: `错误: npu_runner未找到`

**解决方案**:
```bash
# 查找npu_runner
find /home/admin -name "npu_runner"

# 检查执行权限
ls -la /home/admin/test/bin/npu_runner
chmod +x /home/admin/test/bin/npu_runner

# 测试手动运行
cd /home/admin/test/model/test_model
/home/admin/test/bin/npu_runner -m test_model -g golden
```

### 4. 权限问题

**问题**: `Permission denied`

**解决方案**:
```bash
# 修改目录权限
sudo chown -R admin:admin /home/admin/test
chmod -R 755 /home/admin/test

# 或者使用sudo运行（不推荐）
sudo python3 arm_server.py
```

### 5. 超时问题

**问题**: `错误: 连接超时`

**可能原因和解决方案**:
- 模型文件太大：增加超时时间（修改代码中的timeout参数）
- 网络慢：检查网络状况
- ARM板性能问题：减少并发或增加超时时间

## 日志和调试

### 查看详细日志

在代码中已包含详细的日志输出，包括：
- 文件传输进度
- 命令执行细节
- 错误堆栈信息

### 启用更详细的调试

如需更多调试信息，可以在脚本开头添加：

```python
import logging
logging.basicConfig(level=logging.DEBUG)
```

## 测试流程说明

```
┌─────────────────────────────────────────────────────────────┐
│ 1. x86客户端扫描模型目录                                      │
│    - 识别所有模型文件夹                                       │
│    - 验证每个文件夹包含.bin, .param和golden目录               │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 2. 连接ARM开发板                                              │
│    - TCP连接到192.168.23.62:9999                             │
│    - 发送运行命令和模型名称                                   │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 3. 传输文件                                                   │
│    - model_name.bin                                          │
│    - model_name.param                                        │
│    - golden/*.bin (所有golden测试文件)                        │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 4. ARM端执行模型                                              │
│    - 接收并保存文件到/home/admin/test/model/<model_name>/    │
│    - 执行: npu_runner -m <model> -g <golden>                 │
│    - 捕获stdout和stderr输出                                   │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 5. 返回结果                                                   │
│    - 发送执行结果回x86端                                      │
│    - 清理模型文件                                             │
│    - 准备下一个模型                                           │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 6. x86端保存结果                                              │
│    - 保存单个模型结果: <model>_<timestamp>.txt                │
│    - 生成汇总报告: summary_<timestamp>.txt                    │
└─────────────────────────────────────────────────────────────┘
```

## 预期输出示例

### ARM服务器输出
```
============================================================
ARM模型测试服务器已启动
监听端口: 9999
模型目录: /home/admin/test/model
Runner路径: /home/admin/test/bin/npu_runner
============================================================

等待客户端连接...

============================================================
客户端连接: 192.168.23.1:54321
============================================================
收到命令: {'command': 'run_model', 'model_name': 'alexnet_imagenet_224x224x3'}
准备接收 6 个文件...
  已接收: alexnet_imagenet_224x224x3.bin (12345678 bytes) -> ...
  已接收: alexnet_imagenet_224x224x3.param (123456 bytes) -> ...
  已接收: golden/36.bin (4096 bytes) -> ...
  ...

所有文件接收完成!

执行命令: /home/admin/test/bin/npu_runner -m ... -g ...
工作目录: /home/admin/test/model/alexnet_imagenet_224x224x3

发送执行结果给客户端...

清理模型文件...
已清理模型文件: ...

任务完成!
连接关闭: 192.168.23.1:54321
```

### x86客户端输出
```
找到 10 个模型文件夹

[1/10] 处理模型文件夹: alexnet_imagenet_224x224x3

============================================================
开始测试模型: alexnet_imagenet_224x224x3
============================================================
连接到ARM开发板 192.168.23.62:9999...
连接成功!
准备发送 6 个文件 (.bin, .param, 4 个golden文件)
发送模型文件...
  已发送: alexnet_imagenet_224x224x3.bin (12345678 bytes)
  已发送: alexnet_imagenet_224x224x3.param (123456 bytes)
发送golden文件...
  已发送: golden/36.bin (4096 bytes)
  ...

等待ARM开发板执行模型...

收到执行结果:
------------------------------------------------------------
=== STDOUT ===
Loading model...
Running inference...
Result: PASS

=== 返回码: 0 ===
执行成功!
------------------------------------------------------------

结果已保存到: test_results/alexnet_imagenet_224x224x3_20251224_153045.txt

等待3秒后处理下一个模型...
```

## 安全提示

1. **网络安全**: 确保在可信网络环境中使用
2. **文件权限**: 定期检查模型目录的权限设置
3. **磁盘空间**: 定期清理测试文件，避免磁盘填满
4. **资源监控**: 注意ARM板的CPU和内存使用情况

## 后续优化建议

1. 添加身份验证机制
2. 支持批量测试的断点续传
3. 添加进度条显示
4. 支持并行测试多个模型（需要ARM板支持）
5. 添加WebUI界面
6. 集成到CI/CD流程

## 联系和支持

如遇问题，请检查：
1. 本文档的故障排查章节
2. 脚本输出的详细日志
3. ARM板的系统日志: `dmesg | tail`
