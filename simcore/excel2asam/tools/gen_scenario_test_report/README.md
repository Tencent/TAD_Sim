# 数据准备
27寸显示器, 分辨率 3840 * 2160, tadsim 全屏录制, Display 可以作为小窗口放在右下角

# 环境准备
```bash
# 1. 推荐使用 python 虚拟环境 (此步骤可跳过)
python -m venv myvenv

# 2. 激活虚拟环境 (此步骤可跳过)
# # windows (手动敲, 不能复制, windows 不知道为啥)
myvnev\Scripts\activate
# # linux
source myvenv/bin/activate

# 3. 安装依赖库
pip install -r requirements.txt
```

# 运行
```bash
# 运行脚本  (your_video_path 替换为实际视频文件)
python video_processor.py -f your_video_path

```
