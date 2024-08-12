# TAD Sim Docs Generation

基于 code 和 markdown 文件, 自动构建生成 html, 支持
- TAD Sim 单机版 - 用户使用手册生成 (user_guidelines_desktop)

# Getting Started

## Prerequisites

Git & Docker: should be installed.

## Installing

```bash
# Docker build and run by dockerfile and Configure development environment
cd docs
python3 startup.py  # for all with python
```

# Usage

## 通过标记符号, 区分其中内容在不同版本出现 (对空格不明感)
```markdown
<!-- ifconfig:: edition in (education, community) -->
<!-- ifconfig:: edition not in (education, community) -->

xxxx  infos

<!-- end -->
```

## 插入图片方式
```
<div align="center"><img src="./_static/images/image179.png" alt="" width="700px"></div><br>
```

## 插入视频的方式
```
<figure class="center-video">
  <video controls width="700">
    <source src="./_static/videos/xxx.mp4" type="video/mp4">
    Your browser does not support the video tag.
  </video>
  <figcaption>xxxxxx</figcaption>
</figure>
```

## Build html

```bash
# gen user guidelines of desktop
python3 generate_doc.py -p desktop -e standard  # ["standard", "education", "community"]

```

# Tools

## Convert word file to markdown file

```bash
# 下载 pandoc
./pandoc -s input.docx -t markdown -o output.md --extract-media=./images
```

转换为 .md 文档后, 图片如果需要设置居中及大小可调整, 需要进行如下替换 (可以全局替换)
```
![](
<div align="center"><img src="

.png)
.png" alt="" width="700px"></div><br>
```

# Known Issues
- 中文搜索不支持

# Contributing

# Versioning

- V1.0.0

# Authors


# License

This project is licensed under the Tencent Co. Ltd.
