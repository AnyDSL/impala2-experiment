#include <fstream>
#include <vector>
#include <cctype>
#include <stdexcept>

#include "thorin/util/log.h"

#include "impala/ast.h"
#include "impala/compiler.h"
#include "impala/parser.h"

//------------------------------------------------------------------------------

using namespace thorin;
using namespace std;

typedef vector<string> Names;

//------------------------------------------------------------------------------

#ifndef NDEBUG
#define LOG_LEVELS "{error|warn|info|verbose|debug}"
#else
#define LOG_LEVELS "{error|warn|info}"
#endif

ostream* open(ofstream& stream, const string& name) {
    if (name == "-")
        return &cout;

    stream.open(name);
    return &stream;
}

int main(int argc, char** argv) {
    try {
        if (argc < 1) throw logic_error("bad number of arguments");

        impala::Compiler compiler;
        string prgname = argv[0];
        Names infiles;
        string out_name, log_name("-"), log_level("error");
        bool emit_ast = false, fancy = false;

        for (int i = 1; i != argc; ++i) {
            auto cmp = [&](const char* s) { return strcmp(argv[i], s) == 0; };
            auto get_arg = [&] {
                if (i+1 == argc) throw invalid_argument("log level must be one of " LOG_LEVELS);
                return argv[++i];
            };

            if (cmp("-h") || cmp("--help")) {
                outln("Usage: {} [options] file...", prgname);
                outln("<infiles>      input files");
                outln("-help          produce this help message");
                outln("-log-level " LOG_LEVELS);
                outln("               set log level");
                outln("-log <arg>     specifies log file; use '-' for stdout (default)");
                outln("-o             specifies the output module name");
                outln("-emit-ast      emit AST of Impala program");
                outln("-fancy         use fancy output: Impala's AST dump uses only parentheses where necessary");
#ifndef NDEBUG
                outln("-break <args>  breakpoint at definition generation with global id <arg>; may be used multiple times separated by space or '_'");
                outln("-track-history track history of names - useful for debugging");
#endif
                return EXIT_SUCCESS;
            } else if (cmp("-fancy")) {
                fancy = true;
            } else if (cmp("-log")) {
                log_name = get_arg();
            } else if (cmp("-log-level")) {
                log_level = get_arg();
#ifndef NDEBUG
            } else if (cmp("-break")) {
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
            } else if (cmp("-track-history")) {
                compiler.world.enable_history();
#endif
            } else {
                infiles.emplace_back(argv[i]);
            }
        }

        ofstream log_stream;
        if (log_level == "error") {
            Log::set(Log::Error, open(log_stream, log_name));
        } else if (log_level == "warn") {
            Log::set(Log::Warn, open(log_stream, log_name));
        } else if (log_level == "info") {
            Log::set(Log::Info, open(log_stream, log_name));
        } else if (log_level == "verbose") {
            Log::set(Log::Verbose, open(log_stream, log_name));
        } else if (log_level == "debug") {
            Log::set(Log::Debug, open(log_stream, log_name));
        } else
            throw invalid_argument("log level must be one of " LOG_LEVELS);

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
                    throw invalid_argument("input file '" + infile + "' does not have '.impala' extension");
                auto rest = infile.substr(0, i);
                auto f = rest.find_last_of('/');
                if (f != string::npos) {
                    rest = rest.substr(f+1);
                }
                if (rest.empty())
                    throw invalid_argument("input file '" + infile + "' has empty module name");
                module_name = rest;
            }
        }

        auto filename = infiles.front().c_str();
        std::ifstream file(filename);
        auto expr = impala::parse(compiler, file, filename);

        if (emit_ast)
            expr->print(std::cout, fancy);

        return EXIT_SUCCESS;
    } catch (exception const& e) {
        cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        cerr << "unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}
