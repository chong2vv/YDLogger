# YDLogger
## YDLogger 日志库介绍
众所周知，日志作为排查问题最重要的手段是不容忽视的，尤其是上线APP排查远程用户的应用问题更是不可或缺，因此YDLogger日志库正是为此而来。同时，欢迎使用作者其他的开源库，友情提示，YDLogger搭配[YDPreventCrash](https://github.com/chong2vv/YDPreventCrash)一起使用效果更加。

YDLogger目前支持以下功能：
- 自定义日志录入及控制台输出；
- 开启hook可hook各类下的方法调用；
- 支持文件导出；
- 通过YDLoggerUI可直接查看日志。


## 安装及使用方式
### 使用CocoaPods导入

```
pod 'YDLogger'
```
### 使用方法

#### YDLogger 日志库使用

使用时引入头文件：

```
#import <YDLogger/YDLogger.h>
```

之后建议在AppDelegate的didFinishLaunchingWithOptions方法中的最初始位置添加如下代码，让YDLogger生效:

```
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    // 开启日志服务，如果需要hook类下所有方法可开启hook
    // 日志默认保存10天，可以在开启前通过[[YDLogService sh****ared] clearLogWithDayTime:5]设置
    [[YDLogService shared] startLogNeedHook:NO];
    return YES;
}
```

#### YDLogger使用
如果想使用YDLogger日志收集自定义信息，可使用如下宏：

```
/**
 日志记录宏，只记录到本地，使用方法和NSLog相同，引用当前文件后可直接使用
 根据日志level的不同，记录的日志不同
 当调用setLogLevel:设置需要记录的日志level为YDLogDebug时，那么YDLogDebug等级以下的等级（含YDLogDebug）都会被记录
 默认设置为YDLogDetail
 
 YDLogError()   记录错误信息，适用于线上/线下环境，格式：@"Erro timeStamp error"
 YDLogInfo()    记录极简信息，适用于线上/线下环境，格式：@"Info timeStamp info"
 YDLogDetail()  记录详细信息，适用于线上/线下环境，格式：@"Deta timeStamp [thread] func str"
 YDLogDebug()   记录开发信息，适用于Debug环境，格式：@"Dbug timeStamp str"
 YDLogVerbose() 记录复杂信息，适用于Debug环境，格式：@"Verb timeStamp [thread] func in file:line desc"
 详细使用可参考具体宏定义
 */
```
同时，为了方便快速查看日志，可以用YDLogger自带的YDLoggerUI：

```
 YDLogListViewController *vc = [[YDLogListViewController alloc] init];
 [self.navigationController pushViewController:vc animated:YES];
```

## 更新

#### v0.2.1
1. 完成YDLogger日志库库。

## 写在最后的话
一个人的精力是有限的，如果你在使用过程中发现了BUG或者有更好的解决方法、完善功能等，欢迎你能issue，我将万分感谢！
