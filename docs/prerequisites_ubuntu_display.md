# ubuntu 平台开发指引 - display

## 2. 编译镜像 - display

### 2.1 获得 UnrealEngine 私有仓库权限
- 注册 Epic Game 账号并登录且关联 github id, 细节请查看 [Epic Games README](https://github.com/EpicGames)
  <div align="center"><img src="./images/epicgames_req.png" alt="" width="900px"></div><br>

- github 中 follow EpicGames, 需要确保出现下方的 private 仓库, 如未出现请以 UnrealEngine 最新的说明寻找原因.
  <div align="center"><img src="./images/epicgames_unrealengine_github_private_repo.png" alt="" width="900px"></div><br>

### 2.2 获得 github Container registry
- github 中创建 Tokerns, 细节请查看 [Working with the Container registry](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry)
- 复制 TOKEN
- 登录
  ```bash
  # 替换 USERNAME 和 TOKEN 为使用者 github 的信息
  docker login ghcr.io -u USERNAME -p TOKEN
  ```

### 2.3 基于 Dockerfile_display 编译镜像
```bash
# 切换路径为项目根目录
cd TAD_Sim

# 使用 docker build 按当前平台进行构建
# 注意如果本机是 arm 芯片, 则需要使用下面 buildx 方式进行构建
docker build . -t tadsim/display:v1.0 -f ./Dockerfile_display

# 使用 docker buildx 选择平台（linux/amd64 或 linux/arm64）进行构建
# 目前仅支持 linux/amd64
docker buildx build . --platform linux/amd64 -t tadsim/display:v1.0 -f ./Dockerfile_display

```
