#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import subprocess
from pathlib import Path

# Define variables
USER_NAME = "excel2asam"
CONTAINER_NAME = "ubuntu2004_py3118"
TAG = "v1.0"
WORK_DIR = "work"
GPU_OPTIONS = ""
BASE_MIRROR = "ccr.ccs.tencentyun.com/library/"

# Current script path
SCRIPT_DIR = Path(__file__).resolve().parent


def set_permissions() -> None:
    SCRIPT_DIR.chmod(0o777)


def check_images() -> bool:
    print("\033[42;37m[INFO] Checking images \033[0m")
    images = subprocess.run(
        f"docker images -q {USER_NAME}/{CONTAINER_NAME}:{TAG}",
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    ).stdout
    print(f"{images = }")
    return len(images.strip()) > 0


def docker_pull() -> bool:
    print("\033[42;37m[INFO] Docker pulling \033[0m")
    result = subprocess.run(f"docker pull {USER_NAME}/{CONTAINER_NAME}:{TAG}", shell=True)
    return result.returncode == 0


def docker_build() -> bool:
    print("\033[42;37m[INFO] Docker pulling failed, start building \033[0m")
    result = subprocess.run(
        f"docker buildx build . --platform=linux/amd64 -t {USER_NAME}/{CONTAINER_NAME}:{TAG} --build-arg={BASE_MIRROR}",
        shell=True,
    )
    return result.returncode == 0


def check_container_id() -> bool:
    print("\033[42;37m[INFO] Checking container id \033[0m")
    # Check is container already exits
    docker_ps_args = ["docker", "ps", "--filter", f"name={CONTAINER_NAME}", "-q", "-a"]
    result = subprocess.run(docker_ps_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    container_id = result.stdout.strip()
    print(f"{container_id = }")
    return container_id != ""


def stop_rm_container() -> None:
    print("\033[42;37m[INFO] Stop and remove container \033[0m")
    subprocess.run(f"docker stop {CONTAINER_NAME}", shell=True)
    subprocess.run(f"docker rm {CONTAINER_NAME}", shell=True)


def run_container() -> None:
    print("\033[42;37m[INFO] Docker running \033[0m")
    docker_run_args = [
        "docker",
        "run",
        "-d",
        "--network",
        "host",
        "--privileged",
        "--name",
        CONTAINER_NAME,
        "-v",
        f"{SCRIPT_DIR}:/{WORK_DIR}/",
        "-it",
        f"{USER_NAME}/{CONTAINER_NAME}:{TAG}",
    ]
    if GPU_OPTIONS:
        docker_run_args.insert(2, GPU_OPTIONS)
    subprocess.run(docker_run_args)


def setup_env() -> None:
    print("\033[42;37m[INFO] Setup working environment \033[0m")
    subprocess.run(f"docker exec -it {CONTAINER_NAME} python3 setup.py develop", shell=True)


def finish_all() -> None:
    print("\033[42;37m[INFO] Finished All \033[0m")
    subprocess.run(f"docker exec -it {CONTAINER_NAME} bash", shell=True)


def main() -> None:
    set_permissions()
    if not check_images():
        if not docker_pull():
            docker_build()
    if check_container_id():
        stop_rm_container()
    run_container()
    setup_env()
    finish_all()


if __name__ == "__main__":
    main()
