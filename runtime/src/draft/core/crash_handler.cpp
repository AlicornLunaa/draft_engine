#include "draft/core/crash_handler.hpp"

#include <csignal>
#include <cstdio>
#include <ctime>
#include <exception>
#include <filesystem>
#include <string>

#if defined(_WIN32)
    #include <windows.h>
    #include <dbghelp.h>
#else
    #include <signal.h>
    #include <execinfo.h>
    #include <unistd.h>
#endif

namespace Draft {
    namespace {
        unsigned long current_process_id(){
            #if defined(_WIN32)
                return GetCurrentProcessId();
            #else
                return static_cast<unsigned long>(getpid());
            #endif
        }

        // Opens "crashes/crash_<timestamp>_<pid>.log" (creating the directory if needed) and
        // writes the header every report shares.
        std::FILE* open_crash_report(const std::string& reason){
            std::error_code ec;
            std::filesystem::create_directories("crashes", ec);

            std::time_t now = std::time(nullptr);
            char timestamp[32];
            std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", std::localtime(&now));

            std::string path = "crashes/crash_" + std::string(timestamp) + "_" + std::to_string(current_process_id()) + ".log";
            std::FILE* file = std::fopen(path.c_str(), "w");
            if(!file)
                return nullptr;

            std::fprintf(file, "Crash report\n");
            std::fprintf(file, "Time: %s\n", timestamp);
            std::fprintf(file, "Reason: %s\n\n", reason.c_str());
            std::fprintf(file, "Backtrace:\n");

            return file;
        }

        #if defined(_WIN32)
            void write_backtrace(std::FILE* file){
                void* addresses[64];
                USHORT count = CaptureStackBackTrace(0, 64, addresses, nullptr);

                HANDLE process = GetCurrentProcess();
                bool symbolsReady = SymInitialize(process, nullptr, TRUE) != FALSE;

                alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
                SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                symbol->MaxNameLen = MAX_SYM_NAME;

                for(USHORT i = 0; i < count; i++){
                    std::fprintf(file, "  #%u %p", static_cast<unsigned int>(i), addresses[i]);

                    DWORD64 displacement = 0;
                    if(symbolsReady && SymFromAddr(process, reinterpret_cast<DWORD64>(addresses[i]), &displacement, symbol))
                        std::fprintf(file, " %s+0x%llx", symbol->Name, static_cast<unsigned long long>(displacement));

                    std::fprintf(file, "\n");
                }

                if(symbolsReady)
                    SymCleanup(process);
            }

            const char* exception_name(DWORD code){
                switch(code){
                    case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
                    case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
                    case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
                    case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
                    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
                    case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
                    case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
                    default: return "unhandled structured exception";
                }
            }

            LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS* info){
                std::FILE* file = open_crash_report(exception_name(info->ExceptionRecord->ExceptionCode));
                if(file){
                    write_backtrace(file);
                    std::fclose(file);
                }

                // Lets Windows' own default handling (WER dialog, debugger attach, non-zero exit)
                // still happen after our report is written.
                return EXCEPTION_CONTINUE_SEARCH;
            }

            void install_platform_handler(){
                SetUnhandledExceptionFilter(unhandled_exception_filter);
            }
        #else
            void write_backtrace(std::FILE* file){
                void* addresses[64];
                int count = backtrace(addresses, 64);

                // backtrace_symbols_fd() writes straight to the fd without allocating, unlike
                // backtrace_symbols(), which mallocs the whole result array.
                std::fflush(file);
                backtrace_symbols_fd(addresses, count, fileno(file));
            }

            const char* signal_name(int sig){
                switch(sig){
                    case SIGSEGV: return "SIGSEGV (segmentation fault)";
                    case SIGABRT: return "SIGABRT (abort)";
                    case SIGILL: return "SIGILL (illegal instruction)";
                    case SIGFPE: return "SIGFPE (floating point exception)";
                    case SIGBUS: return "SIGBUS (bus error)";
                    default: return "unknown signal";
                }
            }

            constexpr int CRASH_SIGNALS[] = { SIGSEGV, SIGABRT, SIGILL, SIGFPE, SIGBUS };

            // Dedicated stack the handler runs on instead of the faulting thread's own, so a crash
            // caused by stack overflow (which leaves no usable space on the original stack) can
            // still be handled instead of immediately re-faulting.
            constexpr std::size_t ALT_STACK_SIZE = 65536;
            char s_altStack[ALT_STACK_SIZE];

            void signal_handler(int sig, siginfo_t*, void*){
                std::FILE* file = open_crash_report(signal_name(sig));
                if(file){
                    write_backtrace(file);
                    std::fclose(file);
                }

                // Restore the default disposition and re-raise, rather than calling abort()
                // ourselves, so the OS's own default behavior for this signal
                signal(sig, SIG_DFL);
                raise(sig);
            }

            void install_platform_handler(){
                stack_t altStack{};
                altStack.ss_sp = s_altStack;
                altStack.ss_size = ALT_STACK_SIZE;
                altStack.ss_flags = 0;
                sigaltstack(&altStack, nullptr);

                struct sigaction action{};
                action.sa_sigaction = signal_handler;
                action.sa_flags = SA_SIGINFO | SA_ONSTACK;
                sigemptyset(&action.sa_mask);

                for(int sig : CRASH_SIGNALS)
                    sigaction(sig, &action, nullptr);
            }
        #endif

        // Not a crash signal/SEH exception, an uncaught C++ exception unwound all the way out instead
        void terminate_handler(){
            std::string reason = "Unhandled C++ exception";

            if(std::exception_ptr eptr = std::current_exception()){
                try {
                    std::rethrow_exception(eptr);
                } catch(const std::exception& e){
                    reason += ": ";
                    reason += e.what();
                } catch(...){
                    reason += " (of a type that isn't std::exception)";
                }
            }

            std::FILE* file = open_crash_report(reason);
            if(file){
                write_backtrace(file);
                std::fclose(file);
            }

            // A terminate handler must not return control to its caller.
            std::signal(SIGABRT, SIG_DFL);
            std::abort();
        }
    }

    void CrashHandler::install(){
        std::set_terminate(terminate_handler);
        install_platform_handler();
    }
}
