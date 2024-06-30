# 项目构建说明文档

## 简介

本项目是一个自定义的Shell程序，能够解析和执行用户输入的命令，并支持管道、输入输出重定向、脚本执行等功能。

## 环境要求

操作系统：Linux/Unix
编译器：GCC或任何支持C++的编译器
C++标准：C++11或更高

## 文件说明

main.cpp：主程序文件，包含了Shell的实现逻辑。

## 构建步骤

### 获取源代码

1. 下载或克隆本项目的源代码到本地：

   ```shell
   git clone https://github.com/junyanaa/shell.git
   cd shell-project
   ```

2. 使用g++编译器编译源代码：

   ```shell
   g++ -o my_shell main.cpp
   ```

   这将生成一个可执行文件my_shell

3. 运行程序:

   ```shell
   ./my_shell
   ```

### 功能说明

#### 命令执行

用户可以在提示符下输入命令，程序将解析并执行输入的命令。

#### 管道

支持使用管道|将多个命令连接起来，例如：

```shell
ls | grep txt
```

#### 输入重定向

支持输入重定向符号<和<<，例如：

```shel
command < input.txt
command << EOF
```

#### 输出重定向

支持输出重定向符号>和>>，例如：

```shell
command > output.txt
command >> output.txt
```

#### 内部命令

cd：切换工作目录

exit：退出Shell

help：显示帮助信息

联系方式
如果你有任何问题或建议，请通过以下方式联系我们：

邮箱：2512728420@qq.com

GitHub：https://github.com/junyanaa/shell.git

作者

名字：Winnie

邮箱：2512728420@qq.com

此文档提供了从获取源代码到运行和使用Shell程序的完整说明。如果你有其他需要添加的功能或文档，请随时联系我。
