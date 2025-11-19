在 .vscode 创建 c_cpp_properties.json 文件

```json
{
    "configurations": [
        {
            "name": "TMS570LC43xx",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/HALCoGen/include",
                "D:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include"
            ],
            "defines": [
                "__TMS470__",
                "__TI_ARM__",
                "__LITTLE_ENDIAN__=0",
                "__TI_COMPILER_VERSION__=20020700",
                "__TMS570LC43xx__",
                "true=1",
                "false=0"
            ],
            "compilerPath": "D:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl.exe",
            "cStandard": "c99",
            "cppStandard": "c++11",
            "intelliSenseMode": "gcc-arm",
            "browse": {
                "path": [
                    "${workspaceFolder}",
                    "${workspaceFolder}/HALCoGen/include",
                    "D:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include"
                ],
                "limitSymbolsToIncludedHeaders": false
            },
            "configurationProvider": "ms-vscode.makefile-tools"
        }
    ],
    "version": 4
}
```

然后在 settings.json 中添加

```json
{
    // Clangd 配置
    "clangd.arguments": [
        "--log=error",
        "--background-index=false",
        "--clang-tidy=false",
        "--completion-style=detailed",
        "--header-insertion=never",
        "--pch-storage=memory",
        "--function-arg-placeholders=false",
        "--limit-references=0",
        "--limit-results=0"
    ],
    "clangd.fallbackFlags": [
        "-I${workspaceFolder}",
        "-I${workspaceFolder}/HALCoGen/include"
    ],
    // 禁用默认的 C/C++ IntelliSense（如果使用 clangd）
    "C_Cpp.intelliSenseEngine": "disabled",
    // 或者如果不使用 clangd，启用 C/C++ 扩展
    // "C_Cpp.intelliSenseEngine": "default",
    // "clangd.onConfigChanged": "restart",
    
    // 文件关联 - 强制 .h 文件为 C 语言
    "files.associations": {
        "*.h": "c",
        "*.c": "c"
    },
    
    // 强制 C 语言模式
    "[c]": {
        "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
    },
    
    // 编码设置
    "files.encoding": "utf8",
    
    // Makefile 工具配置
    "makefile.configurationCachePath": ".vscode/configurationCache.log",
    "makefile.makeDirectory": "${workspaceFolder}/Debug",
    "makefile.makefilePath": "${workspaceFolder}/Debug/makefile",
    
    // 排除不需要索引的目录
    "files.exclude": {
        "**/Debug/**/*.obj": true,
        "**/Debug/**/*.d": true
    }
}
```