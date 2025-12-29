#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
x86端客户端脚本
功能：扫描模型文件夹，将模型文件传输到ARM开发板，触发执行，收集结果
"""

import os
import socket
import json
import struct
import time
from pathlib import Path
from datetime import datetime
import re
from typing import Dict, List, Optional

try:
    import pandas as pd

    HAS_PANDAS = True
except ImportError:
    HAS_PANDAS = False


class ModelTesterClient:
    def __init__(self, arm_host, arm_port=9999, runner_args=None, use_golden=True):
        """
        初始化客户端
        :param arm_host: ARM开发板的IP地址
        :param arm_port: ARM开发板监听的端口
        :param runner_args: npu_runner 的命令行参数列表，例如: ["-r", "20", "-n", "0,1"]
        :param use_golden: 是否使用 golden 数据对比
        """
        self.arm_host = arm_host
        self.arm_port = arm_port
        self.runner_args = runner_args or []
        self.use_golden = use_golden
        self.result_dir = Path("./test_results")
        self.result_dir.mkdir(exist_ok=True)

    def send_file(self, sock, file_path):
        """通过socket发送文件"""
        file_size = os.path.getsize(file_path)
        file_name = os.path.basename(file_path)

        # 发送文件名长度和文件名
        file_name_bytes = file_name.encode("utf-8")
        sock.sendall(struct.pack("!I", len(file_name_bytes)))
        sock.sendall(file_name_bytes)

        # 发送文件大小
        sock.sendall(struct.pack("!Q", file_size))

        # 发送文件内容
        with open(file_path, "rb") as f:
            sent = 0
            while sent < file_size:
                chunk = f.read(8192)
                if not chunk:
                    break
                sock.sendall(chunk)
                sent += len(chunk)

        print(f"  已发送: {file_name} ({file_size} bytes)")

    def receive_message(self, sock):
        """接收消息"""
        # 接收消息长度
        length_data = self.recv_exact(sock, 4)
        if not length_data:
            return None
        msg_length = struct.unpack("!I", length_data)[0]

        # 接收消息内容
        msg_data = self.recv_exact(sock, msg_length)
        if not msg_data:
            return None

        return msg_data.decode("utf-8")

    def recv_exact(self, sock, size):
        """精确接收指定字节数"""
        data = b""
        while len(data) < size:
            chunk = sock.recv(size - len(data))
            if not chunk:
                return None
            data += chunk
        return data

    def test_model(self, model_folder_path):
        """
        测试单个模型
        :param model_folder_path: 模型文件夹路径
        :return: (success, result_message)
        """
        model_folder = Path(model_folder_path)
        model_name = model_folder.name

        print(f"\n{'='*60}")
        print(f"开始测试模型: {model_name}")
        print(f"{'='*60}")

        # 查找.bin和.param文件
        bin_file = None
        param_file = None

        for file in model_folder.iterdir():
            if file.suffix == ".bin" and file.stem == model_name:
                bin_file = file
            elif file.suffix == ".param" and file.stem == model_name:
                param_file = file

        if not bin_file or not param_file:
            error_msg = f"错误: 模型文件不完整，缺少 .bin 或 .param 文件"
            print(error_msg)
            return False, error_msg

        # 检查golden文件夹（仅当需要数据对比时）
        golden_dir = model_folder / "golden"
        golden_files = []
        if self.use_golden:
            if not golden_dir.exists() or not golden_dir.is_dir():
                error_msg = f"错误: 缺少 golden 数据文件夹"
                print(error_msg)
                return False, error_msg
            golden_files = [f for f in golden_dir.glob("*") if f.is_file()]

        try:
            # 连接到ARM开发板
            print(f"连接到ARM开发板 {self.arm_host}:{self.arm_port}...")
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(300)  # 5分钟超时
            sock.connect((self.arm_host, self.arm_port))
            print("连接成功!")

            # 构建命令，直接传递 npu_runner 的参数
            command = {
                "command": "run_model",
                "model_name": model_name,
                "use_golden": self.use_golden,
                "runner_args": self.runner_args,  # 直接传递命令行参数列表
            }

            # 发送命令
            command_json = json.dumps(command)
            command_bytes = command_json.encode("utf-8")
            sock.sendall(struct.pack("!I", len(command_bytes)))
            sock.sendall(command_bytes)

            # 发送文件数量（.bin, .param, golden文件夹下的所有文件）
            total_files = 2 + len(golden_files)
            if self.use_golden and golden_files:
                print(
                    f"准备发送 {total_files} 个文件 (.bin, .param, {len(golden_files)} 个golden文件)"
                )
            else:
                print(f"准备发送 2 个文件 (.bin, .param)")
            sock.sendall(struct.pack("!I", total_files))

            # 发送.bin和.param文件
            print("发送模型文件...")
            self.send_file(sock, bin_file)
            self.send_file(sock, param_file)

            # 发送golden文件夹下的所有文件（仅当use_golden=True时）
            if self.use_golden and golden_files:
                print("发送golden文件...")
                for golden_file in golden_files:
                    # 发送文件时带上golden前缀，让服务端知道放到golden子目录
                    file_path_rel = f"golden/{golden_file.name}"
                    file_size = os.path.getsize(golden_file)

                    file_name_bytes = file_path_rel.encode("utf-8")
                    sock.sendall(struct.pack("!I", len(file_name_bytes)))
                    sock.sendall(file_name_bytes)
                    sock.sendall(struct.pack("!Q", file_size))

                    with open(golden_file, "rb") as f:
                        sent = 0
                        while sent < file_size:
                            chunk = f.read(8192)
                            if not chunk:
                                break
                            sock.sendall(chunk)
                            sent += len(chunk)
                    print(f"  已发送: golden/{golden_file.name} ({file_size} bytes)")

            print("\n等待ARM开发板执行模型...")

            # 接收执行结果
            result = self.receive_message(sock)

            if result:
                print("\n收到执行结果:")
                print("-" * 60)
                print(result)
                print("-" * 60)
                # 检查实际执行结果是否成功
                success = self.check_result_success(result)
                if not success:
                    print("\n⚠️  注意: 模型执行失败（返回码非0）")
                return success, result
            else:
                error_msg = "错误: 未收到执行结果"
                print(error_msg)
                return False, error_msg

        except socket.timeout:
            error_msg = "错误: 连接超时"
            print(error_msg)
            return False, error_msg
        except ConnectionRefusedError:
            error_msg = f"错误: 无法连接到ARM开发板 {self.arm_host}:{self.arm_port}，请确认服务器已启动"
            print(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"错误: {str(e)}"
            print(error_msg)
            import traceback

            traceback.print_exc()
            return False, error_msg
        finally:
            # 确保socket被关闭
            try:
                sock.close()
            except:
                pass

    def check_result_success(self, result):
        """检查模型执行结果是否成功"""
        # 检查是否包含返回码
        if "=== 返回码:" in result:
            # 提取返回码
            try:
                returncode_line = [
                    line for line in result.split("\n") if "=== 返回码:" in line
                ][0]
                returncode_str = returncode_line.split(":")[1].strip().split()[0]
                returncode = int(returncode_str)
                return returncode == 0
            except (IndexError, ValueError) as e:
                print(f"警告: 无法解析返回码，默认为失败: {e}")
                return False
        # 如果没有返回码信息，检查是否包含"执行成功"标记
        elif "执行成功!" in result:
            return True
        elif "执行失败!" in result:
            return False
        else:
            # 如果都没有，检查是否有错误信息
            if "错误:" in result or "ERROR" in result or "[ERROR]" in result:
                return False
            # 默认情况下，如果收到了结果就认为成功（兼容旧版本）
            return True

    def filter_result(self, result):
        """过滤结果，仅保留Performance及之后的内容"""
        if "Performance:" in result:
            # 找到Performance的位置
            perf_index = result.find("Performance:")
            return result[perf_index:]
        else:
            # 如果没有Performance，返回所有内容
            return result

    def parse_and_export_results(
        self, input_file: str, output_prefix: str = None
    ) -> bool:
        """
        Parse NPU test results file and generate Markdown/CSV files.

        Args:
            input_file: Path to the input results file
            output_prefix: Output file prefix (without extension), defaults to input filename

        Returns:
            True if successful, False otherwise
        """
        if not HAS_PANDAS:
            print("Warning: pandas not installed, skipping result parsing")
            return False

        input_path = Path(input_file)
        if not input_path.exists():
            print(f"Error: Input file not found: {input_file}")
            return False

        # Set output prefix
        if output_prefix is None:
            output_prefix = str(input_path.with_suffix(""))

        output_md = f"{output_prefix}.md"
        output_csv = f"{output_prefix}.csv"

        print(f"Reading file: {input_file}")
        with open(input_file, "r", encoding="utf-8") as f:
            content = f.read()

        print("Parsing test results...")

        # --- Parse model blocks ---
        results = []
        model_blocks = re.split(r"#{20,}\n# 模型 \d+/\d+:", content)

        for block in model_blocks[1:]:  # Skip first empty block
            # Extract model name
            model_match = re.search(r"模型名称:\s*(\S+)", block)
            if not model_match:
                continue
            model_name = model_match.group(1)

            # Check if test succeeded
            if "✓ 成功" not in block:
                continue

            parsed = {"model_name": model_name}

            # Keys to extract for each core
            each_core_keys = [
                ("Graph average time", "graph_latency_core"),
                ("Npu average time", "npu_latency_core"),
                ("Npu mem used", "npu_mem"),
                ("Cpu mem used", "cpu_mem"),
                ("Total mem used", "total_mem"),
                ("Npu MAC Utilization", "npu_mac_core"),
                ("ops", "ops"),
                ("params", "params"),
            ]

            # Keys for all cores combined
            all_core_keys = [
                ("Npu total FPS", "npu_total_fps"),
                ("Graph total FPS", "graph_total_fps"),
            ]

            # Split by core blocks
            core_blocks = re.split(r"Npu core:\s*(\d+|All)", block)

            for i in range(1, len(core_blocks), 2):
                core_id = core_blocks[i]
                core_data = core_blocks[i + 1]

                if core_id == "All":
                    for key, col_name in all_core_keys:
                        pattern = rf"{re.escape(key)}:\s*(\S+)"
                        match = re.search(pattern, core_data)
                        if match:
                            parsed[col_name] = float(match.group(1))
                else:
                    for key, col_name in each_core_keys:
                        pattern = rf"{re.escape(key)}:\s*(\S+)"
                        match = re.search(pattern, core_data)
                        if match:
                            if col_name in [
                                "npu_mem",
                                "cpu_mem",
                                "total_mem",
                                "ops",
                                "params",
                            ]:
                                parsed[col_name] = match.group(1)
                            else:
                                parsed[f"{col_name}{core_id}"] = match.group(1)

            results.append(parsed)

        print(f"Successfully parsed {len(results)} model results")

        if not results:
            print("Warning: No valid test results found")
            return False

        # --- Create DataFrame with ordered columns ---
        df = pd.DataFrame(results)
        core_numbers = sum(col.startswith("graph_latency_core") for col in df.columns)

        ordered_columns = [
            "model_name",
            "npu_mem",
            "cpu_mem",
            "total_mem",
            "graph_total_fps",
            "npu_total_fps",
        ]

        for core in range(core_numbers):
            ordered_columns.append(f"graph_latency_core{core}")
        for core in range(core_numbers):
            ordered_columns.append(f"npu_latency_core{core}")
        for core in range(core_numbers):
            ordered_columns.append(f"npu_mac_core{core}")

        ordered_columns.extend(["ops", "params"])
        final_columns = [col for col in ordered_columns if col in df.columns]
        df = df[final_columns]

        print(f"DataFrame shape: {df.shape}")

        # --- Write output files ---
        with open(output_md, "w", encoding="utf-8") as f:
            f.write(df.to_markdown(index=False))
        print(f"✓ Markdown file saved: {output_md}")

        df.to_csv(output_csv, index=False, encoding="utf-8")
        print(f"✓ CSV file saved: {output_csv}")

        return True

    def save_result(self, model_name, success, result):
        """保存测试结果到文件"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        result_file = self.result_dir / f"{model_name}_{timestamp}.txt"

        with open(result_file, "w", encoding="utf-8") as f:
            f.write(f"模型名称: {model_name}\n")
            f.write(f"测试时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"测试结果: {'成功' if success else '失败'}\n")
            f.write(f"\n{'='*60}\n")
            f.write("执行输出:\n")
            f.write(f"{'='*60}\n")
            f.write(result)

        print(f"\n结果已保存到: {result_file}")

    def run_all_models(self, models_root_dir):
        """
        运行所有模型测试
        :param models_root_dir: 模型根目录路径
        """
        models_root = Path(models_root_dir)

        if not models_root.exists():
            print(f"错误: 路径不存在: {models_root}")
            return

        # 获取所有子文件夹
        model_folders = [d for d in models_root.iterdir() if d.is_dir()]

        if not model_folders:
            print(f"错误: 未找到任何模型文件夹")
            return

        print(f"找到 {len(model_folders)} 个模型文件夹")

        # 创建统一的时间戳
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

        # 创建汇总结果文件
        summary_file = self.result_dir / f"summary_{timestamp}.txt"
        # 创建详细结果文件（包含所有模型的完整输出）
        detailed_file = self.result_dir / f"all_results_{timestamp}.txt"

        results = []

        # 先写入详细结果文件的头部信息，然后保持文件打开以便实时追加
        with open(detailed_file, "w", encoding="utf-8") as f:
            f.write(f"{'='*80}\n")
            f.write(f"所有模型详细测试结果\n")
            f.write(f"{'='*80}\n")
            f.write(f"测试开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"总计模型数: {len(model_folders)}\n")
            f.write(f"{'='*80}\n\n\n")

        for i, model_folder in enumerate(model_folders, 1):
            print(f"\n\n[{i}/{len(model_folders)}] 处理模型文件夹: {model_folder.name}")

            success, result = self.test_model(model_folder)

            # 过滤结果，只保留Performance及之后的内容
            filtered_result = self.filter_result(result)

            results.append(
                {
                    "model_name": model_folder.name,
                    "success": success,
                    "result": filtered_result,
                }
            )

            # 实时追加写入详细结果到文件
            with open(detailed_file, "a", encoding="utf-8") as f:
                f.write(f"\n\n{'#'*80}\n")
                f.write(f"# 模型 {i}/{len(model_folders)}: {model_folder.name}\n")
                f.write(f"{'#'*80}\n\n")
                f.write(f"模型名称: {model_folder.name}\n")
                f.write(f"测试时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f"测试结果: {'✓ 成功' if success else '✗ 失败'}\n")
                f.write(f"\n{'-'*80}\n")
                f.write(f"执行输出:\n")
                f.write(f"{'-'*80}\n")
                f.write(filtered_result)
                f.write(f"\n{'-'*80}\n")
                f.flush()  # 立即刷新到磁盘

            print(f"结果已实时写入: {detailed_file}")

            # 等待一下再处理下一个模型
            if i < len(model_folders):
                print("\n等待3秒后处理下一个模型...")
                time.sleep(3)

        # 在详细结果文件末尾追加汇总信息
        with open(detailed_file, "a", encoding="utf-8") as f:
            f.write(f"\n\n\n{'='*80}\n")
            f.write(f"测试完成汇总\n")
            f.write(f"{'='*80}\n")
            f.write(f"测试结束时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"总计模型数: {len(results)}\n")
            f.write(f"成功: {sum(1 for r in results if r['success'])}\n")
            f.write(f"失败: {sum(1 for r in results if not r['success'])}\n")
            f.write(f"{'='*80}\n")

        # 保存汇总结果
        with open(summary_file, "w", encoding="utf-8") as f:
            f.write(f"模型测试汇总报告\n")
            f.write(f"测试时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"总计模型数: {len(results)}\n")
            f.write(f"成功: {sum(1 for r in results if r['success'])}\n")
            f.write(f"失败: {sum(1 for r in results if not r['success'])}\n")
            f.write(f"\n{'='*80}\n\n")

            for i, result in enumerate(results, 1):
                f.write(
                    f"{i}. {result['model_name']}: {'✓ 成功' if result['success'] else '✗ 失败'}\n"
                )

        self.parse_and_export_results(
            str(detailed_file), str(self.result_dir / f"results_{timestamp}")
        )

        print(f"\n\n{'='*80}")
        print(f"所有模型测试完成!")
        print(f"汇总报告已保存到: {summary_file}")
        print(f"详细结果已保存到: {detailed_file}")
        print(f"{'='*80}")


def main():
    import argparse

    parser = argparse.ArgumentParser(description="x86端模型测试客户端")
    parser.add_argument("--host", required=True, help="ARM开发板的IP地址")
    parser.add_argument(
        "--port", type=int, default=9999, help="ARM开发板监听的端口 (默认: 9999)"
    )
    parser.add_argument("--models-dir", required=True, help="模型根目录路径")
    parser.add_argument(
        "--no-golden",
        action="store_true",
        help="不使用golden文件进行数据对比（默认会使用）",
    )
    parser.add_argument(
        "--repeat-count", type=int, help="重复执行次数（默认使用服务器默认值10）"
    )
    parser.add_argument(
        "--npu-cores",
        type=str,
        help="NPU核心编号，例如: '0,1,2,3'（默认使用服务器默认值'0,1,2,3'）",
    )
    parser.add_argument(
        "--peak-performance",
        type=float,
        help="峰值性能，例如: 4.0（默认使用服务器默认值4.0）",
    )

    args = parser.parse_args()

    # 直接构建 npu_runner 的命令行参数列表
    runner_args = []
    if args.repeat_count is not None:
        runner_args.extend(["-r", str(args.repeat_count)])
    if args.npu_cores is not None:
        runner_args.extend(["-n", args.npu_cores])
    if args.peak_performance is not None:
        runner_args.extend(["--peak_performance", str(args.peak_performance)])

    client = ModelTesterClient(
        args.host,
        args.port,
        runner_args=runner_args,
        use_golden=not args.no_golden,
    )
    client.run_all_models(args.models_dir)


if __name__ == "__main__":
    main()
