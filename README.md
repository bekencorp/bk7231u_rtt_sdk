# RT-Thread for Beken7231平台说明

## 发布目录说明
| 目录 | 说明 |
| -- | -- |
| applications | RT-Thread OS的应用代码，包含main函数入口 |
| beken378 | Beken7231官方SDK |
| drivers | RT-Thread提供的Beken7231的驱动程序代码，包含板级入口代码 |
| libcpu | 该目录包含芯片启动代码，中断处理等 |
| rt-thread | RT-Thread 开源核心及外围组件代码（3.0.2版本）|
| simples | RT-Thread提供的应用Demo |
| test | 测试的Demo |


## 支持的功能  
* [x] station
* [x] soft-AP
* [x] airkiss
* [x] http client
* [x] web client
* [x] TLS
* [x] OTA升级
* [x] MQTT
* [x] netio  
* [x] iperf  


## 编译和下载
### 编译  
- 启动env  
执行 tools/env/console.exe  
*32位系统请使用 console_32.exe*

- 编译
```
cd ../../bk7231
scons
```

- 下载  
执行`encrypt.bat`脚本，生成`rtthread_crc.bin`  
通过`Hid Download Tool`工具，下载`rtthread_crc.bin`文件到`Flash`  

## 平台初始化说明
该工程默认使用串口1，下载代码后，连接串口1，打开终端工具，运行现象如下图所示:  

![系统正常运行现象](./doc/figures/系统正常运行现象.png)  


## 功能说明
### station

执行命令:`wifi w0 join rtt-SSID2 12345678`  
此时可通过`ifconfig`命令查看网络状态。  
该命令以板子作为station，去连接一个热点。  

参数说明：

| 参数 | 详细描述 |  
| -- | -- |  
| wifi | 命令类别 |  
| w0 | station的网卡设备名(可通过list_device查看) |  
| join | wifi执行的动作 |  
| rtt-SSID2  | SSID |  
| 12345678 | 密码 |  

![系统正常运行现象](./doc/figures/WiFi加入测试.png)  

### soft-AP
执行命令:``wifi ap ap hello 12345678``  
参数说明：

| 参数 | 详细描述 |  
| -- | -- |  
| wifi | 命令类别 |  
| ap | soft-ap的网卡设备名(可通过list_device查看) |  
| ap | wifi执行的动作 |  
| hello  | SSID |  
| 12345678 | 密码 |  

通过该命令开启板载的soft-ap功能，正常执行后，可通过设备搜索到一个SSID为`hello`，密码为`12345678`的热点。  

### ifconfig
在加入wifi之后，执行`ifconfig`命令，可查看网络状态：   

```
msh >ifconfig
network interface: w0 (Default)
MTU: 1500
MAC: c8 93 48 44 aa 00
FLAGS: UP LINK_UP ETHARP IGMP
ip address: 192.168.12.55
gw address: 192.168.10.1
net mask  : 255.255.0.0
dns server #0: 192.168.10.1
dns server #1: 0.0.0.0
```

### ping测试
加入wifi之后，在shell里，输入`ping + IP地址`，进行ping1测试  

```
msh >ping 123.125.114.144
60 bytes from 123.125.114.144 icmp_seq=0 ttl=52 time=35 ticks
60 bytes from 123.125.114.144 icmp_seq=1 ttl=52 time=53 ticks
60 bytes from 123.125.114.144 icmp_seq=2 ttl=52 time=28 ticks
60 bytes from 123.125.114.144 icmp_seq=3 ttl=52 time=28 ticks

```

### TLS测试
该示例为一个简单的TLS client，与外网建立TLS连接并传输数据。  
使用方法:在加入wifi之后，执行命令`tls_test`  

现象  

```
msh />tls_test   
[tls]mbedtls client struct init success...   
[tls]Loading the CA root certificate success...   
[tls]mbedtls client context init success...   
[tls]Connected www.howsmyssl.com:443 success...   
[tls]Certificate verified success...   
[tls]Writing HTTP request success...   
[tls]Getting HTTP response...   
（get response data）....   
```

### webclient测试
在加入wifi之后，执行命令`webclient_test https://www.rt-thread.org/test.txt`  
现象  
```
msh />webclient_test https://www.rt-thread.org/test.txt
argc : 2
ready webclient open : https://www.rt-thread.org/test.txt
[tls]mbedtls client struct init success...
[tls]Loading the CA root certificate success...
[tls]mbedtls client context init success...
[tls]Connected www.rt-thread.org:443 success...
[tls]Certificate verified success...
response : 200, content_length : 1545
RT-Thread

RT-Thread is an open source IoT operating system from China, which has strong scalability: from a tiny kernel running on a tiny core, for example ARM Cortex-M0, or Cortex-M3/4/7, to a rich feature system running on MIPS32, ARM Cortex-A8, ARM Cortex-A9 DualCore etc.

content pos : 1545, content_length : 1545
msh />
```

### Airkiss测试
在shell里执行命令：`airkiss`  
然后通过微信的配网工具，输入SSID后进行配网测试。  

## 测试用例说明  
### Iperf测试  
#### 介绍  
``Iperf``测试最大 TCP 和 UDP 带宽性能，可以报告带宽、延迟抖动和数据包丢失。  

#### 使用说明  
在menuconfig中配置使能`Iperf`，如下图所示
```
Test samples  --->
    [*]Enable Iperf Test
```

然后编译并下载固件。  

进入tools下的jperf-2.0.2，执行``jperf.bat``，启动iperf测试工具，并设置为Server模式。  
注意：该工具需要jre环境的支持。  

打开板载终端，在shell里执行命令：
```
wifi w0 join rtt-SSID2 12345678
iperf PC的IP地址 1
eg:iperf 192.168.10.135 1
```

#### 测试结果  
现象如下:  
![iperf测试](./doc/figures/iperf测试.png)  

### Netio测试  
#### 介绍
``netio``是一个测试网络的吞吐量的工具。  
#### 使用说明  
在shell里执行命令:
```
wifi w0 join rtt-SSID2 12345678
netio_init
```

#### 测试现象  
启动tools下的NetIO-GUI.exe工具，测试结果如下：  
![netio测试](./doc/figures/netio测试.png)  

## MQTT说明
使用说明见``\packages\pahomqtt\README.md``。  

## OTA说明  
RT-Thread的OTA功能提供固件升级的功能，详细描述见samples/ota路径下的说明。  

## 代码管理策略 

随着项目的日益增多，决定以RTT-Ayla/Hisense/Roobo为试验项目，使用submodule+branch的方式来管理
1. 每类项目单独创建一个repository
2. 同类项目用分支区分不同产品、不同代理商、不同客户
3. beken378、wifi_stack、ble_stack使用submodule关联，beken378不包含协议栈源码，相当于design kit
4. 在项目稳定前，可直接使用submodule主分支，如果submodule有重大功能合入，可提前创建项目分支
5. 在提交代码时，不同目的的修改分开提交，便于向其它分支合入
6. 在提交代码后，如果需要将修改合入其它分支，使用`git cherry-pick commitid`
7. 在版本发布后，使用`git tag -a -m "notes" tag_name`创建标签，并用`git push --tags`提交

## submodule使用说明

### 创建submodule
1. `git submodule add http://192.168.0.6/wifi/beken_wifi_sdk.git beken378`
2. `git submodule add http://192.168.0.6/wifi/beken_wifi_stack.git`
3. `git submodule add http://192.168.0.6/wifi/beken_ble_stack.git`
4. 修改.submodule中的url为相对路径，便于ssh和http同时使用
5. `git add .submodule beken378`
6. `git commit -m "xxx"`

注：
1. 所有submodule可能使用不同的分支，建议使用如下命令，指定所有submodules（包括主repository）的分支
`git branch --set-upstream-to=origin/xxx master`

### 获取submodule
1. `git clone --recurse-submodules http://xxx.git`
或者
1. `git clone http://xxx.git`
2. `cd xxx`
3. `git submodule update --init --recursive`
4. `git config --global credential.helper store`
   (将用户名密码保存到本地文件中，解决每次push都要输入用户名密码问题)

### 更新submodule
可以在repository根目录使用`git pull --recurse-submodules`命令更新整个工程
当然也可以进入特定submodule通过`git pull`单独更新

推荐使用前者更新，因为更新后submodules仍然停留在当前commit，不会主动切换到HEAD

### 提交submodule
需要先在submodule中提交修改，然后在root repo中更新submodule
1. `cd beken378` (the submodle name)
2. `git add xxx`
3. `git commit -m ""`
4. `git push origin xxx:yyy`
5. cd .. (the root of repo)
6. `git add beken378`
7. `git commit -m ""`
8. `git push origin`

## 版本发布 
在发布版本时，ip stack中的代码编译以库的形式打包，然后对代码打标签并发布，方法如下

- 根据项目实际情况修改beken378/app/config/sys_config.h中的CFG_SOC_NAME，并返回项目根目录

编译库文件
- `scons --buildlib=beken_ip` 并根据CFG_SOC_NAME重命名为beken378/ip/libip72xx.a
- `scons --buildlib=beken_ble` 并重命名为beken378/driver/ble/libble.a

编译好库并提交代码以后，需要打标签，由于submodule本身就是以commit id的形式存在，无需单独打标签。
当然，如果为了方便查找，可以用相同名称在相应代码库中打标签
- `git tag -a -m "" tag_name`
- `git push --tags`

运行脚本打包文件并发布
- run release_tool.bat
