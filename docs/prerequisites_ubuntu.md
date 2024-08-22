# ubuntu 平台开发指引 - desktop

## 1. 编译镜像 - desktop

### 1.1 基于 Dockerfile 编译镜像
```bash
# 切换路径为项目根目录
cd TAD_Sim

# 使用 docker build 按当前平台进行构建
# 注意如果本机是 arm 芯片, 则需要使用下面 buildx 方式进行构建
# 此处指定了腾讯云镜像源, 如果不指定将默认使用 docker.io/library/ 官方镜像源
docker build . -t tadsim:desktop_v1.0 --build-arg BASE_MIRROR=ccr.ccs.tencentyun.com/library/

# 使用 docker buildx 选择平台（linux/amd64 或 linux/arm64）进行构建
# 目前仅 linux/amd64 被验证, 此处保留扩展能力
docker buildx build . --platform linux/amd64 -t tadsim:desktop_v1.0 BASE_MIRROR=ccr.ccs.tencentyun.com/library/

# 对于网络限制情况, 提供本地方式
# 需注意此时版本升级将受到限制, 除了 dockerfile 中更改外, 还需要同步更新 TAD_Sim/tools/docker_libraries 下面的压缩包版本
docker build . -t tadsim:desktop_v1.0 --build-arg BASE_MIRROR=ccr.ccs.tencentyun.com/library/ -f ./Dockerfile_local

```
