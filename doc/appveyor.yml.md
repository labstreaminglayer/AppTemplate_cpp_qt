 The appveyor configuration tells AppVeyor how to build the app
 on windows. You should only need to edit the following keys:
 - version
 - artifacts.name
 - deploy.username
 - deploy.api_key.secure
 - deploy.package

``` yml
version: 1.12.0.{build}
pull_requests:
  do_not_increment_build_number: true
shallow_clone: true
environment:
  lslversion: 1.12.0
  VCVER: 14.0
  QTVER: 5.10.1
  QTCOMPILER: msvc2015_64
  ARCH: x64
  CMakeArgs: ""
  DODEPLOY: false
install:
- cmd: appveyor DownloadFile https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-win.zip -FileName ninja.zip
- cmd: appveyor DownloadFile https://bintray.com/labstreaminglayer/LSL/download_file?file_path=liblsl-%lslversion%-Win64-MSVC%VCVER%.7z -FileName liblsl_x64.7z
- cmd: 7z x ninja.zip -oC:\projects\deps\ninja > nul
- cmd: 7z x liblsl_x64.7z
- cmd: set PATH=C:\projects\deps\ninja;%PATH%
- cmd: ninja --version
build_script:
- cmd: call "C:\Program Files (x86)\Microsoft Visual Studio %VCVER%\VC\vcvarsall.bat" %ARCH%
- mkdir build
- cd build
- cmd: cmake -G Ninja -DQt5_DIR=C:/Qt/%QTVER%/%QTCOMPILER%/lib/cmake/Qt5 -DBOOST_ROOT=C:/Libraries/boost_1_67_0 -DCMAKE_BUILD_TYPE=Release -DLSL_INSTALL_ROOT=LSL/ %CMakeArgs% ../
- cmake --build . --config Release --target install
artifacts:
- path: build/install
  name: BestPracticesGUI_$(lslversion)-Win64
deploy:
- provider: BinTray
  username: tstenner
  api_key:
    secure: Fo9tIQnGjVnCXB3euItvYrt85A5O9FPqq7LEpsdOuwW+eOg1rwA5Q1HLp4yWDXAl
  subject: labstreaminglayer
  version: $(lslversion)
  repo: LSL
  package: BestPracticesGUI
  publish: true
  override: true
  on:
    dodeploy: true
