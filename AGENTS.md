# 项目指南

Qt 6 (C++17) 桌面聊天应用。无 README、无测试框架、无 CI。

## 构建（VSCode cmake-tools 已配置）

- 工具链：Qt `C:/Qt/6.11.1/mingw_64` + MinGW (`C:/Qt/Tools/mingw1310_64`)，生成器 `MinGW Makefiles`
- 配置：`cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/mingw_64`
- 编译：`cmake --build build`（产物 `ChatUI.exe`；`build_release/` 为旧 release 目录）
- 调试线索：`build/compile_commands.json`（供 VSCode IntelliSense 用）

## 架构

- **新增源文件必须手动加进 `CMakeLists.txt` 的 `add_executable(ChatUI ...)`**，CMake 不做通配收集
- 分层：UI 在 `src/` + `include/`（MainWindow、ChatWindow、LoginWindow、ContactList、ChatArea、MessageItem、ChatInput、StatusBar）；后端在 `backend/` + `include_backend/`（DatabaseManager、ChatBackend、LoginBackend、TcpClient）
- 入口 `src/main.cpp`：先 `LoginWindow`（模态 exec），Accepted 后才创建 `MainWindow`
- 协调中枢 `src/MainBackend`：聚合 `TcpClient`（共享实例）、`LoginBackend`、`ChatBackend`，UI 与后端之间只通过信号槽通信
- 数据载体定义在 `include_backend/ChatBackend.h`：`ContactInfo`、`MessageInfo`
- 数据库：Qt Sql（SQLite，见 `backend/DatabaseManager.cpp`，23KB 为仓库最大文件）

## 约定

- 代码注释保留中文教学式注释（`.project_memory.json` 明确要求保留学习笔记注释，勿删勿改）
- 行为要求：未选联系人时隐藏输入框；输入内容按联系人 ID 记忆（`ChatBackend` 的 inputCache）
- 已知坑（见 `note.txt`）：`QFrame` 的 QSS border 会与原生 frame 冲突，`border: none` 可能无效，需写 `border: 0px solid transparent; outline: none;`
- 修改前先分析头文件/类/信号槽依赖；大改动先列文件清单；禁止随意重构底层（DatabaseManager/TcpClient）逻辑
- Bug 排查优先检查相关头文件与信号槽连接
