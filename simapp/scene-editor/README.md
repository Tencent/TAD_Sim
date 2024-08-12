# scene-editor

## 如何启动

```shell
## 安装依赖
npm install

## 启动项目
npm run dev
```

*注意：启动项目后，若根目录下没有生成 `assets` 文件夹，请重新启动直到出现该文件夹*
*注意：`sim-desktop` 仓库需后端文件(即 `service` 文件夹)，找大佬指点如何获得，然后放入 `sim-desktop` 文件夹中*

## 新加主车步骤
1. 找美术制作模型，注意原点必须在几何中心上
2. 一个模型的所有文件（包括图标）放在一个目录里，然后把目录放进data/model里
3. 将图标文件重命名，名称和fbx文件一样
4. 在catalogs里面创建一个动力学文件
5. 在catalogs里面创建一个传感器组
6. 在catalogs/vehicles/default.xosc里面创建一个主车配置
