# LoreHLR 工作原理（完整说明）

本文档说明 `src/tools/tools/HLR` 的实现机制，包括命令分发、AST 采集、逐阶段重写、批处理编排、产物结构，以及当前实现的边界与注意事项。

## 1. 目标与定位

`LoreHLR`（Lorelei Host Library Rewriter）是一个基于 Clang LibTooling 的源码重写工具链。它的目标是：

1. 从 C/C++ 源码中识别两类风险点。
2. 对风险点插入宏包装。
3. 生成运行时上下文文件（`FileContext.h` / `FileContext.c`）。
4. 让重写后的源码在运行时可以做 callback guard 和 function decay guard。

两类风险点：

1. `callback invoke`：通过函数指针发起调用。
2. `function decay`：函数名衰减为函数指针（非直接调用位点）。

## 2. 构建与可执行结构

对应文件：

1. `src/tools/tools/HLR/CMakeLists.txt`
2. `src/tools/tools/HLR/main.cpp`
3. `src/tools/libs/ToolMain/main.cpp.inc`

### 2.1 目标构建

`LoreHLR` 是一个可执行程序，链接：

1. `LoreToolUtils`
2. `LoreToolMain`
3. Clang/LLVM 组件（通过 `lore_link_clang`）

另外会把资源模板编译进二进制：

1. `Resources/FileContext.h.in`
2. `Resources/Warning.txt.in`

资源在运行时通过 `res_FileContext_h_c` / `res_Warning_txt_c` 直接输出，不依赖外部文件路径。

### 2.2 命令路由

`main.cpp` 声明了命令集合：

1. `batch`
2. `mark-macros`
3. `preprocess`
4. `rewrite`
5. `stat`
6. `help`

`ToolMain/main.cpp.inc` 完成统一入口：

1. `--help/-h` 打印帮助。
2. `--version/-v` 输出版本。
3. 第一个位置参数作为子命令名，后续参数转发给对应命令的 `main()`。

## 3. 端到端执行模型

`HLR` 推荐走 `batch`，它会编排完整流水线：

1. `stat`：先统计全体文件中的 guard 信息。
2. 对每个文件顺序执行：`mark-macros -> preprocess -> rewrite`。
3. 生成共享上下文：`FileContext.h` 与 `FileContext.c`。
4. 给每个文件加 warning 头注释与 `#include ".../FileContext.h"`。
5. 在第一个源文件尾部追加 `#include ".../FileContext.c"`。

这样每个重写文件都能访问统一的 guard 声明，而 guard 定义只在一个 TU 中落地，避免多重定义。

## 4. 核心数据结构

对应文件：`Utils/SourceStatistics.h/.cpp`

`SourceStatistics` 包含两部分：

1. `callbackCheckGuardSignatures`：`set<string>`
2. `functionDecayGuardStats`：`map<string, FunctionDecayGuardData>`

其中 `FunctionDecayGuardData.locations` 是：

1. key：函数声明起始位置字符串（`SourceLocation::printToString`）
2. value：最后引用该函数衰减的文件名

JSON 形状：

1. `callbackCheckGuardSignatures`: `string[]`
2. `functionDecayGuardStats`: `[{ signature, locations }]`

## 5. AST 采集机制

对应文件：`Utils/ASTConsumers.h/.cpp`

`FunctionExprConsumer` 同时注册两个 matcher：

1. `CallbackInvokeExprMatcher`
2. `FunctionDecayExprMatcher`

### 5.1 callback invoke 判定

输入是 `CallExpr`，只收集“callee 是函数指针”的调用：

1. `callee` 类型是 function pointer。
2. 或 `(*fp)(...)` 这种解引用调用。

工具函数：`isFunctionPointerCallee()`。

### 5.2 function decay 判定

输入是 `DeclRefExpr(functionDecl)`，并过滤掉“这个引用就是调用 callee”的情况。

`isDeclRefForCall()` 会向上爬父节点，穿透：

1. cast
2. paren
3. 一元 `*` / `&`

若最终父节点是 `CallExpr` 且该表达式处于 callee 位，则不是 decay；否则记为 function decay。

## 6. 类型签名归一化

对应文件：`ToolUtils/TypeUtils.cpp`

### 6.1 `realCalleeType`

对 `CallExpr` 的 callee 类型做修正：

1. 取 canonical pointee type。
2. 若是 `FunctionNoProtoType` 且调用有参数，则用实参数类型补全成真实函数类型。
3. 最终返回“函数指针类型”。

### 6.2 `getTypeString`

把 `QualType` 转字符串，并把系统相关的 `__va_list_tag` 形式替换成 `va_list`，减少平台差异带来的签名分裂。

## 7. 各子命令内部原理

## 7.1 `stat`

对应文件：`Commands/Stat.cpp`

职责：统计全体源文件中“关注签名”的位点。

流程：

1. 从 `-c <config>` 读取签名白名单（忽略空行和 `#` 注释）。
2. 用 `FunctionExprConsumer` 遍历每个文件 AST。
3. 对 callback invoke：按 `realCalleeType -> getTypeString`，若在白名单中，加入 `callbackCheckGuardSignatures`。
4. 对 function decay：按函数指针签名过滤，记录 `location -> file`。
5. 输出 JSON（`-o` 或 stdout）。

注意：`saveAsJson` 现在是覆盖写入语义（非 append），同一路径重复执行会以最新统计结果为准。

## 7.2 `mark-macros`

对应文件：`Commands/MarkMacros.cpp`，标签定义在 `MarkMacros.h`。

职责：在“宏展开中间态”的风险表达式外层打标记注释，供后续预处理替换。

标签：

1. `/*__LORELEI_MACRO_BEGIN__*/`
2. `/*__LORELEI_MACRO_END__*/`

为什么需要这一步：

1. 直接对宏源码重写会丢语义。
2. 先标注“需要替换的展开片段”，再在 `preprocess` 阶段取真实展开文本回填，可保留宏展开结果。

核心规则：

1. 仅处理主文件（`SM.isInMainFile`）。
2. 仅处理“处于宏展开内部且不是完整边界”的表达式。
3. callback 与 function decay 都按 `stat` 结果过滤签名。
4. 用 `RewriteRangePackerSet` 保证插入区间不重叠。

## 7.3 `preprocess`

对应文件：`Commands/Preprocess.cpp`

职责：执行预处理，把 `mark-macros` 标注区间替换成“真实宏展开文本”。

流程：

1. 继承 `PrintPreprocessedAction`，配置 `PreprocessorOutputOptions`。
2. 生成预处理文本（不输出 line markers / include directives）。
3. 在预处理文本中扫描 `BEGIN/END`，收集展开片段列表。
4. 在原始源码中按同序号标签区间替换为展开片段。
5. 输出结果源码。

这个阶段不引入 guard 宏，只做“宏语义展开对齐”。

## 7.4 `rewrite`

对应文件：`Commands/Rewrite.cpp`

职责：在表达式上插入最终 guard 宏调用。

流程：

1. 读取 `stat` JSON，建立索引。
2. callback invoke 插入：`LORE_CCG_<i>(EXPR)`。
3. function decay 插入：`LORE_FDG_<i>_<j>(EXPR)`。
4. 若当前文件包含某 decay guard 对应函数定义，追加 `LORE_FDG_IMPL(i, j, FuncName)` 到文件末尾。
5. 输出重写源码。

索引来源：

1. `CCGIndexMap`: `signature -> i`
2. `FDGIndexMap`: `signature -> i`
3. `FDGInstanceInfoMaps[i][location] -> j`

## 7.5 `batch`

对应文件：`Commands/Batch.cpp`

职责：编排全流程并生成上下文文件。

关键步骤：

1. 解析参数：`-c`(可选), `-o`(输出目录), `-p`(build path), `sources...`。
2. 自动探测 `compile_commands.json`。
3. 建临时统计文件，运行 `stat`。
4. 对每个文件运行三步：`mark-macros -> preprocess -> rewrite`。
5. 给每个文件写入 warning 头与 `#include ".../FileContext.h"`。
6. 生成 `FileContext.h`/`FileContext.c`：
   1. 按统计结果展开 `LORE_CCG_DECL/IMPL` 与 `LORE_FDG_DECL/IMPL`。
   2. 生成 `LoreFileContext_instance`。
   3. 导出 `__getLoreFileContext()`。
7. 在第一个输入源文件末尾追加 `#include ".../FileContext.c"`。

## 8. 资源模板与运行时接口含义

## 8.1 `Warning.txt.in`

每个重写文件顶部会加自动生成警告头，声明文件由 HLR 生成。

## 8.2 `FileContext.h.in`

定义了 HLR 运行时桥接结构：

1. `LoreThunkProcInfo`
2. `LoreFunctionTrampoline`
3. `LoreFunctionDecayGuard`
4. `LoreFunctionDecayGuardInfo`
5. `LoreRuntimeContext`
6. `LoreFileContext`

以及一组宏：

1. `LORE_CCG_DECL/IMPL/GET`
2. `LORE_FDG_DECL/IMPL/GET`

`LORE_CCG_GET` 的语义：

1. 若地址处在 emu 区域以下，判定为潜在 guest callback/trampoline。
2. 若命中 trampoline 魔数，则还原真实函数地址。
3. 否则调用 `setThreadCallback`，并替换成 host tramp。

`LORE_FDG_GET` 的语义：

1. 把函数衰减引用替换为预先建立的 `guestTramp`。

## 9. 输入输出与落盘规则

输入前提：

1. 必须有可用编译数据库（`-p` 指向 build dir）。
2. `stat` 依赖签名配置文件（`-c`）。
3. `sources` 是要重写的源文件集合。

输出：

1. 原文件被原地改写（batch 里 `-o file` 指向自身）。
2. 输出目录生成：
   1. `FileContext.h`
   2. `FileContext.c`
3. 每个文件头部会插 warning + include。
4. 第一个源文件尾部追加 include `FileContext.c`。

## 10. 关键实现细节与边界

1. `mark-macros` 只处理主文件，不处理头文件直接重写。
2. 区间写入通过 `RewriteRangePackerSet` 去重叠，避免互相覆盖。
3. `preprocess` 用“同序替换”回填展开，若标签数量不匹配会发 warning。
4. `rewrite` 对 FDG 需要稳定 location 字符串来对应实例索引。
5. `batch` 通过子进程调用自己实现阶段解耦，失败会立刻中断。
6. 资源模板内嵌，部署不依赖外部模板文件。
7. 当前实现会直接覆盖/改写输入文件，属于有副作用流程。

## 11. 命令层建议用法

1. 调试单文件：先独立跑 `stat`，再依次 `mark-macros/preprocess/rewrite`。
2. 项目批量改写：优先用 `batch`，保证索引一致性。
3. 变更审阅：在跑 `batch` 前先保存工作区快照，便于回滚和 diff。

## 12. 扩展点

1. 新增识别规则：扩展 `Utils/ASTConsumers.*`。
2. 新增重写策略：扩展 `rewrite` 中的插入宏生成逻辑。
3. 新增模板字段：修改 `Resources/FileContext.h.in` 与 `batch` 代码生成段。
4. 新增命令：在 `main.cpp` 的 `TOOL_MAIN_COMMAND_FOREACH` 注册并实现子命令。

## 13. 一句话总结

`LoreHLR` 的核心是“先全局统计、再逐文件标记宏、再预处理回填、最后插 guard 宏并生成统一 FileContext”，从而把源码中的函数指针调用与函数衰减路径接入 Lorelei 运行时防护机制。

## 14. 调用时序图（文字版）

下面用“调用链”视角描述 HLR 的执行时序，便于把命令层和实现层快速对齐。

### 14.1 `batch` 全流程时序

1. 用户执行：`LoreHLR batch -o <out_dir> -p <build_dir> [-c <sig.conf>] <src...>`。
2. `ToolMain` 分发到 `lore::tool::command::batch::main()`。
3. `batch::main()` 解析参数，探测编译数据库。
4. 创建临时统计文件 `statResultFile`。
5. `batch` 通过 `runSubCommand()` 启动子进程：`LoreHLR stat ...`。
6. `stat` 进程内：
   1. 读取签名配置。
   2. ClangTool 遍历所有输入文件 AST。
   3. 收集 CCG/FDG 统计并写入 JSON。
7. 控制回到 `batch`，加载 `statResultFile` 为 `SourceStatistics`。
8. `batch` 遍历每个输入文件，依次启动 3 个子命令：
   1. `mark-macros`（给宏展开场景打标签）
   2. `preprocess`（把标签区间替换为真实展开文本）
   3. `rewrite`（插入 `LORE_CCG_x` / `LORE_FDG_x_y`）
9. 三步完成后，`batch` 对该文件追加：
   1. 自动生成 warning 注释头
   2. `#include "<relative>/FileContext.h"`
10. 所有文件处理完后，`batch` 生成：
   1. `<out_dir>/FileContext.h`
   2. `<out_dir>/FileContext.c`
11. `batch` 在第一个输入源文件末尾追加：`#include "<relative>/FileContext.c"`。
12. `batch` 返回成功，临时 `statResultFile` 被 scope guard 删除。

### 14.2 单文件重写时序（`mark-macros -> preprocess -> rewrite`）

1. `mark-macros`：
   1. 读取 `stat` 产出的 JSON。
   2. AST 匹配 `CallExpr` 与 `DeclRefExpr(functionDecl)`。
   3. 仅在“宏展开中间态”区间插入 `MACRO_BEGIN/END`。
   4. 输出带标签源码。
2. `preprocess`：
   1. `PrintPreprocessedAction` 输出预处理文本。
   2. 从预处理文本提取每个 `BEGIN/END` 的展开片段。
   3. 回到原文，用同序号展开片段替换标签区间。
   4. 输出“宏语义已落地”的源码。
3. `rewrite`：
   1. 读取 `stat` JSON 建索引映射。
   2. 对 callback invoke 插入 `LORE_CCG_i(...)`。
   3. 对 function decay 插入 `LORE_FDG_i_j(...)`。
   4. 当前文件若命中 FDG 定义位点，末尾追加 `LORE_FDG_IMPL(...)`。
   5. 输出最终重写源码。

### 14.3 运行时接入时序（重写代码执行时）

1. 重写后的源码包含 `FileContext.h`，并在某一 TU 引入 `FileContext.c`。
2. `FileContext.c` 提供 `LoreFileContext_instance` 与 `__getLoreFileContext()`。
3. 当执行到 callback guard 宏：
   1. 进入 `LORE_CCG_GET`。
   2. 根据 `emuAddr` 判断地址域。
   3. 必要时触发 `setThreadCallback()` 或 trampoline 还原。
4. 当执行到 function decay guard 宏：
   1. 进入 `LORE_FDG_GET`。
   2. 读取 guard 表中的 `guestTramp` 并替换目标地址。

### 14.4 与源码文件的快速映射

1. 命令入口与分发：`main.cpp`, `ToolMain/main.cpp.inc`
2. 批处理编排：`Commands/Batch.cpp`
3. 统计阶段：`Commands/Stat.cpp`, `Utils/ASTConsumers.*`, `Utils/SourceStatistics.*`
4. 宏标注阶段：`Commands/MarkMacros.cpp`, `Commands/MarkMacros.h`
5. 预处理回填阶段：`Commands/Preprocess.cpp`
6. 最终改写阶段：`Commands/Rewrite.cpp`
7. 运行时模板：`Resources/FileContext.h.in`, `Resources/Warning.txt.in`
