#pragma once
#include "../Misc/Static.hpp"
#include "../Misc/Updated.hpp"

/* TODO: Move PolyHook files to a separate folder (the project uses relative paths everywhere which breaks that, annoying) */
#include <headers/CapstoneDisassembler.hpp>
#include <headers/Detour/x86Detour.hpp>
#include <headers/Enums.hpp>

/* TODO: Move to a separate file once we fully migrate */
#define r_incr_top(rL) (*(uintptr_t*)((rL) + L_TOP) += sizeof(TValue)) 
#define r_decr_top(rL) (*(uintptr_t*)((rL) + L_TOP) -= sizeof(TValue)) 

#define r_setobj setobj
#define r_tonumber(rL, o, n) (ttype(o) == R_LUA_TNUMBER || (((o) = (rL)->VToNumber(o, n)) != NULL))
#define r_setnilvalue(obj) ((obj)->tt = R_LUA_TNIL)
#define r_setnvalue(obj, x) { TValue *i_o = (obj); i_o->value.n = (x); i_o->tt = R_LUA_TNUMBER; }
#define r_setpvalue(obj, x) { TValue *i_o = (obj); i_o->value.p = (x); i_o->tt = R_LUA_TLIGHTUSERDATA; }
#define r_setbvalue(obj, x) { TValue *i_o = (obj); i_o->value.b = (x); i_o->tt = R_LUA_TBOOLEAN; }
#define r_setsvalue(obj, x) { TValue *i_o = (obj); i_o->value.gc = cast(GCObject*, (x)); i_o->tt = R_LUA_TSTRING; }
#define r_setuvalue(obj, x) { TValue *i_o = (obj); i_o->value.gc = cast(GCObject*, (x)); i_o->tt = R_LUA_TUSERDATA; }
#define r_setthvalue(obj, x){ TValue *i_o = (obj); i_o->value.gc = cast(GCObject*, (x)); i_o->tt = R_LUA_TTHREAD; }
#define r_setclvalue(obj, x){ TValue *i_o = (obj); i_o->value.gc = cast(GCObject*, (x)); i_o->tt = R_LUA_TFUNCTION; }
#define r_sethvalue(obj, x) { TValue *i_o = (obj); i_o->value.gc = cast(GCObject*, (x)); i_o->tt = R_LUA_TTABLE; }

#pragma region RLua C Functions and Signatures
typedef int(*r_lua_CFunction)(DWORD rL);
typedef int(*r_lua_NoUpValCFunction)(DWORD rL, SynCClosure cL);
#pragma endregion

#define TO_STR(s) #s

#pragma region Function Defines
#define R_LUA_CALL(ret, addr, cc, ...) auto Ret = syn::SpoofCall<ret>(cc, (void*)addr, __VA_ARGS__)
#define R_LUA_VOIDCALL(addr, cc, ...)  syn::SpoofCall<void*>(cc, (void*)addr, __VA_ARGS__)
#define R_LUA_RETCALL(ret, addr, cc, ...) return syn::SpoofCall<ret>(cc, (void*)addr, __VA_ARGS__)
#pragma endregion

typedef enum _MEMORY_INFORMATION_CLASS
{
    MemoryBasicInformation, // MEMORY_BASIC_INFORMATION
    MemoryWorkingSetInformation, // MEMORY_WORKING_SET_INFORMATION
    MemoryMappedFilenameInformation, // UNICODE_STRING
    MemoryRegionInformation, // MEMORY_REGION_INFORMATION
    MemoryWorkingSetExInformation, // MEMORY_WORKING_SET_EX_INFORMATION
    MemorySharedCommitInformation, // MEMORY_SHARED_COMMIT_INFORMATION
    MemoryImageInformation, // MEMORY_IMAGE_INFORMATION
    MemoryRegionInformationEx,
    MemoryPrivilegedBasicInformation,
    MemoryEnclaveImageInformation, // MEMORY_ENCLAVE_IMAGE_INFORMATION // since REDSTONE3
    MemoryBasicInformationCapped
} MEMORY_INFORMATION_CLASS;

DWORD GetCurrentThreadIdHook();
DWORD WINAPI SetUEFHook(LPTOP_LEVEL_EXCEPTION_FILTER);
signed int WINAPI RtlImageNtHeaderExHook(int A1, unsigned int Base, int A3, int A4, PIMAGE_NT_HEADERS* Out);
NTSTATUS NTAPI NtQIPHook(HANDLE ProcessHandle, INT ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
NTSTATUS NTAPI NtQVMHook(HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID Buffer, SIZE_T Length, PSIZE_T ResultLength);
NTSTATUS NTAPI ZwFTHook(HANDLE ExistingTokenHandle, ULONG Flags, PTOKEN_GROUPS SidsToDisable, PTOKEN_PRIVILEGES PrivilegesToDelete, PTOKEN_GROUPS SidsToRestricted, HANDLE NewTokenHandle);
NTSTATUS NTAPI GetAdaptersAddressesHook(ULONG Family, ULONG Flags, PVOID Reserved, void* AdapterAddresses, PULONG SizePointer);
void MsgOutHook();
void ErrHook();

extern bool IsLuaU;
extern DWORD SynModule;
extern std::vector<std::string> ChunkNamesVec;
extern PLH::CapstoneDisassembler x86Disasm;
extern std::unordered_map<std::uintptr_t, SynCClosure> HookedFunctionsMap;

extern uint64_t OrigRtlImageNtHeaderEx;
extern uint64_t OriginalSetUEF;
extern uint64_t OriginalNtQIP;
extern uint64_t OriginalGetAdaptersAddresses;
extern uint64_t OriginalGetThreadId;

extern uintptr_t OriginalErrHook;
extern uintptr_t OriginalMsgOutHook;

extern uint32_t ScriptRunCounter;
extern bool SecureLuaFlag;

extern LPVOID OriginalNtQVM;
extern LPVOID OriginalZwFT;

namespace syn
{
    class RbxLua
    {
        static void _PushCFunction(RbxLua* rL, r_lua_CFunction cF);
        static void _PushCClosure(RbxLua* rL, DWORD upvalues);

    public:
        uintptr_t RState;

        static void __declspec(noinline) Initialize();
        static uintptr_t GetBinValue(const uint32_t BinHash) noexcept;

        /* Push */
        void PushNumber(lua_Number n) const;
        void PushInteger(lua_Integer n) const;
        void PushBoolean(bool b) const;
        void PushLightUserData(void* p) const;
        void PushLString(const char* s, size_t len) const;
        void PushString(const char* s) const;
        void PushNil() const;
        void PushThread() const;
        void PushValue(int idx) const;
        void PushCClosure(r_lua_CFunction cf, int nups) const;

        /* Get */
        void VGetTable(const TValue* t, TValue* key, StkId val) const;
        void GetTable(int idx) const;
        void GetField(int idx, const char* k) const;
        void RawGetI(int Index, int N) const;
        void RawSetI(int idx, int n) const;
        int GetMetaTable(int idx) const;
        int GetMetaField(int obj, const char* e) const;
        int GetTop() const;
        const char* GetUpvalue(int idx, int n) const;
        const char* GetLocal(lua_Debug* ar, int n) const;
        int GetStack(int level, lua_Debug* ar) const;
        int GetInfo(const char* what, lua_Debug* ar) const;
        int HGetN(Table* t) const;

        /* Set */
        void VSetTable(const TValue* t, TValue* key, StkId val) const;
        void SetTable(int idx) const;
        int SetMetaTable(int objindex) const;
        void SetField(int idx, const char* k) const;
        void SetTop(int idx) const;
        const char* SetUpvalue(int idx, int n) const;
        const char* SetLocal(lua_Debug* ar, int n) const;

        /* Conversion */
        bool ToBoolean(int idx) const;
        const char* ToLString(int idx, size_t* len = NULL) const;
        lua_Number ToNumber(int idx) const;
        lua_Integer ToInteger(int idx) const;
        void* ToUserData(int idx) const;
        uintptr_t ToPointer(int idx) const;

        /* Creation */
        uintptr_t NewLString(const char* str, size_t l) const;
        RbxLua NewThread(BOOL DoPop = TRUE) const;
        void CreateTable(int narray, int nrec) const;
        uintptr_t NewLClosure(int nups, int maxstacksize, uintptr_t e) const;
        uintptr_t NewProto() const;
        uintptr_t HNew(int narray, int nhash) const;

        /* Calling */
        int PCall(int nargs, int nresults, int nhandler) const;

        /* Alloc */
        void* Realloc(void* block, size_t osize, size_t nsize, int mode) const;
        void* Alloc(size_t size, int mode) const;

        /* Stack Manipulation */
        void XMove(RbxLua To, int N) const;
        void Insert(int idx) const;
        void Remove(int idx) const;

        /* Type Checking */
        void CheckType(int narg, int t) const;
        void CheckAny(int narg) const;
        const char* CheckLString(int narg, size_t* len) const;
        const char* OptLString(int narg, const char* def, size_t* len) const;
        lua_Number CheckNumber(int narg) const;
        lua_Number OptNumber(int narg, lua_Number def) const;
        lua_Integer CheckInteger(int narg) const;
        lua_Integer OptInteger(int narg, lua_Integer def) const;

        /* Erroring */
        int Error() const;
        int LError(const char* fmt, ...) const;
        int ArgError(int narg, const char *extramsg) const;
        int TypeError(int narg, const char* tname) const;
        void TagError(int narg, int tag) const;
        void GTypeError(const TValue* o, const char* op) const;
        void GRunError(const char* fmt, ...) const;

        /* Misc */
        TValue* Index2Adr(int idx) const;
        int Type(int idx) const;
        const char* TypeName(int t) const;
        int ObjLen(int idx) const;
        int Ref(int t) const;
        int Next(int idx) const;
        int RYield(int nresults) const;
        void LinkGC(uintptr_t GCObject, BYTE Type) const;
        void CStep() const;
        void CheckGC() const;
        void CBarrier(uintptr_t o, uintptr_t v) const;
        void CBarrierT(uintptr_t hval, uintptr_t tval) const;
        uintptr_t VNameCall(StkId b) const;

        /* VM Specific */
        void VConcat(int total, int last) const;
        void VArith(StkId ra, const TValue* rb, const TValue* rc, int op) const;
        UpVal* FFindUpval(StkId level) const;
        void FClose(StkId level) const;
        int CallBinTM(const TValue* p1, const TValue* p2, StkId res, int event) const;
        int LessThan(const TValue* l, const TValue* r) const;
        int LessEqual(const TValue* l, const TValue* r) const;
        int VEqualVal(const TValue* t1, const TValue* t2) const;
        void HResizeArray(Table* t, int nasize) const;
        TValue* HSetNum(Table* t, int key) const;
        int DCall(StkId func, int nResults) const;
        int DPreCall(StkId ra, int nresults) const;
        int DPreCallU(StkId ra, int nresults) const;
        int DPosCall(StkId ra) const;
        void DGrowStk(int n) const;
        void CBarRF(uintptr_t o, uintptr_t v) const;

        /* Helper */
        static double XorDouble(double num);
        const TValue* VToNumber(const TValue* obj, TValue* n) const;
		TValue* VToNumberForPrep(TValue* obj, TValue* n) const;
        int VToString(StkId obj) const;

        void SetupThread() const;
        void SetIdentity(BYTE ctx) const;
        DWORD GetIdentity() const;

        static int LTypeToRType(int LType);
        static int RTypeToLType(int LType);

        void PushCFunction(r_lua_CFunction cf);
        void PushLambda(std::function<int(uintptr_t)> lam);
        void CreateCClosure(int upvalues);
        void PushRawObject(uintptr_t o, int type) const;
        void PushObject(const TValue* o) const;
        int RawSLen(TString* ts) const;

        /* Type Equality */
        bool IsFunction(int n) const { return (Type(n) == R_LUA_TFUNCTION); };
        bool IsTable(int n) const { return (Type(n) == R_LUA_TTABLE); };
        bool IsLightUserData(int n) const { return (Type(n) == R_LUA_TLIGHTUSERDATA); };
        bool IsNil(int n) const { return (Type(n) == R_LUA_TNIL); };
        bool IsProto(int n) const { return (Type(n) == R_LUA_TPROTO); };
        bool IsBoolean(int n) const { return (Type(n) == R_LUA_TBOOLEAN); };
        bool IsThread(int n) const { return (Type(n) == R_LUA_TTHREAD); };
        bool IsNone(int n) const { return (Type(n) == R_LUA_TNONE); };
        bool IsNoneOrNil(int n) const { return (Type(n) <= 0); };
        bool IsCFunction(int idx) const;
        bool IsNumber(int idx) const;
        bool IsString(int idx) const;
        bool IsUserData(int idx) const;

        /* Macros */
        const char* ToString(int idx) const { return ToLString(idx, 0); };
        void GetGlobal(const char* k) const { GetField(LUA_GLOBALSINDEX, k); };
        void SetGlobal(const char* k) const { SetField(LUA_GLOBALSINDEX, k); };
        void NewTable() const { CreateTable(0, 0); };
        void Pop(int n) const { SetTop(-(n)-1); };
        DWORD SNew(const char* s) const { return NewLString(s, strlen(s)); };

        [[maybe_unused]] int ArgCheck(bool cond, int numarg, const char* extramsg) const { return cond || ArgError(numarg, extramsg); };
        const char* CheckString(int narg) const { return CheckLString(narg, NULL); };
        const char* OptString(int narg, const char* def) const { return OptLString(narg, def, NULL); };
        int CheckInt(int narg) const { return (int)CheckInteger(narg); };
        int OptInt(int narg, int def) const { return (int)OptInteger(narg, def); };
        int CheckLong(int narg) const { return (long)CheckInteger(narg); };
        int OptLong(int narg, long def) const { return (long)OptInteger(narg, def); };

        const char* GetStr(TString* o) const { return (const char*)((uintptr_t)(o) + 0x18); };

        static uintptr_t GlobalState(uintptr_t RS = 0);
        operator uintptr_t() const
        {
            return RState;
        }

        RbxLua(uintptr_t RS);
    };

}
