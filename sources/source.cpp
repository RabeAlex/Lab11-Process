// Copyright 2018 Your Name <your_email>

#include <header.hpp>

void CheckTime(boost::process::child& process, const time_t& period){
    time_t start = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
    while (true) {
        if ((std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now()) - start > period)
                && process.running()) {
            std::error_code ec;
            process.terminate(ec);
            std::cout << ec;
            break;
        } else if (!process.running()) {
            break;
        }
    }
}

void ProcessHandler(const std::string& command,
    const time_t& period) {
    std::string line;
    boost::process::ipstream out;
    boost::process::child process(command, boost::process::std_out > out);
    std::thread checkTime(CheckTime, std::ref(process), std::ref(period));
    while (out && std::getline(out, line) && !line.empty()) {
        std::cerr << line << std::endl;
    }
    checkTime.join();
}

void ProcessHandler(const std::string& command,
    const time_t& period, int& resultat) {
    std::string line;
    boost::process::ipstream out;
    boost::process::child process(command, boost::process::std_out > out);
    std::thread checkTime(CheckTime, std::ref(process), std::ref(period));
    while (out && std::getline(out, line) && !line.empty()) {
        std::cerr << line << std::endl;
    }
    checkTime.join();
    resultat = process.exit_code();
}

int main(int argc, char* argv[]) {
    boost::program_options::options_description AllOpt("Allowed options:");
    AllOpt.add_options()
            ("help", "display an auxiliary message")
            ("config", boost::program_options::value<std::string>(),
             "specify the assembly configuration (default Debug)")
            ("install",
             "add the installation step (to the _install directory)")
            ("pack",
             "add the packaging step (to the tar.gz format archive)")
            ("timeout", boost::program_options::value<time_t>(),
             "specify the timeout (in seconds)");
    boost::program_options::variables_map vm;
    try {
        store(parse_command_line(argc, argv, AllOpt), vm);

        if (vm.count("help") && !vm.count("config") && !vm.count("pack")
            && !vm.count("timeout") && !vm.count("install")) {
            std::cout << AllOpt << "\n";
            return 0;
        } else {
            std::string config = "Debug";
            time_t TimeOut = std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now());
            time_t TimeSpent = 0;
            if (vm.count("TimeOut")) {
                TimeOut = vm["TimeOut"].as<time_t>();
            }
            notify(vm);
            if (vm.count("config")) {
                config = vm["config"].as<std::string>();
            }

            std::string FirstRequest = "cmake -H. -B_build -DCMAKE_INSTALL_" +
                std::string("PREFIX=_install -DCMAKE_BUILD_TYPE=");
            std::string SecondRequest = "cmake --build _builds";
            std::string ThirdRequest =
                    "cmake --build _builds --target install";
            std::string FourthRequest =
                    "cmake --build _builds --target package";

            int BanThirdRequest = 0;
            int BanFourthRequest = 0;

            if (config == "Debug" || config == "Release") {
                FirstRequest += config;
                auto t1 = async::spawn([&BanThirdRequest, config, TimeOut,
                &TimeSpent, FirstRequest, SecondRequest]() mutable {
                    time_t FirstStart = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now());

                    ProcessHandler(FirstRequest, TimeOut);

                    time_t FirstEnd = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now());
                    TimeSpent += FirstEnd - FirstStart;
                    time_t TimeLeft = TimeOut - TimeSpent;
                    time_t SecondStart = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now());

                    ProcessHandler(SecondRequest, TimeLeft, BanThirdRequest);

                    time_t SecondEnd = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now());
                    TimeSpent += SecondEnd - FirstEnd;
                });
            }
            if (vm.count("install") && BanThirdRequest == 0) {
                auto t2 = async::spawn([&BanThirdRequest, &BanFourthRequest,
                ThirdRequest, TimeOut, &TimeSpent]() mutable {
                    time_t TimeLeft = TimeOut - TimeSpent;
                    time_t start = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now());

                    ProcessHandler(ThirdRequest, TimeLeft, BanFourthRequest);

                    time_t end = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now());
                    TimeSpent += end - start;
                });
            }
            if (vm.count("pack") && BanFourthRequest == 0) {
                auto t3 = async::spawn([&BanFourthRequest, FourthRequest,
                TimeOut, &TimeSpent]() mutable {
                    time_t TimeLeft = TimeOut - TimeSpent;

                    ProcessHandler(FourthRequest, TimeLeft);
                });
            }
        }
    }
    catch (boost::program_options::error & e) {
        std::cerr << e.what() << std::endl;
        std::cerr << AllOpt << std::endl;
        return 1;
    }
    catch (std::exception & ex) {
        std::cerr << ex.what() << std::endl;
        return 2;
    }
}

