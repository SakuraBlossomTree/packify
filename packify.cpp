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
        std::cout<<pkg_num+1<<" "<<json_response["results"][pkg_num]["Description"].get<std::string>()<<std::endl;
        std::string pkg_name = json_response["results"][pkg_num]["PackageBase"].get<std::string>();
        std::cout<<json_response["results"][pkg_num]["Name"].get<std::string>()<<std::endl;
        std::cout<<json_response["results"][pkg_num]["Maintainer"].get<std::string>()<<std::endl;
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

}

void updatePackages(){
    std::system("sudo pacman -Syu");
}

void updatePackagesAUR(){
    std::system("paru");
}

void removePackage(std::string packageName){
    std::string command = "paru -Rns " + packageName;
    std::system(command.c_str());
}

int main (int argc, char *argv[]) {
    
    CLI::App app{"Packify: A Small AUR Wrapper"};

    std::string packageName;
    std::string removePackageName;
    bool checkupdate{false};
    bool checkupdateAUR{false};

    app.add_flag("-u, --update", checkupdate, "Updates Package");
    app.add_flag("--uA, --updateAUR", checkupdateAUR, "Updates AUR Packages");
    app.add_option("-i,--install", packageName, "Install Package");
    app.add_option("-r,--remove", removePackageName, "Remove Package");
    
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

    return 0;
}
