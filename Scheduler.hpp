#pragma once

#include "../Misc/Static.hpp"

#include "RbxLua.hpp"
#include "RbxInstance.hpp"
#include "Conversion/RbxConversion.hpp"
#include "../Security/AntiDump.hpp"

#include <queue>
#include "../Misc/Profiler.hpp"
#include "../../Utilities/SafeQueue.hpp"

namespace syn
{
	class Task
	{
	public:
		bool isC;
		std::uint8_t ScriptMode;
		std::string ScriptValue;
		std::function<void(DWORD)> FunctionValue;

		Task(const std::string& Script, const std::uint8_t Mode = 0)
		{
			ScriptValue = Script;
			ScriptMode = Mode;
			isC = false;
		}

		Task(std::function<void(DWORD)> Function)
		{
			FunctionValue = std::move(Function);
			isC = true;
		}
	};

	static BOOL ResetScheduler = FALSE;

	class Scheduler
	{
	private:
		syn::SafeQueue<Task> ScriptQueue;

	public:
		static void StepSchedule();

		DWORD OldFunc = 0;
		DWORD CurrentJob = 0;
		RbxLua MainThread = 0;

		int Step = 0;
		bool Initialized = false;

		static Scheduler* GetSingleton()
		{
			static Scheduler* scheduler = nullptr;
			if (scheduler == nullptr)
				scheduler = new Scheduler();
			return scheduler;
		}

		unsigned int Count()
		{
			return ScriptQueue.size();
		}

		Task Pop()
		{
			return ScriptQueue.dequeue();
		}

		void Push(const std::string& Script)
		{
			syn::Profiler::GetSingleton()->AddProfile(OBFUSCATE_STR("Scheduler push script"));
			ScriptQueue.enqueue({ Script });
		}

		void Push(const std::string& Script, const std::uint8_t Mode)
		{
			syn::Profiler::GetSingleton()->AddProfile(OBFUSCATE_STR("Scheduler push script"));
			ScriptQueue.enqueue({ Script, Mode });
		}

		void Push(std::function<void(DWORD)> FunctionValue)
		{
			syn::Profiler::GetSingleton()->AddProfile(OBFUSCATE_STR("Scheduler push function"));
			ScriptQueue.enqueue(std::move(FunctionValue));
		}

		void Clear()
		{
			syn::Profiler::GetSingleton()->AddProfile(OBFUSCATE_STR("Scheduler clear"));
			ScriptQueue.clear();
		}

		bool Empty()
		{
			return !ScriptQueue.size();
		}

		void Attach();
	};
}
