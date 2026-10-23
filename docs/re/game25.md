# Steam macOS 2.5 bootstrap evidence

Image UUID: 7D7382D0-EADD-36F0-BA1C-CEBEC1D5BEBC.
All code offsets below are relative to the Mach-O __TEXT base 0x100000000.
Read from the installed game on 2026-10-22 using LC_FUNCTION_STARTS and ARM64
instruction decoding. Profile: data/addresses/2.5.json. All hashes from the 2.3.1 catalog are retained; unported entries have zero
offsets and verified=false so plugin preflight cannot silently overlook them. This is a bootstrap profile, not a full framework release.

| Name | Offset | Identity and ABI evidence |
| --- | --- | --- |
| WinMain | 0x4e3274c | main at 0x323a0 calls this with argc x0 / argv x1 at 0x323f8, immediately before application main at 0x3249c. Same engine initialization role as 2.3.1 0x344cbfc. |
| CGameApplication_AddState | 0x4d6a27c | Application main calls four times at 0x324fc, 0x32510, 0x32524, 0x32538 after four state constructors. Reads state vtable +0x18 (GetType), inserts state into app +0x268; bool in w0. |
| CRTTISystem_Get | 0x10b3cac | Guarded singleton 0x108055e98; initializer 0x10c0e98 invokes ctor 0x10b3384, which installs vtable 0x107831010. GetClass/Enum were also exercised by the existing console port. |
| CGlobalFunction_ctor | 0x109ba60 | Calls base ctor 0x1097de8 with names x1/x2 and flags=3, installs vtable 0x10782ff00, registers native x3 through 0x11a9590 and stores returned index at +0xf0. Returns original object x0. |
| CClassFunction_ctor | 0x109b838 | Parent x1 at +0xf0; names x2/x3 passed to base ctor; native member pointer x4/x5 and flags w6. Registration index +0xf8. |
| CClassStaticFunction_ctor | 0x109b910 | Parent x1, names x2/x3, function x4, flags w5 OR 3; native registration 0x11a9590; index +0xf8. |
| CNamePool_AddCstr | 0x4e389d0 | Computes string length, FNV-1a with 0xcbf29ce484222325 and 0x100000001b3, special None case, interns through 0x4e3865c. const char* x0; uint64 hash returned x0. |
| CNamePool_AddCString | 0x4e38a98 | Explicit (text x0,length w1) variant of same hash/interner, returning hash x0. |
| CNamePool_AddPair | 0x4e38978 | Hash x0 preserved, text x1 length obtained by strlen then tail-calls 0x4e3865c(hash,text,length). |
| CNamePool_Get | 0x4e38940 | Loads hash from reference x0, tail-calls 0x4e387bc; returns string pointer x0 (and length x1). |
| DynArray_Realloc | 0x28674 | Called by RTTI AddPostRegisterCallback with array x0, capacity w1, element size 0x48 w2, alignment 8 w3, move callback x4. Matches generic reserve/move operation in 2.3.1 0x286e8. |
| CString_ctor_str | 0x2cbdc | Clears pointer/SSO length +0x14 and allocator +0x18, strlen(x1), reserve 0x2c3d4, memcpy, terminates and returns this. |
| CString_copy | 0x2b0ac | Copies allocator +0x18 and inline 24 bytes or allocates/copies heap bytes using length & 0x3fffffff. Called for stack CString at 0x4274408, followed by destructor 0x2914c. |
| CString_dtor | 0x2914c | Reads flags +0x14, calls reserve(0) for owned heap storage. Does not free object. Called on startup and stack CString temporaries. |
| CStackFrame_dtor | 0x11ab310 | Existing console-port disassembly verified release of root call map at +0x68 through allocator +0x90; does not free frame or argument buffers. |
| Memory_PoolScript_vtbl | 0x7cddb78 | Existing console-port root-frame constructor evidence and live console calls identify allocator vtable for the root map at +0x90. Stored as a data-segment offset in this profile. |
| CBaseFunction_InternalExecute | 1:0x1099fbc | Called by the FinalFunc opcode handler (table entry 0x24 = 0x12011e4, dumped live) as (func x0, context x1, caller frame x2, result x3, result type x4) - the SDK CallFunction_t ABI. Same first 16 prologue bytes (fc6fbaa9fa6701a9f85f02a9f65703a9) as 2.3.1 InternalExecute 0x2173120; tests func +0xa8/+0xac as in 2.3.1. Builds the child frame through 0x11ab268 (frame ctor) and destroys it with CStackFrame_dtor 0x11ab310. Used live by the NightCity 2.5 console for every scripted call. |
| OpcodeHandlers | 3:0x1d2c940 | Data table (0x109a5c940, __DATA+0x1d2c940) indexed by the opcode byte in every native ScriptFunc (e.g. Casting::CastFunc<uchar,schar>::ScriptFunc at 0x11ad0dc: adrp/add 0x109a5c940; ldr x8,[x9,x8,lsl#3]; blr). Dumped live on 2.5: 0x6b handlers, entry 0x24 is the FinalFunc handler that calls InternalExecute. 2.3.1 entry 3:0x1CF7798 is the same table (0x10908b798) used by the 2.3.1 console. |
| CBaseFunction_ExecuteNative | 1:0x109b16c | Instruction-for-instruction match of 2.3.1 ExecuteNative 0x21732dc (bool(CBaseFunction* x0, CStack* x1)): calls CStack vtable +0x30 (GenerateCode) into a stack buffer, builds a CStackFrame with ctx = stack+0x10 and func = stack+8 (frame is 0x40 bytes larger, matching the 0x68->0xa8 growth), calls the native body 0x1099c44(func, stack, frame), destroys the frame with CStackFrame_dtor 0x11ab310 and returns its bool. |
| CBaseFunction_ExecuteScripted | 1:0x1098fc0 | Matches 2.3.1 ExecuteScripted 0x2172bf4 (bool(CBaseFunction* x0, CStack* x1, void* x2)): reads func +0xac (locals size), __chkstk alloca, CStack vtable +0x28 into the locals, the same CStack helper (2.3.1 0x21742cc <-> 2.5 0x109d20c, also paired in ExecuteNative), builds the frame with the 2.5 frame ctor 0x11ab268 (2.3.1 0x21ff030), result type via vtable +0x10/+0x18, then the scripted run 0x10997f8 (2.3.1 0x2172d54). Reached by tail branch from 0x109b21c. |
| CStack_vtbl | 2:0xd80d8 | Vtable address point 0x1078300d8 (__DATA_CONST+0xd80d8). Slot +0x10 is GetResultAddr 0x109d62c, the only 2.5 function with the 2.3.1 GetResultAddr bytes (ldr x8,[x0,#0x40]; cbz; ldr x0,[x8,#8]; ret; mov x0,#0; ret), and the only slot pointing at it. +0x18 GetResultType 0x109d644, +0x28 0x109d48c, +0x30 GenerateCode 0x109d5b8 emits opcode 0x1b per argument then 0x26 (ParamEnd), as in 2.3.1 (vtable 2:0x21aea0). Referenced by the CStack construction code at 0x109d384/0x109d3a8/0x109d3fc. |
| StatsDataSystem_InitializeRecords | 1:0x23b2f68 | Same body as 2.3.1 0x3a939a0 (void(StatsDataSystem* x0)): CRTTISystem::Get thunk 0x10b3288, GetEnum (vtable +0x18) for gamedataStatType hash 0x743066319f27468d, reserves the stat array at this+0xd8 (count read at +0xe0). Stat count grew 0x6ad -> 0x6b0. Only 2.5 function referencing the 2.3.1 string 'BaseStats.'. |
| TweakDB_TryLoad | 1:0x37d60cc | Only 2.5 function referencing both 2.3.1 TryLoad strings ('Binary blob not found', 'Binary blob header is not valid'). Called twice from TweakDB_Load/LoadOptimized (0x37ce10c, 0x37ce128) exactly like 2.3.1 0x2b7561c/0x2b75634: x0 = TweakDB+0x28, x1 = TweakDB*, x2 = path CString, x3 = out buffer. 2.5 adds a 5th argument, a bool in x4 (caller's flag, 0 on the fallback call), matching CET 2.5's note that LoadOptimized gained a bool. Returns bool in w0. |
| TweakDB_CreateRecord | 1:0x3309cc4 | Instruction-for-instruction match of 2.3.1 0x26b8db8 (void(TweakDB* x0, uint32 type w1, TweakDBID x2)): computes w1 & 0x1f, dispatches through a byte jump table (2.5 0x1073ce970) to per-record-type creators called as (x1, x2), then shares one epilogue. Top call-graph vote (3 matched neighbours) and fuzzy score 0.854. |
| StatsDataSystem_CheckStatFlag | 1:0x23b3438 | Same position relative to StatsDataSystem_InitializeRecords (0x23b2f68) as in 2.3.1 and an instruction-for-instruction match: stat index bound 0x6ac -> 0x6af (3 stats added in 2.5, matching InitializeRecords' 0x6ad -> 0x6b0), lock at this+0xfc via 0x15c8 (2.3.1 0x2098, an exact candidate pair), 12-byte stat records at this+0xe8; tests record +8 flags against w2 and returns bool (2.3.1 0x3a93e74). |
| StatsDataSystem_GetStatFlags | 1:0x23b34b4 | Same position relative to StatsDataSystem_InitializeRecords (0x23b2f68) as in 2.3.1 and an instruction-for-instruction match: stat index bound 0x6ac -> 0x6af (3 stats added in 2.5, matching InitializeRecords' 0x6ad -> 0x6b0), lock at this+0xfc via 0x15c8 (2.3.1 0x2098, an exact candidate pair), 12-byte stat records at this+0xe8; returns record +8 flags in w0, 0 when out of range (2.3.1 0x3a93ef0). |
| StatsDataSystem_GetStatRange | 1:0x23b3cb0 | Same position relative to StatsDataSystem_InitializeRecords (0x23b2f68) as in 2.3.1 and an instruction-for-instruction match: stat index bound 0x6ac -> 0x6af (3 stats added in 2.5, matching InitializeRecords' 0x6ad -> 0x6b0), lock at this+0xfc via 0x15c8 (2.3.1 0x2098, an exact candidate pair), 12-byte stat records at this+0xe8; returns the record's float min/max in s0/s1, 0/0 when out of range (2.3.1 0x3a9472c). |
| TweakDB_Init | 1:0x1ef64c8 | Call-for-call match of 2.3.1 init lambda 0x35f1bac (void(void* x0, void* x1)): same JobInternals_SetLocalThreadParam calls, TweakDB singleton create 0x37cc7d4 (2.3.1 0x2b73b50), TweakDB_Get 0x37cc900 (verified; 2.3.1 0x2b73c7c), 0x37cdd90 (2.3.1 0x2b75294) with w1 from this+0x6a, then TweakDB_Load 0x37ce05c with the new 2.5 bool (w1 = 1) and path this+0x48 in x2, then CString_dtor 0x2914c on this+0xa0/+0x80/+0x48. One of only two callers of TweakDB_Load, as in 2.3.1. |
| JobInternals_SetLocalThreadParam | 1:0x4df0d14 | Instruction match of 2.3.1 0x9e5f94: resolves a thread_local through its TLV descriptor (2.5 0x107eb2eb8) and stores w0 as a byte. Several thread-local setters compile to identical code (fuzzy found 0x15212e0 too); this one is the function TweakDB_Init (0x1ef64c8) calls at the same two positions where 2.3.1 TweakDB_Init calls 0x9e5f94. |
| TweakDB_Load | 1:0x37ce05c | LoadOptimized. Contains both TweakDB_TryLoad calls (0x37ce10c, 0x37ce128) like 2.3.1 0x2b75570; called by TweakDB_Init. 2.5 signature void(TweakDB* x0, bool w1, CString& path x2) (2.3.1: (TweakDB*, CString&)): the init lambda now passes w1 = 1 and moves the path to x2, as CET 2.5 documents for LoadOptimized. |
| JobInternalHandle_Acquire | 1:0x4de2d7c | Byte-identical to 2.3.1 0x9d8388 apart from the allocator call: allocates 0x28, zeroes it, refcount +0x1c = 1, +0x21 = 0xff, stores x1 (name) and x2 (unk) at +0x8/+0x10; x0 ignored (same ABI mismatch with the SDK as noted for 2.3.1). Exact function-candidates match. |
| JobDispatcher | 3:0x1fde550 | job::Dispatcher* global at 0x109d0e550 (__DATA). The dispatch wrapper 0x4ddf514 (2.3.1 0x9d46f0, 0x28 bytes, 676 callers) loads it into x0 and tail-calls DispatchJob exactly where 2.3.1 loads 0x108dc7ff0 (2.3.1 JobDispatcher 3:0x1a33ff0); ~Counter 0x4ddf828 releases through it too. |
| JobDispatcher_DispatchJob | 1:0x4de3088 | Job module laid out in the same order as 2.3.1 (docs/re/jobs.md) with exact anchors HandleWait 0x4ddf5c0, Join 0x4ddf868, Acquire 0x4de2d7c, EmptyJob builder 0x4de3058. Matches 2.3.1 0x9d8690 (Dispatcher*, const JobInstance&, u8 prio, Counter* wait, Counter* signal): if signal set, atomically increments signal+0x18 and on 0->1 refcount +0x1c, then tail-calls the queue routine 0x4de30f4 (2.3.1 0x9d8720). Target of the wrapper 0x4ddf514. |
| JobHandle_dtor | 1:0x4ddf828 | Job module laid out in the same order as 2.3.1 (docs/re/jobs.md) with exact anchors HandleWait 0x4ddf5c0, Join 0x4ddf868, Acquire 0x4de2d7c, EmptyJob builder 0x4de3058. ~Counter: if [x0] set, Release(*JobDispatcher, x1) via 0x4de2ae0, clears [x0], returns x0 - as 2.3.1 0x9d4a00. Picked the called twin (394 callers; zero-caller twin 0x4ddf7e8), as for 2.3.1. |
| JobQueue_ctor_FromParams | 1:0x4ddef64 | Job module laid out in the same order as 2.3.1 (docs/re/jobs.md) with exact anchors HandleWait 0x4ddf5c0, Join 0x4ddf868, Acquire 0x4de2d7c, EmptyJob builder 0x4de3058. Builder ctor (Builder*, u8 prio x1, u8 threadParam x2, uintptr x3): stores the name pointer and x3 at +0, as 2.3.1 0x9d40c8. Called twin (2 callers; zero-caller twin 0x4ddeed0), matching the 2.3.1 pair 0x9d4020/0x9d40c8. |
| JobQueue_SyncWait | 1:0x4ddf354 | Job module laid out in the same order as 2.3.1 (docs/re/jobs.md) with exact anchors HandleWait 0x4ddf5c0, Join 0x4ddf868, Acquire 0x4de2d7c, EmptyJob builder 0x4de3058. void(Builder*): tests this+0x18 through 0x4ddf8e8, then this+0x10 / +0x18 into 0x4ddf760 (exact pair of 2.3.1 0x9d493c), as 2.3.1 0x9d451c. Called twin (666 callers; zero-caller twins 0x4ddf280/0x4ddf2e8 precede it as in 2.3.1). |
| JobQueue_dtor | 1:0x4ddf488 | Job module laid out in the same order as 2.3.1 (docs/re/jobs.md) with exact anchors HandleWait 0x4ddf5c0, Join 0x4ddf868, Acquire 0x4de2d7c, EmptyJob builder 0x4de3058. ~Builder body: checks captured flag this+0x30, finalizes this+0x18 through 0x4ddf8e8, as 2.3.1 0x9d4660. The 4-byte twin 0x4ddf510 is 'b 0x4ddf488' (2.3.1 0x9d46ec / 0x9d4660 pair); same body choice as 2.3.1. |
| ResourceDepot | 3:0x32a7d8 | ResourceDepot* global 0x10805a7d8 (__DATA). Getter 0x10f1230 (53 callers) / setter 0x10f123c are the adjacent 'ldr x0,[g]; ret / str x0,[g]; ret' pair like 2.3.1 0x21c6820/0x21c682c (42/2 callers). The setter's two callers are the depot init 0x442d290 (matches 2.3.1 InitResourceDepot 0x1704194: branch on [x1+0xc0]==2, path from x1+0xc8; stores the new depot at app+0x198) and its shutdown path, as in 2.3.1. LAYOUT CHANGE: 2.5 allocates 0xf0 bytes for the depot (2.3.1: 0x80), ctor 0x151a4e0 - re-derive the struct before using SDK/ArchiveXL field offsets. |
| CRTTIRegistrator_RTTIAsyncId | 3:0x325a60 | Async-id counter (uint32) at 0x108055a60 (__DATA). Its incrementer 0x10af5dc (15667 callers, the type-registration id allocator) is the 2.3.1 0x2184d7c pattern: guard byte at counter+8 (0x108055a68, ldaprb), ldaddal +1 on the counter, returns new value; 2.3.1 counter 3:0x9d5e50 = 0x107d69e50 with guard 0x107d69e58. Only 2.5 function with this sequence and >10 callers. |
| CharacterCustomizationSystem_Uninitialize | 1:0x4bc83a8 | Match of 2.3.1 0x102459f78 (instruction similarity 1.0, size 0x44/0x44, candidate sources map, mapped-callee agreement 1.0). normalized diff: identical (0 differing instructions). |
| AppearanceNameVisualTagsPreset_GetVisualTags | 1:0x2605214 | Match of 2.3.1 0x103cd7234 (instruction similarity 1.0, size 0xdc/0xdc, candidate sources map, mapped-callee agreement None). normalized diff: identical (0 differing instructions). |
| ObjectPackageReader_ReadHeader | 1:0x19be360 | Match of 2.3.1 0x100caaa74 (instruction similarity 1.0, size 0xb0/0xb0, candidate sources map+fuzzy, mapped-callee agreement 1.0). normalized diff: identical (0 differing instructions). |
| QuestsSystem_OnGameRestored | 1:0x2a0b26c | Match of 2.3.1 0x102ef8c58 (instruction similarity 1.0, size 0x40/0x40, candidate sources map, mapped-callee agreement None). normalized diff: identical (0 differing instructions). |
| ResourceLoader_IssueLoadingRequest | 1:0x10e710c | Match of 2.3.1 0x1021b71cc (instruction similarity 1.0, size 0x84/0x84, candidate sources map+fuzzy, mapped-callee agreement 1.0). normalized diff: identical (0 differing instructions). |
| CMesh_GetAppearance | 1:0x519e118 | Match of 2.3.1 0x100e199d8 (instruction similarity 1.0, size 0x44/0x44, candidate sources map, mapped-callee agreement None). normalized diff: only the low 12 bits of one ADRP/ADD data address differ. |
| ResourceLoader_IssueLoadingRequestByPath | 1:0x10e7060 | Match of 2.3.1 0x1021b7130 (instruction similarity 0.902, size 0x9c/0xac, candidate sources map+fuzzy, mapped-callee agreement 1.0). same calls (all mapped callees present); the on-stack request is built differently: extra byte set at +0x19, fields moved - the SDK-documented 2.5 change ResourceRequest -> 0x30 bytes. Path in x1 as before. |
| ObjectPackageReader_OnReadHeader | 1:0x19be410 | Match of 2.3.1 0x100caab24 (instruction similarity 0.97, size 0x84/0x84, candidate sources fuzzy, mapped-callee agreement None). normalized diff: same operations, different register choice (ldrsh x8 vs w8; vtable +0x20 loaded into x10). |
| CharacterCustomizationGenitalsController_OnAttach | 1:0x4bae148 | Match of 2.3.1 0x102440418 (instruction similarity 1.0, size 0x4c/0x4c, candidate sources map, mapped-callee agreement 1.0). normalized diff: only a member offset changes, +0x230 -> +0x328 (class layout grew in 2.5); all mapped callees present. |
| JobHandle_Join | 1:0x4ddf868 | Match of 2.3.1 0x1009d4a3c (instruction similarity 1.0, size 0x80/0x80, candidate sources map+fuzzy, mapped-callee agreement 1.0). normalized diff: only the JobDispatcher global page offset (0x108dc7ff0 -> 0x109d0e550, the verified 2.5 JobDispatcher). |
| BufferReader_MakeType1 | 1:0x12174ac | Match of 2.3.1 0x10226442c (instruction similarity 1.0, size 0x9c/0x9c, candidate sources map, mapped-callee agreement None). normalized diff: only the low 12 bits of one data address differ. |
| CBaseEngine_LoadGatheredResources | 1:0x4d7fba8 | Match of 2.3.1 0x103d9efc8 (instruction similarity 1.0, size 0x64/0x64, candidate sources map, mapped-callee agreement 1.0). normalized diff: stack-guard GOT slot and one engine field offset, +0xd8 -> +0x100 (engine layout grew in 2.5); all mapped callees present. |
| ObjectPackageExtractor_ExtractAsync | 1:0x1075b34 | Match of 2.3.1 0x102150a60 (instruction similarity 0.983, size 0xec/0xec, candidate sources fuzzy, mapped-callee agreement 1.0). normalized diff: one cbnz w8 -> tbnz w8,#0 (bool test); all mapped callees present. |
| JobQueue_Capture | 1:0x4ddf3c4 | Match of 2.3.1 0x9d4590 (similarity 0.93, all mapped callees present). sret: 'mov x19, x8' before any write (same ABI as 2.3.1; the macOS SDK path declares JobHandle(*)(JobQueue*) so x8 is passed), finalizes this+0x18 through 0x4ddf8e8, moves the wait Counter out, sets captured flag this+0x30. Sits after SyncWait/Builder::Wait in the same order as 2.3.1. |
| JobQueue_ctor_FromGroup | 1:0x4ddf1c4 | Builder ctor (Builder*, const RunContext& x1): reads [x1+0x30] and builds this+0x10/+0x18 like 2.3.1 0x9d4374; only register allocation differs. Called twin (286 callers) - its zero-caller twin 0x4ddf108 precedes it, matching the 2.3.1 pair 0x9d42a8/0x9d4374 (0/283). |
| CString_ctor_span | 1:0x2933c | Match of 2.3.1 0x2940c (similarity 0.90, all mapped callees present): CString from (data, length) - reserve, copy, NUL-terminate; differences are a folded store (strb wzr,[x8,w20,uxtw]) and branch layout. |
| CClass_InitializeProperties | 1:0x10c37d8 | Match of 2.3.1 0x2197be4: same 87 instructions (similarity 0.97), only register allocation differs; all mapped callees present. |
| AnimatedComponent_InitializeAnimations | 1:0x18a5174 | Exact-map candidate of 2.3.1 0xb97524 (similarity 0.92, all mapped callees present); normalized diff is register allocation only (same member offsets, e.g. +0x1b8). |
| ResourceLoader_FindTokenFast | 1:0x10e7f6c | Match of 2.3.1 0x21b80f4: same 85 instructions (similarity 0.98), the inlined hash-map probe (key fold, udiv/msub bucket, -1 empty marker) is scheduled earlier; all mapped callees present. |
| ObjectPackageExtractor_Initialize | 1:0x1075718 | Match of 2.3.1 0x2150648 (similarity 0.95): copies the same 0x58-byte header into this+0x10..0x60 and +0xa0, then resizes/copies the u32 array at +0x68/+0x70/+0x74 identically; instruction scheduling differs only. |
| CClass_AssignDefaultValuesToProperties | 1:0x10c3934 | Match of 2.3.1 0x2197d40: recurses into the parent class (calls itself on this+0x10), reads default-value count +0x164, flags +0x178, names +0x158, values +0x168 exactly as 2.3.1; the in-place sort of defaults is compiled differently and the property lookup is a call to 0x10c4c14 (CClass_GetProperty candidate) instead of inline. |
| JournalTree_ProcessJournalIndex | 1:0x52360b4 | Match of 2.3.1 0x101ea3410: shares its only string ('journal'), all 9 mapped callees present, similarity 0.92 (0x7c4/0x7e0 bytes). |
| MorphTargetMesh_PostLoad | 1:0x51ca1a4 | Match of 2.3.1 0x100e467bc: shares its only string ('Normal'), all 12 mapped callees present; body grew/shrank (0x5e8/0x520) with the SDK-documented skinned/morph mesh layout changes. |
| MeshMaterialBuffer_LoadMaterialAsync | 1:0x51ad0e8 | Match of 2.3.1 0x100e27a4c: only 2.5 function with the format string 'localmaterial_%08X[%08X].res' used by 2.3.1; all 6 mapped callees present; similarity 0.94. |
| CClass_GetProperties | 1:0x10c352c | Match of 2.3.1 0x102197928: all mapped callees (3/3) and callers (3/3) agree; same CClass property-array walk. |
| JournalRootFolderEntry_Initialize | 1:0x5207e14 | Match of 2.3.1 0x101e7528c: shares 'descriptor.journaldesc'; mapped callees 3/3 and caller 1/1 agree. |
| CClass_GetProperty | 1:0x10c4c14 | Match of 2.3.1 0x102198b40: mapped callers 7/7 and callees 2/2 agree; also called from the verified 2.5 CClass_AssignDefaultValuesToProperties where 2.3.1 inlined the lookup. |
| ResourceDepot_LoadArchives | 1:0x151b18c | Match of 2.3.1 0x103eda488: the per-scope consume step: called right after the '*.archive' glob 0x151b064 (only 2.5 function referencing that string) as in 2.3.1; mapped callers 2/2, callees 4/4. |
| ResourceSerializer_Load | 1:0x1207588 | Match of 2.3.1 0x102255b4c: all 6 mapped callees present; similarity 0.84 over 0x920/0x94c bytes. |
| ResourceDepot_InitializeArchives | 1:0x151a118 | Match of 2.3.1 0x103ed96b0: shares both strings ('content', 'patch'); mapped callees 5/5; calls the 2.5 LoadArchives 0x151b18c and the glob. |
| GameApplication_InitResourceDepot | 1:0x442d290 | Match of 2.3.1 0x101704194: same branch on [x1+0xc0]==2 and path from x1+0xc8 as 2.3.1, allocates the depot (0xf0 in 2.5, 0x80 in 2.3.1), stores it at app+0x198 and publishes it through the ResourceDepot setter 0x10f123c (verified ResourceDepot global). |
| CBaseEngine_InitEngine | 1:0x4d7efb8 | Match of 2.3.1 0x103d9e494: shares 16 of its 17 strings (AsyncMaterialCompiler, ContentManager, DebugFilters, DlcManifests, ...); mapped callees 8/8. |
| ResourceToken_OnLoaded | 1:0x10ef37c | Match of 2.3.1 0x1021c4e84: caller/callee counts 45/17 vs 2.3.1 44/19, mapped callee 1/1; its 2.3.1 profiler label 'ResourceToken/OnLoaded' now lives in helper 0x10f0230, whose only caller is this function. |
| DeferredDataBuffer_LoadAsync | 1:0x1215524 | Match of 2.3.1 0x10226258c: caller/callee counts 11/14 vs 2.3.1 10/16, mapped callee 1/1; its 2.3.1 label 'DeferredDataBuffer/CreateLoadingJobs' now lives in helper 0x1216198, whose only caller is this function. |
| ResourceLoader | 3:0x1cac690 | ResourceLoader* global 0x1099dc690 (__DATA). Written by 0x442d4ac, which builds the loader from the depot (app+0x198) through 0x10e9b30 (2.3.1 0x21b9b58), stores it at app+0x1a0 and then 'str x8,[0x1099dc690]' - the 2.3.1 sequence (0x17043a4 -> app+0x1a0 -> 0x10900b580). All 135 callers of the verified IssueLoadingRequestByPath load x0 from this global (as 2.3.1 callers load 0x10900b580). |
| CGameEngine | 3:0x1fde288 | CBaseEngine*/CGameEngine* global 0x109d0e288 (__DATA). The engine creator 0x4d6b34c (exact function-candidates match of 2.3.1 creator 0x3d8bfc4, which also references 'Started initializing engine' / 'Initialize Engine') calls CClass_CreateInstance 0x10c36a0 (verified) and immediately 'str x0,[0x109d0e288]' - the 2.3.1 'CreateInstance -> str x0,[0x1090dc3a0]' sequence. NOTE: engine layout changed again in 2.5 (e.g. CBaseEngine_LoadGatheredResources reads +0x100 where 2.3.1 read +0xd8). |
| AppearanceDefinition_ExtractPartComponents | 1:0x17d3158 | bigmatch of 2.3.1 0x100acca30: register-masked instruction similarity 0.8902 (runner-up margin 0.22), mapped callees 7/7, mapped callers 1/1, size 0x40c/0x410. |
| ResourceDepot_CheckResource | 1:0x151aa2c | bigmatch of 2.3.1 0x103ed9e9c: register-masked instruction similarity 1.0 (runner-up margin 0.2), mapped callees 2/2, mapped callers 0/0, size 0x50/0x50. |
| AppearanceChangeSystem_ChangeAppearance2 | 1:0x1a5c6e0 | bigmatch of 2.3.1 0x100d45fd0: register-masked instruction similarity 0.8692 (runner-up margin 0.103), mapped callees 4/5, mapped callers 2/2, size 0x568/0x558. |
| PersistencySystem_SetPersistentStateData | 1:0x2d0d090 | bigmatch of 2.3.1 0x103febc30: register-masked instruction similarity 0.9057 (runner-up margin 0.513), mapped callees 14/14, mapped callers 1/1, size 0x490/0x48c. |
| AppearanceChangeSystem_ChangeAppearance1 | 1:0x1a59e30 | bigmatch of 2.3.1 0x100d43218: register-masked instruction similarity 0.817 (runner-up margin 0.285), mapped callees 4/5, mapped callers 2/2, size 0x6dc/0x660. |
| Localization_LoadLipsyncs | 1:0x3f2bd00 | bigmatch of 2.3.1 0x101af9ccc: register-masked instruction similarity 0.9431 (runner-up margin 0.327), mapped callees 2/2, mapped callers 1/1, size 0xf8/0xf4. |
| Entity_Reassemble | 1:0x19a7a30 | bigmatch of 2.3.1 0x100c95744: register-masked instruction similarity 0.7899 (runner-up margin 0.194), mapped callees 3/3, mapped callers 0/0, size 0x6e0/0x610. |
| GarmentAssembler_ProcessMorphedMesh | 1:0x17ebc80 | bigmatch of 2.3.1 0x100ae3cc0: register-masked instruction similarity 0.8298 (runner-up margin 0.316), mapped callees 8/9, mapped callers 0/0, size 0x344/0x370. |
| CharacterCustomizationHairstyleController_CheckState | 1:0x4bb0ac0 | bigmatch of 2.3.1 0x102442d8c: register-masked instruction similarity 0.8905 (runner-up margin 0.35), mapped callees 5/6, mapped callers 0/0, size 0x468/0x470. |
| CharacterCustomizationSystem_Initialize | 1:0x4bc779c | bigmatch of 2.3.1 0x1024593dc: register-masked instruction similarity 0.8724 (runner-up margin 0.312), mapped callees 6/6, mapped callers 0/0, size 0x334/0x388. |
| ResourceDepot_RequestResource | 1:0x151a748 | bigmatch of 2.3.1 0x103ed9b94: register-masked instruction similarity 0.8338 (runner-up margin 0.308), mapped callees 3/4, mapped callers 0/0, size 0x308/0x2e4. |
| EntitySpawner_SpawnFromTemplate | 1:0x46e03b8 | bigmatch of 2.3.1 0x1014084b0: register-masked instruction similarity 0.9402 (runner-up margin 0.307), mapped callees 5/5, mapped callers 0/0, size 0x3b0/0x3a0. |
| ItemFactoryRequest_LoadAppearance | 1:0x20133f4 | bigmatch of 2.3.1 0x1037034a4: register-masked instruction similarity 0.8785 (runner-up margin 0.383), mapped callees 9/10, mapped callers 1/1, size 0x3bc/0x3fc. |
| EntityTemplate_OnLoad | 1:0x19c6380 | bigmatch of 2.3.1 0x100cb2a6c: register-masked instruction similarity 0.8134 (runner-up margin 0.235), mapped callees 11/12, mapped callers 0/0, size 0x3b8/0x43c. |
| AppearanceChanger_SelectAppearanceName | 1:0x2043b04 | bigmatch of 2.3.1 0x103732c68: register-masked instruction similarity 0.8994 (runner-up margin 0.374), mapped callees 7/8, mapped callers 0/0, size 0x2ac/0x24c. |
| ResourceSerializer_Deserialize | 1:0x120aff4 | bigmatch of 2.3.1 0x10225921c: register-masked instruction similarity 0.8668 (runner-up margin 0.528), mapped callees 5/5, mapped callers 1/1, size 0x66c/0x67c. |
| ItemFactoryAppearanceChangeRequest_LoadAppearance | 1:0x200cd00 | bigmatch of 2.3.1 0x1036fce14: register-masked instruction similarity 0.8038 (runner-up margin 0.287), mapped callees 5/5, mapped callers 0/0, size 0x24c/0x2a4. |
| TPPRepresentationComponent_OnItemEquipped | 1:0x1ea5b54 | bigmatch of 2.3.1 0x1035a6174: register-masked instruction similarity 0.8634 (runner-up margin 0.342), mapped callees 4/4, mapped callers 0/0, size 0x3b8/0x42c. |
| StreamingSector_PostLoad | 1:0x1c05360 | bigmatch of 2.3.1 0x103426204: register-masked instruction similarity 0.8011 (runner-up margin 0.123), mapped callees 7/7, mapped callers 0/0, size 0x2bc/0x2c4. |
| ResourcePath_Create | 1:0x106c8c4 | bigmatch of 2.3.1 0x1021c90a4: register-masked instruction similarity 0.9109 (runner-up margin 0.311), mapped callees 0/0, mapped callers 68/72, size 0x17c/0x1ac. |
| TPPRepresentationComponent_OnAttach | 1:0x1e9ff18 | bigmatch of 2.3.1 0x1035a0604: register-masked instruction similarity 0.9542 (runner-up margin 0.35), mapped callees 10/10, mapped callers 0/0, size 0x724/0x734. |
| Localization_LoadVoiceOvers | 1:0x3fe6c94 | bigmatch of 2.3.1 0x101405334: register-masked instruction similarity 0.7907 (runner-up margin 0.331), mapped callees 9/9, mapped callers 0/0, size 0x7cc/0x7bc. |
| CharacterCustomizationHairstyleController_OnDetach | 1:0x4baf2c0 | bigmatch of 2.3.1 0x1024415c0: register-masked instruction similarity 0.9705 (runner-up margin 0.189), mapped callees 4/4, mapped callers 0/0, size 0x264/0x260. |
| TPPRepresentationComponent_OnItemUnequipped | 1:0x1ea50bc | bigmatch of 2.3.1 0x1035a56dc: register-masked instruction similarity 0.9626 (runner-up margin 0.521), mapped callees 1/1, mapped callers 1/1, size 0x178/0x174. |
| AttachmentSlots_IsSlotEmpty | 1:0x1e0d97c | Pair split vs 0x1e0dadc: both match 2.3.1 IsSlotEmpty 0x1035104e4 / IsSlotSpawning 0x103510644 (seq ~0.92, mapped callees 2/2). Only IsSlotEmpty ends 'bl 0x...; eor w0,w0,#1' (size 0xc4 vs 0xc0); 2.5 0x1e0d97c has that eor and size 0xc4. |
| AttachmentSlots_IsSlotSpawning | 1:0x1e0dadc | Pair split vs 0x1e0d97c: same body as IsSlotEmpty without the final 'eor w0,w0,#1', size 0xc0 as 2.3.1 0x103510644; mapped callees 2/2. |
| CharacterCustomizationState_GetHeadAppearances1 | 1:0x4bbf004 | One of six consecutive 0x2c0 getters (2.3.1: six consecutive 0x218 at 0x102451620..0x102452144, same order). Reads its array at state+0x70/+0x7c exactly like 2.3.1 0x102451620 (Head 0x70, Body 0x80, Arms 0x90); callees map 4/4 (0x1000015c8, 0x10101b880, 0x100028674 x2). Variants 1/2 are byte-identical bodies and ArchiveXL hooks both with the same handler; 1 = lower address as in 2.3.1. |
| CharacterCustomizationState_GetHeadAppearances2 | 1:0x4bbf438 | One of six consecutive 0x2c0 getters (2.3.1: six consecutive 0x218 at 0x102451620..0x102452144, same order). Reads its array at state+0x70/+0x7c exactly like 2.3.1 0x1024518e4 (Head 0x70, Body 0x80, Arms 0x90); callees map 4/4 (0x1000015c8, 0x10101b880, 0x100028674 x2). Variants 1/2 are byte-identical bodies and ArchiveXL hooks both with the same handler; 1 = lower address as in 2.3.1. |
| CharacterCustomizationState_GetBodyAppearances1 | 1:0x4bbf6f8 | One of six consecutive 0x2c0 getters (2.3.1: six consecutive 0x218 at 0x102451620..0x102452144, same order). Reads its array at state+0x80/+0x8c exactly like 2.3.1 0x102451afc (Head 0x70, Body 0x80, Arms 0x90); callees map 4/4 (0x1000015c8, 0x10101b880, 0x100028674 x2). Variants 1/2 are byte-identical bodies and ArchiveXL hooks both with the same handler; 1 = lower address as in 2.3.1. |
| CharacterCustomizationState_GetBodyAppearances2 | 1:0x4bbf9b8 | One of six consecutive 0x2c0 getters (2.3.1: six consecutive 0x218 at 0x102451620..0x102452144, same order). Reads its array at state+0x80/+0x8c exactly like 2.3.1 0x102451d14 (Head 0x70, Body 0x80, Arms 0x90); callees map 4/4 (0x1000015c8, 0x10101b880, 0x100028674 x2). Variants 1/2 are byte-identical bodies and ArchiveXL hooks both with the same handler; 1 = lower address as in 2.3.1. |
| CharacterCustomizationState_GetArmsAppearances1 | 1:0x4bbfc78 | One of six consecutive 0x2c0 getters (2.3.1: six consecutive 0x218 at 0x102451620..0x102452144, same order). Reads its array at state+0x90/+0x9c exactly like 2.3.1 0x102451f2c (Head 0x70, Body 0x80, Arms 0x90); callees map 4/4 (0x1000015c8, 0x10101b880, 0x100028674 x2). Variants 1/2 are byte-identical bodies and ArchiveXL hooks both with the same handler; 1 = lower address as in 2.3.1. |
| CharacterCustomizationState_GetArmsAppearances2 | 1:0x4bbff38 | One of six consecutive 0x2c0 getters (2.3.1: six consecutive 0x218 at 0x102451620..0x102452144, same order). Reads its array at state+0x90/+0x9c exactly like 2.3.1 0x102452144 (Head 0x70, Body 0x80, Arms 0x90); callees map 4/4 (0x1000015c8, 0x10101b880, 0x100028674 x2). Variants 1/2 are byte-identical bodies and ArchiveXL hooks both with the same handler; 1 = lower address as in 2.3.1. |
| GarmentAssemblerState_AddItem | 1:0x2007dcc | Pair split vs 0x2007ebc (both ~0.94 similar to 2.3.1 AddItem 0x1036f8208 / AddCustomItem 0x1036f82f4). 2.3.1 AddItem's distinct callee maps to 2.5 0x10200938c, which only 0x2007dcc calls. |
| GarmentAssemblerState_AddCustomItem | 1:0x2007ebc | Pair split vs 0x2007dcc: calls 0x102009678 (the 2.5 match of 2.3.1 AddCustomItem's distinct callee) plus the extra add, as 2.3.1 0x1036f82f4. |
| Localization_LoadSubtitles | 1:0x1c2b4c8 | Pair split vs 0x1c2b6dc: identical 0x1f4 bodies (2.3.1 pair 0x102f67be0/0x102f67db4 identical 0x1b4, mapped callees 5/5) differing only in the global they load; 2.3.1 Subtitles reads 0x1090d1500 and OnScreens +0x20; 2.5 0x1c2b4c8 reads 0x109cc58c8 and 0x1c2b6dc +0x20, same order. |
| Localization_LoadOnScreens | 1:0x1c2b6dc | Pair split vs 0x1c2b4c8: reads the global +0x20 after LoadSubtitles' (0x109cc58e8), as 2.3.1 0x102f67db4 reads 0x1090d1520. |
| InkWidgetLibrary_AsyncSpawnFromLocal | 1:0x4353ba8 | Four consecutive InkWidgetLibrary spawners in the same order as 2.3.1 (0x104965980/b38/de0/ec0 -> 0x4353ba8/d74/0x435413c/24c). This one: small frame, 'mov #1' async flag, 0xf8 vs 0xfc, seq 0.91. |
| InkWidgetLibrary_AsyncSpawnFromExternal | 1:0x4353d74 | Group of four (see AsyncSpawnFromLocal): sub-sp frame, external-path calls 0x101025e4c/0x104353edc and the 'mov #1' async flag, like 2.3.1 0x104965b38; 0x168 vs 0x158. |
| InkWidgetLibrary_SpawnFromLocal | 1:0x435413c | Group of four: small frame, no async flag, like 2.3.1 0x104965de0 (0xdc vs 0xe0, seq 0.88). |
| InkWidgetLibrary_SpawnFromExternal | 1:0x435424c | Group of four: sub-sp frame, external-path calls 0x101025e4c/0x104353edc, no async flag, like 2.3.1 0x104965ec0 (0x174 vs 0x144). |
| GarmentAssembler_OnGameDetach | 1:0x2006b0c | Garment block layout preserved 1:1 (2.3.1 0x1036f6f4c..0x1036f8664 vs 2.5 0x102006b0c..0x102008230, 11 interior functions already matched in order). First function of the block, 0x54 bytes like 2.3.1 0x1036f6f4c (seq 1.0); the identical-bodied 0x10431b458 lies outside the block. |
| GarmentAssembler_FindState | 1:0x2007bc4 | In the preserved garment block between matched 0x102007780 and AddItem 0x102007dcc, as 2.3.1 0x1036f8030; seq 0.94, mapped callees 2/2, mapped callers 3/3; also the sret callee (add x8,sp,#8; bl) of the 2.5 Remove/Change wrappers. |
| GarmentAssembler_RemoveItem | 1:0x2008310 | 2.5 outlines the body: 0x38 wrapper 'mov x19,x2; add x8,sp,#8; bl FindState 0x102007bc4; bl worker 0x102008348' (2.3.1 0x1036f8744 inlined the work after the same FindState sret call). Only common callee of both 2.5 matches of 2.3.1's two RemoveItem callers (0x102010854, 0x102014c3c). Same (assembler, entity&, request& in x2) entry. |
| GarmentAssembler_ChangeItem | 1:0x200857c | 0x38 wrapper (FindState sret then worker 0x1020085b4), called by 0x10200dd68 = match of 2.3.1's ChangeItem/ChangeCustomItem caller 0x1036fddd4. Worker calls 0x102008dcc and 0x10200938c, the 2.5 matches of 2.3.1 ChangeItem's callees 0x1036f8d24 and 0x1036f9358. |
| GarmentAssembler_ChangeCustomItem | 1:0x200882c | 0x38 wrapper (FindState sret then worker 0x102008864), also called by 0x10200dd68. Worker calls 0x102008dcc and 0x102009678, the 2.5 matches of 2.3.1 ChangeCustomItem's callees 0x1036f8d24 and 0x1036f96ac. |
| AppearanceResource_FindAppearanceDefinition | 1:0x17dd6c0 | Two independent picks agree. Layout: 2.3.1 0x100ad7048 is function 0/1 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x1017dd6c0 at that index. Bigmatch top for 2.3.1 0x100ad7048: similarity 0.6416 (margin 0.316), mapped callees 2/2, mapped callers 4/5, size 0x1b0/0x2e4. |
| QuestLoader_ProcessPhaseResource | 1:0x29e8c48 | Two independent picks agree. Layout: 2.3.1 0x102ed6090 is function 8/9 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x1029e8c48 at that index. Bigmatch top for 2.3.1 0x102ed6090: similarity 0.8969 (margin 0.079), mapped callees 2/2, mapped callers 0/0, size 0x184/0x184. |
| QuestRootInstance_Start | 1:0x2812f24 | Two independent picks agree. Layout: 2.3.1 0x102d01878 is function 12/15 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x102812f24 at that index. Bigmatch top for 2.3.1 0x102d01878: similarity 0.753 (margin 0.026), mapped callees 3/3, mapped callers 0/0, size 0x24c/0x2e4. |
| ItemFactoryAppearanceChangeRequest_LoadTemplate | 1:0x200c940 | Two independent picks agree. Layout: 2.3.1 0x1036fca88 is function 0/2 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x10200c940 at that index. Bigmatch top for 2.3.1 0x1036fca88: similarity 0.8815 (margin 0.278), mapped callees 2/2, mapped callers 0/0, size 0x2c0/0x2ec. |
| ObjectPackageExtractor_ExtractSync | 1:0x1075dbc | Two independent picks agree. Layout: 2.3.1 0x102150ce4 is function 2/3 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x101075dbc at that index. Bigmatch top for 2.3.1 0x102150ce4: similarity 0.5143 (margin 0.094), mapped callees 5/5, mapped callers 1/2, size 0x160/0x15c. |
| CMesh_PostLoad | 1:0x519a58c | Two independent picks agree. Layout: 2.3.1 0x100e16b28 is function 0/1 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x10519a58c at that index. Bigmatch top for 2.3.1 0x100e16b28: similarity 0.7129 (margin 0.281), mapped callees 4/5, mapped callers 0/0, size 0x2c0/0x234. |
| AIWorkspotManager_RegisterSpots | 1:0x267f100 | Two independent picks agree. Layout: 2.3.1 0x103d515d0 is function 0/1 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x10267f100 at that index. Bigmatch top for 2.3.1 0x103d515d0: similarity 0.9392 (margin 0.329), mapped callees 1/1, mapped callers 0/0, size 0x16c/0x168. |
| CMesh_FindAppearance | 1:0x519c4b0 | Two independent picks agree. Layout: 2.3.1 0x100e189f4 is function 2/4 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x10519c4b0 at that index. Bigmatch top for 2.3.1 0x100e189f4: similarity 0.5667 (margin 0.075), mapped callees 0/1, mapped callers 1/1, size 0x140/0x190. |
| CharacterCustomizationFeetController_CheckState | 1:0x4bb13fc | Two independent picks agree. Layout: 2.3.1 0x1024436d0 is function 3/15 between its nearest matched neighbours, and the 2.5 window between their matches has the same count with 0x104bb13fc at that index. Bigmatch top for 2.3.1 0x1024436d0: similarity 0.9586 (margin 0.012), mapped callees 2/2, mapped callers 0/0, size 0x154/0x150. |
| GarmentAssembler_ProcessGarment | 1:0x17ebff0 | Same wrapper as 2.3.1 0x100ae4004: (x0 def handle&, x1, x2 params, sret x8) -> local ctor(sp+8,0,0) 0x104ddf080, bl core 0x1017ec094 with x8 preserved, dtor 0x104ddf510; trailing 'b core' stub 0x1017ec090 like 2.3.1 0x100ae406c. Core 0x1017ec094 is the 2.5 match of 2.3.1 core 0x100ae4070 (mapped callees 16/16; earlier propagation pick 0x1017ef024 had 0/16 and was corrected). Only caller of the core; callers include 0x102010854 = match of 2.3.1 caller 0x103700904. New in 2.5: if global 0x109cc59a0 is set it tail-calls that object's vtable+0xf0 with (obj, x0, x1, x2). |
| GarmentAssembler_ProcessSkinnedMesh | 1:0x17eb4ac | Layout: first function after matched 0x1017eb3a0 followed by two 0xf0 helpers, as 2.3.1 0x100ae3840 (0x2a0 then two 0xf0). Mapped callees 7/8. Its callers 0x1017eda2c/0x1017eee74 are in the 2.5 garment block like 2.3.1's (0x100ae5c9c/0x100ae72ec); the near-twin 0x1017eb9ac (also 7/8) is called only from unrelated code (0x101cca200, 0x101ccc664, 0x101d18ea8). |
| MeshAppearance_LoadMaterialSetupAsync | 1:0x51a3c24 | Layout window (after matched neighbour, 0xc0/0xc8-sized helper first, then this, then the 8-byte stub, as 2.3.1); size 0x548 vs 0x5a8; mapped callees 9/11 and callers 7/17 (the next-best candidate 0x1020d2c24: 5/11 and 0/17). |
| MappinSystem_OnStreamingWorldLoaded | 1:0x30fb9f0 | Layout: 5th function in the window between matched neighbours (2.3.1 0x1042a71a4 is 5th, 0x4d0; 2.5 0x494); best exact-text similarity in the window (0.46, others <=0.14); mapped callees 3/3. |
| AttachmentSlots_InitializeSlots | 1:0x1e05ea4 | Layout: 3rd function in the AttachmentSlots window as 2.3.1 0x103508db0 (preceded by the two 0x74 functions in both builds), exact instruction text identical (1.00, 0x48), mapped callees 1/1. The identical-bodied 0x102562198 lies outside the block. |
| TPPRepresentationComponent_UnregisterAffectedItem | 1:0x1ea5f80 | Only function between matched neighbours 0x101ea5b54 (OnItemEquipped, verified) and 0x101ea619c, as 2.3.1 0x1035a6654 follows OnItemEquipped; similarity 0.93, size 0x21c vs 0x220. |
| FactoryIndex_LoadFactoryAsync | 1:0x19d50c8 | Layout window: 2.3.1 0x74, 0x2a0, 0xf0, 0xf0, [this 0x1fc], 0x2e0 vs 2.5 0x74, 0x2a8, 0x100, [0x19c], 0x2b8 (two 0xf0 helpers merged); best exact-text similarity 0.51 (others <=0.15); mapped callees 2/2. |
| EntityBuilder_ScheduleExtractComponentsJob | 1:0x19b3e70 | 2.3.1 0x100ca08a0 is the only function referencing 'EntityBuilder_ExtractComponents'; in 2.5 the only reference is the outlined one-time name initialiser 0x1019b882c, whose only caller is 0x1019b3e70. Also best instruction-shape match among the three 3/3-callee candidates (0.77 vs 0.61/0.60), mapped callees 3/3. |
| GarmentAssembler_ExtractComponentsJob | 1:0x17ee098 | 2.3.1 0x100ae6348 is the only function referencing 'ExtractGarmentComponent'; in 2.5 the only reference is the outlined one-time name initialiser 0x1017ef13c, whose only caller is 0x1017ee098. Mapped callees 7/7, instruction shape 0.80 (vs 0.28 for 0x1017ee34c), size 0x2b4 vs 0x318. |
| CMesh_LoadMaterialsAsync | 1:0x519c6dc | Two independent picks agree: layout (only function between 2.3.1 0x100e18c28's matched neighbours' 2.5 matches) and call voting (calls 10 of the 14 2.5 matches of 2.3.1's mapped neighbours, next best 9). Grew 0x8a8 -> 0x1468 (inlined helpers). |
| TPPRepresentationComponent_RegisterAffectedItem | 1:0x1ea0738 | Layout (index 1/2 between matched neighbours as 2.3.1 0x1035a0e10) and call voting (3/3 mapped neighbours, next best 2) agree; exact text 0.69. |
| AppearanceResource_PostLoad | 1:0x17df10c | Layout (index 2/3 between matched neighbours as 2.3.1 0x100ad88a8) and call voting (4/4 mapped callees) agree; size 0x178 vs 0x17c (the other 4-vote function 0x103ea26a4 is 0x9b0). |
| AppearanceChanger_RegisterPart | 1:0x19e7bc0 | Layout candidate (index 1/6 between matched neighbours as 2.3.1 0x100cd1f50) with exact text 0.64 (others <=0.16); call voting 2/2 (tied only with unrelated 0x1017df054, exact 0.07). |
| AppearanceChanger_GetSuffixes | 1:0x201dcc8 | Call voting: calls/called-by 16 of 24 2.5 matches of 2.3.1 0x10370d924's mapped neighbours (next 10); bigmatch top (0.57); shares 3 strings with 2.3.1. Shrunk 0xb94 -> 0x568 (part split out to 0x10201e330). |
| AppearanceChanger_ComputePlayerGarment | 1:0x2020a80 | Call voting: 7 of 8 mapped neighbours (next 5); mapped callees 6/7 and the single mapped caller 1/1; size 0x268 vs 0x2f8. |
| EntityTemplate_FindAppearance | 1:0x19c4808 | Mapped callee 1/1 and mapped callers 2/3 (the higher-similarity 0x1019c5e2c: callers 0/3); call voting 3 (next 2). |
| CharacterCustomizationSystem_InitializeAppOption | 1:0x4bd3f5c | One of three Initialize*Option functions kept in 2.3.1 order (App < Morph < Switcher); bigmatch top; mapped callees 2/2, caller 1/1; the distinguishing [x,#0xa0] load of 2.3.1 0x102464ed4 is present. |
| CharacterCustomizationSystem_InitializeMorphOption | 1:0x4bd448c | Three Initialize*Option functions in 2.3.1 order; bigmatch top; mapped callees 2/2, caller 1/1; distinguishing [x,#0xc0]/[x,#0xcc] loads of 2.3.1 0x1024652d8 present. |
| CharacterCustomizationSystem_InitializeSwitcherOption | 1:0x4bd53ac | Three Initialize*Option functions in 2.3.1 order; bigmatch top; mapped callees 2/2, caller 1/1; distinguishing 'cmn #1' check and 0xa0 stack slots of 2.3.1 0x1024661a0 present. |
| CMesh_ShouldPreloadAppearances | 1:0x519ada4 | Called by verified MeshAppearance_LoadMaterialSetupAsync 0x1051a3c24 as 2.3.1 0x100e173fc is called by 0x100e1f7b4; best exact-text match among that caller's callees (0.49, next 0.09); size 0x118 vs 0x108; it was also the bigmatch runner-up. |
| CMesh_AddStubAppearance | 1:0x519a844 | Called by verified CMesh_PostLoad 0x10519a58c as 2.3.1 0x100e16e68 is called by 0x100e16b28; best exact-text match among its callees (0.61, next 0.23); size 0x114 vs 0x11c; also the layout candidate (index 0/2 between matched neighbours). |
| ResourceSerializer_OnDependenciesReady | 1:0x12090b0 | 2.3.1 0x1022575c0 is only referenced as a function pointer (adrp+add) from 0x102257548; that function's 2.5 match 0x101209038 references 0x1012090b0, the best exact-text match among its refs (0.28, next 0.09); also the layout candidate (index 0/3); size 0x230 vs 0x270. |
| BufferReader_MakeType0 | 1:0x121743c | Called by 0x10120bb80 = 2.5 match of 2.3.1's only caller 0x102259da4; best exact-text match among its callees (0.76, next 0.36); size 0x70 vs 0x6c; also the layout candidate (index 0/1). |
| ResourceToken_CancelUnk38 | 1:0x4153d5c | Same body as 2.3.1 0x10418fb00 (0x60 both, shape 0.83): 'mov w0,#8; bl ...; tbnz' guard, compare [tok+X]!=-1, test/set byte flag, then tail-call [[tok+0x18]]+0x30 - with the token fields shifted +0x20 (0x48->0x68, flag 0x56->0x76: LAYOUT CHANGE for ResourceToken). Its two callers pair with 2.3.1's two (0x10095eef4->0x101216088 shape 0.95, 0x1021c4a14->0x1010eef3c 0.87). Bigmatch top (margin 0.35). |
| InkSpawner_FinishAsyncSpawn | 1:0x4368f34 | 2.3.1 0x1049799b8 is slot +0x30 of the callback vtable 0x107380e60 installed by the async spawn job 0x104979398. 2.5's match of that job, 0x104368b58 (0x3dc vs 0x3e8, layout and bigmatch top), installs a 4-entry functor table 0x107e81078 whose invoke entry is 0x104368f34 (also the best text match in the layout window). Same body: writes request+0x90 instance, +0x60 rootWidget, +0x70 gameController, then swpalb #3 to request+0x1ac. LAYOUT CHANGE: arg0 is now a vtable-less capture {WeakHandle +0, SharedPtr<InkSpawningRequest> +0x10} (2.3.1: vtable object with the request SharedPtr at +0x18). |
| MappinResource_GetMappinData | 1:0x30f2ff0 | 2.3.1 0x10429eed0 is called only by MappinSystem virtual 0x1042b2748 ('ldr x0,[x0,#0x58]; mov x19,x2; bl lookup; copy [r+4]/[r+0xc] to x19'). Rare-anchor vtable slot voting (11/11) gives 2.5 0x103106d24, an instruction-identical body that calls the new wrapper 0x1030f7934; that wrapper loads its +0x58 and calls 0x1030f2ff0 with the hash still in w1 (falls back via [obj+0x40] on miss). 0x1030f2ff0 is the 2.3.1 lookup unrolled x4: array +0x40, count +0x4c, stride 0x20, compare u32 [e]==w1, returns entry or null. Same (resource, u32) contract as the macOS ArchiveXL hook. |
| PointOfInterestMappinResource_GetMappinData | 1:0x31138f4 | Instruction-identical twin (1.00) of the verified 2.5 MappinResource lookup 0x1030f2ff0, as 2.3.1's POI lookup 0x1042be574 is identical to 0x10429eed0; lies in the POI layout window (0x1031128ec..0x103113bf8); its only caller 0x1030f7c54 is the POI wrapper next to the Mappin wrapper 0x1030f7934. |

## Layouts

Base function constructor 0x1097de8 initializes sets at +0xb0 and +0xc8,
redirect pointer +0xe8. Derived constructor stores prove CBaseFunction size
0xf0, CGlobalFunction index +0xf0 (size 0xf8), CClassFunction parent +0xf0 and
index +0xf8 (size 0x100). Script frame size 0xa8 and root map lifecycle were
already verified in the working NightCity 2.5 port. Compile with
RED4EXT_GAME_VERSION_25; without it the SDK retains 2.3.1 layouts.

RTTI AddPostRegisterCallback is vtable +0xc8 -> 0x10bc858. It uses element
stride 0x48 and handler pointer +0x40, establishing callback inline capacity
64. The RTTI vtable has four new entries at +0x138..+0x150 before destructor
at +0x158. These agree with the upstream 2.5 SDK changes but were checked
in the macOS image independently.

Candidate matching output in the surrounding work folder is NOT verification.
For example the strongest fuzzy CGlobalFunction_ctor candidate was actually
CBaseFunction's ctor; only examining its callers identified the derived ctor.

## Native boot and plugin preflight

The isolated test-game boot reached Running on 2026-10-22 and passed the
loader checks for the default memory pool, RTTI lookup, CName, CString, and
native function registration. The harness stopped it after 45 seconds; this
is not a clean-shutdown test. Its RTTI dump is data/rtti_layout_macos_25.json.

Both XL plugins compile with RED4EXT_GAME_VERSION_25. They are not ready to
load: preflight with the complete hash catalog reports 45 unresolved hashes
for TweakXL and 155 for ArchiveXL. A profile containing only the 16 verified
bootstrap entries incorrectly passed the scanner, which intersects compiled
constants with the profile catalog. Retaining unresolved hashes prevents
that false result for known SDK/plugin hashes. This scanner is not proof
that an arbitrary third-party plugin has no additional requirements.

## TweakDB singleton

TweakDB_Get (hash 914361828) is function start 0x37cc900, signature
TweakDB*(). It acquire-loads singleton 0x108c9c640 and calls initializer
0x37cc7d4 when null. That initializer allocates exactly 0x168 bytes at
0x37cc844, calls constructor 0x37cca34, and publishes with swpal. The
constructor independently matches the documented TweakDB structure: locks
+0x20/+0x21, allocations 0x158 and 0xf8 at +0x28/+0x30, flag +0x38,
flat array +0x40, maps +0x58/+0x88 (sentinels +0x78/+0xa8, allocators
+0x80/+0xb0), query arrays +0xb8/+0xc8, group arrays +0xe0/+0xf0,
default map +0x108, buffer fields +0x148/+0x150/+0x158 and byte generation
+0x160. No arguments are consumed by the getter; x0 is the returned
singleton. This establishes identity and pointer-return ABI separately
from the initial function-similarity candidate. Other TweakDB operations
and their layout-dependent usage remain under audit.

## Additional layout checks

CRTTISystem constructor 0x10b3384 initializes mutexes at +0x1e0/+0x228
with the byte lock at +0x220, then two arrays at +0x268/+0x278 (sizes
zeroed at +0x274/+0x284) and mutexes at +0x288/+0x2c8. The 2.5 SDK
therefore declares the additional tail and asserts size 0x308. This fixes
a previously suppressed Windows-size assertion; default macOS field sizes
give 0x268, whereas Windows remains 0x238.

The 2.5 RTTI dump places entSkinnedMeshComponent mesh/chunkMask/version
at 0x228/0x248/0x268 and entMorphTargetSkinnedMeshComponent
morphResource/meshAppearance/tags/version at 0x200/0x238/0x308/0x361.
These match the existing Windows field block, while 2.3.1 keeps its
8-byte-earlier macOS block. ISkinTargetComponent gained the reflected
boundingBoxLocalBoundsOverride field at 0x180; skinning moved to 0x188.
Native handle/proxy fields are not reflected and still require a separate
usage audit before ArchiveXL can run.

## ISerializable / IScriptable and weak references

ISerializable ctor 0x10b0608 installs vtable 0x107830ca0, clears +8..+0x27,
and assigns its generated ID at +0x28. IScriptable ctors 0x11e48ec and
0x11e4910 call it, install vtable 0x1078378c0, and clear nativeType/holder
at +0x30/+0x38. These establish both vtable identities. Chained pointer
slots read from the image agree between the two tables:

| Hash/name | Function offset | macOS slot / ABI evidence |
| --- | --- | --- |
| 114701897 ISerializable_sub_30 | 0x10b09a4 | +0x38; this x0, resolves owner via 0x10b09d0 then owner virtual +0xe0, default bool true w0. |
| 2293436409 ISerializable_sub_40 | 0x10b0860 | +0x48; this x0, stream x1; tests byte stream+8 bit1, forwards to class serialization with object and stream; bool return. |
| 2715367827 ISerializable_sub_78 | 0x10b0c08 | +0x80; this x0 and arguments x1..x4, saves hidden result x8 in x19; writes the 16-byte handle at +0x10b0d54, zero result on failure. |
| 3978435378 ISerializable_sub_A0 | 0x10b0b6c | +0xa8; GetType virtual+8 then default-instance helper 0x10c5e7c, which loads class+0xe0 or calls CreateInstance. Pointer returned x0. |
| 2149588021 ISerializable_sub_C0 | 0x10b0b44 | +0xc8; resolves owner then 0x10e3284 returns owner+0x30; pointer result x0. |
| 4175699423 IScriptable_sub_D8 | 0x11e4da0 | IScriptable +0xe0; gets type through virtual+8, forwards original x2 as x1 to 0x10c5074. |
| 55710513 IScriptable_DestructValueHolder | 0x11e4964 | Called by IScriptable destructor 0x11e4934 before base destructor. Iterates flagged scripted props and calls type destructor +0x48; frees holder through class allocator+0xc0, clears object+0x38. void(this x0). |
| 859509764 Handle_DecWeakRef | 0xf37704 | Called from sub_78 weak-handle replacement. Loads control block from handle+8, atomically decrements weak count at control+4, frees control on last reference, nulls handle+8. void(handle* x0). |

All function addresses above are LC_FUNCTION_STARTS entries. The ID helper
0x10b158c atomically increments the uint64 counter at 0x1099dc5e8 with
ldaddal and returns the previous value plus one. The ctor stores it at
object+0x28, matching ISerializable_Counter hash 2630817091 and the SDK
constructor's counter operation. The data address uses the profile's
segment-relative format.

## CClass virtuals and instance construction

The image contains 9072 vtables with GetName/GetSize/GetType slots
+0x10/+0x18/+0x28 pointing to 0x10c31c4/0x10c31d4/0x10c31dc.
Their bodies read name at class+0x18, size at +0x68, and return RTTI kind
2 (Class), respectively. All final slots below are identical across these
tables, giving identity independently of fuzzy matching. GetMaxAlignment
and sub_D0 also independently establish layout semantics.

| Name/hash | Offset | Slot and ARM64 ABI evidence |
| --- | --- | --- |
| CClass_CreateInstance (1518341917) | 0x10c36a0 | nonvirtual: class x0, ignored size w1, zero flag w2; allocates using size+0x68/alignment+0x74, constructs via +0xe0, initializes script properties, returns instance x0. |
| TTypedClass_IsEqual (1482886888) | 0xf611f8 | +0x50 in 8351 tables: tail-branches to 0x10c3bc8, takes class/lhs/rhs/flags in x0..x3, compares property values via their +0x50 slots and returns bool. |
| CClass_Unserialize (3836349613) | 0x10c5a0c | +0x68: branch to 0x10c5a10, preserves class/stream/instance/context x0..x3; reads stream through virtual +0x18; lazily obtains class default object+0xe0. |
| CClass_ToString (1697201926) | 0x10c31ec | +0x70: class x0, instance x1, CString output x2; reads class name+0x18 through CNamePool and builds text. |
| CClass_sub_80 (298916620) | 0x10c5b40 | +0x88: branch to 0x10c5b44, consumes class/context/instance x0..x2, consults class flags+0x70 and default object+0xe0, bool return. |
| CClass_sub_88 (1272912818) | 0x10c5d40 | +0x90: branch to 0x10c5d44, class/context/instance x0..x2, context virtual calls +0x18/+0x48; bool w0. |
| CClass_sub_90 (2725066574) | 0x10c557c | +0x98: saves arguments x0..x4, uses CString x3, returns boolean in x0. |
| CClass_sub_98 (1220550858) | 0x10c56f0 | +0xa0: saves arguments x0..x5, additional flag preserved in x20; CString x3; bool result. |
| CClass_sub_A0 (368188783) | 0x10c58ac | +0xa8: arguments class/context/CString/bool-pointer x0..x3; stores result byte through original x3 at 0x10c59f4/0x10c59fc. |
| CClass_sub_B0 (2693863796) | 0x10c9d38 | +0xb8: consumes class/instance/context x0..x2; walks property list+0x118 with count+0x124 and dispatches property+0xb8. No return value consumed by SDK. |
| CClass_sub_C0 (3523744305) | 0x10c6388 | +0xc8 in 9071 tables (one override): class-only x0; rebuilds property data from class array+0x28/count+0x34; void. |
| CClass_GetMaxAlignment (3511554608) | 0x10c5f74 | +0xd0: class-only x0; starts with alignment+0x74, walks parent+0x10 and property alignments via type+0x20; returns max in w0. |
| CClass_sub_D0 (930418367) | 0x10c6520 | +0xd8: class-only x0; verifies property offset+size against native size+0x68 or holder size+0x6c and checks pairwise overlap; bool w0. |

All entries are LC_FUNCTION_STARTS addresses, including the one-instruction
branch veneers. The TTypedClass equality entry is the dominant typed-class
slot rather than the CClass base false-return stub.

## ResourceDepot (2.5)

Constructor 0x101519f6c (the 4-byte stub 0x10151a4e0 branches to it; the destructor
follows at 0x10151a4e4). Stores: vtables at +0/+8, DynArray groups +0x10 with size+flags
cleared together at +0x1c (groupFlags +0x20, as 2.3.1), DynArrays +0x28, +0x38 (as
2.3.1), then new DynArrays +0x48 (0x50-byte entries per the destructor), +0x58 (0x20 =
CString), +0x68, +0x78, an 8-byte clear at +0x84, CString copies of the argument at +0x90
(rootPath, 2.3.1 +0x48) and arg+0x20 at +0xb0, byte arg+0x40 at +0xd0, the 0x140-byte
entry array at +0xd8 (2.3.1 +0x68), lock byte +0xe8 (2.3.1 +0x78); size 0xf0. Element
strides in InitializeArchives/LoadArchives are unchanged: ArchiveGroup 0x38, Archive 0x50.

## CGameEngine framework and runtime scene (2.5)

The GameFramework init job 0x10158e3f8 (match of 2.3.1 0x103f23330) stores the framework with
`str x8,[engine,#0x378]` where 2.3.1 had `#0x338`; its engine vtable calls also moved +0x28
(0xe8->0x110, 0xf0->0x118). RTTI: BaseGameEngine 0x350 (2.3.1 0x310), CGameEngine 0x3c0 (0x380).
Reading +0x338 on 2.5 gives null; ArchiveXL's GetRuntimeSystem crashed on it (run
20261022-201352-archivexl, far 0x18). worldRuntimeScene stays 0x4b8 and every runtime system
ArchiveXL maps has the same RTTI size as 2.3.1. The StreamingWorld getter
`add x0,x0,#0x270; ret` is 0x101bfe958 in 2.5 (2.3.1 0x10341fdfc); both have 11 call sites, 9 of
which load the system from scene `+0x1b0` (index 27) just before, so the RedLib slot table holds.
