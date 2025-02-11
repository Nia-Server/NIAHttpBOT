#ifndef WEBUI_HPP
#define WEBUI_HPP

#include <string>
#include <algorithm>
#include <httplib.h>
#include <thread>

class WebUI{

private:


    std::string filePath, websiteAddress;
    httplib::Server *svr;

    WebUI();
    void initial(const std::string &a, const std::string &b);

public:

    
    WebUI(const std::string &a, const std::string &b, httplib::Server*);

    void start(const std::string &a, int b);




};

WebUI::WebUI(const std::string &a, const std::string &b, httplib::Server *svr){
    this->svr = svr;
    initial(a, b);
}

inline void WebUI::initial(const std::string &a, const std::string &b){
    filePath = a;
    websiteAddress=b;
    svr->set_mount_point(a, b);
}

void WebUI::start(const std::string &ip, int port){



}


#endif
