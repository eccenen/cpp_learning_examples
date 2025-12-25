# 模型测试脚本使用说明

## 概述

这是一个用于在x86端（WSL2-Ubuntu）和ARM开发板之间进行模型测试的自动化工具。系统包含两个Python脚本：

- **x86_client.py**: 运行在本地x86端，负责发送模型文件并收集测试结果
- **arm_server.py**: 运行在ARM开发板端，负责接收模型、执行测试、返回结果

## 系统架构

```
┌─────────────────────────┐           ┌──────────────────────────┐
│   x86端 (WSL2-Ubuntu)   │           │  ARM开发板 (Ubuntu Linux)  │
│                         │           │                          │
│  ┌──────────────────┐   │           │   ┌─────────────────┐    │
│  │  x86_client.py   │   │  Socket   │   │  arm_server.py  │    │
│  │                  │◄──┼───────────┼──►│                 │    │
│  │  - 扫描模型目录   │   │   通信    │   │  - 接收文件      │    │
│  │  - 传输文件       │   │           │   │  - 运行模型      │    │
│  │  - 收集结果       │   │           │   │  - 返回结果      │    │
│  │  - 保存日志       │   │           │   │  - 清理文件      │    │
│  └──────────────────┘   │           │   └─────────────────┘    │
└─────────────────────────┘           └──────────────────────────┘
```

## 前置条件

### 两端都需要
- Python 3.6+
- 网络连通性（x86端能访问ARM开发板的IP）

### ARM开发板端
- `/home/admin/test/bin/npu_runner` 可执行文件存在
- `/home/admin/test/model` 目录有写权限

## 安装步骤

### 1. 在ARM开发板端部署

```bash
# 1. 上传脚本到ARM开发板
scp arm_server.py admin@<ARM_IP>:/home/admin/

# 2. SSH登录到ARM开发板
ssh admin@<ARM_IP>

# 3. 给脚本添加执行权限
chmod +x /home/admin/arm_server.py

# 4. 确保必要的目录存在
mkdir -p /home/admin/test/model
mkdir -p /home/admin/test/bin

# 5. 确认npu_runner存在
ls -la /home/admin/test/bin/npu_runner
```

### 2. 启动ARM开发板端服务

```bash
# 在ARM开发板上运行服务器（前台运行）
python3 /home/admin/arm_server.py

# 或使用nohup在后台运行
nohup python3 /home/admin/arm_server.py > server.log 2>&1 &

# 或使用自定义参数
python3 /home/admin/arm_server.py --port 9999 \
    --model-dir /home/admin/test/model \
    --runner /home/admin/test/bin/npu_runner
```

**服务器参数说明:**
- `--port`: 监听端口（默认: 9999）
- `--model-dir`: 模型存放目录（默认: /home/admin/test/model）
- `--runner`: npu_runner路径（默认: /home/admin/test/bin/npu_runner）

### 3. 在x86端运行客户端

```bash
# 在x86端（WSL2-Ubuntu）运行
cd /home/eccenen/project/my_project/cpp_learning_examples

# 给脚本添加执行权限
chmod +x x86_client.py

# 运行客户端
python3 x86_client.py \
    --host <ARM_IP> \
    --models-dir /mnt/f/测试模型v2.9.3/v293_model_collect
```

**客户端参数说明:**
- `--host`: ARM开发板的IP地址（必需）
- `--port`: ARM开发板监听端口（默认: 9999）
- `--models-dir`: 模型根目录路径（必需）

## 使用示例

### 示例1: 基本使用

```bash
# 1. 在ARM开发板启动服务器
python3 arm_server.py

# 2. 在x86端运行客户端
python3 x86_client.py \
    --host 192.168.1.100 \
    --models-dir /mnt/f/测试模型v2.9.3/v293_model_collect
```

### 示例2: 自定义端口

```bash
# ARM端
python3 arm_server.py --port 8888

# x86端
python3 x86_client.py \
    --host 192.168.1.100 \
    --port 8888 \
    --models-dir /mnt/f/测试模型v2.9.3/v293_model_collect
```

### 示例3: 后台运行ARM服务器

```bash
# 启动后台服务
nohup python3 arm_server.py > /tmp/arm_server.log 2>&1 &

# 查看日志
tail -f /tmp/arm_server.log

# 停止服务
pkill -f arm_server.py
```

## 目录结构要求

### x86端模型目录结构
```
/mnt/f/测试模型v2.9.3/v293_model_collect/
├── alexnet_imagenet_224x224x3/
│   ├── alexnet_imagenet_224x224x3.bin
│   ├── alexnet_imagenet_224x224x3.param
│   └── golden/
│       ├── 36.bin
│       ├── NTensor_18.bin
│       ├── NTensor_19.bin
│       └── NTensor_2.bin
├── resnet50_imagenet_224x224x3/
│   ├── resnet50_imagenet_224x224x3.bin
│   ├── resnet50_imagenet_224x224x3.param
│   └── golden/
│       └── ...
└── ...
```

### 结果保存目录
客户端运行后会在当前目录创建 `test_results/` 文件夹：

```
test_results/
├── alexnet_imagenet_224x224x3_20251224_143025.txt
├── resnet50_imagenet_224x224x3_20251224_143156.txt
├── summary_20251224_143300.txt
└── ...
```

## 工作流程

1. **x86_client.py** 扫描指定目录下的所有模型文件夹
2. 对每个模型：
   - 连接到ARM开发板服务器
   - 发送 `.bin` 和 `.param` 文件
   - 发送 `golden/` 目录下的所有文件
   - 等待ARM端执行完成
   - 接收执行结果
   - 保存结果到本地文件
3. **arm_server.py** 在ARM开发板上：
   - 监听客户端连接
   - 接收模型文件到 `/home/admin/test/model/<model_name>/`
   - 执行命令: `npu_runner -m <model_path> -g <golden_path>`
   - 捕获输出（stdout和stderr）
   - 返回结果给客户端
   - 清理模型文件
   - 等待下一个任务

## 输出结果格式

### 单个模型结果文件
```
模型名称: alexnet_imagenet_224x224x3
测试时间: 2025-12-24 14:30:25
测试结果: 成功

============================================================
执行输出:
============================================================
=== STDOUT ===
Loading model...
Running inference...
Result: PASS

=== STDERR ===
[INFO] Model loaded successfully

=== 返回码: 0 ===
执行成功!
```

### 汇总报告
```
模型测试汇总报告
测试时间: 2025-12-24 14:35:00
总计模型数: 10
成功: 8
失败: 2

================================================================================

1. alexnet_imagenet_224x224x3: ✓ 成功
2. resnet50_imagenet_224x224x3: ✓ 成功
3. mobilenet_v2: ✗ 失败
...
```

## 故障排查

### 1. 连接失败
```
错误: [Errno 111] Connection refused
```
**解决方案:**
- 检查ARM开发板服务器是否已启动
- 检查IP地址是否正确
- 检查防火墙设置
- 验证端口是否开放

### 2. 文件传输失败
```
错误: 接收文件失败
```
**解决方案:**
- 检查网络连接稳定性
- 检查磁盘空间
- 检查文件权限

### 3. 模型运行失败
```
错误: 执行命令时发生异常
```
**解决方案:**
- 确认 `npu_runner` 路径正确
- 确认 `npu_runner` 有执行权限
- 检查模型文件完整性
- 查看详细错误日志

### 4. 超时错误
```
错误: 连接超时
```
**解决方案:**
- 增加超时时间（修改代码中的timeout参数）
- 检查模型文件大小和网络速度
- 检查ARM开发板性能

## 高级配置

### 修改超时时间

在 `x86_client.py` 中：
```python
sock.settimeout(300)  # 修改为更长时间，如600秒
```

在 `arm_server.py` 中：
```python
timeout=180  # 修改为更长时间，如300秒
```

### 并发支持

当前服务器是单线程的，一次只能处理一个客户端。如需并发支持，可修改 `arm_server.py`：

```python
import threading

def start(self):
    # ... 启动代码 ...
    while self.running:
        try:
            conn, addr = server_socket.accept()
            # 使用线程处理
            thread = threading.Thread(target=self.handle_client, args=(conn, addr))
            thread.daemon = True
            thread.start()
        except Exception as e:
            print(f"服务器错误: {e}")
```

## 安全注意事项

1. **网络安全**: 当前脚本使用未加密的Socket通信，建议在可信网络环境中使用
2. **文件权限**: 确保模型目录有适当的权限限制
3. **资源限制**: 服务器会自动清理模型文件，但建议定期检查磁盘空间
4. **访问控制**: 考虑添加IP白名单或认证机制

## 许可证

本脚本为内部测试工具，请根据项目需求调整。
