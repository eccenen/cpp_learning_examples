#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ARM开发板端服务器脚本
功能：接收模型文件，执行npu_runner，返回结果，清理文件
"""

import os
import socket
import json
import struct
import subprocess
import shutil
from pathlib import Path
import signal
import sys


class ModelTesterServer:
    def __init__(
        self,
        port=9999,
        model_dir="/home/admin/test/model",
        runner_path="/home/admin/test/bin/npu_runner",
    ):
        """
        初始化服务器
        :param port: 监听端口
        :param model_dir: 模型存放目录
        :param runner_path: npu_runner可执行文件路径
        """
        self.port = port
        self.model_dir = Path(model_dir)
        self.runner_path = runner_path
        self.running = True

        # 确保模型目录存在
        self.model_dir.mkdir(parents=True, exist_ok=True)

        # 设置信号处理
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)

    def signal_handler(self, sig, frame):
        """处理退出信号"""
        print("\n收到退出信号，正在关闭服务器...")
        self.running = False
        sys.exit(0)

    def recv_exact(self, conn, size):
        """精确接收指定字节数"""
        data = b""
        while len(data) < size:
            chunk = conn.recv(size - len(data))
            if not chunk:
                return None
            data += chunk
        return data

    def receive_file(self, conn, save_path):
        """通过socket接收文件"""
        # 接收文件名长度和文件名
        name_length_data = self.recv_exact(conn, 4)
        if not name_length_data:
            return False, None
        name_length = struct.unpack("!I", name_length_data)[0]

        file_name_bytes = self.recv_exact(conn, name_length)
        if not file_name_bytes:
            return False, None
        file_name = file_name_bytes.decode("utf-8")

        # 接收文件大小
        size_data = self.recv_exact(conn, 8)
        if not size_data:
            return False, None
        file_size = struct.unpack("!Q", size_data)[0]

        # 确定保存路径
        if "/" in file_name:  # 如果文件名包含路径（如golden/xxx.bin）
            full_path = save_path / file_name
            full_path.parent.mkdir(parents=True, exist_ok=True)
        else:
            full_path = save_path / file_name

        # 接收文件内容
        received = 0
        with open(full_path, "wb") as f:
            while received < file_size:
                chunk_size = min(8192, file_size - received)
                chunk = conn.recv(chunk_size)
                if not chunk:
                    return False, None
                f.write(chunk)
                received += len(chunk)

        print(f"  已接收: {file_name} ({file_size} bytes) -> {full_path}")
        return True, file_name

    def send_message(self, conn, message):
        """发送消息"""
        msg_bytes = message.encode("utf-8")
        conn.sendall(struct.pack("!I", len(msg_bytes)))
        conn.sendall(msg_bytes)

    def run_model(self, model_path, cmd_args):
        """
        运行模型
        :param model_path: 模型路径
        :param cmd_args: 已解析的命令行参数列表
        :return: 执行结果字符串
        """
        # 构建完整命令
        cmd = [self.runner_path] + cmd_args

        print(f"\n执行命令: {' '.join(cmd)}")
        print(f"工作目录: {model_path}")

        try:
            # 执行命令并捕获输出
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=180,  # 3分钟超时
                cwd=str(model_path),  # 设置工作目录
            )

            # 合并stdout和stderr
            output = ""
            if result.stdout:
                output += "=== STDOUT ===\n"
                output += result.stdout
            if result.stderr:
                output += "\n=== STDERR ===\n"
                output += result.stderr

            output += f"\n\n=== 返回码: {result.returncode} ===\n"

            if result.returncode == 0:
                output += "执行成功!\n"
            else:
                output += "执行失败!\n"

            return output

        except subprocess.TimeoutExpired:
            error_msg = "错误: 命令执行超时 (3分钟)"
            print(error_msg)
            return error_msg
        except FileNotFoundError:
            error_msg = f"错误: npu_runner未找到: {self.runner_path}"
            print(error_msg)
            return error_msg
        except Exception as e:
            error_msg = f"错误: 执行命令时发生异常: {str(e)}"
            print(error_msg)
            import traceback

            traceback.print_exc()
            return error_msg

    def cleanup_model(self, model_path):
        """清理模型文件"""
        try:
            if model_path.exists():
                shutil.rmtree(model_path)
                print(f"已清理模型文件: {model_path}")
                return True
        except Exception as e:
            print(f"清理模型文件失败: {e}")
            return False
        return False

    def parse_command_args(self, command, model_path, model_name):
        """
        解析命令行参数
        :param command: 从客户端接收的命令字典
        :param model_path: 模型路径
        :param model_name: 模型名称
        :return: (success, cmd_args or error_msg)
        """
        model_file_base = model_path / model_name

        # 检查模型文件是否存在
        bin_file = Path(str(model_file_base) + ".bin")
        param_file = Path(str(model_file_base) + ".param")

        if not bin_file.exists():
            return False, f"错误: 模型文件不存在: {bin_file}"
        if not param_file.exists():
            return False, f"错误: 参数文件不存在: {param_file}"

        # 构建命令行参数列表
        cmd_args = []

        # -m 参数（必需）
        cmd_args.extend(["-m", str(model_file_base)])

        # -g 参数（可选，用于数据对比）
        if "golden" in command and command["golden"]:
            golden_path = model_path / "golden"
            if not golden_path.exists():
                return False, f"错误: Golden目录不存在: {golden_path}"
            cmd_args.extend(["-g", str(golden_path)])

        # -r 参数（可选）
        if "repeat_count" in command:
            cmd_args.extend(["-r", str(command["repeat_count"])])

        # -n 参数（可选）
        if "npu_cores" in command:
            cmd_args.extend(["-n", str(command["npu_cores"])])

        # --peak_performance 参数（可选）
        if "peak_performance" in command:
            cmd_args.extend(["--peak_performance", str(command["peak_performance"])])

        return True, cmd_args

    def handle_client(self, conn, addr):
        """处理客户端连接"""
        print(f"\n{'='*60}")
        print(f"客户端连接: {addr[0]}:{addr[1]}")
        print(f"{'='*60}")

        try:
            # 接收命令
            cmd_length_data = self.recv_exact(conn, 4)
            if not cmd_length_data:
                print("错误: 未收到命令")
                return

            cmd_length = struct.unpack("!I", cmd_length_data)[0]
            cmd_data = self.recv_exact(conn, cmd_length)
            if not cmd_data:
                print("错误: 命令数据不完整")
                return

            command = json.loads(cmd_data.decode("utf-8"))
            print(f"收到命令: {command}")

            if command.get("command") != "run_model":
                print("错误: 不支持的命令")
                self.send_message(conn, "错误: 不支持的命令")
                return

            model_name = command.get("model_name")
            if not model_name:
                print("错误: 缺少模型名称")
                self.send_message(conn, "错误: 缺少模型名称")
                return

            # 创建模型目录
            model_path = self.model_dir / model_name
            if model_path.exists():
                print(f"清理旧的模型目录: {model_path}")
                shutil.rmtree(model_path)
            model_path.mkdir(parents=True, exist_ok=True)

            # 接收文件数量
            file_count_data = self.recv_exact(conn, 4)
            if not file_count_data:
                print("错误: 未收到文件数量")
                return

            file_count = struct.unpack("!I", file_count_data)[0]
            print(f"准备接收 {file_count} 个文件...")

            # 接收所有文件
            for i in range(file_count):
                success, file_name = self.receive_file(conn, model_path)
                if not success:
                    print(f"错误: 接收第 {i+1} 个文件失败")
                    self.send_message(conn, f"错误: 接收文件失败")
                    self.cleanup_model(model_path)
                    return

            print(f"\n所有文件接收完成!")

            # 解析命令行参数
            success, result_data = self.parse_command_args(
                command, model_path, model_name
            )
            if not success:
                error_msg = result_data
                print(error_msg)
                self.send_message(conn, error_msg)
                self.cleanup_model(model_path)
                return

            cmd_args = result_data
            print(f"解析的命令行参数: {cmd_args}")

            # 运行模型
            result = self.run_model(model_path, cmd_args)

            # 发送结果
            print(f"\n发送执行结果给客户端...")
            self.send_message(conn, result)

            # 清理模型文件
            print(f"\n清理模型文件...")
            self.cleanup_model(model_path)

            print(f"\n任务完成!")

        except Exception as e:
            print(f"处理客户端请求时发生错误: {e}")
            import traceback

            traceback.print_exc()
            try:
                self.send_message(conn, f"服务器错误: {str(e)}")
            except Exception as send_err:
                print(f"发送错误消息失败: {send_err}")
        finally:
            try:
                conn.close()
            except:
                pass
            print(f"连接关闭: {addr[0]}:{addr[1]}")

    def start(self):
        """启动服务器"""
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(("0.0.0.0", self.port))
        server_socket.listen(5)

        print(f"{'='*60}")
        print(f"ARM模型测试服务器已启动")
        print(f"监听端口: {self.port}")
        print(f"模型目录: {self.model_dir}")
        print(f"Runner路径: {self.runner_path}")
        print(f"{'='*60}\n")
        print("等待客户端连接...\n")

        while self.running:
            try:
                server_socket.settimeout(1.0)  # 设置超时以便能响应退出信号
                try:
                    conn, addr = server_socket.accept()
                    self.handle_client(conn, addr)
                except socket.timeout:
                    continue
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"服务器错误: {e}")

        server_socket.close()
        print("\n服务器已关闭")


def main():
    import argparse

    parser = argparse.ArgumentParser(description="ARM开发板端模型测试服务器")
    parser.add_argument("--port", type=int, default=9999, help="监听端口 (默认: 9999)")
    parser.add_argument(
        "--model-dir",
        default="/home/admin/test/model",
        help="模型存放目录 (默认: /home/admin/test/model)",
    )
    parser.add_argument(
        "--runner",
        default="/home/admin/test/bin/npu_runner",
        help="npu_runner可执行文件路径 (默认: /home/admin/test/bin/npu_runner)",
    )

    args = parser.parse_args()

    server = ModelTesterServer(args.port, args.model_dir, args.runner)
    server.start()


if __name__ == "__main__":
    main()
