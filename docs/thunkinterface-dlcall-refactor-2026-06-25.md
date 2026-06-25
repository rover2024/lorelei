# ThunkInterface / DLCall 重构记录(2026-06-25)

本文记录本轮对 lorelei-v2 运行时桥接层与 manifest 层(`ThunkInterface`)的重构,以及配套的
`DLCall` 迁移与 `lorelei-thunks` manifest 扫尾。HLR(`src/tools/HLR/**`)本轮**不在范围内**。

## 1. 背景

lorelei-v2 是 lorelei v1 的重构版。最大架构变化:v1 用 RPC 式 GuestClient/HostServer,v2 改用
**QEMU TCG 插件 `dlcall`**(`../qemu2/contrib/plugins/dlcall.c`,编为 `libdlcall.so`)拦截一个
**魔法 syscall(号 4096,`DLCallSyscallNumber`)**。guest 发它让 host 侧 dlopen/dlsym/调用 host
函数。前提是 `guest_base == 0`(共享地址空间、指针零翻译),所以跑在打了 `reserved_va` patch 的
qemu2 上。对应 v1 的 `Base/PassThrough` → v2 的 `DLCall`。

## 2. 本轮改动

### 2.1 运行时端点
- **GuestRT**(`src/modules/GuestRT/main.cpp`):初始化时 `loadLibrary("libLoreHostRT.so")` →
  `getProcAddress("LoreCommonHostEntry")` → `GuestClient::setCommonHostEntry(...)`。必须在任何
  `invokeHost`(logMessage / invokeFunction / getThunkInfo ...)之前设好。
- **HostRT**(`src/modules/HostRT/HostServer.{h,cpp}`):去掉已删除的 `IServer<HostServer>` 基类,
  把 `reenter*` 系列内联进 `HostServer`;导出 `extern "C" LoreCommonHostEntry(secondaryId, payload)`,
  按 `DLCallSecondaryID` 分发(`InvokeFunction / ResumeFunction / LogMessage / GetModulePath /
  GetThunkInfo`)。其余顶层请求(LoadLibrary/GetProcAddress/...)由 dlcall 插件直接处理。
- `Invocation.cpp`、`HostRT/main.cpp`:清掉已删的 `Base/PassThrough` / `Base/Support` 死引用。

### 2.2 DLCall 协议(`include/lorelei/DLCall/Protocol.h`)
- `DLCallRequestID` / `DLCallSecondaryID` 由 `enum class` 改为普通 `enum`,值加前缀 `DR_` / `DS_`。
- 全仓用法同步;QEMU 插件用自己的 `DlcallID`(`DLCALL_ID_*`),靠数值对齐,不受影响。

### 2.3 注释规范(LLVM 风格)
- 类型文档注释统一为 `TypeName - 描述`(不用 `\brief`)。涉及 `GuestClient` / `HostServer` /
  `Protocol` 的枚举与结构体 / `CallbackTrampoline` / `FunctionTrampoline` / `VariadicAdaptor`。
- 该约定已记入 agent 记忆(comment-style)。成员/函数注释保持普通 `///`。

### 2.4 CCG / FDG 从 manifest 层移除
- `LoreExchangeContext` 不再回传 `numCCGs/CCGs/numFDGs/FDGs`,只拷 proc 表 + `emuAddr`。
- CCG/FDG 现仅存于 HLR(本轮当它不存在)。

### 2.5 `CProc.h` → `DLCall/ProcDefs.h` 迁移(.inc 层)
旧 `Base/PassThrough/c/CProc.h`(已删)→ 新 `DLCall/ProcDefs.h`:
- `CStaticProcInfo{name,addr}` → `ProcInfoPair{key,addr}`(字段 `name`→`key`)
- `CStaticProcInfoArrayRef` → `ProcArrayRef`,`CStaticThunkContext` → `StaticThunkContext`
- `CProcKind_* / CProcDirection_*` → `lore::ProcKind / ProcDirection`(枚举值)
- 新 `StaticThunkContext` 字段:`guestProcs/hostProcs/thisProcs/emuAddr/autoLink`(无 CCG/FDG)
- `Proc.h` 已由用户迁好(include ProcDefs.h,去掉自有 enum)。

### 2.6 ThunkInterface 重组(本轮主体)
见第 3 节。要点:新增 `Detail/` 放内部件;消除 Guest/Host 目录与 `../Guest/` 反向引用;
角色化重命名;入口提到顶层。

### 2.7 generateOutput(`src/libs/TLCApi/DocumentContext.cpp`)
- 顶部 `Tools/ThunkInterface/{Callback,Variadic}.h` → `ThunkInterface/Detail/...`
- 末尾原本 Host/Guest 二选一的 ManifestImpl include → 单个 `ThunkInterface/Detail/ProcImpl.cpp.inc`
  (`LORE_THUNK_HOST` 由 manifest 的 host 写法定义)。

### 2.8 lorelei-thunks 扫尾(另一仓库)
~20 个 manifest 的 include 迁到新布局,残留 `Tools/ThunkInterface` = 0。

## 3. ThunkInterface 最终结构

```
include/lorelei/ThunkInterface/
  Proc.h                       公开:manifest 作者特化 ProcFn/ProcCbDesc/ProcArgFilter ...
  PassTags.h                   公开:描述符里引用 pass::printf<> / GetProcAddress<> ...
  Manifest.cpp.inc             入口:manifest #include 这个;选 Pre/OnBuild
  Detail/                      内部装配件(== lore::thunk::detail 命名空间)
    Callback.h                 trampoline / CallbackContext / 读寄存器宏 / isHostAddress 声明
    Traits.h                   CommonFunctionThunk / PrependCallbackToArgs
    Variadic.h
    ManifestPreBuild.cpp.inc   非构建态:ProcFn/ProcCb 占位 = nullptr(去重:per-行 #ifdef)
    ManifestOnBuild.cpp.inc    构建态:LocalThunkContext + ProcFn/ProcCb Exec 跨端实现
                               (per-特化 #ifdef LORE_THUNK_HOST —— 见 §5 已知短板)
    ProcTable.cpp.inc          枚举/索引映射/proc 表/staticThunkContext(原 ProcDefShared)
    ProcInit.cpp.inc           preInitialize 填本侧表(原 ProcImpl;去重:THIS/PEER 别名宏)
    ProcImpl.cpp.inc           运行时实现尾:isHostAddress 定义 + #include ProcInit +
                               (host) LoreExchangeContext 导出
```

### Manifest 编写契约(新)
```cpp
// guest manifest
#include <lorelei/ThunkInterface/Manifest.cpp.inc>

// host manifest
#define LORE_THUNK_HOST
#include <lorelei/ThunkInterface/Manifest.cpp.inc>
```
公开面只有 `Proc.h` + `PassTags.h` + `Manifest.cpp.inc`。不再有 Guest/Host 目录。

### 装配链(一个生成的 GTL/HTL .cpp)
```
manifest 文件 → #include Manifest.cpp.inc
                  ├─ Detail/Callback.h + Detail/Variadic.h
                  └─ LORE_THUNK_BUILD ? Detail/ManifestOnBuild : Detail/ManifestPreBuild
                                          └─ (OnBuild) Detail/ProcTable.cpp.inc
TLC generate 注入:#define LORE_THUNK_BUILD(顶部)/ 每 proc 的 Entry/Caller 四象限 invoke /
                  LORE_THUNK_FUNCTION_*_FOREACH + CALLBACK_FOREACH / 末尾 #include Detail/ProcImpl.cpp.inc
运行期:static localContext 构造 → preInitialize 填本侧表 → (GTL) GuestThunkContext::initialize 载 HTL
        调 LoreExchangeContext 对齐双端 proc 表 → ProcFn<...,Exec>::get() 取对端入口完成跨端调用
```

## 4. 验证方式

整库无法直接 build(需跑 TLC 生成完整 TU)。本轮用"伪生成 TU + 空 FOREACH"做 `-fsyntax-only`:
四条路径 guest/host × PreBuild/OnBuild 全部通过,并按 generateOutput 真实顺序复核了 host TU。
运行时各 .cpp(HostServer/GuestClient/Invocation 等)单文件 `-fsyntax-only` 通过。

## 5. 已知短板 / 未做

1. **`ManifestOnBuild.cpp.inc` 是最弱的文件**:4 个 Exec 特化各用 `#ifdef LORE_THUNK_HOST` 夹着
   guest/host 两套完整身体(本地直调 vs `GuestClient`/`HostServer` 跨端)。一个文件,但逻辑写了两遍。
   真去重需 option C:把 "client/server" 与 "local/cross" 抽成 trait/宏参数化。未做。
2. **`ProcImpl.cpp.inc` 是杂物尾**:isHostAddress + 引 ProcInit + host 交换三件凑一起,名字盖不全。
3. `.cpp.inc` 文本拼装范式固有重(无 include guard、强顺序依赖、靠宏状态串联),是 codegen 方案的代价。
4. **HLR**:本轮明确不碰(`src/tools/HLR/**`,CCG/FDG 源码重写器)。
