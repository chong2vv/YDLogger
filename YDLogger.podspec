#
#  Be sure to run `pod spec lint YDLogger.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see https://guides.cocoapods.org/syntax/podspec.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |spec|

  spec.name         = "YDLogger"
  spec.version      = "0.1.0"
  spec.summary      = "日志库"

  spec.homepage     = "https://github.com/chong2vv/YDLogger"

  spec.license      = "MIT"

  spec.author             = { "王远东" => "chong2vv@gmail.com" }

  spec.platform     = :ios, "9.0"

  spec.source       = { :git => "https://github.com/chong2vv/YDLogger.git", :tag => "#{spec.version}" }
  spec.source_files = "YDLogger/*"
  
  spec.subspec 'YDLogger' do |log_ss|
      log_ss.libraries = 'c++'
      log_ss.source_files = "YDLogger/YDLogger/**/*"
  end
  
  spec.subspec 'YDLoggerUI' do |logui_ss|
      logui_ss.source_files = "YDLogger/YDLoggerUI/**/*"
      logui_ss.dependency 'YDLogger/YDLogger'
  end
  
  spec.static_framework = true
  spec.requires_arc = true
  spec.frameworks = "Foundation", "UIKit"

end
