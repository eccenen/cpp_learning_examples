#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ARM Board Server Script
Functions: Receive model files, execute npu_runner, return results, cleanup files
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
import re


class ModelTesterServer:
    def __init__(
        self,
        port=9999,
        model_dir="",
        runner_path="",
    ):
        """
        Initialize server
        :param port: Listening port
        :param model_dir: Model storage directory
        :param runner_path: npu_runner executable path
        """
        self.port = port
        self.model_dir = Path(model_dir)
        self.runner_path = runner_path
        self.running = True

        # Ensure model directory exists
        self.model_dir.mkdir(parents=True, exist_ok=True)

        # Setup signal handlers
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)

    def signal_handler(self, sig, frame):
        """Handle exit signal"""
        print("\nReceived exit signal, shutting down server...")
        self.running = False
        sys.exit(0)

    def recv_exact(self, conn, size):
        """Receive exact number of bytes"""
        data = b""
        while len(data) < size:
            chunk = conn.recv(size - len(data))
            if not chunk:
                return None
            data += chunk
        return data

    def receive_file(self, conn, save_path):
        """Receive file via socket"""
        # Receive filename length and filename
        name_length_data = self.recv_exact(conn, 4)
        if not name_length_data:
            return False, None
        name_length = struct.unpack("!I", name_length_data)[0]

        file_name_bytes = self.recv_exact(conn, name_length)
        if not file_name_bytes:
            return False, None
        file_name = file_name_bytes.decode("utf-8")

        # Receive file size
        size_data = self.recv_exact(conn, 8)
        if not size_data:
            return False, None
        file_size = struct.unpack("!Q", size_data)[0]

        # Determine save path
        if "/" in file_name:  # If filename contains path (e.g. golden/xxx.bin)
            full_path = save_path / file_name
            full_path.parent.mkdir(parents=True, exist_ok=True)
        else:
            full_path = save_path / file_name

        # Receive file content
        received = 0
        with open(full_path, "wb") as f:
            while received < file_size:
                chunk_size = min(8192, file_size - received)
                chunk = conn.recv(chunk_size)
                if not chunk:
                    return False, None
                f.write(chunk)
                received += len(chunk)

        print(f"  Received: {file_name} ({file_size} bytes) -> {full_path}")
        return True, file_name

    def send_message(self, conn, message):
        """Send message"""
        msg_bytes = message.encode("utf-8")
        conn.sendall(struct.pack("!I", len(msg_bytes)))
        conn.sendall(msg_bytes)

    def run_model(self, model_path, cmd_args):
        """
        Run model
        :param model_path: Model path
        :param cmd_args: Parsed command line arguments list
        :return: Execution result string
        """
        # Build complete command
        cmd = [self.runner_path] + cmd_args

        print(f"\nExecuting command: {' '.join(cmd)}")
        print(f"Working directory: {model_path}")

        try:
            # Execute command and capture output
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=180,  # 3 minute timeout
                cwd=str(model_path),  # Set working directory
            )

            # Merge stdout and stderr
            output = ""
            if result.stdout:
                output += "=== STDOUT ===\n"
                output += result.stdout
            if result.stderr:
                output += "\n=== STDERR ===\n"
                output += result.stderr

            output += f"\n\n=== Return code: {result.returncode} ===\n"

            if result.returncode == 0:
                output += "Execution successful!\n"
            else:
                output += "Execution failed!\n"

            return output

        except subprocess.TimeoutExpired:
            error_msg = "Error: Command execution timeout (3 minutes)"
            print(error_msg)
            return error_msg
        except FileNotFoundError:
            error_msg = f"Error: npu_runner not found: {self.runner_path}"
            print(error_msg)
            return error_msg
        except Exception as e:
            error_msg = f"Error: Exception occurred during command execution: {str(e)}"
            print(error_msg)
            import traceback

            traceback.print_exc()
            return error_msg

    def cleanup_model(self, model_path):
        """Cleanup model files"""
        try:
            if model_path.exists():
                shutil.rmtree(model_path)
                print(f"Cleaned up model files: {model_path}")
                return True
        except Exception as e:
            print(f"Failed to cleanup model files: {e}")
            return False
        return False

    def parse_command_args(self, command, model_path, model_name):
        """
        Parse command line arguments
        :param command: Command dictionary received from client
        :param model_path: Model path
        :param model_name: Model name
        :return: (success, cmd_args or error_msg)
        """
        model_file_base = model_path / model_name

        # Check if model files exist
        bin_file = Path(str(model_file_base) + ".bin")
        param_file = Path(str(model_file_base) + ".param")

        if not bin_file.exists():
            return False, f"Error: Model file does not exist: {bin_file}"
        if not param_file.exists():
            return False, f"Error: Parameter file does not exist: {param_file}"

        # Build command line arguments list: -m parameter (required)
        cmd_args = ["-m", str(model_file_base)]

        # Handle golden parameter (-g)
        if command.get("use_golden"):
            golden_path = model_path / "golden"
            if not golden_path.exists():
                return False, f"Error: Golden directory does not exist: {golden_path}"
            cmd_args.extend(["-g", str(golden_path)])

        # Directly append runner_args from client (already in npu_runner format)
        runner_args = command.get("runner_args", [])
        if runner_args:
            cmd_args.extend(runner_args)
            print(f"  Appending client args: {runner_args}")

        return True, cmd_args

    def calculate_repeat_count(self, model_path, cmd_args, execution_times):
        """
        Calculate repeat count based on execution time
        """
        print(f"Calculating repeat count for execution time: {execution_times}ms")

        # Create temp args with -r 1
        temp_args = cmd_args.copy()
        if "-r" in temp_args:
            idx = temp_args.index("-r")
            if idx + 1 < len(temp_args):
                temp_args[idx + 1] = "1"
        else:
            temp_args.extend(["-r", "1"])

        # Run model once
        print("Running single inference to measure time...")
        result = self.run_model(model_path, temp_args)

        # Parse Graph average time
        # Look for "Graph average time: 65.8252 ms"
        matches = re.findall(r"Graph average time:\s*(\d+(?:\.\d+)?)", result)
        if matches:
            print(f"Found Graph average times: {matches}")
            times = [float(t) for t in matches]
            max_time = max(times)
            print(f"Measured Graph average time (max): {max_time} ms")

            if max_time > 0:
                new_r = int(execution_times / max_time)
                print(f"Calculated new repeat count: {new_r}")
                return new_r

        print(
            "Warning: Could not measure graph time or time is 0, keeping original settings"
        )
        return None

    def handle_client(self, conn, addr):
        """Handle client connection"""
        print(f"\n{'='*60}")
        print(f"Client connected: {addr[0]}:{addr[1]}")
        print(f"{'='*60}")

        try:
            # Receive command
            cmd_length_data = self.recv_exact(conn, 4)
            if not cmd_length_data:
                print("Error: No command received")
                return

            cmd_length = struct.unpack("!I", cmd_length_data)[0]
            cmd_data = self.recv_exact(conn, cmd_length)
            if not cmd_data:
                print("Error: Command data incomplete")
                return

            command = json.loads(cmd_data.decode("utf-8"))
            print(f"Received command: {command}")

            if command.get("command") != "run_model":
                print("Error: Unsupported command")
                self.send_message(conn, "Error: Unsupported command")
                return

            model_name = command.get("model_name")
            if not model_name:
                print("Error: Missing model name")
                self.send_message(conn, "Error: Missing model name")
                return

            # Create model directory
            model_path = self.model_dir / model_name
            if model_path.exists():
                print(f"Cleaning old model directory: {model_path}")
                shutil.rmtree(model_path)
            model_path.mkdir(parents=True, exist_ok=True)

            # Receive file count
            file_count_data = self.recv_exact(conn, 4)
            if not file_count_data:
                print("Error: No file count received")
                return

            file_count = struct.unpack("!I", file_count_data)[0]
            print(f"Preparing to receive {file_count} files...")

            # Receive all files
            for i in range(file_count):
                success, file_name = self.receive_file(conn, model_path)
                if not success:
                    print(f"Error: Failed to receive file {i+1}")
                    self.send_message(conn, f"Error: Failed to receive file")
                    self.cleanup_model(model_path)
                    return

            print(f"\nAll files received!")

            # Parse command line arguments
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
            print(f"Parsed command line args: {cmd_args}")

            # Check if we need to calculate repeat count based on execution_times
            execution_times = command.get("execution_times", -1)
            if execution_times != -1:
                new_r = self.calculate_repeat_count(
                    model_path, cmd_args, execution_times
                )
                if new_r is not None:
                    # If calculated repeat count is 0 (execution_times < model run time), set to 1
                    if new_r == 0:
                        new_r = 1
                        print("Calculated repeat count is 0, setting to 1")

                    # Update cmd_args with new -r
                    if "-r" in cmd_args:
                        idx = cmd_args.index("-r")
                        if idx + 1 < len(cmd_args):
                            cmd_args[idx + 1] = str(new_r)
                    else:
                        cmd_args.extend(["-r", str(new_r)])
                    print(
                        f"Updated command line args with new repeat count: {cmd_args}"
                    )

            # Run model
            result = self.run_model(model_path, cmd_args)

            # Send result
            print(f"\nSending execution result to client...")
            self.send_message(conn, result)

            # Cleanup model files
            print(f"\nCleaning up model files...")
            self.cleanup_model(model_path)

            print(f"\nTask completed!")

        except Exception as e:
            print(f"Error occurred while handling client request: {e}")
            import traceback

            traceback.print_exc()
            try:
                self.send_message(conn, f"Server error: {str(e)}")
            except Exception as send_err:
                print(f"Failed to send error message: {send_err}")
        finally:
            try:
                conn.close()
            except:
                pass
            print(f"Connection closed: {addr[0]}:{addr[1]}")

    def start(self):
        """Start server"""
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(("0.0.0.0", self.port))
        server_socket.listen(5)

        print(f"{'='*60}")
        print(f"ARM Model Test Server Started")
        print(f"Listening port: {self.port}")
        print(f"Model directory: {self.model_dir}")
        print(f"Runner path: {self.runner_path}")
        print(f"{'='*60}\n")
        print("Waiting for client connection...\n")

        while self.running:
            try:
                server_socket.settimeout(1.0)  # Set timeout to respond to exit signal
                try:
                    conn, addr = server_socket.accept()
                    self.handle_client(conn, addr)
                except socket.timeout:
                    continue
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"Server error: {e}")

        server_socket.close()
        print("\nServer closed")


def main():
    import argparse

    parser = argparse.ArgumentParser(description="ARM Board Model Test Server")
    parser.add_argument(
        "--port", type=int, default=9999, help="Listening port (default: 9999)"
    )
    parser.add_argument(
        "--model-dir",
        default=str(Path(__file__).resolve().parent / "models"),
        help="Model storage directory (default: ./models)",
    )
    parser.add_argument(
        "--runner",
        default=str(
            Path(__file__).resolve().parent / "nrt" / "npuv1" / "bin" / "npu_runner"
        ),
        help="npu_runner executable path (default: ./nrt/npuv1/bin/npu_runner)",
    )

    args = parser.parse_args()

    server = ModelTesterServer(args.port, args.model_dir, args.runner)
    server.start()


if __name__ == "__main__":
    main()
