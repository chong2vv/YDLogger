#
#  Be sure to run `pod spec lint YDLogger.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see https://guides.cocoapods.org/syntax/podspec.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |spec|
  spec.name         = "YDLogger"
  spec.version      = "0.2.1"
  spec.summary      = "日志库"
  spec.homepage     = "https://github.com/chong2vv/YDLogger"
  spec.license      = "MIT"
  spec.author       = { "王远东" => "chong2vv@gmail.com" }
  spec.ios.deployment_target = '10.0'
  spec.source       = { :git => "https://github.com/chong2vv/YDLogger.git", :tag => "#{spec.version}" }
  spec.source_files = "YDLogger/YDLogger.h"

  spec.subspec 'YDLog' do |ss|
      ss.source_files = "YDLogger/YDLog/**/*"
      ss.public_header_files = "YDLogger/YDLog/{YDLogService.h,YDMmapLogService.h}"
      ss.libraries = 'c++'
  end
  
  spec.subspec 'YDLoggerUI' do |ss|
      ss.source_files = 'YDLogger/YDLoggerUI/**/*'
      ss.dependency 'YDLogger/YDLog'
  end
  
  spec.static_framework = true
  spec.requires_arc = true
  spec.frameworks = "Foundation", "UIKit"
end
