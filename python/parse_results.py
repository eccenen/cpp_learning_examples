#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
解析NPU测试结果文件，生成Markdown和CSV文件
"""

import re
from typing import Dict, List, Optional
import pandas as pd


def read_file(filepath: str) -> str:
    """读取文件内容"""
    with open(filepath, "r", encoding="utf-8") as f:
        return f.read()


def write_markdown(df: pd.DataFrame, filepath: str) -> None:
    """将DataFrame写入Markdown文件"""
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(df.to_markdown(index=False))
    print(f"✓ Markdown文件已保存: {filepath}")


def write_csv(df: pd.DataFrame, filepath: str) -> None:
    """将DataFrame写入CSV文件"""
    df.to_csv(filepath, index=False, encoding="utf-8")
    print(f"✓ CSV文件已保存: {filepath}")


def parse_model_block(block: str) -> Optional[Dict]:
    """
    解析单个模型的测试结果块

    Args:
        block: 包含单个模型完整测试结果的文本块

    Returns:
        包含解析结果的字典，如果解析失败则返回None
    """
    # 提取模型名称
    model_match = re.search(r"模型名称:\s*(\S+)", block)
    if not model_match:
        return None

    model_name = model_match.group(1)

    # 检查测试是否成功
    if "✓ 成功" not in block:
        return None

    result = {"model_name": model_name}

    # 定义需要提取的键值对映射
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

    all_core_keys = [
        ("Npu total FPS", "npu_total_fps"),
        ("Graph total FPS", "graph_total_fps"),
    ]

    # 分割不同核心的数据块
    core_blocks = re.split(r"Npu core:\s*(\d+|All)", block)

    # 处理每个核心的数据块
    for i in range(1, len(core_blocks), 2):
        core_id = core_blocks[i]
        core_data = core_blocks[i + 1]

        if core_id == "All":
            for key, col_name in all_core_keys:
                pattern = rf"{re.escape(key)}:\s*(\S+)"
                match = re.search(pattern, core_data)
                if match:
                    result[col_name] = float(match.group(1))
        else:
            for key, col_name in each_core_keys:
                pattern = rf"{re.escape(key)}:\s*(\S+)"
                match = re.search(pattern, core_data)
                if match:
                    if col_name in ["npu_mem", "cpu_mem", "total_mem", "ops", "params"]:
                        result[col_name] = match.group(1)
                    else:
                        result[f"{col_name}{core_id}"] = match.group(1)

    return result


def parse_results(content: str) -> List[Dict]:
    """
    解析整个文件内容，提取所有模型的测试结果

    Args:
        content: 完整的文件内容

    Returns:
        包含所有模型测试结果的列表
    """
    results = []

    # 使用正则表达式分割不同的模型块
    # 模型块以 "# 模型 数字/数字: 模型名称" 开始
    model_blocks = re.split(r"#{20,}\n# 模型 \d+/\d+:", content)

    for block in model_blocks[1:]:  # 跳过第一个空块
        parsed = parse_model_block(block)
        if parsed:
            results.append(parsed)

    return results


def create_dataframe(results: List[Dict]) -> pd.DataFrame:
    """
    将解析结果转换为DataFrame，并按照指定顺序排列列

    Args:
        results: 解析得到的结果列表

    Returns:
        整理好的DataFrame
    """
    if not results:
        return pd.DataFrame()

    df = pd.DataFrame(results)

    core_numbers = sum(col.startswith("graph_latency_core") for col in df.columns)

    # 按照示例顺序构建列名
    ordered_columns = [
        "model_name",
        "npu_mem",
        "cpu_mem",
        "total_mem",
        "graph_total_fps",
        "npu_total_fps",
    ]

    # 添加各个核心的延迟列
    for core in range(core_numbers):
        ordered_columns.append(f"graph_latency_core{core}")

    for core in range(core_numbers):
        ordered_columns.append(f"npu_latency_core{core}")

    # 添加各个核心的MAC利用率列
    for core in range(core_numbers):
        ordered_columns.append(f"npu_mac_core{core}")

    # 添加ops和params
    ordered_columns.extend(["ops", "params"])

    # 只保留存在的列
    final_columns = [col for col in ordered_columns if col in df.columns]

    return df[final_columns]


def main():
    """主函数"""
    # 输入输出文件路径
    input_file = "all_results_20251225_101754.txt"
    output_md = "all_results_20251225_144031.md"
    output_csv = "all_results_20251225_144031.csv"

    print(f"正在读取文件: {input_file}")
    content = read_file(input_file)

    print("正在解析测试结果...")
    results = parse_results(content)
    print(f"成功解析 {len(results)} 个模型的测试结果")

    if not results:
        print("警告: 没有找到有效的测试结果")
        return

    print("正在创建DataFrame...")
    df = create_dataframe(results)

    print(f"DataFrame形状: {df.shape}")
    print("\n前5行预览:")
    print(df.head())

    # 写入文件
    write_markdown(df, output_md)
    write_csv(df, output_csv)

    print("\n处理完成!")


if __name__ == "__main__":
    main()
