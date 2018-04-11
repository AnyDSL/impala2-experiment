#include <fstream>
#include <vector>
#include <cctype>
#include <stdexcept>

#include "thorin/util/log.h"

#include "impala/ast.h"
#include "impala/compiler.h"
#include "impala/parser.h"

//------------------------------------------------------------------------------

typedef std::vector<std::string> Names;
using thorin::outln;
using thorin::errln;

//------------------------------------------------------------------------------

#ifndef NDEBUG
#define LOG_LEVELS "error|warn|info|verbose|debug"
#else
#define LOG_LEVELS "error|warn|info|verbose"
#endif

int main(int argc, char** argv) {
    try {
        if (argc < 1) throw std::logic_error("bad number of arguments");

        impala::Compiler compiler;
        std::string prgname = argv[0];
        Names infiles;
        std::string out_name, log_name("-"), log_level("error");
        bool emit_ast = false, fancy = false;

        for (int i = 1; i != argc; ++i) {
            auto cmp = [&](const char* s) { return strcmp(argv[i], s) == 0; };
            auto get_arg = [&] {
                if (i+1 == argc) throw std::invalid_argument("log level must be one of {" LOG_LEVELS "}");
                return argv[++i];
            };

            if (cmp("-h") || cmp("--help")) {
                outln("Usage: {} [options] file...", prgname);
                outln("\noptions:");
                outln("    <infiles>                  input files");
                outln("-h, --help                     produce this help message");
                outln("    --log-level {{" LOG_LEVELS "}}");
                outln("                               set log level");
                outln("    --log <arg>                specifies log file; use '-' for stdout (default)");
                outln("-o, --output                   specifies the output module name");
                outln("    --emit-ast                 emit AST of Impala program");
                outln("    --fancy                    use fancy output: Impala's AST dump uses only parentheses where necessary");
#ifndef NDEBUG
                outln("Developer options:");
                outln("-b, ---break <args>            breakpoint at definition generation with global id <arg>; may be used multiple times separated by space or '_'");
                outln("    ---track-history           track history of names - useful for debugging");
#endif
                return EXIT_SUCCESS;
            } else if (cmp("--emit-ast")) {
                emit_ast = true;
            } else if (cmp("--fancy")) {
                fancy = true;
            } else if (cmp("--log")) {
                log_name = get_arg();
            } else if (cmp("--log-level")) {
                log_level = get_arg();
            } else if (cmp("-o") || cmp("--output")) {
                out_name = get_arg();
#ifndef NDEBUG
            } else if (cmp("-b") || cmp("--break")) {
                std::string b = get_arg();
                size_t num = 0;
                for (size_t i = 0, e = b.size(); i != e; ++i) {
                    char c = b[i];
                    if (c == '_') {
                        if (num != 0) {
                            compiler.world.breakpoint(num);
                            num = 0;
                        }
                    } else if (std::isdigit(c)) {
                        num = num*10 + c - '0';
                    } else {
                        errln("invalid breakpoint '{}'", b);
                        return EXIT_FAILURE;
                    }
                }

                if (num != 0)
                    compiler.world.breakpoint(num);
            } else if (cmp("--track-history")) {
                compiler.world.enable_history();
#endif
            } else {
                infiles.emplace_back(argv[i]);
            }
        }

        std::ofstream log_stream;
        auto open = [&]() -> std::ostream* {
            if (log_name == "-")
                return &std::cout;
            log_stream.open(log_name);
            return &log_stream;
        };

        if (log_level == "error") {
            thorin::Log::set(thorin::Log::Error, open());
        } else if (log_level == "warn") {
            thorin::Log::set(thorin::Log::Warn, open());
        } else if (log_level == "info") {
            thorin::Log::set(thorin::Log::Info, open());
        } else if (log_level == "verbose") {
            thorin::Log::set(thorin::Log::Verbose, open());
        } else if (log_level == "debug") {
            thorin::Log::set(thorin::Log::Debug, open());
        } else {
            throw std::invalid_argument("log level must be one of {" LOG_LEVELS "}");
        }

        if (infiles.empty()) {
            errln("no input files");
            return EXIT_FAILURE;
        }
        if (infiles.size() != 1)
            errln("at the moment there is only one input file supported");

        std::string module_name;
        if (out_name.length()) {
            module_name = out_name;
        } else {
            for (const auto& infile : infiles) {
                auto i = infile.find_last_of('.');
                if (infile.substr(i + 1) != "impala")
                    throw std::invalid_argument("input file '" + infile + "' does not have '.impala' extension");
                auto rest = infile.substr(0, i);
                auto f = rest.find_last_of('/');
                if (f != std::string::npos) {
                    rest = rest.substr(f+1);
                }
                if (rest.empty())
                    throw std::invalid_argument("input file '" + infile + "' has empty module name");
                module_name = rest;
            }
        }

        auto filename = infiles.front().c_str();
        std::ifstream file(filename);
        auto expr = impala::parse(compiler, file, filename);

        if (emit_ast)
            expr->print(std::cout, fancy);

        return EXIT_SUCCESS;
    } catch (std::exception const& e) {
        errln("{}'",  e.what());
        return EXIT_FAILURE;
    } catch (...) {
        errln("unknown exception");
        return EXIT_FAILURE;
    }
}
