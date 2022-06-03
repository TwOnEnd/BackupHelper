# BackupHelper 中文Wiki
一、简介
====
## 1.这是什么
BackupHelper 是基于 [QuickBackupM](https://github.com/TISUnion/QuickBackupM) 开发的一个基岩版服务端插件<br>
BackupHelper提供了较好的备份功能 , 它由以下基础功能组成: 
* 创建备份
* 删除备份
* 恢复备份
* 列出所有备份
## 2.插件特色
* 稳定的热备份
* 在线管理备份
* 恢复指定备份
* 定时备份(该功能暂时没做)
## 3.长期维护的版本
* 1.16.40

二、如何使用
====
## 1.请务必阅读免责声明
### [免责声明]()<br><br>
## 2.下载BDS
插件是依托于BDS的，因此你需要下载BDS<br>
前往 https://www.minecraft.net/en-us/download/server/bedrock 下载正确版本的Windows版的BDS文件，并解压到任意文件夹

    文件夹中不建议包含中文路径

<br>

## 3.解压
解压 BackupHelper-x.y.z-1.a.b.zip<br>
压缩包里面有以下几个文件,注意确认:
* BackupHelper-x.y.z-1.a.b.dll
* BackupHelper/backupHelper.log
* BackupHelper/config.json
* BackupHelper/backup/
* BackupHelper/overwrite/

将 BackupHelper-x.y.z-1.a.b.dll 放入 plugins 文件夹<br>
将 BackupHelper 文件夹放在 BDS根目录<br><br>
## 4.注入dll并启动BDS
### (完成第三节后 , 有两种注入方案)<br>
如果你使用其它的加载器，请去访问它们的wiki页面<br>

### 方案一:<br>
下载并使用由 [zhkj-liuxiaohua](https://github.com/zhkj-liuxiaohua) 开发的dll加载器 [MCDllInject](https://github.com/zhkj-liuxiaohua/ASPMCServer-CS/tree/master/MCDllInject)<br>
将 MCDllInject.exe 放入 BDS根目录 <br>
 然后在 BDS根目录 编写 mc_start.bat<br>

    title mc_start
    c hcp 65001
    MCDllInject.exe bedrock_server.exe plugins

最后 双击 mc_start.bat 启动服务器

### 方案二:
使用 [LiteLoaderBDS](https://github.com/LiteLDev/LiteLoaderBDS) 加载器<br>
双击 bedrock_server_mod.exe<br><br>

三、功能 (可能讲解不到位)
====
## 1.说明

在以下的所有指令说明中

    [string]:
        可不填写参数
        表示字符串，可以填任意字符
    [int]:
        必填参数
        表示正整数
    备份不会被压缩,他将以文件夹的形式储存
    储存名称为:
    [Time] [Size] [Slot] [PlayerName] [Note]
    对应的意思是:
    [开始执行备份的时间] [备份大小(有误差)] [槽位] [备份者的名字] [备份注释]
    例如:
    [2022-06-03 14：16：44] [1.62MB] [1] [TwOnEnd] [第一个备份]

    其中[Slot]跟[int]对应

## 2.指令
    /backup make [string]
    /backup delete[int]
    /backup restore [int]
    /backup list [int]
    /backup info
    /backup about 
    隐藏指令
    /backup server stop

## 3.详解
### /backup make [string] : 创建一个备份
    若不填写[string]

    则备份名为:
    [Time] [Size] [Slot] [PlayerName] [Null]
    否则:
    [Time] [Size] [Slot] [PlayerName] [string]

### /backup delete [int] : 删除指定备份
    若存在以下备份:
    [2022-05-03 14：16：44] [1.62MB] [1] [TwOnEnd] [第一个备份]
    [2022-06-03 14：16：44] [1.62GB] [2] [TwOnEnd] [第二个备份]
    [2022-06-03 16：16：44] [1.92GB] [3] [TwOnEnd] [被熊前的备份]

    若想删除第一个备份,那么根据第一节的说明 [Slot]跟[int]对应:
    /backup delete 1
    删除备份

### /backup restore [int] : 恢复指定存档
    跟上面差不多的用法
    /backup restore 3
    10秒后开始恢复到第三个备份

### /backup list [int] : 列出全部备份
    值得注意的是,这里的[int]跟[Slot]没任何关系
    列出的备份只会以5个为一页,[int]的作用是翻页
    列出顺序是[Slot]从大到小
    /backup list [int]
    列出备份:
    当前页数/最大页数 | 总备份 | 总大小
    [2022-06-03 16：16：44] [1.92GB] [3] [TwOnEnd] [被熊前的备份]
    [2022-06-03 14：16：44] [1.62GB] [2] [TwOnEnd] [第二个备份]
    [2022-05-03 14：16：44] [1.62MB] [1] [TwOnEnd] [第一个备份]

### /backup info
    输出最近的备份信息

### /backup about
    输出作者,联系方式,项目地址

## 4.隐藏指令
    /backup server stop
    如果在 创建备份,删除备份,恢复备份   其中任何一个执行过程中在后台输入了stop指令,那么
    往后无法向服务端输入任何字符,只能通过该指令安全的关闭服务器








四、配置文件
====

## 1.重启设置
到 BDS根目录 找到server.properties文件,将下面的指令添加到文件中

    start-program-name=xxx
    #如果使用的是方案二,xxx改为bedrock_server_mod.exe
    #如果使用的是方案一,xxx改为mc_start.bat


## 2.权限设置
BackupHelper/config.json 是隐藏指令的权限<br>
至于为什么有个隐藏指令,在第五节(Q&A)中会讲<br>

    [
        {
            "name":"TwOnEnd"
        },
        {
            "name":"PlayerName"
        }
    ]



五、Q&A
====

## Q:无法向服务端输入任何指令怎么办
## A:请看 第三章第四节 与 第四章第二节

## Q:有bug,会崩服怎么办
## A:QQ群:745253558

## Q:因为这个插件我的存档坏了怎么办
## A:如果你是正常情况下正常操作,我无法修复你的存档,但可以了解你的坏档过程,防止其他人也坏档

## 其他问题可以加群提问,写wiki太遭罪了
# QQ群:745253558