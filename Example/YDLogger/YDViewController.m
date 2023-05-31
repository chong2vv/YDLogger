//
//  YDViewController.m
//  YDLogger
//
//  Created by chong2vv on 05/27/2023.
//  Copyright (c) 2023 chong2vv. All rights reserved.
//

#import "YDViewController.h"
#import <YDLogger.h>
#import <YDLoggerUI.h>

@interface YDViewController ()

@property (nonatomic, strong) NSTimer *timer;

@end

@implementation YDViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    UIButton *logTestButton = [UIButton buttonWithType:UIButtonTypeSystem];
    logTestButton.frame = CGRectMake(100, 100, 100, 30);
    [logTestButton addTarget:self action:@selector(logTestAction) forControlEvents:UIControlEventTouchUpInside];
    [logTestButton setTitle:@"测试记录日志" forState:UIControlStateNormal];
    [self.view addSubview:logTestButton];
    
    UIButton *logUITestButton = [UIButton buttonWithType:UIButtonTypeSystem];
    logUITestButton.frame = CGRectMake(100, 200, 100, 30);
    [logUITestButton addTarget:self action:@selector(logUITestAction) forControlEvents:UIControlEventTouchUpInside];
    [logUITestButton setTitle:@"查看日志" forState:UIControlStateNormal];
    [self.view addSubview:logUITestButton];
    
    
}

- (void)logTestAction {
    self.timer = [NSTimer scheduledTimerWithTimeInterval:0.2
                                                  target:self
                                                selector:@selector(timerFired:)
                                                userInfo:nil
                                                 repeats:YES];
}

- (void)timerFired:(NSTimer *)timer {
   // 在这里编写要执行的定时操作的代码
    YDLogError(@"这是一条测试日志");
}

- (void)logUITestAction {
    YDLogListViewController *vc = [[YDLogListViewController alloc] init];
    [self.navigationController pushViewController:vc animated:YES];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
