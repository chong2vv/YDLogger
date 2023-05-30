//
//  YDViewController.m
//  YDLogger
//
//  Created by chong2vv on 05/27/2023.
//  Copyright (c) 2023 chong2vv. All rights reserved.
//

#import "YDViewController.h"
#import <YDLogger.h>
//#import <YDLoggerUI.h>

@interface YDViewController ()

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
    YDLogError(@"这是一条测试日志");
}

- (void)logUITestAction {
//    YDLogListViewController *vc = [[YDLogListViewController alloc] init];
//    [self.navigationController pushViewController:vc animated:YES];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
