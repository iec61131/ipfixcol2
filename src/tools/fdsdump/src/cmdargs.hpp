/**
 * \file src/cmdargs.hpp
 * \author Michal Sedlak <xsedla0v@stud.fit.vutbr.cz>
 * \brief Command line arguments
 *
 * Copyright (C) 2021 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is, and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */
#pragma once

#include <vector>
#include <string>

struct CmdArgs {
    std::vector<std::string> input_file_patterns;
    std::string input_filter;
    std::string aggregate_keys;
    std::string aggregate_values;
    std::string output_filter;
    std::string sort_fields;
    std::string output_mode;
    unsigned int num_threads;
    unsigned int output_limit;
    bool translate_ip_addrs;
    bool print_help;
    bool stats;
};

/**
 * \brief      Print the program usage
 */
void
print_usage();

/**
 * \brief      Parse command line arguments
 *
 * \param[in]  argc  The argc
 * \param[in]  argv  The argv
 *
 * \return     The command line arguments structure
 *
 * \throw      ArgError if invalid arguments are found
 */
CmdArgs
parse_cmd_args(int argc, char **argv);
