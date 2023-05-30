#
#  Be sure to run `pod spec lint YDLogger.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see https://guides.cocoapods.org/syntax/podspec.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |spec|
  spec.name         = "YDLogger"
  spec.version      = "0.1.7"
  spec.summary      = "日志库"
  spec.homepage     = "https://github.com/chong2vv/YDLogger"
  spec.license      = "MIT"
  spec.author       = { "王远东" => "chong2vv@gmail.com" }
  spec.ios.deployment_target = '10.0'
  spec.source       = { :git => "https://github.com/chong2vv/YDLogger.git", :tag => "#{spec.version}" }
  
  spec.source_files = "YDLogger/**/*.{h,cpp,m,mm}"
  spec.static_framework = false
  spec.requires_arc = true
  spec.frameworks = "Foundation", "UIKit"
  spec.pod_target_xcconfig = {
      'OTHER_LDFLAGS' => '-lc++',
      'CLANG_CXX_LANGUAGE_STANDARD' => 'c++14',
      'CLANG_CXX_LIBRARY' => 'libc++',
      'GCC_PREPROCESSOR_DEFINITIONS' => '$(inherited) COCOAPODS=1'
    }
  
  spec.libraries = 'c++'

  spec.subspec 'YDLoggerUI' do |ss|
      ss.source_files = 'YDLoggerUI/**/*.{h,m}'
  end
  
end
