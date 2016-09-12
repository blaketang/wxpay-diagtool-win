/******************************************************************************
  * Copyright (C) 2016, Tecent Co.Ltd. 
  * All Rights Reserved. 
  * Created:    2016-01-07 by blaketang
  * Last Modify:2016-08-10
  * @By blaketang
  * Description:
  * 
 ******************************************************************************/

#ifndef WXCONFIG_H
#define WXCONFIG_H

#include <QString>
class WxConfig
{
public:
    static WxConfig &g(){
        static WxConfig wx;
        return wx;
    }

    WxConfig();
public:
    QString WXPAY_WORKDIR;		//程序主目录

    QString PROXY_IP;			//代理IP
    QString PROXY_USER;			//代理用户名，可为空
    QString PROXY_PWD; 			//代理密码，可为空
    uint PROXY_PORT;			//代理端口

    QString TEST_URL;			//测试服务联通性url
};

#endif // WXCONFIG_H
