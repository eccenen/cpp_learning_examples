#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
x86 Client Script
Functions: Scan model folders, transfer model files to ARM board, trigger execution, collect results
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
    def __init__(
        self,
        arm_host,
        arm_port=9999,
        runner_args=None,
        use_golden=False,
        execution_times=-1,
    ):
        """
        Initialize client
        :param arm_host: ARM board IP address
        :param arm_port: ARM board listening port
        :param runner_args: npu_runner command line arguments list, e.g.: ["-r", "10", "-n", "0,1"]
        :param use_golden: Whether to use golden data comparison
        """
        self.arm_host = arm_host
        self.arm_port = arm_port
        self.runner_args = runner_args or []
        self.use_golden = use_golden
        self.execution_times = execution_times
        self.result_dir = Path(__file__).resolve().parent / "test_results"
        self.result_dir.mkdir(exist_ok=True)

    def send_file(self, sock, file_path):
        """Send file via socket"""
        file_size = os.path.getsize(file_path)
        file_name = os.path.basename(file_path)

        # Send filename length and filename
        file_name_bytes = file_name.encode("utf-8")
        sock.sendall(struct.pack("!I", len(file_name_bytes)))
        sock.sendall(file_name_bytes)

        # Send file size
        sock.sendall(struct.pack("!Q", file_size))

        # Send file content
        with open(file_path, "rb") as f:
            sent = 0
            while sent < file_size:
                chunk = f.read(8192)
                if not chunk:
                    break
                sock.sendall(chunk)
                sent += len(chunk)

        print(f"  Sent: {file_name} ({file_size} bytes)")

    def receive_message(self, sock):
        """Receive message"""
        # Receive message length
        length_data = self.recv_exact(sock, 4)
        if not length_data:
            return None
        msg_length = struct.unpack("!I", length_data)[0]

        # Receive message content
        msg_data = self.recv_exact(sock, msg_length)
        if not msg_data:
            return None

        return msg_data.decode("utf-8")

    def recv_exact(self, sock, size):
        """Receive exact number of bytes"""
        data = b""
        while len(data) < size:
            chunk = sock.recv(size - len(data))
            if not chunk:
                return None
            data += chunk
        return data

    def test_model(self, model_folder_path):
        """
        Test a single model
        :param model_folder_path: Model folder path
        :return: (success, result_message)
        """
        model_folder = Path(model_folder_path)
        model_name = model_folder.name

        print(f"\n{'='*60}")
        print(f"Testing model: {model_name}")
        print(f"{'='*60}")

        # Find .bin and .param files
        bin_file = None
        param_file = None

        for file in model_folder.iterdir():
            if file.suffix == ".bin" and file.stem == model_name:
                bin_file = file
            elif file.suffix == ".param" and file.stem == model_name:
                param_file = file

        if not bin_file or not param_file:
            error_msg = f"Error: Model files incomplete, missing .bin or .param file"
            print(error_msg)
            return False, error_msg

        # Check golden folder (only when data comparison is needed)
        golden_dir = model_folder / "golden"
        golden_files = []
        if self.use_golden:
            if not golden_dir.exists() or not golden_dir.is_dir():
                error_msg = f"Error: Missing golden data folder"
                print(error_msg)
                return False, error_msg
            golden_files = [f for f in golden_dir.glob("*") if f.is_file()]

        try:
            # Connect to ARM board
            print(f"Connecting to ARM board {self.arm_host}:{self.arm_port}...")
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(300)  # 5 minute timeout
            sock.connect((self.arm_host, self.arm_port))
            print("Connected!")

            # Build command with npu_runner args
            command = {
                "command": "run_model",
                "model_name": model_name,
                "use_golden": self.use_golden,
                "execution_times": self.execution_times,
                "runner_args": self.runner_args,  # Pass command line args directly
            }

            # Send command
            command_json = json.dumps(command)
            command_bytes = command_json.encode("utf-8")
            sock.sendall(struct.pack("!I", len(command_bytes)))
            sock.sendall(command_bytes)

            # Send file count (.bin, .param, all files in golden folder)
            total_files = 2 + len(golden_files)
            if self.use_golden and golden_files:
                print(
                    f"Preparing to send {total_files} files (.bin, .param, {len(golden_files)} golden files)"
                )
            else:
                print(f"Preparing to send 2 files (.bin, .param)")
            sock.sendall(struct.pack("!I", total_files))

            # Send .bin and .param files
            print("Sending model files...")
            self.send_file(sock, bin_file)
            self.send_file(sock, param_file)

            # Send all files in golden folder (only when use_golden=True)
            if self.use_golden and golden_files:
                print("Sending golden files...")
                for golden_file in golden_files:
                    # Send file with golden prefix so server knows to put it in golden subdirectory
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
                    print(f"  Sent: golden/{golden_file.name} ({file_size} bytes)")

            print("\nWaiting for ARM board to execute model...")

            # Receive execution result
            result = self.receive_message(sock)

            if result:
                print("\nReceived execution result:")
                print("-" * 60)
                print(result)
                print("-" * 60)
                # Check if execution was successful
                success = self.check_result_success(result)
                if not success:
                    print("\nWarning: Model execution failed (non-zero return code)")
                return success, result
            else:
                error_msg = "Error: No execution result received"
                print(error_msg)
                return False, error_msg

        except socket.timeout:
            error_msg = "Error: Connection timeout"
            print(error_msg)
            return False, error_msg
        except ConnectionRefusedError:
            error_msg = f"Error: Cannot connect to ARM board {self.arm_host}:{self.arm_port}, please confirm server is running"
            print(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error: {str(e)}"
            print(error_msg)
            import traceback

            traceback.print_exc()
            return False, error_msg
        finally:
            # Ensure socket is closed
            try:
                sock.close()
            except:
                pass

    def check_result_success(self, result):
        """Check if model execution result is successful"""
        # Check if return code is present
        if "=== Return code:" in result:
            # Extract return code
            try:
                returncode_line = [
                    line for line in result.split("\n") if "=== Return code:" in line
                ][0]
                returncode_str = returncode_line.split(":")[1].strip().split()[0]
                returncode = int(returncode_str)
                return returncode == 0
            except (IndexError, ValueError) as e:
                print(f"Warning: Cannot parse return code, defaulting to failure: {e}")
                return False
        # If no return code info, check for success marker
        elif "Execution successful!" in result:
            return True
        elif "Execution failed!" in result:
            return False
        else:
            # If neither, check for error messages
            if "Error:" in result or "ERROR" in result or "[ERROR]" in result:
                return False
            # Default: if result received, consider success (for backward compatibility)
            return True

    def filter_result(self, result):
        """Filter result, keep only content from Performance onwards"""
        if "Performance:" in result:
            # Find Performance position
            perf_index = result.find("Performance:")
            return result[perf_index:]
        else:
            # If no Performance, return all content
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
        model_blocks = re.split(r"#{20,}\n# Model \d+/\d+:", content)

        for block in model_blocks[1:]:  # Skip first empty block
            # Extract model name
            model_match = re.search(r"Model name:\s*(\S+)", block)
            if not model_match:
                continue
            model_name = model_match.group(1)

            # Check if test succeeded
            if "[PASS]" not in block:
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
        print(f"Markdown file saved: {output_md}")

        df.to_csv(output_csv, index=False, encoding="utf-8")
        print(f"CSV file saved: {output_csv}")

        return True

    def save_result(self, model_name, success, result):
        """Save test result to file"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        result_file = self.result_dir / f"{model_name}_{timestamp}.txt"

        with open(result_file, "w", encoding="utf-8") as f:
            f.write(f"Model name: {model_name}\n")
            f.write(f"Test time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Test result: {'PASS' if success else 'FAIL'}\n")
            f.write(f"\n{'='*60}\n")
            f.write("Execution output:\n")
            f.write(f"{'='*60}\n")
            f.write(result)

        print(f"\nResult saved to: {result_file}")

    def run_all_models(self, models_root_dir, model_list: str):
        """
        Run all model tests
        :param models_root_dir: Root directory path for models
        :param model_list: Comma-separated list of model names to test
        """
        models_root = Path(models_root_dir)
        model_names_list = (
            [name.strip() for name in model_list.split(",")] if model_list else []
        )

        if not models_root.exists():
            print(f"Error: Path does not exist: {models_root}")
            return

        # Get all subdirectories
        model_folders = [d for d in models_root.iterdir() if d.is_dir()]

        # Filter by model list if provided
        if model_names_list:
            model_folders = [d for d in model_folders if d.name in model_names_list]

            # Check if any specified models were not found
            found_models = {d.name for d in model_folders}
            missing_models = set(model_names_list) - found_models
            if missing_models:
                print(
                    f"Warning: The following specified models were not found: {', '.join(missing_models)}"
                )

        if not model_folders:
            print(f"Error: No model folders found")
            return

        print(f"Found {len(model_folders)} model folders")

        # Create unified timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

        # Create summary result file
        summary_file = self.result_dir / f"summary_{timestamp}.txt"
        # Create detailed result file (contains complete output of all models)
        detailed_file = self.result_dir / f"all_results_{timestamp}.txt"

        results = []

        # Write header info to detailed result file first
        with open(detailed_file, "w", encoding="utf-8") as f:
            f.write(f"{'='*80}\n")
            f.write(f"All Models Detailed Test Results\n")
            f.write(f"{'='*80}\n")
            f.write(
                f"Test start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n"
            )
            f.write(f"Total models: {len(model_folders)}\n")
            f.write(f"{'='*80}\n\n\n")

        for i, model_folder in enumerate(model_folders, 1):
            print(
                f"\n\n[{i}/{len(model_folders)}] Processing model folder: {model_folder.name}"
            )

            success, result = self.test_model(model_folder)

            # Filter result, keep only content from Performance onwards
            filtered_result = self.filter_result(result)

            results.append(
                {
                    "model_name": model_folder.name,
                    "success": success,
                    "result": filtered_result,
                }
            )

            # Append detailed results to file in real-time
            with open(detailed_file, "a", encoding="utf-8") as f:
                f.write(f"\n\n{'#'*80}\n")
                f.write(f"# Model {i}/{len(model_folders)}: {model_folder.name}\n")
                f.write(f"{'#'*80}\n\n")
                f.write(f"Model name: {model_folder.name}\n")
                f.write(f"Test time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f"Test result: {'[PASS]' if success else '[FAIL]'}\n")
                f.write(f"\n{'-'*80}\n")
                f.write(f"Execution output:\n")
                f.write(f"{'-'*80}\n")
                f.write(filtered_result)
                f.write(f"\n{'-'*80}\n")
                f.flush()  # Flush to disk immediately

            print(f"Results written to: {detailed_file}")

            # Wait before processing next model
            if i < len(model_folders):
                print("\nWaiting 3 seconds before processing next model...")
                time.sleep(3)

        # Append summary info at the end of detailed result file
        with open(detailed_file, "a", encoding="utf-8") as f:
            f.write(f"\n\n\n{'='*80}\n")
            f.write(f"Test Summary\n")
            f.write(f"{'='*80}\n")
            f.write(f"Test end time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Total models: {len(results)}\n")
            f.write(f"Passed: {sum(1 for r in results if r['success'])}\n")
            f.write(f"Failed: {sum(1 for r in results if not r['success'])}\n")
            f.write(f"{'='*80}\n")

        # Save summary results
        with open(summary_file, "w", encoding="utf-8") as f:
            f.write(f"Model Test Summary Report\n")
            f.write(f"Test time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Total models: {len(results)}\n")
            f.write(f"Passed: {sum(1 for r in results if r['success'])}\n")
            f.write(f"Failed: {sum(1 for r in results if not r['success'])}\n")
            f.write(f"\n{'='*80}\n\n")

            for i, result in enumerate(results, 1):
                f.write(
                    f"{i}. {result['model_name']}: {'[PASS]' if result['success'] else '[FAIL]'}\n"
                )

        self.parse_and_export_results(
            str(detailed_file), str(self.result_dir / f"results_{timestamp}")
        )

        print(f"\n\n{'='*80}")
        print(f"All model tests completed!")
        print(f"Summary report saved to: {summary_file}")
        print(f"Detailed results saved to: {detailed_file}")
        print(f"{'='*80}")


def main():
    import argparse

    parser = argparse.ArgumentParser(description="x86 Model Test Client")
    parser.add_argument("--ip", required=True, help="ARM board IP address")
    parser.add_argument(
        "--port",
        type=int,
        default=9999,
        help="ARM board listening port (default: 9999)",
    )
    parser.add_argument("--model_dir", required=True, help="Models root directory path")
    parser.add_argument(
        "-m",
        "--model_list",
        type=str,
        default="",
        help="List of specific models to test (default: all models in directory)",
    )
    parser.add_argument(
        "-e",
        "--execution_times",
        type=int,
        default=-1,
        help="Execution times per model (ms)",
    )
    parser.add_argument(
        "-r",
        "--repeat_count",
        type=int,
        default=10,
        help="Repeat execution count (default: server default 10)",
    )
    parser.add_argument(
        "-n",
        "--npu_cores",
        type=str,
        default="0,1,2,3",
        help="NPU core IDs, e.g.: '0,1,2,3' (default: server default '0,1,2,3')",
    )
    parser.add_argument(
        "--peak_performance",
        type=float,
        default=4.0,
        help="Peak performance, e.g.: 4.0 (default: server default 4.0)",
    )
    parser.add_argument(
        "--use_golden",
        action="store_true",
        help="Do not use golden files for data comparison (default: False)",
    )

    args = parser.parse_args()

    # Build npu_runner command line arguments list directly
    runner_args = [
        "-r",
        str(args.repeat_count),
        "-n",
        args.npu_cores,
        "--peak_performance",
        str(args.peak_performance),
    ]

    client = ModelTesterClient(
        args.ip,
        args.port,
        runner_args=runner_args,
        use_golden=args.use_golden,
        execution_times=args.execution_times,
    )
    client.run_all_models(args.model_dir, args.model_list)


if __name__ == "__main__":
    main()
