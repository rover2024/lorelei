# lorelei-v1 TLC 与 `src/thunks` 工作原理详解

本文面向迁移和重构，解释 `lorelei-v1` 里 TLC（Thunk Layer Compiler）+ thunks 的完整机制。
重点回答两件事：

1. `src/thunks` 目录里每个文件在做什么。
2. TLC 如何把 manifest/描述信息编译成 GTL/HTL 可运行 thunk 代码，并在运行时完成双端交换与调用。

---

## 1. 总体目标与术语

v1 thunk 系统本质是在 Guest 与 Host 之间建立一个“函数/回调桥接层”：

- GTL（Guest Thunk Library）：跑在 guest 侧，导出给 guest 程序调用。
- HTL（Host Thunk Library）：跑在 host 侧，实际连到 host 库。
- TLC（`LoreTLC`）：代码生成器，读取 manifest + 配置 + 头文件 AST，生成 `Thunk_guest.cpp`/`Thunk_host.cpp`。

调用路径（注释里也反复出现）可概括为：

- Host Function：`GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL`
- Guest Function：`HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL`

其中：

- `GTP/HTP`：两端暴露出来的 thunk 入口。
- `*_IMPL`：更接近运行时调用执行器（`MetaProcExec` / `MetaProcCBExec`）。
- GRT/HRT：GuestRT / HostRT。

---

## 2. `src/thunks` 目录结构怎么理解

顶层入口：`lorelei-v1/src/thunks/CMakeLists.txt`

- 逐个 `add_subdirectory(...)` 注册模块（SDL/OpenGL/GL/GLX/EGL/vulkan/zlib/zstd 等）。
- 每个模块目录一般都有同一套文件：
  - `Thunk_procs_desc.h`
  - `Thunk_manifest_guest.cpp`
  - `Thunk_manifest_host.cpp`
  - `Thunk_config.conf`
  - `Thunk_config_symbols.conf`
  - `CMakeLists.txt`

### 2.1 模块内文件职责

#### `Thunk_procs_desc.h`

声明该模块的元信息与可选策略：

- `MetaConfig<MCS_User>`：常见只填 `moduleName`（如 `libz`、`libGL`、`libvulkan`）。
- 可选 `MetaProcDesc<函数>`：给某个函数指定 overlay 类型、builder pass。
- 可选 `MetaProcCBDesc<回调类型>`：给 callback 指定名字 hint、builder pass。

示例：

- `SDL2-2.0/Thunk_procs_desc.h`：`SDL_LogMessageV` 绑定 `MetaPass_vprintf<>`。
- `_sample/Thunk_procs_desc.h`：示范 `overlay_type`、`MetaPass_scanf<>`、`MetaProcCBDesc`。

#### `Thunk_manifest_guest.cpp` / `Thunk_manifest_host.cpp`

这是 TLC 的输入翻译单元（TU）：

- `#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>` 或 host 版本。
- 可 `#define LORETHUNK_CALLBACK_REPLACE` 打开回调替换逻辑。
- 可 include 自定义 filter/header（如 `GL_procFilters_host.h`）。
- 可手动覆写某些 `MetaProc<...>`（例如 GL 对 `glDebugMessageCallback*` 做空实现）。

#### `Thunk_config.conf` / `Thunk_config_symbols.conf`

提供“要纳入 thunk 的符号清单”：

- 常见写法是主配置 `include "Thunk_config_symbols.conf"`。
- `Thunk_config_symbols.conf` 里按 section 列符号：
  - `[Function]`（最常见）
  - `[Guest Function]`
  - `[Variable]`
  - `[Callback]`

> TLC 在 `DocumentContext::endSourceFileAction()` 里根据这个清单从 AST 采样；找不到的会记录为 missing 并输出到生成文件尾注释。

#### 模块 `CMakeLists.txt`

通过 `include("../ThunkAPI.cmake")` + `add_thunk()` 一键生成并构建 GTL/HTL。

可选设置：

- `GTL_alias` / `HTL_alias`
- `*_extra_includes` / `*_extra_links` / `*_extra_options`
- `*_force_links`
- `PLUGIN_target`（把 TLC plugin 注入生成过程）

---

## 3. 构建链路：从 CMake 到 `LoreTLC`

### 3.1 `add_thunk()` 做了什么

`lorelei-v1/src/thunks/ThunkAPI.cmake` 的 `add_thunk()`：

1. 若启用 guest 且允许生成：调用 `lore_generate_thunk(...)` 生成 `Thunk_guest.cpp`。
2. 编译 GTL 动态库并链接 `LoreGuestRT`。
3. 若启用 host：生成 `Thunk_host.cpp`，同时可输出 `Thunk_callbacks.txt`。
4. 编译 HTL 动态库并链接 `LoreHostRT`。

并且给 thunk 目标加了固定寄存器编译选项：

- guest：`lore_guest_thunk_disable_register`
- host：`lore_host_thunk_disable_register`

### 3.2 `lore_generate_thunk()` 参数拼装

定义在 `lorelei-v1/src/BuildAPI.cmake`。

核心命令形态：

- `LoreTLC -o <out> -s <config> [--out-callbacks=...] [--preinc=...] [--plugin=...] <manifest.cpp> -- <clang args...>`

它会把：

- 输入 manifest（`Thunk_manifest_guest.cpp` / `Thunk_manifest_host.cpp`）
- 预包含（通常 `Thunk_procs_desc.h`）
- thunk config
- clang include 路径

统一交给 `LoreTLC` 做 AST 解析与代码生成。

---

## 4. TLC 主流程（`LoreTLC`）

入口：`src/tools/tlc/Commands/ThunkGen.cpp`

### 4.1 命令行阶段

读取参数：

- `-s` thunk config
- `-o` 目标 cpp
- `--out-callbacks`
- `--preinc`
- `--plugin`

然后：

1. 加载 plugin 动态库（找 `TLC_PluginInstance` 导出）。
2. 初始化 `DocumentContext`。
3. 跑 clang AST frontend action。

### 4.2 AST 阶段

`MyConsumer::Initialize`：

- `docCtx.initialize(config)`
- 调 plugin add-on `initialize(doc)`

`MyConsumer::HandleTranslationUnit`：

- `docCtx.handleTranslationUnit(ast)`
- 调 plugin add-on `handleTranslationUnit(doc)`

`EndSourceFileAction`：

- `docCtx.endSourceFileAction()`
- plugin `endSourceFileAction()`
- `docCtx.generateOutput(file)` 输出最终生成代码
- 需要时输出 callbacks 列表

---

## 5. AST 元信息采样（`ASTMetaContext`）

`DocumentContext` 继承 `ASTMetaContext`。`ASTMetaContext::handleTranslationUnit()` 会通过 matcher 收集：

- `MetaConfig`（builtin/user）
- `MetaProcDesc` / `MetaProcCBDesc`
- `MetaProc` / `MetaProcCB`（用户手写 specialization）
- C linkage `FunctionDecl` / `VarDecl`
- function pointer typedef

这一步得到“原材料池”，后续 `DocumentContext` 再按 config 清单筛选。

---

## 6. `DocumentContext` 的核心算法

重点在 `src/libs/TLC/Core/DocumentContext.cpp::endSourceFileAction()`。

### 6.1 读取配置并定稿模块信息

- 从 `MetaConfig` 读 `isHost`、`moduleName`。
- 默认 moduleName 是 `unknown`，一般会被 `MetaConfig<MCS_User>` 覆盖。

### 6.2 按配置筛选 symbols

从 config section 读取要导出的函数/变量/guest函数/callback：

- 找到就纳入 `_functions/_guestFunctions/...`
- 找不到记入 `_missingFunctions/_missingVars/_missingCallbacks`

### 6.3 callback 集合扩展与追踪

callback 不只来自 `[Callback]`，还会自动扩展：

1. 来自 `MetaProcCBDesc` 的显式 callback 类型。
2. 来自函数参数/返回值、结构体字段递归中的函数指针。
3. 来自 overlay type（如果 desc 指定了 overlay）。

并建立 trace 信息（`_callbackTraceInfos`），用于 `--out-callbacks` 可读输出。

### 6.4 构建 `ProcContext`

为四类 proc 建上下文：

- `CProcKind_HostFunction`
- `CProcKind_GuestFunction`
- `CProcKind_HostCallback`
- `CProcKind_GuestCallback`

`ProcContext` 会绑定：

- 对应 desc（若存在）
- 已有手写 definition（若存在）
- overlay type（若存在）
- real function type view

### 6.5 Pass 调度

Pass 分三相：

- Builder
- Guard
- Misc

调度策略：

1. 每个 proc 先选一个 builder（匹配不到就 `DefaultBuilder`）。
2. Guard/Misc 可叠加多个（`testProc` 命中即加入）。
3. 统一执行 `beginHandleProc`，再逆序执行 `endHandleProc`。

这保证了可组合性与前后处理对称。

---

## 7. 内建 Pass 各自做什么

### 7.1 DefaultBuilder（基础骨架）

文件：`Pass/Builder/DeafultBuilder.cpp`

为每个 proc kind 自动生成四阶段 `MetaProc/MetaProcCB` 框架：

- 函数参数打包为 `void* args[]`
- 解包 `args` / `ret`
- 调用 `MetaProcExec` / `MetaProcCBExec`
- 生成标准 `invoke` 声明/定义

这是整个系统可运行的最小闭环。

### 7.2 TypeFilter（参数/返回过滤）

文件：`Pass/Guard/TypeFilter.cpp`

- 扫描 `MetaProcArgFilter<T>` / `MetaProcReturnFilter<T>` specialization。
- 若 proc 的参数或返回命中 `T`，在 thunk 前/后插入 `filter(...)` 调用。

GL/X11 桥接里这个 pass 非常关键（指针翻译、返回对象重建等）。

### 7.3 CallbackSubstituter（回调替换）

文件：`Pass/Guard/CallbackSubstituter.cpp`

目的：当结构体/参数里包含函数指针回调时，自动把跨端 callback 替换为 trampoline。

关键行为：

- 递归分析参数结构，定位函数指针成员。
- 生成局部 `ctx` 结构记录原始 fp 与替换后 fp。
- 在 forward 阶段 `CallbackContext_init(...)` 替换。
- 在 backward 阶段 `CallbackContext_fini(...)` 还原。
- 逻辑包裹在 `#ifdef LORETHUNK_CALLBACK_REPLACE`。

### 7.4 GetProcAddress（地址反查）

文件：`Pass/Misc/GetProcAddress.cpp`

用于 `*GetProcAddress*` 类 API：

- 在 HostFunction 的 GTP 回程阶段，把 host 地址转为 guest 可调用地址：
  - `client->convertHostProcAddress(name, ret)`

---

## 8. 代码生成产物长什么样

`DocumentContext::generateOutput()` 会输出：

1. `#define LORETHUNK_BUILD`
2. 必要 include（`ManifestlGlobal.h`、`ManifestCallbackDefs.h`、`MetaProc.h`）
3. 导出符号别名（alias 到 `MetaProc<...>::invoke`）
4. `LORETHUNK_MODULE_NAME`
5. `LORETHUNK_HOST_FUNCTION_FOREACH` / `GUEST_FUNCTION_FOREACH` / `CALLBACK_FOREACH`
6. 每个 proc 在对应 phase 的声明与定义（含 pass 注入片段）
7. include 原 manifest 主文件
8. 缺失声明列表（missing sections）
9. include `ManifestContext_{host|guest}_impl.inc.h` 完成上下文初始化实现

因此生成文件是“完整可编译 C++”，而不只是片段。

---

## 9. `ManifestContext_*` 如何把生成代码接到运行时

### 9.1 两套模式：非 build 与 build

`ManifestContext_guest.inc.h` / `host.inc.h`：

- 非 `LORETHUNK_BUILD`：给模板默认 `invoke = nullptr` 占位。
- `LORETHUNK_BUILD`：引入 shared impl，绑定 `GuestClient/HostServer`，创建 `LocalThunkContext LTC`。

### 9.2 shared 上下文（`ManifestContext_shared.inc.h`）

定义：

- HostFunction / GuestFunction / Callback 枚举
- `getHostFunctionIndex<>` / `getGuestFunctionIndex<>` / `getCallbackIndex<>`
- `CStaticProcInfo` 数组：GTP/HTP/lib entries
- `CStaticProcInfoContext staticProcInfoContext`

这是 GTL 与 HTL 交换信息的载体。

### 9.3 guest 侧预初始化（`guest_impl.inc.h`）

- 填充本侧 `*_GTPs` 的 `name + addr`（addr 指向生成的 `MetaProc...::invoke`）。
- 给对侧 `*_HTPs` 先填 `name`（等待交换时拿到地址）。
- 填 `libraryFunctions`（静态链接模式下直接绑 `::NAME`，否则只记 name，稍后 dlsym）。

### 9.4 host 侧预初始化与交换导出（`host_impl.inc.h`）

- 填本侧 `*_HTPs` 的 `name + addr`。
- 给 `*_GTPs` 填 name。
- 设置 `staticProcInfoContext.emuAddr = server->runTaskEntry()`。
- 导出 `extern "C" LORETHUNK_exchange(CStaticProcInfoContext *ctx)` 完成双向拷贝。

### 9.5 真正交换发生在 GuestThunkContext

`GuestThunkContext::initialize()`：

1. 通过 `client->getThunkInfo(_moduleName,false)` 找到对应 HTL 路径。
2. 远端加载 HTL（`client->loadLibrary(...)`）。
3. 初始化 GTL 的 `libEntries`（非静态时 `dlsym(nullptr, name)`）。
4. 调用 HTL 导出的 `LORETHUNK_exchange`，把两端 `CStaticProcInfoContext` 对齐。

### 9.6 HostThunkContext 的职责

`HostThunkContext::initialize()`：

1. 从 host config 拿 forward thunk -> host library path。
2. `dlopen` 真实 host 库。
3. 非静态模式下按 `libEntries` 对真实库 `dlsym`。
4. 若 host 库提供 `s_LoreThunk_HLContext`，写入地址边界/线程回调/pthread hook/guest callback thunk 列表。

---

## 10. 运行时调用与重入机制

### 10.1 客户端侧（GuestClient）

`GuestClient::invokeProc_impl()`：

1. 发 `REQUEST_INVOKE_PROC` 到 host。
2. 若 host 返回 `RETURN_NEXT_TASK`，说明 host 在执行中需要 guest 协助执行下一段任务（典型是 callback/reentry）。
3. guest 按 `ClientTask.id` 执行：
  - `TASK_FUNCTION`
  - `TASK_CALLBACK`
  - `TASK_PTHREAD_CREATE/EXIT`
  - `TASK_HOST_LIBRARY_OPEN`
4. 每处理一轮发 `REQUEST_RESUME_PROC`，直到 host 返回非 NEXT_TASK。

这就是 reentry/nested reentry 的根机制。

### 10.2 服务端侧（HostServer）

`HostServer::dispatch_impl(REQUEST_INVOKE_PROC)`：

- 把当前 `ClientTask*` push 到 `thread_local` 任务栈 `thread_ctx.tasks`。
- 按 convention 直接调用 thunk/function。
- 调用中若触发 `runTaskFunction/runTaskCallback`，会通过 `currentTask()` 命中当前栈顶任务，交给 emu 继续调 guest。
- 返回后 pop。

由于是栈结构，天然支持嵌套调用深度（也就是 nested reentry）。

---

## 11. 回调寄存器约定与 trampoline

`ManifestCallbackDefs.h` 提供：

- `allocCallbackTrampoline`：按线程维护 trampoline table，把原 callback 映射到 thunk 指令。
- `CallbackContext_init/fini`：在跨端前替换，回程后恢复。
- 取“最近 callback”寄存器宏：
  - x86_64：`r11`
  - arm64：`x16`
  - riscv64：`t1`

v1 在这层把 callback “看起来像本地函数指针”地桥过去。

---

## 12. 插件扩展点（可选）

TLC 支持动态插件（`--plugin`）：

- 接口：`ASTActionPlugin`（`beginSourceFileAction/endSourceFileAction/createASTConsumerAddOn`）
- 插件可注册新 pass（`PassRegistration<T>`）
- 入口导出：`TLC_PluginInstance`

`src/thunks/_sample/plugin` 是最小示例。

---

## 13. 一次完整生命周期（浓缩版）

1. 模块 `CMakeLists` 调 `add_thunk()`。
2. `lore_generate_thunk()` 调 `LoreTLC` 读 manifest + config + desc。
3. TLC AST 抽取 meta + symbol，构建 `ProcContext`，跑 Builder/Guard/Misc pass。
4. 生成 `Thunk_guest.cpp` / `Thunk_host.cpp`。
5. 编译成 GTL/HTL。
6. 运行时 GTL 初始化时加载 HTL，调用 `LORETHUNK_exchange` 对齐 `CStaticProcInfoContext`。
7. `MetaProcExec/MetaProcCBExec` 通过 `GuestClient/HostServer` 完成跨端 invoke。
8. 遇到 callback/reentry 时，靠 `ClientTask` + `RETURN_NEXT_TASK/REQUEST_RESUME_PROC` 循环完成嵌套往返。

---

## 14. 对迁移到 v2（HLR/TLC 重写）的直接启发

从 v1 迁移时，最不该丢的是这几层语义：

1. `ProcKind + ThunkPhase` 四象限模型（函数/回调 × GTP/HTP）。
2. Pass 插入点语义（forward/center/backward/prolog/epilog）。
3. callback 替换生命周期（init/fini 成对）。
4. `CStaticProcInfoContext` 风格的双端交换协议。
5. reentry 的任务栈模型（不是简单单次 RPC）。

如果 v2 的 HLR 替掉了 v1 CFIC/PPF 等模块，建议至少保持上述“行为契约”等价，否则运行时容易出现：

- callback 地址归属判断错误
- GetProcAddress 映射错误
- 嵌套 reentry 中 task 栈错位
- host/guest 符号表未对齐导致空地址调用

