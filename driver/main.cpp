#include <fstream>
#include <vector>
#include <cctype>
#include <stdexcept>

#include "thorin/util/log.h"

#include "impala/ast.h"
#include "impala/compiler.h"
#include "impala/parser.h"

//------------------------------------------------------------------------------

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
        std::vector<std::string> infiles;
        std::string prgname = argv[0], log_name("-"), module_name;
        bool emit_ast = false, fancy = false;

        for (int i = 1; i != argc; ++i) {
            std::string cur_option;

            auto cmp = [&](const char* opt) {
                if (strcmp(argv[i], opt) == 0) {
                    cur_option = opt;
                    return true;
                }
                return false;
            };

            auto get_arg = [&] {
                if (i+1 == argc)
                    throw std::invalid_argument("missing argument for option '" + cur_option + ("'"));
                return std::string(argv[++i]);
            };

            if (cmp("-h") || cmp("--help")) {
                outln("Usage: {} [options] file...", prgname);
                outln("");
                outln("Options:");
                outln("-h, --help                 produce this help message");
                outln("    --emit-ast             emit AST of Impala program");
                outln("    --fancy                use fancy output: Impala's AST dump uses only");
                outln("                           parentheses where necessary");
                outln("    --log <arg>            specifies log file; use '-' for stdout (default)");
                outln("    --log-level {{" LOG_LEVELS "}}");
                outln("                           set log level");
                outln("-o, --output               specifies the output module name");
#ifndef NDEBUG
                outln("");
                outln("Developer options:");
                outln("-b, ---break <args>        trigger a breakpoint when creating a definition of");
                outln("                           global id <arg>; may be used multiple times separated");
                outln("                           by space or '_'");
                outln("    ---track-history       track history of names - useful for debugging");
#endif
                outln("");
                outln("Mandatory arguments to long options are mandatory for short options too.");
                return EXIT_SUCCESS;
            } else if (cmp("--emit-ast")) {
                emit_ast = true;
            } else if (cmp("--fancy")) {
                fancy = true;
            } else if (cmp("--log")) {
                log_name = get_arg();
            } else if (cmp("--log-level")) {
                auto log_level = get_arg();
                if (false) {}
                else if (log_level == "error"  ) thorin::Log::set_min_level(thorin::Log::Error  );
                else if (log_level == "warn"   ) thorin::Log::set_min_level(thorin::Log::Warn   );
                else if (log_level == "info"   ) thorin::Log::set_min_level(thorin::Log::Info   );
                else if (log_level == "verbose") thorin::Log::set_min_level(thorin::Log::Verbose);
                else if (log_level == "debug"  ) thorin::Log::set_min_level(thorin::Log::Debug  );
                else throw std::invalid_argument("log level must be one of {" LOG_LEVELS "}");
            } else if (cmp("-o") || cmp("--output")) {
                module_name = get_arg();
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
                auto infile = get_arg();
                auto i = infile.find_last_of('.');
                if (infile.substr(i + 1) != "impala")
                    throw std::invalid_argument("input file '" + infile + "' does not have '.impala' extension");
                auto rest = infile.substr(0, i);
                auto f = rest.find_last_of('/');
                if (f != std::string::npos)
                    rest = rest.substr(f+1);
                if (rest.empty())
                    throw std::invalid_argument("input file '" + infile + "' has empty module name");
                if (module_name.empty())
                    module_name = rest;
                infiles.emplace_back(infile);
            }
        }

        std::ofstream log_file;
        thorin::Log::set_stream(log_name == "-" ? std::cout : (log_file.open(log_name), log_file));

        if (infiles.empty()) {
            errln("no input files");
            return EXIT_FAILURE;
        }

        if (infiles.size() != 1)
            errln("at the moment there is only one input file supported");

        auto filename = infiles.front().c_str();
        std::ifstream file(filename);
        auto expr = impala::parse(compiler, file, filename);

        if (emit_ast)
            expr->print(std::cout, fancy);

        return EXIT_SUCCESS;
    } catch (std::exception const& e) {
        errln("{}",  e.what());
        return EXIT_FAILURE;
    } catch (...) {
        errln("unknown exception");
        return EXIT_FAILURE;
    }
}
