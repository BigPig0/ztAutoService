# ztAutoService

#### 项目介绍

windows开机自启指定程序的服务，不需要用户登录。
本项目以vs2012建立，可自行升级

#### 配置文件说明

在exe文件目录放置一个名为config.ini的配置文件，内容按如下格式
[common]
number = 2
;需要管理的进程数量，各个进程的配置分别是从file1到fileN

[file1]
path = D:/a.exe
;可执行程序、脚本等文件路径或命令
cmd = -a -b -c -d
;进程启动参数
rshour = 5
;进程每天定时重启的时间，为小时，有效值0~23
rsdur = 8
;进程运行达到多少小时就重启一次，>0时有效

[file2]
path = D:/b.exe
cmd = -a -b -c -d
rsdur = 100


#### 安装教程

1. ztAutoService -i 安装系统服务，并自动启动
2. ztAutoService -u 卸载系统服务
3. ztAutoService -s 以用户进程方式执行程序，作为进程管理工具使用
