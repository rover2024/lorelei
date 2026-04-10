# Lorelei v2 Runtime + HLR 全量检查与初始化机制总结（2026-04-10）

## 1. 检查范围

本次检查覆盖以下三部分：

1. `GuestRT`
2. `HostRT`
3. `HLR`（`stat / mark-macros / preprocess / rewrite / batch`）

并重点梳理：

1. GTL / HTL 初始化链路
2. `CCG` / `FDG` 的数据结构、生成方式、运行时填充与调用路径

---

## 2. 关键数据结构（跨层契约）

核心上下文是 `CStaticThunkContext`（`include/lorelei/Base/PassThrough/c/CProc.h`）：

1. `guestProcs[kind][dir]`：guest 侧 proc 表
2. `hostProcs[kind][dir]`：host 侧 proc 表
3. `thisProcs`：本侧真实库符号表（用于非静态链接 `dlsym`）
4. `emuAddr`：地址边界（判断 guest/host 地址域）
5. `numCCGs + CCGs`
6. `numFDGs + FDGs`

HLR 生成侧 `FileContext.h.in` 中的 `LoreFileContext` 与之对齐，已经增加：

1. `numCCGs`
2. `numFDGs`

并通过 `__Lore_GetFileContext()` 暴露给运行时读取。

---

## 3. GTL / HTL 初始化全链路

## 3.1 编译期注入的静态初始化入口

`include/lorelei/Tools/ThunkInterface/*/ProcDefOnBuild.cpp.inc` 都定义了：

1. `static LocalThunkContext localContext;`
2. 构造顺序固定：
   1. `preInitialize()`
   2. `commonContext.initialize()`
   3. `postInitialize()`

所以库加载时会自动触发初始化。

## 3.2 GTL 侧（GuestThunkContext）

`src/modules/libs/GuestRT/GuestThunkContext.cpp`：

1. `dladdr(m_staticContext)` 获取当前 GTL 模块路径
2. `GuestSyscallClient::getThunkInfo(modulePath, false)` 查 forward thunk
3. 取 `hostThunk` 路径并远端加载 HTL（`SPID_LoadLibrary`）
4. 获取 HTL 导出 `__Lore_ExchangeContext`
5. 通过 `invokeFormat("v_p")` 传入 GTL 的 `CStaticThunkContext*` 完成上下文交换
6. 交换完成后执行 FDG 运行时填充（见第 5 节）

## 3.3 HTL 侧（HostThunkContext）

`src/modules/libs/HostRT/HostThunkContext.cpp`：

1. `dladdr(m_staticContext)` 获取当前 HTL 路径并归一化 thunk 名
2. 通过 `HostSyscallServer::thunkConfig()` 查 forward 信息
3. `dlopen(forward->hostLibrary)` 加载真实 host 库
4. 非 `LORETHUNK_CONFIG_HOST_FUNCTION_STATIC_LINK` 模式：按 `thisProcs` 做 `dlsym`
5. 尝试 `dlsym("__Lore_GetFileContext")`：
   1. 存在则注入 `emuAddr` 与 `setThreadCallback`
   2. 读取 `numCCGs/CCGs/numFDGs/FDGs`，挂到 `m_staticContext`

## 3.4 交换函数（HTL 导出）

`include/lorelei/Tools/ThunkInterface/Host/ManifestImpl.cpp.inc` 的 `__Lore_ExchangeContext`：

1. 双向 copy proc 表（按 `kind x direction`）
2. 回传 `emuAddr`
3. 回传 `numCCGs/CCGs`
4. 回传 `numFDGs/FDGs`

GTL 在 exchange 后就拿到了 HLR guard 元数据。

---

## 4. CCG 工作原理与调用路径

## 4.1 HLR 生成

1. `stat` 收集 callback 调用点签名集合（`callbackCheckGuardSignatures`）
2. `rewrite` 将 callback callee 重写为 `LORE_CCG_i(EXPR)`
3. `batch` 生成：
   1. `LoreFileContext_CCG_Tramp_i`（初始为 0）
   2. `LoreFileContext_CCGs[] = { signature, &LoreFileContext_CCG_Tramp_i }`

## 4.2 运行时语义（`__LoreFileContext_CCG`）

在 `FileContext.h.in`：

1. 若 `addr < emuAddr`，认为是 guest 域地址
2. 若命中 trampoline magic，恢复 `saved_function`
3. 否则：
   1. `setThreadCallback(addr)`
   2. `addr = tramp`

因此 `CCG` 的关键是 `tramp` 必须是对应 callback entry。

## 4.3 当前实现状态

`HostThunkContext` 现已在 CCG 初始化阶段完成 `*pTramp` 写回：

1. 先按签名在 `hostToGuestCallbackThunkByName` 查 callback entry
2. 再执行 `*ccgInfo.pTramp = callbackEntryAddr`

因此 `LORE_CCG_i(EXPR)` 路径上的 tramp 目标已具备有效值。

---

## 5. FDG 工作原理与调用路径

## 5.1 HLR 生成

1. `stat` 收集 function decay（函数衰减为函数指针）签名与实例 location
2. `rewrite` 把 decay 位点改为 `LORE_FDG_i_j(EXPR)`
3. 同时生成 `LORE_FDG_IMPL(i,j,FUNC)`，其静态对象初始为：
   1. `hostAddr = &FUNC`
   2. `guestTramp = 0`
4. `batch` 产出 `FDGs[]`（每个签名一组，组内有 `count + pProcs`）

## 5.2 GuestRT 填充逻辑（当前实现）

`GuestThunkContext::initialize()` 在 exchange 后：

1. 建立 `guestToHostCallbackThunkByName`（来自 `guestProcs[Callback][GuestToHost]`）
2. 遍历 `FDGs[i]`：
   1. 用 `FDG.signature` 找对应 callback entry target
   2. 为该签名组创建 `FunctionTrampolineTable(count, target, magic)`
   3. 组内每个实例 `pair`：
      1. `tramp.saved_function = pair->hostAddr`
      2. `pair->guestTramp = tramp.thunk_instr`

## 5.3 调用路径语义

当重写后的代码执行到 `LORE_FDG_i_j(func)`：

1. 取到的是 `guestTramp`，不是原始 `&func`
2. 后续若被当 callback 使用并进入 CCG：
   1. CCG 检测 magic 命中
   2. 可回溯到 `saved_function`（原始 host 函数地址）

这就是 FDG + FunctionTrampoline 的配合目的：

1. decay 场景下先重定向到可跨端调用入口
2. 需要原始地址时还能通过 magic trampoline 还原

---

## 6. Reentry/Invoke 路径（运行时执行面）

## 6.1 Guest -> Host

1. GTL `ProcFn<GuestToHost, Exec>` / `ProcCb<GuestToHost, Exec>`
2. `GuestSyscallClient::invoke*`
3. `SPID_InvokeProc`
4. Host `Invocation::invoke`
5. 若发生 reentry，guest 循环 `SPID_ResumeInvocation`

## 6.2 Host -> Guest

1. HTL `ProcFn<HostToGuest, Exec>` / `ProcCb<HostToGuest, Exec>`
2. `HostSyscallServer::reenter*`
3. `Invocation::reenter` 切回 guest
4. guest 执行后 `SPID_ResumeInvocation` 返回 host

`Invocation.cpp` 采用协程切栈 + `thread_local` invocation 栈，支持嵌套 reentry。

---

## 7. 本次检查结论（含修复项）

## 7.1 已确认正确/一致的点

1. Thunk config 已从 `HostSyscallServer.cpp` 挪到 `HostRT/main.cpp` 加载并注入
2. `__Lore_ExchangeContext` 已同步 `numCCGs/numFDGs`
3. HLR 资源模板与 batch 生成已包含 guard 数量字段
4. FDG 已按“signature -> guestToHost callback entry”进行 target 绑定（不是按索引硬绑定）

## 7.2 本轮发现并修复的问题

1. `GuestThunkContext` 中 `FunctionTrampolineTableGuard` 曾存在所有权错误风险
   1. 该 guard 不可拷贝/移动，放入 `std::vector<Guard>` 会在扩容时编译失败
   2. 现已改为 `std::vector<FunctionTrampolineTable *>` + 析构统一 `destroy`
2. `HostThunkContext` 已补上 CCG tramp 写回
   1. 之前只校验签名存在，未执行 `*pTramp = ...`
   2. 现已在 CCG 遍历中完成写回

## 7.3 当前剩余风险点

1. `HostRT/main.cpp` 在 config load 失败时仅 warning，不会提前 fail-fast
   1. 后续 `SPID_GetThunkInfo` 处是 `assert(config != nullptr)` + 查表
   2. 建议按运行模式决定是否改为启动即失败

---

## 8. 建议的下一步（最小闭环）

1. 给 FDG 增加一个最小 E2E 回归：
   1. function decay -> 间接调用 -> nested reentry
   2. 校验 `saved_function` 回溯路径
2. 给 CCG 增加一个最小 E2E 回归：
   1. 直接 callback 调用路径触发 `LORE_CCG_i`
   2. 验证 `pTramp` 已被正确填入并可完成回调转发
3. 给 `HostRT` 增加 “config load 失败策略” 开关（strict / permissive）
