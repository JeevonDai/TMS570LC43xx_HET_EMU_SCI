@echo off
REM =============================================================================
REM SRAM 版本 BIN 文件构建脚本
REM =============================================================================
REM 此脚本用于：
REM 1. 使用 SRAM 链接脚本构建程序
REM 2. 将 .out 文件转换为 bin 文件
REM 3. 生成 C 头文件用于嵌入到主 FLASH 中
REM =============================================================================

set CG_TOOL_ROOT=D:\ti\ccs1281\ccs\tools\compiler\ti-cgt-arm_20.2.7.LTS
set PATH=%CG_TOOL_ROOT%\bin;%PATH%

set OBJ2BIN_PATN=D:\ti\ccs1281\ccs\utils\tiobj2bin

echo ============================================
echo 构建 SRAM 版本的 bin 文件
echo ============================================

REM 创建 SRAM 版本的输出目录
if not exist "Debug_SRAM" mkdir Debug_SRAM

REM 步骤 1: 使用 SRAM 链接脚本重新编译
echo [1/2] 使用 SRAM 链接脚本进行链接...

cd Debug
armcl -mv7R5 --code_state=32 --float_support=VFPv3D16 -g ^
    --diag_warning=225 --diag_wrap=off --display_error_number ^
    --enum_type=packed --abi=eabi ^
    -z -m"..\Debug_SRAM\TMS570LC43xx_SRAM.map" ^
    --heap_size=0x800 --stack_size=0x800 ^
    -i"%CG_TOOL_ROOT%\lib" ^
    -i"%CG_TOOL_ROOT%\include" ^
    --reread_libs --diag_wrap=off --display_error_number ^
    --warn_sections --rom_model --be32 ^
    -o "..\Debug_SRAM\TMS570LC43xx_SRAM.out" ^
    HALCoGen\source\*.obj ^
    "..\HALCoGen\source\HL_sys_link_sram.cmd" ^
    -llibc.a

if errorlevel 1 (
    echo 编译失败！
    cd ..
    exit /b 1
)
cd ..

REM 步骤 2: 使用 tiobj2bin 直接生成 bin 文件
echo [2/2] 生成 bin 文件...

%OBJ2BIN_PATN%\tiobj2bin.bat Debug_SRAM\TMS570LC43xx_SRAM.out Debug_SRAM\TMS570LC43xx_SRAM.bin ^
    "%CG_TOOL_ROOT%\bin\armofd" "%CG_TOOL_ROOT%\bin\armhex" ^
    "%OBJ2BIN_PATN%\mkhex4bin"

if not exist "Debug_SRAM\TMS570LC43xx_SRAM.bin" (
    echo BIN 文件生成失败
)

echo ============================================
echo 构建完成！
echo 输出文件:
echo   - Debug_SRAM\TMS570LC43xx_SRAM.out
echo   - Debug_SRAM\TMS570LC43xx_SRAM.bin
echo ============================================

