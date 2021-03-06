// 2015-01-30T11:45+08:00

#include "help_command.h"

#include <cassert>

#include "../azure_cloud_storage_service.h"

HelpCommand::HelpCommand()
{
    options_desc_.add_options()
        ("command,c", boost::program_options::wvalue<utility::string_t>(&command_), "")
        ;
}

bool HelpCommand::parse(const std::vector<utility::string_t> &vargs)
{
    command_.clear();

    boost::program_options::positional_options_description command_name_option;
    command_name_option.add("command", 1);

    boost::program_options::variables_map args_map;

    try {
        boost::program_options::store(boost::program_options::basic_command_line_parser<CharT>(vargs).
            options(options_desc_).positional(command_name_option).run(), args_map);
        boost::program_options::notify(args_map);

        return true;
    } catch (const std::exception &e) {
        notify_err(e.what(), HELP_COMMAND_STR);
        return false;
    }
}

bool HelpCommand::run(AzureCloudStorageService *storage_service)
{
    assert(storage_service != nullptr);
    if (command_.empty()) {
        ucout << U("All available commands:\n");
        DumpLineByLine(storage_service->available_commands_);
        ucout << U("Type \"") << HELP_COMMAND_STR << U(" [command]\" to learn the usage of target command.\n");
    } else {
        try {
            std::shared_ptr<Command> command_ptr = storage_service->get_command(command_);
            command_ptr->help();
        } catch (const utility::string_t &err_msg) {
            ucerr << err_msg << std::endl;
        }
    }

    return true;
}

void HelpCommand::help() const
{
    ucout << U("Usage: help [command]\n");
    Command::help();
}
