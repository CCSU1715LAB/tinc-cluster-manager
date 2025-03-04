#!/bin/bash

install()
{
    #安装依赖
    apt update && apt-get install  -y ca-certificates curl gnupg lsb-release apt-transport-https software-properties-common
    #安装gpg密钥
    curl -fsSL http://mirrors.aliyun.com/docker-ce/linux/ubuntu/gpg | sudo apt-key add -
    #添加仓库
    sudo add-apt-repository "deb [arch=amd64] http://mirrors.aliyun.com/docker-ce/linux/ubuntu $(lsb_release -cs) stable"
    #安装docker
    apt-get install -y docker-ce docker-ce-cli containerd.io && systemctl start docker


}


install