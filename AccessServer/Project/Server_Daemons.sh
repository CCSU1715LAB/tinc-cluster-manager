#!/bin/sh

# 获取脚本所在的目录
workdir=$(dirname "$(readlink -f "$0")")

# 检查工作目录中是否存在需要的动态链接库
if [ -d "$workdir" ]; then
  export LD_LIBRARY_PATH="$workdir"
else
  echo "无法找到工作目录：$workdir"
  exit 1
fi

# 切换到工作目录
cd "$workdir" || exit 1

# 执行access_daemons程序
./access_daemons
