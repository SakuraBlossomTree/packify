#include <cstdlib> 
#include <iostream> 
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include "./include/CLI11.hpp"

using json = nlohmann::json;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
void cleanPackages(std::string packageName){
    std::string command = "sudo rm -r " + packageName;
    std::system(command.c_str());
}

void installPackage(std::string installpackage){

    CURL *curl = curl_easy_init();
    CURLcode res;

    std::string aur_url = "https://aur.archlinux.org/rpc/v5/search?arg=" + installpackage;
    std::string readBuffer;

    curl_easy_setopt(curl, CURLOPT_URL, aur_url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    if (curl){
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    } else{
        fprintf(stderr, "Failed to initialize CURL\n");
    }

    auto json_response = json::parse(readBuffer);
    if (json_response["resultcount"] == 0){
        std::cerr<<"No packages found"<<std::endl;
    }

    std::vector<std::string> pkg_names;
    for (int pkg_num=0;pkg_num<json_response["resultcount"];pkg_num++){
        const auto pkg = json_response["results"][pkg_num];
        std::cout<<pkg_num+1<<" "<<pkg["Description"].get<std::string>()<<std::endl;
        std::string pkg_name = pkg["PackageBase"].get<std::string>();
        std::cout<<pkg["Name"].get<std::string>()<<std::endl;
        std::string maintainer;
        if(pkg["Maintainer"].is_null()){
            maintainer = "None";
        } else {
            maintainer = pkg["Maintainer"].get<std::string>();
        }
        std::cout<<maintainer<<std::endl;
        std::cout<<"---------------------------------------------------------------------------------"<<std::endl;
        pkg_names.push_back(pkg_name);
    }

    std::cout<<"Select the package you want to install: ";
    int choice;
    std::cin>>choice;

    if(choice < 1 || choice > pkg_names.size()){
        std::cerr<<"Invaild choice.\n";
    }

    std::string selected_pkg = pkg_names[choice - 1];
    std::string command = "git clone https://aur.archlinux.org/" + selected_pkg + ".git";
    std::string install_command = "cd " + selected_pkg + "&& makepkg -si";

    std::system(command.c_str());

    std::system(install_command.c_str());

    cleanPackages(selected_pkg);
}

void updatePackages(){
    std::system("sudo pacman -Syu");
}

void updatePackagesAUR(){
    std::system("paru");
}

void removePackage(std::string packageName){
    std::string command = "paru -R " + packageName;
    std::system(command.c_str());
}

void listPackages(){
    std::string command = "sudo pacman -Q";
    std::system(command.c_str());
}


int main (int argc, char *argv[]) {
    
    CLI::App app{"Packify: A Small AUR Wrapper"};

    std::string packageName;
    std::string removePackageName;
    bool checkupdate{false};
    bool checkupdateAUR{false};
    bool packageList{false};

    app.add_flag("--Syu, --update", checkupdate, "Updates Package");
    app.add_flag("--SSyu, --updateAUR", checkupdateAUR, "Updates AUR Packages");
    app.add_flag("-Q, --search", packageList, "Search Packages");
    app.add_option("-S,--install", packageName, "Install Package");
    app.add_option("-R,--remove", removePackageName, "Remove Package");

    
    CLI11_PARSE(app, argc, argv);

    if(!packageName.empty()){
        installPackage(packageName);
    }
    if(checkupdate){
        updatePackages();
    }
    if(checkupdateAUR){
        updatePackagesAUR();
    }
    if(!removePackageName.empty()){
        removePackage(removePackageName);
    }

    if(packageList){
        listPackages(); 
    }
    
    return 0;
}
