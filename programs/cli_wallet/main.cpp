/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <blurt/utilities/key_conversion.hpp>
#include <blurt/protocol/protocol.hpp>
#include <blurt/wallet/wallet.hpp>

#include <fc/io/json.hpp>
#include <fc/io/stdio.hpp>
#include <fc/network/http/server.hpp>
#include <fc/network/http/websocket.hpp>
#include <fc/rpc/cli.hpp>
#include <fc/rpc/http_api.hpp>
#include <fc/rpc/websocket_api.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/interprocess/signals.hpp>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>




using namespace blurt::utilities;
using namespace blurt::chain;
using namespace blurt::wallet;
using namespace std;
namespace bpo = boost::program_options;

int main(int argc, char **argv) {
    try {
        boost::program_options::options_description opts;
        opts.add_options()
                ("help,h", "Print this help message and exit.")
                ("server-rpc-endpoint,s", bpo::value<string>()->implicit_value("ws://127.0.0.1:8090"),
                 "Server websocket RPC endpoint")
                ("cert-authority,a", bpo::value<string>()->default_value("_default"),
                 "Trusted CA bundle file for connecting to wss:// TLS server")
                ("wallet-file,w", bpo::value<string>()->implicit_value("wallet.json"), "wallet to load")
                ("chain-id", bpo::value<string>(), "chain ID to connect to");

        bpo::variables_map options;

        bpo::store(bpo::parse_command_line(argc, argv, opts), options);

        if (options.count("help")) {
            std::cout << opts << "\n";
            return 0;
        }

        fc::path data_dir;
        fc::logging_config cfg;
        fc::path log_dir = data_dir / "logs";

        fc::file_appender::config ac;
        ac.filename = log_dir / "rpc" / "rpc.log";
        ac.flush = true;
        ac.rotate = true;
        ac.rotation_interval = fc::hours(1);
        ac.rotation_limit = fc::days(1);

        std::cout << "Logging RPC to file: " << (data_dir / ac.filename).preferred_string() << "\n";

        cfg.appenders.push_back(fc::appender_config("default", "console", fc::variant(fc::console_appender::config())));
        cfg.appenders.push_back(fc::appender_config("rpc", "file", fc::variant(ac)));

        cfg.loggers = {fc::logger_config("default"), fc::logger_config("rpc")};
        cfg.loggers.front().level = fc::log_level::info;
        cfg.loggers.front().appenders = {"default"};
        cfg.loggers.back().level = fc::log_level::debug;
        cfg.loggers.back().appenders = {"rpc"};


        //
        // TODO:  We read wallet_data twice, once in main() to grab the
        //    socket info, again in wallet_api when we do
        //    load_wallet_file().  Seems like this could be better
        //    designed.
        //
        wallet_data wdata;

        fc::path wallet_file(options.count("wallet-file") ? options.at("wallet-file").as<string>() : "wallet.json");
        if (fc::exists(wallet_file)) {
            wdata = fc::json::from_file(wallet_file).as<wallet_data>();
        } else {
            std::cout << "Starting a new wallet\n";
        }

        // but allow CLI to override
        if (options.count("server-rpc-endpoint"))
            wdata.ws_server = options.at("server-rpc-endpoint").as<std::string>();

        fc::http::websocket_client client(options["cert-authority"].as<std::string>());
        idump((wdata.ws_server));
        auto con = client.connect(wdata.ws_server);
        auto apic = std::make_shared<fc::rpc::websocket_api_connection>(*con);

        auto wapiptr = std::make_shared<wallet_api>(wdata, *apic);
        wapiptr->set_wallet_filename(wallet_file.generic_string());
        wapiptr->load_wallet_file();

        fc::api<wallet_api> wapi(wapiptr);

        auto wallet_cli = std::make_shared<fc::rpc::cli>();
        for (auto &name_formatter : wapiptr->get_result_formatters())
            wallet_cli->format_result(name_formatter.first, name_formatter.second);

        boost::signals2::scoped_connection closed_connection(con->closed.connect([=] {
            cerr << "Server has disconnected us.\n";
            wallet_cli->stop();
        }));
        (void) (closed_connection);

        if (wapiptr->is_new()) {
            std::cout << "Please use the set_password method to initialize a new wallet before continuing\n";
            wallet_cli->set_prompt("new >>> ");
        } else
            wallet_cli->set_prompt("locked >>> ");

        boost::signals2::scoped_connection locked_connection(wapiptr->lock_changed.connect([&](bool locked) {
            wallet_cli->set_prompt(locked ? "locked >>> " : "unlocked >>> ");
        }));

        wallet_cli->register_api(wapi);
        wallet_cli->start();
        wallet_cli->wait();

        wapi->save_wallet_file(wallet_file.generic_string());
        locked_connection.disconnect();
        closed_connection.disconnect();
    } catch (const fc::exception &e) {
        std::cout << e.to_detail_string() << "\n";
        return -1;
    }

    return 0;
}
