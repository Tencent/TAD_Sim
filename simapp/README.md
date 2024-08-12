# simapp

## 目录结构
    --desktop            桌面端业务界面
    --map-editor         地图编辑器界面
    --scene-editor       场景编辑器

## 如何启动

```shell
## 安装依赖
npm install

## 启动项目
npm run dev:scene
npm run dev:desktop
npm run dev:map
```

## 开发注意事项

* 初次运行 `dev:scene` 可能会缺少 `assets` 资源，再次重启即可
* 将 `service/` 文件夹拷贝至 `simapp/desktop/service/`，桌面应用才能正常访问接口
