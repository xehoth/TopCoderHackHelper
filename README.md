# TopCoderHackHelper
[English Version](https://github.com/xehoth/TopCoderHackHelper/blob/master/README_EN.md)
Topcoder 在 hack 的时候不能复制别人的代码，TopCoderHackHelper 能把你当前查看的代码保存至本地。

**请不要在比赛的时候使用 !!!**

## 运行
从 [release page](https://github.com/xehoth/TopCoderHackHelper/releases) 下载

### Linux
请先安装 [libpcap](http://www.tcpdump.org/)。

#### For unbuntu
``` bash
sudo apt update
sudo apt install libpcap0.8
```

用以下命令运行
``` bash
./run.sh
```

### Windows
请先安装 [winpcap](https://www.winpcap.org/install/)。

## 编译
### Linux
先安装 `libpcap-dev`.

#### For unbuntu
``` bash
sudo apt update
sudo apt install libpcap-dev
```


``` bash
./build.sh
```

### Windows
在 VisualStudio 导入项目。

## 使用
请用管理员身份运行（否则可能找不到网卡），运行后会列出你的网卡，请选择你所使用的网卡（输入对应的数字）。

然后打开 TopCoder 客户端，点开对应的代码，若 TopCoderHackHelper 成功保存代码，会提示 `code saved to file [problem]_[user].txt`，代码保存在运行目录下，名称为 `[problem]_[user].txt`。

若没有提示，则可能丢包，请重新打开该代码。

## Sample
![Sample](/sample.png)

## 已知问题
可能发生丢包导致代码只有一半，此时在 TopCoder 中重新打开即可。
