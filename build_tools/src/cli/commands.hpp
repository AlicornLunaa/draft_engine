#pragma once

// draft_buildtools's subcommands (see src/cli/main.cpp for dispatch). Each owns its own argv
// parsing and usage message; argv[1] is always the subcommand name itself.
int run_validate(int argc, char** argv);
int run_pack(int argc, char** argv);
int run_export(int argc, char** argv);
